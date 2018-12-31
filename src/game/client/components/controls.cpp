/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
//ChillerBot.png ST
#include <base/math.h>

#include <engine/shared/config.h>

#include <game/client/gameclient.h>
#include <game/client/component.h>
#include <game/client/components/chat.h>


#include "controls.h"

CControls::CControls()
{
	mem_zero(&m_LastData, sizeof(m_LastData));
}

void CControls::OnReset()
{
	m_LastData.m_Direction = 0;
	m_LastData.m_Hook = 0;
	// simulate releasing the fire button
	if((m_LastData.m_Fire&1) != 0)
		m_LastData.m_Fire++;
	m_LastData.m_Fire &= INPUT_STATE_MASK;
	m_LastData.m_Jump = 0;
	m_InputData = m_LastData;

	m_InputDirectionLeft = 0;
	m_InputDirectionRight = 0;
}

void CControls::OnRelease()
{
	OnReset();
}

void CControls::OnPlayerDeath()
{
	m_LastData.m_WantedWeapon = m_InputData.m_WantedWeapon = 0;
}

static void ConKeyInputState(IConsole::IResult *pResult, void *pUserData)
{
	((int *)pUserData)[0] = pResult->GetInteger(0);
}

static void ConKeyInputCounter(IConsole::IResult *pResult, void *pUserData)
{
	int *v = (int *)pUserData;
	if(((*v)&1) != pResult->GetInteger(0))
		(*v)++;
	*v &= INPUT_STATE_MASK;
}

struct CInputSet
{
	CControls *m_pControls;
	int *m_pVariable;
	int m_Value;
};

static void ConKeyInputSet(IConsole::IResult *pResult, void *pUserData)
{
	CInputSet *pSet = (CInputSet *)pUserData;
	if(pResult->GetInteger(0))
		*pSet->m_pVariable = pSet->m_Value;
}

static void ConKeyInputNextPrevWeapon(IConsole::IResult *pResult, void *pUserData)
{
	CInputSet *pSet = (CInputSet *)pUserData;
	ConKeyInputCounter(pResult, pSet->m_pVariable);
	pSet->m_pControls->m_InputData.m_WantedWeapon = 0;
}

void CControls::OnConsoleInit()
{
	// game commands
	Console()->Register("+left", "", ConKeyInputState, &m_InputDirectionLeft, "Move left");
	Console()->Register("+right", "", ConKeyInputState, &m_InputDirectionRight, "Move right");
	Console()->Register("+jump", "", ConKeyInputState, &m_InputData.m_Jump, "Jump");
	Console()->Register("+hook", "", ConKeyInputState, &m_InputData.m_Hook, "Hook");
	Console()->Register("+fire", "", ConKeyInputCounter, &m_InputData.m_Fire, "Fire");

	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 1}; Console()->Register("+weapon1", "", ConKeyInputSet, (void *)&s_Set, "Switch to hammer"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 2}; Console()->Register("+weapon2", "", ConKeyInputSet, (void *)&s_Set, "Switch to gun"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 3}; Console()->Register("+weapon3", "", ConKeyInputSet, (void *)&s_Set, "Switch to shotgun"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 4}; Console()->Register("+weapon4", "", ConKeyInputSet, (void *)&s_Set, "Switch to grenade"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 5}; Console()->Register("+weapon5", "", ConKeyInputSet, (void *)&s_Set, "Switch to rifle"); }

	{ static CInputSet s_Set = {this, &m_InputData.m_NextWeapon, 0}; Console()->Register("+nextweapon", "", ConKeyInputNextPrevWeapon, (void *)&s_Set, "Switch to next weapon"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_PrevWeapon, 0}; Console()->Register("+prevweapon", "", ConKeyInputNextPrevWeapon, (void *)&s_Set, "Switch to previous weapon"); }
}

void CControls::OnMessage(int Msg, void *pRawMsg)
{
	if(Msg == NETMSGTYPE_SV_WEAPONPICKUP)
	{
		CNetMsg_Sv_WeaponPickup *pMsg = (CNetMsg_Sv_WeaponPickup *)pRawMsg;
		if(g_Config.m_ClAutoswitchWeapons)
			m_InputData.m_WantedWeapon = pMsg->m_Weapon+1;
	}
}

int CControls::SnapInput(int *pData, int cDir)
{
	static int64 LastSendTime = 0;
	bool Send = false;

	// update player state
	if(m_pClient->m_pChat->IsActive())
		m_InputData.m_PlayerFlags = PLAYERFLAG_CHATTING;

	m_InputData.m_PlayerFlags = PLAYERFLAG_PLAYING;

	if(m_LastData.m_PlayerFlags != m_InputData.m_PlayerFlags)
		Send = true;

	m_LastData.m_PlayerFlags = m_InputData.m_PlayerFlags;

	// we freeze the input if chat or menu is activated
	if(!(m_InputData.m_PlayerFlags&PLAYERFLAG_PLAYING))
	{
		OnReset();

		mem_copy(pData, &m_InputData, sizeof(m_InputData));

		// send once a second just to be sure
		if(time_get() > LastSendTime + time_freq())
			Send = true;
	}
	else
	{

		//=============
		//ChillerDragon
		//=============

		//reset values
		//m_InputData.m_Direction = 0;
		//m_InputData.m_Jump = 0;
		//m_InputData.m_Fire = 0;
		//m_InputData.m_WantedWeapon = 1;
		m_InputData.m_TargetX = 10;
		m_InputData.m_TargetY = 0;

		//m_InputDirectionLeft = false;
		//m_InputDirectionRight = false;

		//m_InputData.m_Direction = cDir;


		//Chat movement
		m_InputData.m_Jump = m_pClient->m_pChat->m_IsChillJump;
		if (m_pClient->m_pChat->m_ChillHookTick)
		{
			m_InputData.m_Hook = 1;
			m_pClient->m_pChat->m_ChillHookTick--;
		}
		else
		{
			m_InputData.m_Hook = 0;
		}
		if (m_pClient->m_pChat->m_ChillEyeMode == 1) //left
		{
			m_InputData.m_TargetX = -200;
			m_InputData.m_TargetY = 0;
		}
		else if (m_pClient->m_pChat->m_ChillEyeMode == 2) //right
		{
			m_InputData.m_TargetX = 200;
			m_InputData.m_TargetY = 0;
		}
		else if (m_pClient->m_pChat->m_ChillEyeMode == 3) //up
		{
			m_InputData.m_TargetX = 0;
			m_InputData.m_TargetY = -200;
		}
		else if (m_pClient->m_pChat->m_ChillEyeMode == 4) //down
		{
			m_InputData.m_TargetX = 0;
			m_InputData.m_TargetY = 200;
		}
		else if (m_pClient->m_pChat->m_ChillEyeMode == 5) //right down
		{
			m_InputData.m_TargetX = 200;
			m_InputData.m_TargetY = 30;
		}
		else if (m_pClient->m_pChat->m_ChillEyeMode == 6) //left down
		{
			m_InputData.m_TargetX = -200;
			m_InputData.m_TargetY = 30;
		}
		else if (m_pClient->m_pChat->m_ChillEyeMode == 7) //left up
		{
			m_InputData.m_TargetX = -200;
			m_InputData.m_TargetY = -30;
		}
		else if (m_pClient->m_pChat->m_ChillEyeMode == 8) //right up
		{
			m_InputData.m_TargetX = 200;
			m_InputData.m_TargetY = -30;
		}
		else if (m_pClient->m_pChat->m_ChillEyeMode == 9) //left little up
		{
			m_InputData.m_TargetX = -200;
			m_InputData.m_TargetY = -17;
		}
		else if (m_pClient->m_pChat->m_ChillEyeMode == 10) //right lttle up
		{
			m_InputData.m_TargetX = 200;
			m_InputData.m_TargetY = -17;
		}
		else if (m_pClient->m_pChat->m_ChillEyeMode == 11) //down right
		{
			m_InputData.m_TargetX = 155;
			m_InputData.m_TargetY = 200;
		}
		else if (m_pClient->m_pChat->m_ChillEyeMode == 12) //down left
		{
			m_InputData.m_TargetX = -155;
			m_InputData.m_TargetY = 200;
		}

		if (m_pClient->m_pChat->m_ChillMoveTick)
		{
			m_pClient->m_pChat->m_ChillMoveTick--;
			m_InputData.m_Direction = m_pClient->m_pChat->m_Chillrection;
			//dbg_msg("debug", "movetick %d", m_pClient->m_pChat->m_ChillMoveTick);
		}
		else
		{
			m_InputData.m_Direction = 0;
		}

		//if (m_pClient->m_pChat->m_ChillFlyY) //still not updating -.-
		//{
		//	m_InputData.m_TargetX = 0;
		//	m_InputData.m_TargetY = -200;
		//	if (m_pClient->m_Snap.m_pLocalCharacter->m_Y > m_pClient->m_pChat->m_ChillFlyY * 32)
		//	{
		//		m_InputData.m_Hook = 1;
		//	}
		//	else
		//	{
		//		m_InputData.m_Hook = 0;
		//	}
		//}


		//if (!loggedin)
		//{
		//	m_pClient->m_pChat->Say(0, "/login bot bot");
		//	dbg_msg("png", "logged in");
		//	loggedin = true;
		//}
		//else
		//{
		//	loggedin++;
		//}
		//if (loggedin > 600)
		//{
		//	m_pClient->m_pChat->Say(0, "/insta gdm");
		//	loggedin = 0;
		//}

		if (m_pClient->m_Snap.m_pLocalCharacter)
		{
			//dbg_msg("alive", "ALIVE at x(%d)", m_pClient->m_Snap.m_pLocalCharacter->m_X);
			//dbg_msg("alive", "ALIVE target (%d|%d)", m_InputData.m_TargetX, m_InputData.m_TargetY);
			//float t = rand() % 10;
			//mem_zero(&m_InputData, sizeof(m_InputData));
			//m_InputData.m_Direction = ((int)t / 2) & 1;
			//m_InputData.m_Jump = ((int)t);
			//m_InputData.m_Fire = ((int)(t * 10));
			//m_InputData.m_Hook = ((int)(t * 2)) & 1;
			//m_InputData.m_WantedWeapon = ((int)t) % NUM_WEAPONS;
			//m_InputData.m_TargetX = (int)(sinf(t * 3)*100.0f);
			//m_InputData.m_TargetY = (int)(cosf(t * 3)*100.0f);
			//if (m_pClient->m_Snap.m_pLocalCharacter->m_X > 353 * 32)
			//{
			//	m_pClient->m_pChat->Say(0, "7login chillerbot.png -:-//7SQL(INJECTION)");
			//}
			//if (m_pClient->m_Snap.m_pLocalCharacter->m_VelY > 0.0f)
			//{
			//	m_InputData.m_Hook = 1;
			//	m_InputData.m_TargetX = 0;
			//	m_InputData.m_TargetY = -20;
			//	m_InputData.m_Jump = 1;
			//}

		}


		//Mousemovement and left right protection Vanilla
		//commented out by ChillerDragon cuz we set the values

		//m_InputData.m_TargetX = (int)m_MousePos.x;
		//m_InputData.m_TargetY = (int)m_MousePos.y;
		//if (!m_InputData.m_TargetX && !m_InputData.m_TargetY)
		//{
		//	m_InputData.m_TargetX = 1;
		//	m_MousePos.x = 1;
		//}

		//// set direction
		//m_InputData.m_Direction = 0;
		//if (m_InputDirectionLeft && !m_InputDirectionRight)
		//	m_InputData.m_Direction = -1;
		//if (!m_InputDirectionLeft && m_InputDirectionRight)
		//	m_InputData.m_Direction = 1;

		// check if we need to send input
		if (m_InputData.m_Direction != m_LastData.m_Direction) Send = true;
		else if (m_InputData.m_Jump != m_LastData.m_Jump) Send = true;
		else if (m_InputData.m_Fire != m_LastData.m_Fire) Send = true;
		else if (m_InputData.m_Hook != m_LastData.m_Hook) Send = true;
		else if (m_InputData.m_WantedWeapon != m_LastData.m_WantedWeapon) Send = true;
		else if (m_InputData.m_NextWeapon != m_LastData.m_NextWeapon) Send = true;
		else if (m_InputData.m_PrevWeapon != m_LastData.m_PrevWeapon) Send = true;

		// send at at least 10hz
		if (time_get() > LastSendTime + time_freq() / 25)
			Send = true;
	}

	// copy and return size
	m_LastData = m_InputData;

	if(!Send)
		return 0;

	LastSendTime = time_get();
	mem_copy(pData, &m_InputData, sizeof(m_InputData));
	return sizeof(m_InputData);
}

void CControls::OnRender()
{
	// update target pos
	if(m_pClient->m_Snap.m_pGameInfoObj && !m_pClient->m_Snap.m_SpecInfo.m_Active)
		m_TargetPos = m_pClient->m_LocalCharacterPos + m_MousePos;
	else if(m_pClient->m_Snap.m_SpecInfo.m_Active && m_pClient->m_Snap.m_SpecInfo.m_UsePosition)
		m_TargetPos = m_pClient->m_Snap.m_SpecInfo.m_Position + m_MousePos;
	else
		m_TargetPos = m_MousePos;
}

bool CControls::OnMouseMove(float x, float y)
{
	if((m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED) ||
		(m_pClient->m_Snap.m_SpecInfo.m_Active && m_pClient->m_pChat->IsActive()))
		return false;

	m_MousePos += vec2(x, y); // TODO: ugly

	return true;
}

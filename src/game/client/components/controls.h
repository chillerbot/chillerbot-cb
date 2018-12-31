/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_CONTROLS_H
#define GAME_CLIENT_COMPONENTS_CONTROLS_H
#include <base/vmath.h>
#include <game/client/component.h>

class CControls : public CComponent
{
public:
	vec2 m_MousePos;
	vec2 m_TargetPos;

	CNetObj_PlayerInput m_InputData;
	CNetObj_PlayerInput m_LastData;
	int m_InputDirectionLeft;
	int m_InputDirectionRight;

	CControls();

	virtual void OnReset();
	virtual void OnRelease();
	virtual void OnRender();
	virtual void OnMessage(int MsgType, void *pRawMsg);
	virtual bool OnMouseMove(float x, float y);
	virtual void OnConsoleInit();
	virtual void OnPlayerDeath();

	int SnapInput(int *pData, int cDir);

	//=============
	//ChillerBot ST
	//=============

	//ChillerDragon Vars
	int m_TimeTicker;
	int m_JumpDelay;
	int m_JumpDelay2;  //hiddenway jumps
	int m_DontHook; //speedstart in hiddenway
	int m_StuckTime;
	int m_abc; //iused for stuff idc
	bool m_ReleaseHook;
	//TeeTown
	int m_JumpTicker;
	int m_LifetimeTT; //lifetime on teetown
	int m_AntiAfkTicker;
	//royal
	int m_Fire_delay;
	//copy love box booster
	int m_boost_kill_delay;

	int loggedin;

	//Auto stuff
	int m_autohook_delay;
};
#endif

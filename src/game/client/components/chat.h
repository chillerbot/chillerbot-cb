/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_CHAT_H
#define GAME_CLIENT_COMPONENTS_CHAT_H
#include <engine/shared/ringbuffer.h>
#include <game/client/component.h>

//chiller
#include <string>
#include <iostream>

class CChat : public CComponent
{
	enum
	{
		MAX_LINES = 25,
	};

	struct CLine
	{
		int64 m_Time;
		float m_YOffset[2];
		int m_ClientID;
		int m_Team;
		char m_aName[64];
		char m_aText[512];
	};

	CLine m_aLines[MAX_LINES];
	int m_CurrentLine;

	// chat
	enum
	{
		CHAT_SERVER=0,
		CHAT_CLIENT,
		CHAT_NUM,
	};

	static void ConSay(IConsole::IResult *pResult, void *pUserData);
	static void ConSayTeam(IConsole::IResult *pResult, void *pUserData);

	//ChillerDragon

	bool IsMute;
	bool IsMuteLevel;
	bool IsBlockJoin;
	bool IsTeamchat;

	//int rank; //ultra hacky work around cuz i need the rank saved because of chat message resets rank and stuff
	//char aLastMessage[1024]; //using currentline--

public:
	CChat();

	bool IsActive() const { return 1; }

	void AddLine(int ClientID, int Team, const char *pLine);

	void Say(int Team, const char *pLine);

	virtual void OnReset();
	virtual void OnConsoleInit();
	virtual void OnStateChange(int NewState, int OldState);
	virtual void OnMessage(int MsgType, void *pRawMsg);

	//ChillerDragon

	int m_ChillMoveTick;
	int m_Chillrection;
	bool m_IsChillJump;
	int m_ChillHookTick;
	int m_ChillEyeMode;
	int m_ChillFlyY;
	void GetRankVoter(const char *pMsg);
	void GetRankVoted(const char *pMsg);

	bool is_file_exist(const char *fileName);



	std::string cut_string(std::string const& s);
	std::string cut_string_get_int(std::string const& s);
	std::string cut_string_get_int2(std::string const& s);
	std::string cut_string_get_space2_till_end(std::string const& s);
	std::string cut_string_get_space3_till_end(std::string const& s);
	std::string cut_string_get_flyint(std::string const& s);
	std::string cut_string_get_chatname(std::string const& s);
	std::string* cut_string_get_registername(std::string const& s);
	std::string cut_string_get_rankname(std::string const& s);
	std::string cut_string_get_rankvoter(std::string const& s);
	std::string cut_string_get_rankvoted(std::string const& s);
	std::string cut_string_get_payname(std::string const& s);
	std::string cut_string_get_votername(std::string const& s);
	std::string cut_string_get_votedname(std::string const& s);
	bool IsOwnRank(std::string const& s);
	bool IsOwnRankVoter(std::string const& s);
	bool IsOwnRankVoted(std::string const& s);

	//after reaching the MAX_DDUES limit the client crashes
#define MAX_DUDES 2048
};
#endif

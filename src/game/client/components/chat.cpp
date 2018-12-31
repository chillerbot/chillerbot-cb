/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <engine/engine.h>
#include <engine/shared/config.h>

#include <game/generated/protocol.h>

#include <game/client/gameclient.h>

#include "chat.h"

//chillerdragon includes
#include <fstream> //files
#include <string>
#include <iostream>

CChat::CChat()
{
	OnReset();
}

void CChat::OnReset()
{
	for(int i = 0; i < MAX_LINES; i++)
	{
		m_aLines[i].m_Time = 0;
		m_aLines[i].m_aText[0] = 0;
		m_aLines[i].m_aName[0] = 0;
	}
}

void CChat::OnStateChange(int NewState, int OldState)
{
	if(OldState <= IClient::STATE_CONNECTING)
	{
		for(int i = 0; i < MAX_LINES; i++)
			m_aLines[i].m_Time = 0;
		m_CurrentLine = 0;
	}
}

void CChat::ConSay(IConsole::IResult *pResult, void *pUserData)
{
	((CChat*)pUserData)->Say(0, pResult->GetString(0));
}

void CChat::ConSayTeam(IConsole::IResult *pResult, void *pUserData)
{
	((CChat*)pUserData)->Say(1, pResult->GetString(0));
}

void CChat::OnConsoleInit()
{
	Console()->Register("say", "r", ConSay, this, "Say in chat");
	Console()->Register("say_team", "r", ConSayTeam, this, "Say in team chat");
}

void CChat::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_CHAT)
	{
		CNetMsg_Sv_Chat *pMsg = (CNetMsg_Sv_Chat *)pRawMsg;
		AddLine(pMsg->m_ClientID, pMsg->m_Team, pMsg->m_pMessage);
	}
}

void CChat::GetRankVoter(const char * pMsg)
{
	char aBuf[1024];
	char aRankVoter[32];
	if (IsOwnRankVoter(pMsg))
	{
		Say(0, "Use '!rank_voter (name)'");
	}
	else
	{
		str_format(aRankVoter, sizeof(aRankVoter), "%s", cut_string_get_rankvoter(pMsg).c_str());
	}



	std::ifstream fileVotes(g_Config.m_ClVotePath);
	std::string line;
	char tmp[32];
	int index = 0;
	int found = -1;
	int best_val = 0;
	int index_best_val;
	int tmp_val = 0;
	int tmp_val2 = 0;
	int val = 0;
	int val2 = 0;
	int rank = 1;

	//fetch rank data
	while (std::getline(fileVotes, line))
	{
		str_format(tmp, sizeof(tmp), "%s", cut_string_get_space2_till_end(line).c_str());
		tmp_val = atoi(cut_string_get_int(line).c_str());
		tmp_val2 = atoi(cut_string_get_int2(line).c_str());
		//dbg_msg("voter", "tmp_val['%d:%d'] tmp '%s' req '%s'", tmp_val, tmp_val2, tmp, aRankVoter);
		if (!str_comp(tmp, aRankVoter) && found == -1) //only can be found once Voter
		{
			found = index;
			val = tmp_val;
			val2 = tmp_val2;
			dbg_msg("voter_rank", "[%d:%d] '%s' found", val, val2, tmp);
		}
	}
	fileVotes.close();

	if (found != -1) //found
	{
		//compare rank to all data
		std::ifstream fileComp(g_Config.m_ClVotePath);
		while (std::getline(fileComp, line))
		{
			tmp_val = atoi(cut_string_get_int(line).c_str());
			tmp_val2 = atoi(cut_string_get_int2(line).c_str());
			if (tmp_val > val)
			{
				dbg_msg("rank", "'%d:%s' is better than '%d:%s'", tmp_val, "tmp", val, aRankVoter);
				rank++;
			}
		}
		fileComp.close();


		str_format(aBuf, sizeof(aBuf), "[VOTER] %d. '%s' voter[%d] voted[%d]", rank, aRankVoter, val, val2);
		Say(0, aBuf);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[VOTER] '%s' is unranked", aRankVoter);
		Say(0, aBuf);
	}
}

void CChat::GetRankVoted(const char * pMsg)
{
	char aBuf[1024];
	char aRankVoted[32];
	if (IsOwnRankVoted(pMsg))
	{
		Say(0, "Use '!rank_voted (name)'");
	}
	else
	{
		str_format(aRankVoted, sizeof(aRankVoted), "%s", cut_string_get_rankvoted(pMsg).c_str());
	}


	//str_format(aBuf, sizeof(aBuf), "'%s' blbllb", aRankVoted);
	//Say(0, aBuf);


	std::ifstream fileVotes(g_Config.m_ClVotePath);
	std::string line;
	char tmp[32];
	int index = 0;
	int found = -1;
	int best_val = 0;
	int index_best_val;
	int tmp_val = 0;
	int tmp_val2 = 0;
	int val = 0;
	int val2 = 0;
	int rank = 1;

	//fetch rank data
	while (std::getline(fileVotes, line))
	{
		str_format(tmp, sizeof(tmp), "%s", cut_string_get_space2_till_end(line).c_str());
		tmp_val = atoi(cut_string_get_int(line).c_str());
		tmp_val2 = atoi(cut_string_get_int2(line).c_str());
		dbg_msg("voted", "tmp_val['%d:%d'] tmp '%s' req '%s'", tmp_val, tmp_val2, tmp, aRankVoted);
		if (!str_comp(tmp, aRankVoted) && found == -1) //only can be found once voted
		{
			found = index;
			val = tmp_val;
			val2 = tmp_val2;
			dbg_msg("voted_rank", "[%d:%d] '%s' found", val, val2, tmp);
		}
	}
	fileVotes.close();

	if (found != -1) //found
	{
		//compare rank to all data
		std::ifstream fileComp(g_Config.m_ClVotePath);
		while (std::getline(fileComp, line))
		{
			tmp_val = atoi(cut_string_get_int(line).c_str());
			tmp_val2 = atoi(cut_string_get_int2(line).c_str());
			if (tmp_val2 > val2)
			{
				dbg_msg("rank", "'%d:%s' is better than '%d:%s'", tmp_val, "tmp", val, aRankVoted);
				rank++;
			}
		}
		fileComp.close();


		str_format(aBuf, sizeof(aBuf), "[VOTED] %d. '%s' voter[%d] voted[%d]", rank, aRankVoted, val, val2);
		Say(0, aBuf);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[VOTED] '%s' is unranked", aRankVoted);
		Say(0, aBuf);
	}
}

bool CChat::is_file_exist(const char * fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

std::string CChat::cut_string(std::string const & s)
{
	std::string::size_type pos = s.find(' ');
	if (pos != std::string::npos)
	{
		return s.substr(++pos, s.length());
	}
	else
	{
		return s;
	}
}

std::string CChat::cut_string_get_int(std::string const & s)
{
	std::string::size_type pos = s.find(' ');
	if (pos != std::string::npos)
	{
		return s.substr(0, pos);
	}
	else
	{
		return s;
	}
}

std::string CChat::cut_string_get_int2(std::string const & s)
{
	int pos1 = 0;
	int pos2 = 0;
	for (int i = 0; i < s.length(); i++)
	{
		if (s[i] == ' ')
		{
			pos1 = i;
			break;
		}
	}
	pos1++;
	for (int i = pos1; i < s.length(); i++)
	{
		if (s[i] == ' ')
		{
			pos2 = i;
			break;
		}
	}

	if (pos1 != pos2 && pos2)
	{
		return s.substr(pos1, pos2);
	}
	else
	{
		return "404";
	}
}


std::string CChat::cut_string_get_space2_till_end(std::string const & s)
{
	int pos1 = 0;
	int pos2 = 0;
	int pos3 = 0;
	int len = s.length();
	for (int i = 0; i < len; i++)
	{
		if (s[i] == ' ')
		{
			pos1 = i;
			break;
		}
	}
	pos1++;
	for (int i = pos1; i < len; i++)
	{
		if (s[i] == ' ')
		{
			pos2 = i;
			break;
		}
	}
	pos2++;

	if (pos1 != pos2 && pos2)
	{
		return s.substr(pos2, len);
	}
	else
	{
		dbg_msg("vote-s3-te", "error pos1=%d pos2=%d len=%d", pos1, pos2, len);
		return "404";
	}
}

std::string CChat::cut_string_get_space3_till_end(std::string const & s)
{
	int pos1 = 0;
	int pos2 = 0;
	int pos3 = 0;
	int len = s.length();
	for (int i = 0; i < len; i++)
	{
		if (s[i] == ' ')
		{
			pos1 = i;
			break;
		}
	}
	pos1++;
	for (int i = pos1; i < len; i++)
	{
		if (s[i] == ' ')
		{
			pos2 = i;
			break;
		}
	}
	pos2++;
	for (int i = pos2; i < len; i++)
	{
		if (s[i] == ' ')
		{
			pos3 = i;
			break;
		}
	}

	if (pos1 != pos2 && pos2 != pos3 && pos3)
	{
		return s.substr(pos3, len);
	}
	else
	{
		dbg_msg("vote-s3-te","error pos1=%d pos2=%d pos3=%d len=%d", pos1, pos2, pos3, len);
		return "404";
	}
}

std::string CChat::cut_string_get_flyint(std::string const & s)
{
	std::string::size_type pos = s.find("!fly ");
	int start = pos + 5;
	bool IsNumeric = true;
	if (pos != std::string::npos)
	{
		int i = start;
		while (i < s.length())
		{
			if (s[i] == '0' || s[i] == '1' || s[i] == '2' || s[i] == '3' || s[i] == '4' || s[i] == '5' || s[i] == '6' || s[i] == '7' || s[i] == '8' || s[i] == '9')
			{
				//dbg_msg("flyint", "found num [ %c ]", s[i]);
			}
			else
			{
				IsNumeric = false;
				break;
			}



			i++;
		}



		if (IsNumeric)
		{
			return s.substr(start, s.length());
		}
		else
		{
			return "(invalid)";
		}
	}
	else
	{
		return "(invalid)";
	}
}

std::string CChat::cut_string_get_chatname(std::string const & s)
{
	std::string::size_type pos = s.find(': ');
	if (pos != std::string::npos)
	{
		return s.substr(0, pos - 1);
	}
	else
	{
		return s;
	}
}

std::string* CChat::cut_string_get_registername(std::string const & s)
{
	std::string::size_type pos = s.find("!register ");

	//NAME

	int nameIndex = 0;
	char aName[128];
	int pwstart = pos + 9;
	int i = pwstart;
	int len = s.length();
	bool found = false;

	while (!found && i < len)
	{
		i++;
		//dbg_msg("rank", "i=%d char=%c", i, s[i]);
		//if (isspace(s.at(i)))
		//{
		//	dbg_msg("rank", "'%c','%c',[%c],'%c','%c'", s[i - 2], s[i - 1], s[i], s[i + 1], s[i + 2]);
		//	found = true;
		//}

		if (s[i] == '\0')
		{
			dbg_msg("rank", "i=%d char=%c", i, s[i]);
			aName[nameIndex] = s[i];
			nameIndex++;
			dbg_msg("rank", "string null terimation found");
			found = true;
			break;
		}
		else if (s[i] == '\n')
		{
			dbg_msg("rank", "string newline found");
			found = false;
			break;
		}
		else if (s[i] == NULL)
		{
			dbg_msg("rank", "string null found");
			found = false;
			break;
		}
		else if (i > 127)
		{
			dbg_msg("rank", "reached name limit");
			found = false;
			break;
		}
		else if (isspace(s.at(i)))
		{
			dbg_msg("rank", "'%c','%c',[%c],'%c','%c'", s[i - 2], s[i - 1], s[i], s[i + 1], s[i + 2]);
			found = true;
		}
		else
		{
			dbg_msg("rank", "i=%d char=%c", i, s[i]);
			aName[nameIndex] = s[i];
			nameIndex++;
		}
	}


	//PASSWORD
	dbg_msg("rankPW", "====== password  ======");

	char aPasswd[128];
	bool foundPW = false;
	int pwIndex = 0;

	if (found) //only search pw if name found
	{
		while (!foundPW && i < len)
		{
			i++;
			if (s[i] == '\0')
			{
				dbg_msg("rankPW", "string null terimation found");
				foundPW = true;
				break;
			}
			else if (s[i] == '\n')
			{
				dbg_msg("rankPW", "string newline found");
				foundPW = false;
				break;
			}
			else if (s[i] == NULL)
			{
				dbg_msg("rankPW", "string null found");
				foundPW = false;
				break;
			}
			else if (i > 127)
			{
				dbg_msg("rankPW", "reached name limit");
				foundPW = false;
				break;
			}
			else if (isspace(s.at(i)))
			{
				dbg_msg("rankPW", "'%c','%c',[%c],'%c','%c'", s[i - 2], s[i - 1], s[i], s[i + 1], s[i + 2]);
				foundPW = true;
			}
			else
			{
				dbg_msg("rankPW", "i=%d char=%c", i, s[i]);
				aPasswd[pwIndex] = s[i];
				pwIndex++;
			}
		}
	}

	std::string aOut[2];

	if (pos != std::string::npos && found)
	{
		//dbg_msg("rank", "regname [ %s ]", s.substr(pwstart, i).c_str()); //doesnt work at all idk why better code the stuff ur own then it works perfect
		//return s.substr(pwstart, i);

		dbg_msg("rank", "c_cutted_name [ %s ] found=%d", aName, found);
		dbg_msg("rankPW", "c_cutted_pw [ %s ] found=%d", aPasswd, foundPW);
		aOut[0] = aName;
		aOut[1] = aPasswd;
		dbg_msg("rank", "name '%s'", aOut[0].c_str());
		dbg_msg("rankPW", "pw '%s'", aOut[1].c_str());
		return aOut;
	}
	else
	{
		aOut[0] = "(invalid)";
		aOut[1] = "(invalid)";
		return aOut;
	}
}

std::string CChat::cut_string_get_rankname(std::string const & s)
{
	std::string::size_type pos = s.find("!rank ");
	if (pos != std::string::npos)
	{
		return s.substr(pos+6, s.length());
	}
	else
	{
		return s;
	}
}

std::string CChat::cut_string_get_rankvoter(std::string const & s)
{
	std::string::size_type pos = s.find("!rank_voter ");
	if (pos != std::string::npos)
	{
		//std::string ret;
		//int index = 0;
		//for (int i = pos; i < s.length(); i++)
		//{
		//	ret[index] = s[i];
		//	index++;
		//}
		//ret[index] = '\0';

		//return ret;

		return s.substr(pos + 12, s.length());
	}
	else
	{
		return s;
	}
}

std::string CChat::cut_string_get_rankvoted(std::string const & s)
{
	std::string::size_type pos = s.find("!rank_voted ");
	if (pos != std::string::npos)
	{

		return s.substr(pos + 12, s.length());
	}
	else
	{
		return s;
	}
}

std::string CChat::cut_string_get_payname(std::string const & s)
{
	std::string::size_type pos = s.find("!pay_100 ");
	if (pos != std::string::npos)
	{
		return s.substr(pos + 9, s.length());
	}
	else
	{
		return s;
	}
}

std::string CChat::cut_string_get_votername(std::string const & s)
{
	//ChillerDragons selfmade search algo
	int len = s.length();
	int start = 0;
	int end = 0;
	for (int i = 0; i < len; i++)
	{
		if (s[i] == '\'')
		{
			start = i;
			break;
		}
	}
	start++;
	for (int i = start; i < len; i++)
	{
		if (s[i] == '\'')
		{
			end = i;
			break;
		}
	}
	end--;

	if (start != end && end)
	{
		return s.substr(start, end);
	}
	else
	{
		dbg_msg("vote-cut", "failed start=%d end=%d", start, end);
		return "(invalid)";
	}
}

std::string CChat::cut_string_get_votedname(std::string const & s)
{
	std::string::size_type pos = s.find("called for vote to kick '");
	std::string::size_type pos2 = s.find("' (");
	int end = pos2;
	int start = pos + 25;
	if (start != std::string::npos && end != std::string::npos)
	{
		//dbg_msg("voted", "player '%s' got voted", s.substr(start, end).c_str());
		//dbg_msg("voted", "success start=%d end=%d len=%d", start, end, s.length());
		//dbg_msg("voted", "start '%c','%c','%c' end '%c','%c','%c'", s[start - 1], s[start], s[start + 1], s[end - 1], s[end], s[end + 1]);
		char aReturn[32];
		int index = 0;
		for (int i = start; i < end; i++)
		{
			aReturn[index] = s[i];
			index++;
		}
		aReturn[index] = '\0';
		std::string Return = aReturn;
		return Return;
	}
	else
	{
		dbg_msg("voted", "error start=%d end=%d", start, end);
		return "(invalid)";
	}
}

bool CChat::IsOwnRank(std::string const & s)
{
	std::string::size_type pos = s.find("!rank");


	//s[1] = "a";

	//dbg_msg("debug", "STR[%c] ++STR[%c]", s[pos+4],s[pos+5]);

	if (s[pos + 5] == '\0')
	{
		return true;
	}



	return false;
}

bool CChat::IsOwnRankVoter(std::string const & s)
{
	std::string::size_type pos = s.find("!rank_voter");

	if (s[pos + 11] == '\0')
	{
		return true;
	}



	return false;
}

bool CChat::IsOwnRankVoted(std::string const & s)
{
	std::string::size_type pos = s.find("!rank_voted");

	if (s[pos + 11] == '\0')
	{
		return true;
	}



	return false;
}

void CChat::AddLine(int ClientID, int Team, const char *pLine)
{
	if(*pLine == 0 || (ClientID != -1 && (m_pClient->m_aClients[ClientID].m_aName[0] == '\0')))
		return;

	// trim right and set maximum length to 128 utf8-characters
	int Length = 0;
	const char *pStr = pLine;
	const char *pEnd = 0;
	while(*pStr)
 	{
		const char *pStrOld = pStr;
		int Code = str_utf8_decode(&pStr);

		// check if unicode is not empty
		if(Code > 0x20 && Code != 0xA0 && Code != 0x034F && (Code < 0x2000 || Code > 0x200F) && (Code < 0x2028 || Code > 0x202F) &&
			(Code < 0x205F || Code > 0x2064) && (Code < 0x206A || Code > 0x206F) && (Code < 0xFE00 || Code > 0xFE0F) &&
			Code != 0xFEFF && (Code < 0xFFF9 || Code > 0xFFFC))
		{
			pEnd = 0;
		}
		else if(pEnd == 0)
			pEnd = pStrOld;

		if(++Length >= 127)
		{
			*(const_cast<char *>(pStr)) = 0;
			break;
		}
 	}
	if(pEnd != 0)
		*(const_cast<char *>(pEnd)) = 0;

	char *p = const_cast<char*>(pLine);
	while(*p)
	{
		pLine = p;
		// find line seperator and strip multiline
		while(*p)
		{
			if(*p++ == '\n')
			{
				*(p-1) = 0;
				break;
			}
		}

		m_CurrentLine = (m_CurrentLine+1)%MAX_LINES;
		m_aLines[m_CurrentLine].m_Time = time_get();
		m_aLines[m_CurrentLine].m_YOffset[0] = -1.0f;
		m_aLines[m_CurrentLine].m_YOffset[1] = -1.0f;
		m_aLines[m_CurrentLine].m_ClientID = ClientID;
		m_aLines[m_CurrentLine].m_Team = Team;

		//ChillerDragons vars
		char aBuf[1024];
		char aName[32];
		char aGotVotedName[32];


		if(ClientID == -1) // server message
		{
			str_copy(m_aLines[m_CurrentLine].m_aName, "*** ", sizeof(m_aLines[m_CurrentLine].m_aName));
			str_format(m_aLines[m_CurrentLine].m_aText, sizeof(m_aLines[m_CurrentLine].m_aText), "%s", pLine);

			if (str_find(pLine, "called for vote to kick 'chillerbot.png'"))
			{
				//Say(0, "[INFO] mute me esc->players->chillerbot.png->mute if you get annoyed by the chat spam");
			}
			else if (str_find(pLine, "called for vote to move 'chillerbot.png' to spectators"))
			{
				Say(0, "[INFO] you can also use !spec and !join commands for more help check !help");
			}
			else if (str_find(pLine, "' multi x"))
			{
				IsBlockJoin = true;
			}


			if (str_find(pLine, "called for vote to kick")) //vote statiscs
			{
				//dbg_msg("vote", "%s", pLine);

				//check if we ever met the person
				str_format(aGotVotedName, sizeof(aGotVotedName), "%s", cut_string_get_votedname(pLine).c_str());
				str_format(aName, sizeof(aName), "%s", cut_string_get_votername(pLine).c_str());
				std::string UpgradeName;
				std::string line;
				std::string aNames[MAX_DUDES]; //track max 2048 tees cuz tw hasnt more players lulul if it reaches the limit teh client crashes
				std::ifstream fileVotes(g_Config.m_ClVotePath);
				char tmp[32];
				int index = 0;
				int found = -1;
				int foundVoted = -1;
				int best_val = 0;
				int index_best_val;
				int tmp_val = 0;
				int tmp_val2 = 0;
				int val = 0;
				int val2 = 0;
				int val_voted = 0;
				int val2_voted = 0;
				int total_dudes = 0;
				bool IsFriend = false;

				//str_format(aBuf, sizeof(aBuf), "'%s' got voted", aGotVotedName);
				//Say(0,aBuf);

				while (std::getline(fileVotes, line))
				{
					str_format(tmp, sizeof(tmp), "%s", cut_string_get_space2_till_end(line).c_str());
					aNames[index] = line;
					tmp_val = atoi(cut_string_get_int(line).c_str());
					tmp_val2 = atoi(cut_string_get_int2(line).c_str());
					//dbg_msg("votes", "comp[%d:%d] '%s' '%s'", tmp_val, tmp_val2, tmp, aName);
					if (!str_comp(tmp, aName) && found == -1) //only can be found once Voter
					{
						found = index;
						val = tmp_val;
						val2 = tmp_val2;
						UpgradeName = tmp;
						//dbg_msg("voter", "[%d:%d] '%s' found", val, val2, tmp);
						int level = 0;

						while (level < 900)
						{
							if (val == pow(level, 2))
							{
								if (!str_comp(aName, " "))
								{
									str_format(aBuf, sizeof(aBuf), "[WARNING] dataloose (votes) reached level %d", level);
									Say(0, aBuf);
								}
								else
								{
									if (IsMuteLevel)
									{
										str_format(aBuf, sizeof(aBuf), "/w %s [VOTER] '%s' reached level %d", aName, aName, level);
									}
									else
									{
										str_format(aBuf, sizeof(aBuf), "[VOTER] '%s' reached level %d", aName, level);
									}
									Say(0, aBuf);
								}

								break;
							}
							level++;
						}
						
					}
					if (!str_comp(tmp, aGotVotedName) && foundVoted == -1) //only can be found once Voted
					{
						foundVoted = index;
						val_voted = tmp_val;
						val2_voted = tmp_val2;
						//dbg_msg("voted", "[%d:%d] '%s' found", val_voted, val2_voted, tmp);
						int level = 0;

						while (level < 900)
						{
							if (val == pow(level, 2))
							{
								if (!str_comp(aName, " "))
								{
									str_format(aBuf, sizeof(aBuf), "[WARNING] dataloose (voted) reached level %d", level);
									Say(0, aBuf);
								}
								else
								{
									if (IsMuteLevel)
									{
										str_format(aBuf, sizeof(aBuf), "/w %s [VOTED] '%s' reached level %d", aGotVotedName, aGotVotedName, level);
									}
									else
									{
										str_format(aBuf, sizeof(aBuf), "[VOTED] '%s' reached level %d", aGotVotedName, level);
									}
									Say(0, aBuf);
								}

								break;
							}
							level++;
						}

					}
					index++;
					total_dudes++;
				}
				fileVotes.close();


				bool update = true;

				if (found == -1 && foundVoted != -1) //only voter not found
				{
					//ADD VOTER
					std::ofstream fileW(g_Config.m_ClVotePath, std::ios_base::app);
					str_format(aBuf, sizeof(aBuf), "1 0 %s", aName);
					aNames[total_dudes] = aBuf;
					total_dudes++;
					dbg_msg("voter", "added new '%s'", aBuf);

					//UPDATE VOTED
					val2_voted++; //increase got votes for got voter
					dbg_msg("update-vote", "val[%d] val2[%d] name[%s]", val_voted, val2_voted, aGotVotedName);
					str_format(aBuf, sizeof(aBuf), "%d %d %s", val_voted, val2_voted, aGotVotedName);
					aNames[foundVoted] = aBuf;
				}
				else if (found != -1 && foundVoted == -1) //only voted not found
				{
					//ADD VOTED
					std::ofstream fileW(g_Config.m_ClVotePath, std::ios_base::app);
					str_format(aBuf, sizeof(aBuf), "0 1 %s", aGotVotedName);
					aNames[total_dudes] = aBuf;
					total_dudes++;
					dbg_msg("voted", "added new '%s'", aBuf);

					//UPDATE VOTER
					val++; //increase votes for voter
					dbg_msg("update-vote", "val[%d] val2[%d] name[%s]", val, val2, UpgradeName.c_str());
					str_format(aBuf, sizeof(aBuf), "%d %d %s", val, val2, UpgradeName.c_str());
					aNames[found] = aBuf;
				}
				else if (found != -1 && foundVoted != -1) //all found update
				{
					//UPDATE VOTER
					val++; //increase votes for voter
					dbg_msg("update-vote", "val[%d] val2[%d] name[%s]", val, val2, UpgradeName.c_str());
					str_format(aBuf, sizeof(aBuf), "%d %d %s", val, val2, UpgradeName.c_str());
					aNames[found] = aBuf;


					//UPDATE VOTED
					val2_voted++; //increase got votes for got voter
					dbg_msg("update-vote", "val[%d] val2[%d] name[%s]", val_voted, val2_voted, aGotVotedName);
					str_format(aBuf, sizeof(aBuf), "%d %d %s", val_voted, val2_voted, aGotVotedName);
					aNames[foundVoted] = aBuf;
				}
				else if (found == -1 && foundVoted == -1) //none found -> add both
				{
					//ADD VOTER
					std::ofstream fileW(g_Config.m_ClVotePath, std::ios_base::app);
					str_format(aBuf, sizeof(aBuf), "1 0 %s", aName);
					aNames[total_dudes] = aBuf;
					total_dudes++;
					dbg_msg("voter", "added new '%s'", aBuf);

					//ADD VOTED
					std::ofstream fileW2(g_Config.m_ClVotePath, std::ios_base::app);
					str_format(aBuf, sizeof(aBuf), "0 1 %s", aGotVotedName);
					aNames[total_dudes] = aBuf;
					total_dudes++;
					dbg_msg("voted", "added new '%s'", aBuf);
				}
				else
				{
					Say(0,"[VOTES] an error occured please contact an developer");
					dbg_msg("VOTE", "error voter=%d voted=%d", found, foundVoted);
					update = false;
				}

				if (update)
				{
					//rewrite all
					std::ofstream fileVotesW(g_Config.m_ClVotePath);
					if (fileVotesW.is_open())
					{
						for (int i = 0; i < total_dudes; i++)
						{
							fileVotesW << aNames[i] << "\n";
						}
						fileVotesW.close();
					}
					dbg_msg("votes", "total rewritten %d", total_dudes);
				}
			}
		}
		else
		{
			str_copy(m_aLines[m_CurrentLine].m_aName, m_pClient->m_aClients[ClientID].m_aName, sizeof(m_aLines[m_CurrentLine].m_aName));
			str_format(m_aLines[m_CurrentLine].m_aText, sizeof(m_aLines[m_CurrentLine].m_aText), ": %s", pLine);
			std::string getrank = "(invalid)";
			std::string registername = "(invalid)";
			std::string registerpw = "(invalid)";
			char aMsg[1024];
			bool DoStuffPNG = true;
			m_IsChillJump = false;
			str_format(aName, sizeof(aName), "%s", m_pClient->m_aClients[ClientID].m_aName);

			if (!str_comp(aName, " ")) //more than 16 slots server chatting player too far away
			{
				//dbg_msg("png-CB", "found player without name --> trying to fix it");
				str_format(aName, sizeof(aName), "%s", cut_string_get_chatname(pLine).c_str()); //only works with names without spaces in name

				if (!str_comp(aName, "") || !str_comp(aName, " ") || aName[0] == '\0')
				{
					dbg_msg("WARNING", "fix failed player probably is using a ': ' in name");
					dbg_msg("WARNING", "ignoring this player");
					DoStuffPNG = false;
				}
			}

			if (!str_comp(aName, "chillerbot.png"))
			{
				//dbg_msg("png-CB","ignoring own message");
				DoStuffPNG = false;
			}

			if (m_CurrentLine > 4)
			{
				if (!str_comp(m_aLines[m_CurrentLine].m_aText, m_aLines[m_CurrentLine - 1].m_aText) || !str_comp(m_aLines[m_CurrentLine].m_aText, m_aLines[m_CurrentLine - 2].m_aText) || !str_comp(m_aLines[m_CurrentLine].m_aText, m_aLines[m_CurrentLine - 3].m_aText))
				{
					dbg_msg("spam", "found duplicate '%s'", m_aLines[m_CurrentLine].m_aText);
					DoStuffPNG = false;
				}
			}

			if (DoStuffPNG)
			{
				str_format(aMsg, sizeof(aMsg), "%s", pLine);
				int SendMessagePNG = 0;

				if ((str_find_nocase(m_aLines[m_CurrentLine].m_aText, "chillerbot") && str_find_nocase(m_aLines[m_CurrentLine].m_aText, "?")) || str_find_nocase(m_aLines[m_CurrentLine].m_aText, "info"))
				{
					Say(0, "chillerbot.png [ChillerBot-CB] by ChillerDragon (write !help for help)");
				}
				else if (str_find_nocase(aMsg, "!rank_voter"))
				{
					GetRankVoter(aMsg);
				}
				else if (str_find_nocase(aMsg, "!rank_voted"))
				{
					GetRankVoted(aMsg);
				}
				else if (str_find_nocase(aMsg, "!changelog"))
				{
					Say(0, "[v.0.0.2] + added mutes + added join/spec");
					//Say(0, "[v.0.0.1] + ignoring spam + added '!rank name'");
				}
				else if (str_find_nocase(aMsg, "!top"))
				{
					SendMessagePNG = 1;
				}
				else if (str_find_nocase(aMsg, "!rank"))
				{
					if (IsOwnRank(aMsg))
					{
						SendMessagePNG = 2;
					}
					else
					{
						getrank = cut_string_get_rankname(aMsg);
						//str_format(aBuf, sizeof(aBuf), "requesting rank for '%s'", getrank.c_str());
						//Say(0, aBuf);
						SendMessagePNG = 4;
					}
				}
				else if (str_find_nocase(aMsg, "!help"))
				{
					Say(0, "Write in chat to get points. (spam gets filtert). Check !cmdlist for commands.");
				}
				else if (str_find_nocase(aMsg, "!cmdlist2"))
				{
					Say(0, "!teamchat, !pubchat, !pay_100 (name), !register, !rank_voter, !rank_voted");
				}
				else if (str_find_nocase(aMsg, "!cmdlist") || str_find_nocase(aMsg, "!commands") || str_find_nocase(aMsg, "!cmdlist1"))
				{
					Say(0, "!help, !top, !rank, !totalranks, !changelog, !mute_lvl, !mute, !unmute, !join (red/blue), !spec, !cmdlist2");
				}
				else if (str_find_nocase(aMsg, "!totalranks"))
				{
					SendMessagePNG = 3;
				}
				else if (str_find_nocase(aMsg, "no u"))
				{
					Say(0, "no u");
				}
				else if (str_find_nocase(aMsg, "!test"))
				{
					Say(0, "/login vag vag;/insta boomfng");
				}
				else if (str_find_nocase(aMsg, "!mute_lvl"))
				{
					IsMuteLevel = true;
				}
				else if (str_find_nocase(aMsg, "!mute"))
				{
					IsMute = true;
				}
				else if (str_find_nocase(aMsg, "!unmute"))
				{
					IsMute = false;
					IsMuteLevel = false;
				}
				else if (str_find_nocase(aMsg, "!join"))
				{
					if (!IsBlockJoin)
					{
						if (str_find_nocase(aMsg, "red"))
						{
							Client()->SendSwitchTeam(0);
						}
						else
						{
							Client()->SendSwitchTeam(1);
						}
					}
				}
				else if (str_find_nocase(aMsg, "!spec"))
				{
					Client()->SendSwitchTeam(-1);
				}
				else if (str_find_nocase(aMsg, "!teamchat"))
				{
					IsTeamchat = true;
				}
				else if (str_find_nocase(aMsg, "!pubchat"))
				{
					IsTeamchat = false;
				}
				else if (str_find_nocase(aMsg, "!pay_100"))
				{
					Say(0, "[PAY] you have to be logged in to use this command");
					//getrank = cut_string_get_payname(aMsg);
					//SendMessagePNG = 5;
				}
				else if (str_find_nocase(aMsg, "!register"))
				{
					Say(0,"this command is in development");

					bool doit = false;

					if (doit)
					{
						std::string *aData = cut_string_get_registername(aMsg);
						registername = aData[0];
						registerpw = aData[1];
						dbg_msg("recived", "0='%s' 1='%s'", aData[0].c_str(), aData[1].c_str());
						dbg_msg("recived", "name='%s' pw='%s'", registername.c_str(), registerpw.c_str());
						str_format(aBuf, sizeof(aBuf), "[ACC] name=%s pw=%s", registername.c_str(), registerpw.c_str());
						Say(0, aBuf);

						SendMessagePNG = 6;
					}
				}
				else if (str_find_nocase(aMsg, "!stop"))
				{
					m_ChillFlyY = 0;
					m_Chillrection = 0;
				}
				else if (str_find_nocase(aMsg, "!kill"))
				{
					Client()->SendKill();
				}
				else if (str_find_nocase(aMsg, "!look_right_down"))
				{
					m_ChillEyeMode = 5;
				}
				else if (str_find_nocase(aMsg, "!look_left_down"))
				{
					m_ChillEyeMode = 6;
				}
				else if (str_find_nocase(aMsg, "!look_left_up"))
				{
					m_ChillEyeMode = 7;
				}
				else if (str_find_nocase(aMsg, "!look_right_up"))
				{
					m_ChillEyeMode = 8;
				}
				else if (str_find_nocase(aMsg, "!look_little_left_up"))
				{
					m_ChillEyeMode = 9;
				}
				else if (str_find_nocase(aMsg, "!look_little_right_up"))
				{
					m_ChillEyeMode = 10;
				}
				else if (str_find_nocase(aMsg, "!look_down_right"))
				{
					m_ChillEyeMode = 11;
				}
				else if (str_find_nocase(aMsg, "!look_down_left"))
				{
					m_ChillEyeMode = 12;
				}
				else if (str_find_nocase(aMsg, "!look_left"))
				{
					m_ChillEyeMode = 1;
				}
				else if (str_find_nocase(aMsg, "!look_right"))
				{
					m_ChillEyeMode = 2;
				}
				else if (str_find_nocase(aMsg, "!look_up"))
				{
					m_ChillEyeMode = 3;
				}
				else if (str_find_nocase(aMsg, "!look_down"))
				{
					m_ChillEyeMode = 4;
				}
				//else if (str_find_nocase(aMsg, "!fly"))
				//{
				//	std::string strFlyY = cut_string_get_flyint(aMsg);
				//	m_ChillFlyY = 0;
				//	if (strFlyY == "(invalid)")
				//	{
				//		Say(0,"[FLY] invalid fly height");
				//	}
				//	else
				//	{
				//		m_ChillFlyY = atoi(strFlyY.c_str());
				//		str_format(aBuf, sizeof(aBuf), "[FLY] flying at ( %d )", m_ChillFlyY);
				//		Say(0, aBuf);
				//	}
				//}

				//combinable movements excloude from ifelse
				if (str_find_nocase(aMsg, "!left_2"))
				{
					m_ChillMoveTick = 10;
					m_Chillrection = -1;
				}
				else if (str_find_nocase(aMsg, "!left_1"))
				{
					m_ChillMoveTick = 5;
					m_Chillrection = -1;
				}
				else if (str_find_nocase(aMsg, "!left"))
				{
					m_ChillMoveTick = -1;
					m_Chillrection = -1;
				}
				if (str_find_nocase(aMsg, "!right_2"))
				{
					m_ChillMoveTick = 10;
					m_Chillrection = 1;
				}
				else if (str_find_nocase(aMsg, "!right_1"))
				{
					m_ChillMoveTick = 5;
					m_Chillrection = 1;
				}
				else if (str_find_nocase(aMsg, "!right"))
				{
					m_ChillMoveTick = -1;
					m_Chillrection = 1;
				}
				if (str_find_nocase(aMsg, "!jump"))
				{
					m_IsChillJump = 1;
				}
				if (str_find_nocase(aMsg, "!hook_super_short"))
				{
					m_ChillHookTick = 3;
				}
				else if (str_find_nocase(aMsg, "!hook_short"))
				{
					m_ChillHookTick = 10;
				}
				else if (str_find_nocase(aMsg, "!hook_long"))
				{
					m_ChillHookTick = 200;
				}
				else if (str_find_nocase(aMsg, "!hook"))
				{
					m_ChillHookTick = 100;
				}

				//str_format(aBuf, sizeof(aBuf), "/w %s hello <3", aName);
				//Say(0, aBuf);


				//==============
				//acc stuff
				//==============

				if (SendMessagePNG == 6)
				{
					str_format(aBuf, sizeof(aBuf), "acc/%s.txt", registername.c_str());
					if (is_file_exist(aBuf))
					{
						Say(0, "[ACC] account already exists");
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "acc/%s.txt", registername.c_str());
						std::ofstream fileAccW(aBuf);
						if (fileAccW.is_open())
						{
							fileAccW << "some data" << "\n";
							fileAccW.close();
						}
					}
				}

				//==============
				//process names
				//==============

				//check if we ever met the person
				std::string UpgradeName;
				std::string line;
				std::string aNames[MAX_DUDES]; //track max 2048 tees cuz tw hasnt more players lulul if it reaches the limit teh client crashes
				std::ifstream file(g_Config.m_ClChatPath);
				char tmp[32];
				int index = 0;
				int found = -1;
				int best_val = 0;
				int index_best_val;
				int tmp_val = 0;
				int val = 0;
				int getrank_index = 0;
				int getrank_val = 0;
				int total_dudes = 0;
				bool IsFriend = false;

				while (std::getline(file, line))
				{
					str_format(tmp, sizeof(tmp), "%s", cut_string(line).c_str());
					aNames[index] = line;
					tmp_val = atoi(cut_string_get_int(line).c_str());
					if (SendMessagePNG == 1) //calulate best
					{
						if (tmp_val > best_val)
						{
							index_best_val = index;
							best_val = tmp_val;
						}
					}
					//dbg_msg("dudes", "comp[%d] '%s' '%s'", tmp_val, tmp, aName);
					if ((SendMessagePNG == 4 || SendMessagePNG == 5) && !str_comp(tmp, getrank.c_str())) //request score from specifc player with '!rank name'
					{
						getrank_val = tmp_val;
						if (SendMessagePNG == 5)
						{
							getrank_index = index;
							dbg_msg("pay", "[%d] '%s' found", getrank_val, tmp);
						}
						else
						{
							dbg_msg("rank", "[%d] '%s' found", getrank_val, tmp);
						}
					}
					if (!str_comp(tmp, aName) && found == -1) //only can be found once
					{
						found = index;
						val = tmp_val;
						UpgradeName = cut_string(line);
						dbg_msg("dudes", "[%d] '%s' found", val, tmp);
						//dbg_msg("friends", "'%s' is friend", UpgradeName.c_str());
						//IsFriend = true;
						int level = 0;

						while (level < 900)
						{
							if (val == pow(level, 2))
							{
								if (!str_comp(aName, " "))
								{
									str_format(aBuf, sizeof(aBuf), "[WARNING] dataloose reached level %d", level);
									Say(0, aBuf);
								}
								else
								{
									if (IsMuteLevel)
									{
										str_format(aBuf, sizeof(aBuf), "/w %s [CHAT] '%s' reached level %d", aName, aName, level);
									}
									else
									{
										str_format(aBuf, sizeof(aBuf), "[CHAT] '%s' reached level %d", aName, level);
									}
									Say(0, aBuf);
								}

								break;
							}
							level++;
						}
					}
					index++;
					total_dudes++;
				}
				file.close();

				if (found != -1)
				{
					val++;

					if (SendMessagePNG == 5)
					{
						if (val < 100)
						{
							str_format(aBuf, sizeof(aBuf), "[PAY] '%s' you need atleast 100 points", aName);
							Say(0, aBuf);
						}
						else if (getrank_val == 0)
						{
							str_format(aBuf, sizeof(aBuf), "[PAY] '%s' is an invalid account", getrank.c_str());
							Say(0, aBuf);
						}
						else
						{
							val -= 100;
							getrank_val += 100;

							str_format(aBuf, sizeof(aBuf), "[PAY] '%s' payed '%s' %d points", aName, getrank.c_str(), 100);
							Say(0, aBuf);
						}

					}

					str_format(aBuf, sizeof(aBuf), "%d %s", val, UpgradeName.c_str());
					aNames[found] = aBuf;
					if (!str_comp("(invalid)", getrank.c_str()) && !getrank_index && !getrank_val)
					{
						//ignore this case
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "%d %s", getrank_val, getrank.c_str());
						aNames[getrank_index] = aBuf;
					}

					//rewrite all
					std::ofstream fileW(g_Config.m_ClChatPath);
					if (fileW.is_open())
					{
						for (int i = 0; i < total_dudes; i++)
						{
							fileW << aNames[i] << "\n";
						}
						fileW.close();
					}
					dbg_msg("dudes", "total: %d rewritten", total_dudes);

					if (IsFriend && false)
					{
						//save plain message
						str_format(aBuf, sizeof(aBuf), "friends/%s_raw.txt", UpgradeName.c_str());
						std::ofstream friends(aBuf, std::ios_base::app);
						if (friends.is_open())
						{
							friends << m_aLines[m_CurrentLine].m_aText << "\n";
							friends.close();
						}
						else
						{
							dbg_msg("friends", "failed to add friend");
						}

						//analyse data and save nice data
						if (str_find_nocase(m_aLines[m_CurrentLine].m_aText, "beckyhill") || str_find_nocase(m_aLines[m_CurrentLine].m_aText, "becky") || str_find_nocase(m_aLines[m_CurrentLine].m_aText, "hilly"))
						{
							std::string aData[64];
							str_format(aBuf, sizeof(aBuf), "friends/%s.txt", UpgradeName.c_str());
							std::ifstream file(aBuf);
							int love_points = 1;
							index = 0;

							while (std::getline(file, line))
							{
								aData[index] = line;
								if (index == 0)
								{
									tmp_val = atoi(line.c_str());
									tmp_val += love_points;
								}
								dbg_msg("friend", "[%d]line  DATA [ %s ]", index, line.c_str());
								index++;
							}
							file.close();

							if (!index) //init file
							{
								index = 1;
							}
							str_format(aBuf, sizeof(aBuf), "%d", tmp_val);
							aData[0] = aBuf; //update love points

							str_format(aBuf, sizeof(aBuf), "friends/%s.txt", UpgradeName.c_str());
							std::ofstream fileW(aBuf);
							if (fileW.is_open())
							{
								for (int i = 0; i < index; i++)
								{
									fileW << aData[i] << "\n";
								}
								fileW.close();
							}
							dbg_msg("friends", "total: %d rewritten", index);
						}
					}
				}
				else
				{
					std::ofstream fileW(g_Config.m_ClChatPath, std::ios_base::app);
					str_format(aBuf, sizeof(aBuf), "0 %s", aName);
					fileW << aBuf << "\n";
					dbg_msg("dudes", "added new '%s'", aBuf);
				}


				if (SendMessagePNG == 1) //!top
				{
					str_format(aBuf, sizeof(aBuf), "[TOP] 1. '%s' with [%d]", cut_string(aNames[index_best_val]).c_str(), best_val);
					Say(0, aBuf);
				}
				else if (SendMessagePNG == 2) //!rank
				{
					std::string line;
					std::ifstream rankfile(g_Config.m_ClChatPath);
					char tmp[32];
					int rank = 1;

					while (std::getline(rankfile, line))
					{
						tmp_val = atoi(cut_string_get_int(line).c_str());
						//dbg_msg("TOP", "tmp[%d] own[%d]", tmp_val, val);
						if (tmp_val > val) //if someone better found decrease rank
						{
							rank++;
							//dbg_msg("TOP", "rank[%d]", rank);
						}
					}
					rankfile.close();

					str_format(aBuf, sizeof(aBuf), "[TOP] %d. '%s' with [%d]", rank, aName, val);
					dbg_msg("format", aBuf);
					Say(0, aBuf);
				}
				else if (SendMessagePNG == 4) //!rank name
				{
					std::string line;
					std::ifstream rankfile(g_Config.m_ClChatPath);
					char tmp[32];
					int rank = 1;

					while (std::getline(rankfile, line))
					{
						tmp_val = atoi(cut_string_get_int(line).c_str());
						//dbg_msg("TOP", "tmp[%d] own[%d]", tmp_val, val);
						if (tmp_val > getrank_val) //if someone better found decrease rank
						{
							rank++;
							//dbg_msg("TOP", "rank[%d]", rank);
						}
					}
					rankfile.close();

					if (getrank_val == 0)
					{
						str_format(aBuf, sizeof(aBuf), "[TOP] '%s' is unranked", getrank.c_str());
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "[TOP] %d. '%s' with [%d]", rank, getrank.c_str(), getrank_val);
					}
					dbg_msg("format", aBuf);
					Say(0, aBuf);
				}
				else if (SendMessagePNG == 3) //!totalranks
				{
					str_format(aBuf, sizeof(aBuf), "[TOP] total ranks %d", total_dudes);
					Say(0, aBuf);
				}
			}
		}

		str_format(aBuf, sizeof(aBuf), "%s%s", m_aLines[m_CurrentLine].m_aName, m_aLines[m_CurrentLine].m_aText);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, m_aLines[m_CurrentLine].m_Team?"teamchat":"chat", aBuf);
	}
}

void CChat::Say(int Team, const char *pLine)
{
	if (IsMute)
		return;

	if (IsTeamchat)
	{
		Team = 1;
	}
	else
	{
		//Team = 0;
	}

	// send chat message
	CNetMsg_Cl_Say Msg;
	Msg.m_Team = Team;
	Msg.m_pMessage = pLine;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

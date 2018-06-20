#include "Net/mpLib.h"

const char* mpState_ToString(mpState state)
{
	switch (state)
	{
		case mpState_Idle:			return "Idle";
		case mpState_Hosting:		return "Hosting";
		case mpState_Joining:		return "Joining";
		case mpState_Lobby:			return "Lobby";
		case mpState_StartingGame:	return "StartingGame";
		case mpState_Game:			return "Game";
		case mpState_EndingGame:	return "EndingGame";
		case mpState_Leaving:		return "Leaving";
	}
	return "<Invalid>";
}
#ifndef NT_LEADERBOARDS_H
#define NT_LEADERBOARDS_H

#include "Base/ueBase.h"

struct ueValue;
struct ntLeaderboardLib;

void ntLeaderboards_Startup(ueAllocator* allocator, u32 numLeaderboards);
void ntLeaderboards_Shutdown();

u32 ntLeaderboards_GetNumSupportedLibs();

void ntLeaderboards_Submit(u32 leaderboardIndex, const ueValue* value, nsLeaderboardLib* lib = NULL);
ueBool ntLeaderboards_GetRank(u32 leaderboardIndex, u32& rank, nsLeaderboardLib* lib = NULL);
ueBool ntLeaderboards_GetUsersCount(u32 leaderboardIndex, u32& count, nsLeaderboardLib* lib = NULL);
ueBool ntLeaderboards_GetScore(u32 leaderboardIndex, ueValue* score, nsLeaderboardLib* lib = NULL);
ueBool ntLeaderboards_GetBestScore(u32 leaderboardIndex, ueValue* score, nsLeaderboardLib* lib = NULL);

#ifdef UE_MARMALADE

ntLeaderboardLib* ntGameCenterLeaderboards_Register(const char** leaderboardIds);
void ntGameCenterLeaderboards_Unregister();

ntLeaderboardLib* ntOpenFeintLeaderboards_Register(const char** leaderboardIds);
void ntOpenFeintLeaderboards_Unregister();

#endif

#if defined(UE_WIN32) || defined(UE_MAC) || defined(UE_LINUX)

ntLeaderboardLib* ntSteamLeaderboards_Register(const char** leaderboardIds);
void ntSteamLeaderboards_Unregister();

#endif

#ifdef UE_X360

ntLeaderboardLib* ntX360Leaderboards_Register(const char** leaderboardIds);
void ntX360Leaderboards_Unregister();

#endif

#endif // NT_LEADERBOARDS_H
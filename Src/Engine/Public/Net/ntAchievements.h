#ifndef NT_ACHIEVEMENTS_H
#define NT_ACHIEVEMENTS_H

#include "Base/ueBase.h"

void ntAchievements_Startup(ueAllocator* allocator, u32 numAchievements);
void ntAchievements_Shutdown();

u32 ntAchievements_GetNumSupportedLibs();

void ntAchievements_Unlock(u32 index, f32 percentage);

#ifdef UE_MARMALADE

void ntGameCenterAchievements_Register(const char** achievementIds);
void ntGameCenterAchievements_Unregister();

void ntOpenFeintAchievements_Register(const char** achievementIds);
void ntOpenFeintAchievements_Unregister();

#endif

#if defined(UE_WIN32) || defined(UE_MAC) || defined(UE_LINUX)

void nsSteamAchievements_Register(const char** achievementIds);
void nsSteamAchievements_Unregister();

#endif

#ifdef UE_X360

void nsX360Achievements_Register(const char** achievementIds);
void nsX360Achievements_Unregister();

#endif

#endif // NT_ACHIEVEMENTS_H
#ifndef UE_PLATFORM_H
#define UE_PLATFORM_H

/**
 *	@addtogroup ue
 *	@{
 */

//! Platform enumeration
enum uePlatform
{
	uePlatform_Win32		= 0,	//!< Win32 PC
	uePlatform_Win64		= 1,	//!< Win64 PC
	uePlatform_X360			= 2,	//!< XBox 360
	uePlatform_PS3			= 3,	//!< Playstation 3
	uePlatform_WII			= 4,	//!< Wii
	uePlatform_Linux		= 5,	//!< Linux
	uePlatform_Mac			= 6,	//!< Mac OS
	uePlatform_Marmalade	= 7,	//!< Marmalade (http://www.madewithmarmalade.com)

	uePlatform_Unknown		= 256,	//!< Unknown platform

	uePlatform_MAX
};

#if defined(MARMALADE)

    #define UE_MARMALADE
	#define UE_INLINE inline
	#define DWORD int
	#undef BOOL
	#undef FLOAT

	#ifdef IW_DEBUG
		#define UE_DEBUG
	#else
		#define UE_FINAL
	#endif

	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include <math.h>
	#include <time.h>
	#include "s3e.h"
	#include "IwGx.h"

#elif defined(_WIN32)

    #define UE_WIN32
	#define	UE_TARGET_WIN_XP // Make this build runnable on WinXP
	#define UE_INLINE inline

	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <windows.h>
	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/types.h>

	#if defined(UE_ENABLE_ASSERTION)
		#define UE_ASSERTWinCall(call) { HRESULT hr = call; UE_ASSERT_MSGP(SUCCEEDED(hr), "Windows call '%s' failed, hr = 0x%x", #call, hr); }
	#else
		#define UE_ASSERTWinCall(call) call;
	#endif

#elif defined(_XBOX)

    #define UE_X360
	#define UE_INLINE inline

#elif defined(LINUX)

    #define UE_LINUX
    #define UE_INLINE inline

	#include <stdio.h>
	#include <stdlib.h>
	#include <stdarg.h>
	#include <string.h>
	#include <unistd.h>
	#include <pthread.h>
    #include <errno.h>

#elif defined(__MAC__)

    #define UE_MAC
    #define UE_INLINE inline

	#include <stdio.h>
	#include <stdlib.h>
	#include <stdarg.h>
	#include <string.h>
	#include <unistd.h>
    #include <errno.h>

#endif

#if defined(_DEBUG)
	#define UE_DEBUG
#elif defined(FINAL)
	#define UE_FINAL
#else
	#define UE_RELEASE
#endif

//! Gets platform value for a given platform name string
void uePlatform_FromString(uePlatform& platform, const char* s);
//! Gets current platform's name 
const char* uePlatforms_AsString();

// @}

#endif // UE_PLATFORM_H

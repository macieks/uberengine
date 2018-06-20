#ifndef UE_CONFIG_H
#define UE_CONFIG_H

/**
 *	@addtogroup ue
 *	@{
 */

#include "Base/uePlatform.h"

#if defined(UE_TOOLS)
	#include "Base/ueToolsConfig.h"
#else
	#include "Base/ueRuntimeConfig.h"
#endif

// @}

#endif // UE_CONFIG_H

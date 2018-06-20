#ifndef UE_TOOLS_CONFIG_H
#define UE_TOOLS_CONFIG_H

/**
 *	@addtogroup ue
 *	@{
 */

#include "Base/uePlatform.h"

// Debugging / profiling
// ---------------------------------------------

#if defined(UE_DEBUG)
	//! Enables assertions; if not defined UE_ASSERT and UE_ASSERT_MSG macros are defined as empty operation
	#define UE_ENABLE_ASSERTION
#endif

// Misc.
// ---------------------------------------------

//! Default alignment used by all allocators
#define UE_DEFAULT_ALIGNMENT 4

// Logging
// ---------------------------------------------

//! Enables logging; if not defined all ueLog* macros are defined as empty operation
#define UE_ENABLE_LOGGING

//! Max. number of log listeners that can be registered at the same time
#define UE_MAX_LOG_LISTENERS 8

// Math
// ---------------------------------------------

#if defined(UE_WIN32) || defined(UE_X360)
	#define SM_MATH_XNA
#endif

// IO
// ---------------------------------------------

//! Enables endianess swapping for ioBitBuffer; enabling this slows down read & write operations
#define IO_BIT_BUFFER_SUPPORTS_ENDIANESS_SWAP
//! Enables endianess swapping for ioBuffer; enabling this slows down read & write operations
#define IO_BUFFER_SUPPORTS_ENDIANESS_SWAP

// Graphics
// ---------------------------------------------


// @}

#endif // UE_TOOLS_CONFIG_H
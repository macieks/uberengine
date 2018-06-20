#ifndef UE_RUNTIME_CONFIG_H
#define UE_RUNTIME_CONFIG_H

/**
 *	@addtogroup ue
 *	@{
 */

#include "Base/uePlatform.h"

// Debugging / profiling
// ---------------------------------------------

#if defined(UE_DEBUG)
	//! Enables assertions; if not defined UE_ASSERT and UE_ASSERT_MSG macros are defined as empty operation
	#define UE_ENABLE_ASSERTION 1
#endif

#if !defined(UE_FINAL)
	//! Enables warnings; if not defined ueWarn macro is defined as empty operation
	#define UE_ENABLE_WARNINGS 1
	//! Enables profiler; if not defined UE_PROF_SCOPE macro is defined as empty operation
	#define UE_ENABLE_PROFILER 1
#endif

// Logging
// ---------------------------------------------

#if !defined(UE_FINAL)
	//! Enables logging; if not defined all ueLog* macros are defined as empty operation
	#define UE_ENABLE_LOGGING 1
#endif

//! Max. number of log listeners that can be registered at the same time
#define UE_MAX_LOG_LISTENERS 8

// Memory
// ---------------------------------------------

#if defined(UE_DEBUG)
	//! Enables memory debugging for allocators including stats gathering and more
	#define UE_ENABLE_MEMORY_DEBUGGING 1
#endif

#if !defined(UE_MARMALADE)
	#define UE_NO_GLOBAL_NEW 1
#endif

// Math
// ---------------------------------------------

#if defined(UE_WIN32) || defined(UE_X360)
	//! Enables XNA based SIMD math
	#define UE_SIMD_MATH_XNA 1
#endif

// Misc
// ---------------------------------------------

#if defined(UE_WIN32)
	//! Default alignment used by all allocators
	#define UE_DEFAULT_ALIGNMENT 4
#else
	//! Default alignment used by all allocators
	#define UE_DEFAULT_ALIGNMENT 16
#endif

#if !defined(UE_MARMALADE)
	#define UE_DEFAULT_THREAD_SAFE 1
#endif

// Input
// ---------------------------------------------

#if defined(UE_MARMALADE)
	#define IN_ENABLE_ACCELEROMETER 1
#else
	#define IN_ENABLE_MOUSE 1
	#define IN_ENABLE_GAMEPAD 1
#endif
#define IN_ENABLE_TOUCHPAD 1

// Containers
// ---------------------------------------------

#if !defined(UE_FINAL)
	#define UE_AB_TREE_GATHER_QUERY_STATS 1
#endif

// Physics
// ---------------------------------------------

#if defined(UE_WIN32) || defined(UE_X360) || defined(UE_PS3)
//	#define PH_BULLET 1
	//! Enables PhysX physics library
	#define PH_PHYSX 1
#elif defined(UE_LINUX) || defined(UE_MAC)
	//! Enables Bullet physics library
    #define PH_BULLET 1
#endif

// Networking
// ---------------------------------------------

#if defined(UE_WIN32) || defined(UE_PS3) || defined(UE_LINUX) || defined(UE_MAC)
	//! Enables enet based multiplayer implementation
	#define MP_USE_ENET 1
#endif

// Graphics
// ---------------------------------------------

//! Max. number of vertex streams bound at the same time
#define GL_MAX_BOUND_VERTEX_STREAMS				8
//! Max. number of vertex stream format elements (@see glStreamFormat)
#define GL_MAX_VERTEX_STREAM_ELEMENTS			16
//! Max. number of shader input attributes
#define GL_MAX_VERTEX_SHADER_INPUT_ATTRIBUTES	32

#if 0

#if defined(UE_WIN32)
	//! Enables DirectX 9 based graphics implementation
	//#define GL_D3D9 1
	//! Enables DirectX 11 based graphics implementation
	#define GL_D3D11 1
	//! Enables OpenGL based graphics implementation
	//#define GL_OPENGL 1
	//! Enables OpenGL ES based graphics implementation (requires GL_OPENGL #define)
	//#define GL_OPENGL_ES 1
#elif defined(UE_MARMALADE)
	//#define GL_MARMALADE 1
	//#define GL_FIXED_PIPELINE 1
	#define GL_OPENGL 1
	#define GL_OPENGL_ES 1
#elif defined(UE_X360)
	//! Enables X360's (D3D9 like) graphics implementation
	#define GL_X360 1
#elif defined(UE_PS3)
	//! Enables PS3's (D3D9 like) graphics implementation
	#define GL_PS3 1
#elif defined(UE_LINUX) || defined(UE_MAC)
	#define GL_OPENGL 1
#endif

#else

#if defined(GL_OPENGL_ES) && !defined(GL_OPENGL)
	#define GL_OPENGL 1
#endif

#endif

#if defined(UE_DEBUG)
	#if defined(GL_D3D9)
		//! Enables detailed D3D debugging features (enabled via D3D9 control panel); win32 only
		#define D3D_DEBUG_INFO 1
	#elif defined(GL_OPENGL)
		//! Enables OpenGL error checking for all operations; enabling this slows down OpenGL code
		#define GL_CHECK_OPENGL_ERRORS 1
		//!< Enables shader code preview in debugger via glShader::m_debugSrc; enabling this requires extra memory for shader code text that is otherwise unnecessary after shader compilation
		#define GL_ENABLE_SHADER_PREVIEW 1
	#endif
#endif

#if defined(GL_D3D11)
    #define GL_MAX_COLOR_RENDER_TARGETS 8
#elif defined(GL_D3D9)
    #define GL_MAX_COLOR_RENDER_TARGETS 4
#elif defined(GL_OPENGL_ES)
	#define GL_MAX_COLOR_RENDER_TARGETS 1
#elif defined(GL_OPENGL)
    #define GL_MAX_COLOR_RENDER_TARGETS 4
#endif

// Graphics Extended
// ---------------------------------------------

#if defined(UE_WIN32)
	//! Enables use of Microsoft's WMF movie library
	//#define GX_MOVIE_USE_WMF 1
	//! Enables use of ffmpeg movie library
	#define GX_MOVIE_USE_FFMPEG 1
#endif // defined(UE_WIN32)

#if !defined(UE_MARMALADE)
	#define GX_ENABLE_DEBUG_WINDOWS 1
#endif

//! Default input priority (used by inSys) for the visual profiler
#define GX_DEFAULT_PROFILER_INPUT_PRIORITY		0.5f
//! Default input priority (used by inSys) for the debug menu
#define GX_DEFAULT_DEBUG_MENU_INPUT_PRIORITY	1.0f

// @}

#endif // UE_RUNTIME_CONFIG_H

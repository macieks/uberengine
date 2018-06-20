#ifndef GX_SHADER_H
#define GX_SHADER_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Base/ueResource.h"

UE_DEFINE_RESOURCE_TYPE(gxShader, UE_BE_4BYTE_SYMBOL('s','h','d','r'));

struct glShader;

//! Gets the actual shader object
glShader* gxShader_GetShaderObject(gxShader* s);

void gxShaderMgr_Startup(ueAllocator* allocator, const char* config);
void gxShaderMgr_Shutdown();

// @}

#endif // GX_SHADER_H

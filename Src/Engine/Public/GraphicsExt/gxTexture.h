#ifndef GX_TEXTURE_H
#define GX_TEXTURE_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Base/ueResource.h"
#include "GraphicsExt/gxTexture_Shared.h"

UE_DEFINE_RESOURCE_TYPE(gxTexture, gxTexture_TYPE_ID);

struct glTextureBuffer;

//! Gets texture buffer
glTextureBuffer*		gxTexture_GetBuffer(gxTexture* t);
//! Gets texture width
u32						gxTexture_GetWidth(const gxTexture* t);
//! Gets texture height
u32						gxTexture_GetHeight(const gxTexture* t);
//! Gets texture atlas entry information for this texture or NULL if texture isn't part of an atlas; use gxTexture_IsInAtlas to check whether texture is in atlas
gxTextureAtlasEntry*	gxTexture_GetAtlasEntry(gxTexture* t);
//! Gets whether this texture is texture atlas
ueBool					gxTexture_IsAtlas(const gxTexture* t);
//! Gets whether this texture is part of a texture atlas
ueBool					gxTexture_IsInAtlas(const gxTexture* t);

void gxTextureMgr_Startup(ueAllocator* allocator, const char* config, ueBool supportLocalization = UE_TRUE);
void gxTextureMgr_Shutdown();

// @}

#endif // GX_TEXTURE_H

#ifndef GX_TEXTURE_SHARED_H
#define GX_TEXTURE_SHARED_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Base/ueShapes.h"

struct gxTexture;

static const u32 gxTexture_TYPE_ID				= UE_BE_4BYTE_SYMBOL('t','x','t','r');
static const u32 gxTexture_ATLAS_TYPE_ID		= UE_BE_4BYTE_SYMBOL('a','t','l','s');
static const u32 gxTexture_ATLAS_ENTRY_TYPE_ID	= UE_BE_4BYTE_SYMBOL('a','t','e','n');

//! Description of the texture within atlas
struct gxTextureAtlasEntry
{
	char* m_name;				//!< Texture name
	ueRectI m_rect;				//!< Rectangle within atlas
	f32 m_scale[2];				//!< XY scale to apply to UVs for this texture
	f32 m_translation[2];		//!< XY translation to apply to UVs for this texture
	gxTexture* m_texture;		//!< Texture created at runtime for this entry
};

//! Texture atlas description
struct gxTextureAtlasInfo
{
	u32 m_numEntries;				//!< Number of textures within atlas
	gxTextureAtlasEntry* m_entries; //!< Textures within atlas
	gxTexture* m_texture;			//!< Texture created at runtime for this atlas
};

// @}

#endif // GX_TEXTURE_SHARED_H

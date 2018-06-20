#ifndef GL_LIB_SHARED_H
#define GL_LIB_SHARED_H

#include "Graphics/glStructs_Shared.h"
#include "Base/ueShapes.h"

/**
 *	@addtogroup gl
 *	@{
 */

//! Calculates number of texture mip levels
u32			glUtils_CalcNumMipLevels(u32 width, u32 height, u32 depth);
//! Calculates surface size in bytes
u32			glUtils_CalcSurfaceSize(glTexType type, u32 width, u32 height, u32 depth, glBufferFormat format);
//! Calculates surface row parameters
void		glUtils_CalcSurfaceRowParams(u32 width, u32 height, u32 depth, glBufferFormat format, u32& rowSize, u32& numRows);

//! Gets number of bits per pixel for given format
u32			glUtils_GetFormatBits(glBufferFormat format);
//! Gets number of bytes per pixel for given format; mustn't be called for compressed formats
u32			glUtils_GetFormatBytes(glBufferFormat format);
//! Tells whether format contains depth
ueBool		glUtils_IsDepthFormat(glBufferFormat format);
//! Tells whether format contains stencil
ueBool		glUtils_IsStencilFormat(glBufferFormat format);
//! Tells whether format is compressed with 4x4 blocks of pixels
ueBool		glUtils_Is4x4CompressedFormat(glBufferFormat format);
//! Gets whether format requires square dimensions
ueBool		glUtils_FormatRequiresSquareDimensions(glBufferFormat format);

#ifdef UE_WIN32

#include "d3dx9tex.h"

ueBool		glUtils_HasAlpha(D3DFORMAT format);
ueBool		glUtils_ToStorage(glTexStorage& dst, const D3DXIMAGE_FILEFORMAT& src);

#endif

//! Gets index of the alpha channel; returns 0xFFFFFFFF if there's no alpha in given format
u32			glUtils_GetAlphaChannelIndex(glBufferFormat format);
//! Gets indices of the red/green/blue/alpha channels; gets 0xFFFFFFFF if there's no specific channel in given format
void		glUtils_GetChannelIndices(glBufferFormat format, u32& r, u32& g, u32& b, u32& a);

//! Gets size of a single shader constant element for a given type
u32			glUtils_GetShaderConstantSize(glConstantType type);
//! Gets shader constant type size
u32			glUtils_GetTypeSize(glConstantType type);
//! Tells whether shader constant type is a sampler
ueBool		glUtils_IsSampler(glConstantType type);
//! Tells whether shader constant type is float
ueBool		glUtils_IsFloat(glConstantType type);
//! Tells whether shader constant type is integer
ueBool		glUtils_IsInt(glConstantType type);
//! Tells whether shader constant type is boolean
ueBool		glUtils_IsBool(glConstantType type);

//! Copies one texture into specified area of another texture
void		glUtils_CopyTextureRect(glBufferFormat format, void* dst, u32 dstRowPitch, const ueRectI& dstRect, const void* src, u32 srcRowPitch, u32 numRows);
//! Adds border around specified rectangle within texture
void		glUtils_MakeTextureBorder(glBufferFormat format, void* data, u32 width, u32 height, const ueRectI& rect, u32 border);

// @}

#endif // GL_LIB_SHARED_H
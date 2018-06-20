#ifndef GL_LIB_MARMALADE_H
#define GL_LIB_MARMALADE_H

#include "Graphics/glStructs.h"

class CIwFMat;

const f32 GL_MARMALADE_UV_MULTIPLIER = (f32) (1 << 12);
const f32 GL_MARMALADE_COORD_MULTIPLIER = (f32) (1 << 4);

#define GL_TO_MARMALADE_COORD(floatValue) ((int16) ((floatValue) * GL_MARMALADE_COORD_MULTIPLIER))
#define GL_TO_MARMALADE_UV(floatValue) ((int16) ((floatValue) * GL_MARMALADE_UV_MULTIPLIER))

u32 glDevice_GetUsedTextureMemory();
void glDevice_EnableFakeTextureDownsize2x();

// Texture buffer

CIwTexture* glTextureBuffer_GetIwTexture(glTextureBuffer* tb);
CIwMaterial* glTextureBuffer_GetIwMaterial(glTextureBuffer* tb);

// Utils

void glUtils_ToMarmaladeMatrix(CIwMat* dst, const ueMat44* src);
CIwImage::Format glUtils_ToMarmaladeFormat(glBufferFormat format);

#endif // GL_LIB_MARMALADE_H
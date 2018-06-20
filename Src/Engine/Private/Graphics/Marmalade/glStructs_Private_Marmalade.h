#ifndef GL_STRUCTS_PRIVATE_MARMALADE_H
#define GL_STRUCTS_PRIVATE_MARMALADE_H

#include <GLES/gl.h>
#include <GLES/egl.h>

#include "Containers/ueHashSet.h"
#include "Containers/ueHashMap.h"
#include "Containers/ueList.h"
#include "GL/glStructs_Private.h"
#include "GL/Marmalade/glStructs_Marmalade.h"

class CIwTexture;

// Context

struct glCtx : glCtx_Base
{
};

// Device

struct glDevice : public glDevice_Base
{
	glDeviceStartupParams m_startupParams;
	f32 m_monitorAspectRatio;

	glCtx m_ctx;
	glCaps m_caps;

	u32 m_textureMemory;

	ueBool m_fakeDownsizeTextures2x;

	glDevice() :
		m_fakeDownsizeTextures2x(UE_FALSE)
	{}
};

// Texture buffer

struct glTextureBuffer : public glTextureBuffer_Base, ueList<glTextureBuffer>::Node
{
   	glTextureBufferDesc m_desc;
	CIwTexture m_handle;

	u32 m_gpuDataSize;

	CIwMaterial m_material;
};

// Shaders

struct glShader; // UNUSED
struct glProgram; // UNUSED

// Index & vertex buffer

struct glBuffer; // UNUSED
struct glIndexBuffer; // UNUSED
struct glVertexBuffer; // UNUSED

// Queries

struct glQuery; // UNUSED

// Render group

struct glRenderGroup : glRenderGroup_Base
{
};

#endif // GL_STRUCTS_PRIVATE_MARMALADE_H

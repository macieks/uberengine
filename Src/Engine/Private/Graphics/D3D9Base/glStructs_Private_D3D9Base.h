#pragma once

#include <d3dx9.h>
#include "Graphics/D3D9Base/glStructs_D3D9Base.h"
#include "Graphics/glStructs_Private.h"

struct glVertexDeclaration;

// Vertex declaration

struct glVertexDeclaration
{
	u32 m_refCount;
	u32 m_numFormats;
	glStreamFormat* m_formats[GL_MAX_BOUND_VERTEX_STREAMS];

	IDirect3DVertexDeclaration9* m_handle;

	struct HashPred
	{
		UE_INLINE u32 operator () (const glVertexDeclaration* decl) const
		{
			return ueCalcHashMemberRange(decl->m_numFormats, decl->m_formats[decl->m_numFormats - 1]);
		}
	};

	struct CmpPred
	{
		UE_INLINE s32 operator () (const glVertexDeclaration* a, const glVertexDeclaration* b) const
		{
			if (a->m_numFormats != b->m_numFormats)
				return (s32) (b->m_numFormats - a->m_numFormats);
			return ueCmpMemberRange(
				a->m_numFormats, a->m_formats[a->m_numFormats - 1],
				b->m_numFormats, b->m_formats[b->m_numFormats - 1]);
		}
	};
};

// Device

struct glDevice_D3D9Base : public glDevice_Base
{
	IDirect3D9* m_d3d9;
	IDirect3DDevice9* m_d3dev;

	ueGenericPool m_vertexDeclsPool;
	ueHashSet<glVertexDeclaration*, glVertexDeclaration::HashPred, glVertexDeclaration::CmpPred> m_vertexDecls;
	ueGenericPool m_vertexShaderDeclsPool;
	ueHashSet<glVertexDeclarationEntry*, glVertexDeclarationEntry::HashPred, glVertexDeclarationEntry::CmpPred> m_vertexShaderDecls;

	ueGenericPool m_programsPool;
};

// Context

struct glCtx_D3D9Base : public glCtx_Base
{
	struct State
	{
		ueBool m_isUsed;
		DWORD m_value;

		UE_INLINE State() : m_isUsed(UE_FALSE) {}

		UE_INLINE ueBool Set(DWORD value)
		{
			UE_ASSERT(m_isUsed);
			if (m_value == value) return UE_FALSE;
			m_value = value;
			return UE_TRUE;
		}
	};

	struct SamplerState
	{
		State* m_states;

		UE_INLINE SamplerState() {}
	};

	glVertexDeclaration* m_vertexDeclaration;

	u32 m_numSamplers[glShaderType_MAX];
	glTextureBuffer** m_samplers[glShaderType_MAX];

	u32 m_maxSamplerRenderStates;
	SamplerState* m_samplerStates[glShaderType_MAX];		//!< State cache for each D3DSAMPLERSTATETYPE for each sampler

	u32 m_maxRenderStates;
	State* m_renderStates;				//!< State cache for each D3DRENDERSTATETYPE

	IDirect3DDevice9* m_handle;
};

// Texture buffer

struct glTextureBuffer_D3D9Base : public glTextureBuffer_Base
{
	glTextureBufferDesc m_desc;
	IDirect3DBaseTexture9* m_textureHandle;
	IDirect3DSurface9* m_surfaceHandle;
};

// Program

struct glProgram : glProgram_Base
{
};
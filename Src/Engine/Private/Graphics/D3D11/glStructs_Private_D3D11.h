#pragma once

#include "Base/ueHash.h"
#include "Base/ueCmp.h"
#include "Base/Containers/ueGenericPool.h"
#include "Base/Containers/ueHashMap.h"
#include "Graphics/glStructs_Private.h"

struct glVertexDeclaration;

// Vertex declaration

struct glVertexDeclaration
{
	u32 m_refCount;
	u32 m_numFormats;
	glStreamFormat* m_formats[GL_MAX_BOUND_VERTEX_STREAMS];

	ID3D11InputLayout* m_handle;

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

// Program

struct glProgram : glProgram_Base
{
};

// Device

struct glDevice : public glDevice_Base
{
	IDXGIFactory1* m_dxgiFactory;
	u32 m_numAdapters;
	f32 m_monitorAspectRatio;

	ID3D11Device* m_d3dev;
	D3D_FEATURE_LEVEL m_featureLevel;

	ID3D11DeviceContext* m_d3dCtx;

	IDXGISwapChain* m_swapChain;

	ueGenericPool m_vertexDeclsPool;
	ueHashSet<glVertexDeclaration*, glVertexDeclaration::HashPred, glVertexDeclaration::CmpPred> m_vertexDecls;
	ueGenericPool m_vertexShaderDeclsPool;
	ueHashSet<glVertexDeclarationEntry*, glVertexDeclarationEntry::HashPred, glVertexDeclarationEntry::CmpPred> m_vertexShaderDecls;

	ueHashMap<D3D11_DEPTH_STENCIL_DESC, ID3D11DepthStencilState*> m_depthStencilStateCache;
	ueHashMap<D3D11_RASTERIZER_DESC, ID3D11RasterizerState*> m_rasterStateCache;
	ueHashMap<D3D11_BLEND_DESC, ID3D11BlendState*> m_blendStateCache;
	ueHashMap<D3D11_SAMPLER_DESC, ID3D11SamplerState*> m_samplerStateCache;

	ueGenericPool m_programsPool;

	glDeviceStartupParams m_params;
	glCaps m_caps;
};

// Context

struct glCtx : public glCtx_Base
{
	glVertexDeclaration* m_vertexDeclaration;

	ueBool m_isDepthStencilStateDirty;
	CD3D11_DEPTH_STENCIL_DESC m_depthStencilStateDesc;
	u32 m_stencilRef;

	ueBool m_isRasterStateDirty;
	CD3D11_RASTERIZER_DESC m_rasterStateDesc;

	ueBool m_isBlendStateDirty;
	CD3D11_BLEND_DESC m_blendStateDesc;
	FLOAT m_blendFactor[4];

	u32 m_numSamplers[glShaderType_MAX];
	glTextureBuffer** m_samplers[glShaderType_MAX];
	CD3D11_SAMPLER_DESC* m_samplerStateDescs[glShaderType_MAX];

	struct D3D11SamplerStateArray
	{
		u32 m_minDirtyShaderView;
		u32 m_maxDirtyShaderView;

		ID3D11SamplerState** m_states;
		u32 m_minDirtyState;
		u32 m_maxDirtyState;
	};

	D3D11SamplerStateArray m_samplerStateArrays[glShaderType_MAX];

	ID3D11DeviceContext* m_handle;
};

// Texture buffer

struct glTextureBuffer : public glTextureBuffer_Base
{
	glTextureBufferDesc m_desc;

	ID3D11Resource* m_textureHandle;

	ID3D11ShaderResourceView* m_shaderView;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11DepthStencilView* m_depthStencilView;
};

// Shaders

struct glShader : glShader_Base
{
	struct BufferData
	{
		u8* m_memory;
		u32 m_size;

		u32 m_minDirtyOffset;
		u32 m_maxDirtyOffset;
	};

	ID3D11DeviceChild* m_handle;

	const char* m_code;
	u32 m_codeSize;

	ID3DBlob* m_inputSignature;

	u32 m_numBuffers;
	BufferData* m_bufferDatas;
	ID3D11Buffer** m_buffers;

	glVertexDeclarationEntry* m_vertexDeclEntries;

	UE_INLINE ID3D11GeometryShader* GetGSHandle() const { return (ID3D11GeometryShader*) m_handle; }
	UE_INLINE ID3D11PixelShader* GetPSHandle() const { return (ID3D11PixelShader*) m_handle; }
	UE_INLINE ID3D11VertexShader* GetVSHandle() const { return (ID3D11VertexShader*) m_handle; }
};

// Index and vertex buffers

struct glBuffer
{
	ID3D11Buffer* m_handle;

	ueBool m_isLocked;
	u32 m_lockFlags;
	void* m_scratchPad;
	u32 m_scratchPadOffset;
	u32 m_scratchPadSize;
};

struct glIndexBuffer : glBuffer
{
	glIndexBufferDesc m_desc;
};

struct glVertexBuffer : glBuffer
{
	glVertexBufferDesc m_desc;
};

// Render group

struct glRenderGroup : public glRenderGroup_Base
{
};

// Query

struct glOcclusionQuery
{
	ID3D11Query* m_handle;
	ID3D11Predicate* m_predicateHandle;
};
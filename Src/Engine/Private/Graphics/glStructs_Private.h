#ifndef GL_STRUCTS_PRIVATE_H
#define GL_STRUCTS_PRIVATE_H

#include "Graphics/glStructs.h"
#include "Base/Containers/ueHashSet.h"

struct glConstant;
struct glShaderConstantDesc;

//! Vertex stream format
struct glStreamFormat
{
	u32 m_refCount;		//!< Reference count

	u32 m_stride;		//!< Stride in bytes
	u32 m_numElements;	//!< Number of elements
	glVertexElement m_elements[GL_MAX_VERTEX_STREAM_ELEMENTS];	//!< Elements

	struct HashPred
	{
		UE_INLINE u32 operator () (const glStreamFormat* sf) const
		{
			return ueCalcHashMemberRange(sf->m_stride, sf->m_elements[sf->m_numElements - 1]);
		}
	};

	struct CmpPred
	{
		UE_INLINE s32 operator () (const glStreamFormat* a, const glStreamFormat* b) const
		{
			if (a->m_numElements != b->m_numElements)
				return (s32) (b->m_numElements - a->m_numElements);
			return ueCmpMemberRange(
				a->m_stride, a->m_elements[a->m_numElements - 1],
				b->m_stride, b->m_elements[b->m_numElements - 1]);
		}
	};
};

struct glVertexDeclarationEntry;

//! Vertex shader input
struct glVertexShaderInput
{
	u32 m_refCount;	//!< Reference count

	u32 m_numAttrs;	//!< Number of input vertex shader attributes
	glShaderAttr m_attrs[GL_MAX_VERTEX_SHADER_INPUT_ATTRIBUTES];	//!< Vertex shader input attributes

#if defined(GL_D3D9) || defined(GL_X360)
	glVertexDeclarationEntry* m_entries;	//!< Associated vertex declarations
#endif

	struct HashPred
	{
		UE_INLINE u32 operator () (const glVertexShaderInput* vsi) const
		{
			return ueCalcHashMemberRange(vsi->m_numAttrs, vsi->m_attrs[vsi->m_numAttrs - 1]);
		}
	};

	struct CmpPred
	{
		UE_INLINE s32 operator () (const glVertexShaderInput* a, const glVertexShaderInput* b) const
		{
			return ueCmpMemberRange(
				a->m_numAttrs, a->m_attrs[a->m_numAttrs - 1],
				b->m_numAttrs, b->m_attrs[b->m_numAttrs - 1]);
		}
	};
};

#if defined(GL_D3D9) || defined(GL_D3D11) || defined(GL_X360)

struct glVertexDeclaration;

struct glVertexDeclarationEntry
{
#if defined(GL_D3D11)
	glShader* m_vs;
#elif defined(GL_D3D9) || defined(GL_OPENGL)
	glVertexShaderInput* m_vsi;
#endif
	u32 m_numFormats;
	glStreamFormat* m_formats[GL_MAX_BOUND_VERTEX_STREAMS];

	glVertexDeclaration* m_decl;

	glVertexDeclarationEntry* m_next;

	struct HashPred
	{
		UE_INLINE u32 operator () (const glVertexDeclarationEntry* entry) const
		{
#if defined(GL_D3D11)
			return ueCalcHashMemberRange(entry->m_vs, entry->m_formats[entry->m_numFormats - 1]);
#elif defined(GL_D3D9) || defined(GL_OPENGL)
			return ueCalcHashMemberRange(entry->m_vsi, entry->m_formats[entry->m_numFormats - 1]);
#endif
		}
	};

	struct CmpPred
	{
		UE_INLINE s32 operator () (const glVertexDeclarationEntry* a, const glVertexDeclarationEntry* b) const
		{
#if defined(GL_D3D11)
			return ueCmpMemberRange(
				a->m_vs, a->m_formats[a->m_numFormats - 1],
				b->m_vs, b->m_formats[b->m_numFormats - 1]);
#elif defined(GL_D3D9) || defined(GL_OPENGL)
			return ueCmpMemberRange(
				a->m_vsi, a->m_formats[a->m_numFormats - 1],
				b->m_vsi, b->m_formats[b->m_numFormats - 1]);
#endif
		}
	};
};

#endif

// Device

struct glDevice_Base
{
	ueAllocator* m_stackAllocator;
	ueAllocator* m_freqAllocator;

	glCtx* m_defCtx;

	glTextureBuffer* m_backBuffer;
	glTextureBuffer* m_depthStencilBuffer;
	glRenderGroup* m_mainRenderGroup;

	ueGenericPool m_contextsPool;
	ueGenericPool m_renderBufferGroupsPool;
	ueGenericPool m_textureBuffersPool;
	ueGenericPool m_queriesPool;
	ueGenericPool m_vertexBuffersPool;
	ueGenericPool m_indexBuffersPool;
	ueGenericPool m_streamFormatsPool;
	ueHashSet<glStreamFormat*, glStreamFormat::HashPred, glStreamFormat::CmpPred> m_streamFormats;
	ueGenericPool m_vertexShaderInputsPool;
	ueHashSet<glVertexShaderInput*, glVertexShaderInput::HashPred, glVertexShaderInput::CmpPred> m_vertexShaderInputs;
};

// Context

struct glCtx_Base
{
	struct VertexStreamInfo
	{
		glVertexBuffer* m_VB;	//!< Vertex buffer
		u32 m_offset;			//!< Start offset in bytes
		u32 m_divider;			//!< Divider (used for geometry instancing)
	};

	struct SamplerConstantInfo
	{
		glTextureBuffer* m_textureBuffer;
		glSamplerParams m_samplerParams;
	};

	struct ShaderInfo
	{
		glShader* m_shader;
		ueBool m_isDirty;

#if defined(GL_D3D9) || defined(GL_OPENGL)
		u8* m_data;
		u32 m_dataSize;
#endif
	};

	// Clear values

	f32 m_clearColorRed;
	f32 m_clearColorGreen;
	f32 m_clearColorBlue;
	f32 m_clearColorAlpha;

	f32 m_clearDepth;
	u32 m_clearStencil;

	// Vertex streams / vertex format

	VertexStreamInfo m_vertexStreamInfos[GL_MAX_BOUND_VERTEX_STREAMS];
	glStreamFormat* m_vertexStreamFormats[GL_MAX_BOUND_VERTEX_STREAMS];
	ueBool m_vertexDeclDirty;

	// Index buffer

	glIndexBuffer* m_indexBuffer;

	// Render target / frame buffer

	glRenderGroup* m_renderGroup; // Currently set render group
	glRenderGroup* m_mainRenderGroup;

	// Program, shaders & shader constants

	glProgram* m_program;
	ShaderInfo m_shaderInfos[glShaderType_MAX];

	u8* m_numericShaderConstantsLocal;
	SamplerConstantInfo* m_samplerShaderConstantsLocal;
};

// Texture buffer

struct glTextureBuffer_Base
{
	ueBool m_isMainFrameBuffer;
#ifndef UE_FINAL
	char m_debugName[GL_MAX_TEXTURE_BUFFER_DEBUG_NAME];
#endif

	glTextureBuffer_Base() :
		m_isMainFrameBuffer(UE_FALSE)
	{
#ifndef UE_FINAL
		m_debugName[0] = 0;
#endif
	}
};

// Program

struct glProgram_Base
{
	glShader* m_shaders[glShaderType_MAX];
};

// Shaders

struct glShaderConstantInstance
{
	const glConstant* m_constant;	//!< Pointer to the actual shader constant
	u32 m_offset;						//!< Offset into registers or sampler index (depending on a type)
	ueBool m_isColumnMajor;				//!< Indicates whether the parameter is column major (otherwise row major); valid for matrices only
};

struct glShader_Base
{
	glShaderType m_shaderType;
	u32 m_numConstants;
	glShaderConstantInstance* m_constants;
	u32 m_numOutputs;						//!< Number of shader outputs (used for debugging purposes - to match render target count)
	glVertexShaderInput* m_input;			//!< Input attributes (used only for vertex shader)
	u32 m_numProgramRefs;
};

// Shader constant

struct glConstant
{
	glShaderConstantDesc m_desc;

    struct Numeric
    {
        u32 m_size;
        u32 m_cacheOffset; //!< An offset into per-context cache
    };

    struct Sampler
    {
        u32 m_index;
    };

	union
	{
		Numeric m_numeric;
		Sampler m_sampler;
	};
};

// Render buffer group

enum glRenderGroupState
{
	glRenderGroupState_Initial = 0,
	glRenderGroupState_Begun,
	glRenderGroupState_Resolved,

	glRenderGroupState_MAX
};

struct glRenderGroup_Base
{
	glRenderGroupState m_state;	//!< Current group state

	glCtx* m_currentContext;	//!< Current render context or NULL if none

	glRenderGroupDesc m_desc;	//!< Group description

	// Render targets

	glTextureBuffer* m_depthStencilRenderTarget;
	glTextureBuffer* m_colorRenderTargets[GL_MAX_COLOR_RENDER_TARGETS];

	// Resolved textures (possibly same as render targets)

	glTextureBuffer* m_depthStencilTexture;
	glTextureBuffer* m_colorTextures[GL_MAX_COLOR_RENDER_TARGETS];
};

#endif // GL_STRUCTS_PRIVATE_H

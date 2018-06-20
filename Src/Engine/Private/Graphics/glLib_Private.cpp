#include "Graphics/glLib_Private.h"
#include "IO/ueBuffer.h"

static glDevice* s_dev = NULL;

// glCaps

glCaps_Base::FormatInfo::FormatInfo() :
	m_supportedAsRTOnly(UE_FALSE),
	m_supportedAsTextureOnly(UE_FALSE),
	m_supportedAsRTAndTexture(UE_FALSE),
	m_supportsBlending(UE_FALSE),
	m_supportsFiltering(UE_FALSE),
	m_supportsAutoMips(UE_FALSE)
{}

glCaps_Base::glCaps_Base() :
	m_supportsOcclusionQuery(UE_FALSE),
	m_supportsOcclusionQueryAnySample(UE_FALSE),
	m_supportsOcclusionPredicateQuery(UE_FALSE),
	m_supportsIndependentColorWriteMasks(UE_FALSE),
	m_supportsTwoSidedStencil(UE_FALSE),

	m_hasHalfPixelOffset(UE_FALSE),

	m_maxVertexStreams(0),
	m_maxColorRenderTargets(0),

	m_numMSAALevels(0),
	m_maxAnisotropicFilteringlevel(0),

	m_maxPrimitives(0),
	m_maxUserClippingPlanes(0)
{
	for (u32 i = 0; i < glShaderType_MAX; i++)
	{
		m_maxSamplers[i] = 0;
		m_maxShaderRegisters[i] = 0;
	}
}

#if !defined(GL_MARMALADE)

// glDevice

void glDevice_GetClosestSupportedMode(glDeviceStartupParams* params)
{
	f32 bestDiff = F32_MAX;
	u32 bestWidth = 0xFFFFFFFF;
	u32 bestHeight = 0xFFFFFFFF;
	u32 bestFreq = 0xFFFFFFFF;

	glAdapterInfo info;
	u32 numModes = glUtils_GetAdapterInfo(params->m_adapterIndex, &info);
	for (u32 i = 0; i < info.m_numModes; i++)
	{
		glOutputMode mode;
		glUtils_GetOutputMode(params->m_adapterIndex, i, &mode);

		if (mode.m_bitsPerPixel != 32)
			continue;

		if (mode.m_width < params->m_width || mode.m_height < params->m_height || mode.m_frequency < params->m_frequency)
			continue;

		const f32 diff = ueAbs((f32) mode.m_width / (f32) mode.m_height - (f32) params->m_width / (f32) params->m_height);
		if (diff < bestDiff)
		{
			bestDiff = diff;
			bestWidth = mode.m_width;
			bestHeight = mode.m_height;
			bestFreq = mode.m_frequency;
		}
	}

	if (bestDiff != F32_MAX)
	{
		params->m_width = bestWidth;
		params->m_height = bestHeight;
		params->m_frequency = bestFreq;
	}
}

#endif // !defined(GL_MARMALADE)

ueBool glDevice_IsSet()
{
	return s_dev != NULL;
}

glDevice* glDevice_Get()
{
	UE_ASSERT(s_dev);
	return s_dev;
}

void glDevice_Set(glDevice* dev)
{
	s_dev = dev;
}

ueAllocator* glDevice_GetFreqAllocator()
{
	return s_dev->m_freqAllocator;
}

ueAllocator* glDevice_GetStackAllocator()
{
	return s_dev->m_stackAllocator;
}

const glCaps* glDevice_GetCaps()
{
	return &s_dev->m_caps;
}

const glDeviceStartupParams* glDevice_GetStartupParams()
{
	return &s_dev->m_params;
}

glCtx* glDevice_GetDefaultContext()
{
	return s_dev->m_defCtx;
}

glTextureBuffer* glDevice_GetBackBuffer()
{
	return s_dev->m_backBuffer;
}

glTextureBuffer* glDevice_GetDepthStencilBuffer()
{
	return s_dev->m_depthStencilBuffer;
}

void glDevice_Base_Startup(const glDeviceStartupParams_Base* params)
{
	GLDEV->m_stackAllocator = params->m_stackAllocator;
	GLDEV->m_freqAllocator = params->m_freqAllocator;

#if defined(GL_D3D9) || !defined(UE_FINAL) // D3D9: Need to be able to release/recreate resources on device reset
	const ueBool needResourceIterator = UE_TRUE;
#else
	const ueBool needResourceIterator = UE_FALSE;
#endif

	UE_ASSERT_FUNC(GLDEV->m_contextsPool.Init(GL_STACK_ALLOC, sizeof(glCtx), params->m_maxContexts));
	UE_ASSERT_FUNC(GLDEV->m_renderBufferGroupsPool.Init(GL_STACK_ALLOC, sizeof(glRenderGroup), params->m_maxRenderGroups));
	UE_ASSERT_FUNC(GLDEV->m_textureBuffersPool.Init(GL_STACK_ALLOC, sizeof(glTextureBuffer), params->m_maxTextureBuffers, UE_DEFAULT_ALIGNMENT, needResourceIterator));
#if !defined(GL_MARMALADE)
	UE_ASSERT_FUNC(GLDEV->m_queriesPool.Init(GL_STACK_ALLOC, sizeof(glOcclusionQuery), params->m_maxOcclusionQueries, UE_DEFAULT_ALIGNMENT, UE_TRUE));
	UE_ASSERT_FUNC(GLDEV->m_vertexBuffersPool.Init(GL_STACK_ALLOC, sizeof(glVertexBuffer), params->m_maxVertexBuffers, UE_DEFAULT_ALIGNMENT, needResourceIterator));
	UE_ASSERT_FUNC(GLDEV->m_indexBuffersPool.Init(GL_STACK_ALLOC, sizeof(glIndexBuffer), params->m_maxIndexBuffers, UE_DEFAULT_ALIGNMENT, needResourceIterator));
	UE_ASSERT_FUNC(GLDEV->m_streamFormatsPool.Init(GL_STACK_ALLOC, sizeof(glStreamFormat), params->m_maxStreamFormats));
	UE_ASSERT_FUNC(GLDEV->m_streamFormats.Init(GL_STACK_ALLOC, params->m_maxStreamFormats));
	UE_ASSERT_FUNC(GLDEV->m_vertexShaderInputsPool.Init(GL_STACK_ALLOC, sizeof(glVertexShaderInput), params->m_maxVertexShaderInputs));
	UE_ASSERT_FUNC(GLDEV->m_vertexShaderInputs.Init(GL_STACK_ALLOC, params->m_maxVertexShaderInputs));
#endif
}

void glDevice_Base_Shutdown()
{
#if !defined(GL_MARMALADE)

	UE_ASSERT_MSGP(GLDEV->m_vertexShaderInputsPool.Size() == 0, "Unfreed glVertexShaderInput instances (count = %u). Make sure all glShader instances have been freed.", GLDEV->m_vertexShaderInputsPool.Size());
	GLDEV->m_vertexShaderInputs.Deinit();
	GLDEV->m_vertexShaderInputsPool.Deinit();

	UE_ASSERT_MSGP(GLDEV->m_streamFormatsPool.Size() == 0, "Unfreed glStreamFormat instances (count = %u).", GLDEV->m_streamFormatsPool.Size());
	GLDEV->m_streamFormats.Deinit();
	GLDEV->m_streamFormatsPool.Deinit();

	UE_ASSERT_MSGP(GLDEV->m_indexBuffersPool.Size() == 0, "Unfreed glVertexBuffer instances (count = %u).", GLDEV->m_indexBuffersPool.Size());
	GLDEV->m_indexBuffersPool.Deinit();

	UE_ASSERT_MSGP(GLDEV->m_vertexBuffersPool.Size() == 0, "Unfreed glVertexBuffer instances (count = %u).", GLDEV->m_vertexBuffersPool.Size());
	GLDEV->m_vertexBuffersPool.Deinit();

	UE_ASSERT_MSGP(GLDEV->m_queriesPool.Size() == 0, "Unfreed glOcclusionQuery instances (count = %u).", GLDEV->m_queriesPool.Size());
	GLDEV->m_queriesPool.Deinit();

#endif // !defined(GL_MARMALADE)

	UE_ASSERT_MSGP(GLDEV->m_textureBuffersPool.Size() == 0, "Unfreed glTextureBuffer instances (count = %u).", GLDEV->m_textureBuffersPool.Size());
	GLDEV->m_textureBuffersPool.Deinit();

	UE_ASSERT_MSGP(GLDEV->m_renderBufferGroupsPool.Size() == 0, "Unfreed glRenderGroup instances (count = %u).", GLDEV->m_renderBufferGroupsPool.Size());
	GLDEV->m_renderBufferGroupsPool.Deinit();

	UE_ASSERT_MSGP(GLDEV->m_contextsPool.Size() == 0, "Unfreed glRenderGroup instances (count = %u).", GLDEV->m_contextsPool.Size());
	GLDEV->m_contextsPool.Deinit();
}

u32 glDevice_GetNumTextureBuffers()
{
	return GLDEV->m_textureBuffersPool.Size();
}

void glDevice_GetTextureBuffers(glTextureBuffer** array, u32 capacity, u32 textureBufferFlags, u32& count)
{
	ueGenericPool::Iterator i(GLDEV->m_textureBuffersPool);
	count = 0;
	while (glTextureBuffer* tb = (glTextureBuffer*) i.Next())
	{
		if (count == capacity)
			break;
		if (!((tb->m_desc.m_flags & textureBufferFlags) == textureBufferFlags))
			continue;

		array[count++] = tb;
	}
}

// glCtx_Base

void glCtx_Base_Init(glCtx* ctx)
{
	const glCaps* caps = glDevice_GetCaps();
	const glDeviceStartupParams* params = glDevice_GetStartupParams();

#if defined(GL_D3D9) || defined(GL_OPENGL)
	for (u32 i = 0; i < glShaderType_MAX; i++)
	{
		const u32 dataSize = caps->m_maxShaderRegisters[i] << 4;
		ctx->m_shaderInfos[i].m_dataSize = dataSize;
		if (dataSize > 0)
		{
			ctx->m_shaderInfos[i].m_data = new(GL_STACK_ALLOC) u8[dataSize];
			UE_ASSERT(ctx->m_shaderInfos[i].m_data);
		}
	}
#endif

	ctx->m_renderGroup = NULL;

#if !defined(GL_FIXED_PIPELINE)
	ctx->m_samplerShaderConstantsLocal = new(GL_STACK_ALLOC) glCtx_Base::SamplerConstantInfo[params->m_maxGlobalSamplerConstants];
	UE_ASSERT(ctx->m_samplerShaderConstantsLocal);
	ctx->m_numericShaderConstantsLocal = (u8*) GL_STACK_ALLOC->Alloc(params->m_maxGlobalNumericConstantsDataSize);
	UE_ASSERT(ctx->m_numericShaderConstantsLocal);
#endif // !defined(GL_FIXED_PIPELINE)
}

ueBool glCtx_IsRenderingToTexture(glCtx* ctx)
{
	return glCtx_GetCurrentRenderGroup(ctx) != glDevice_GetMainRenderGroup();
}

void glCtx_Base_Deinit(glCtx* ctx)
{
#if !defined(GL_FIXED_PIPELINE)
	GL_STACK_ALLOC->Free(ctx->m_numericShaderConstantsLocal);
	GL_STACK_ALLOC->Free(ctx->m_samplerShaderConstantsLocal);
#endif // !defined(GL_FIXED_PIPELINE)

#if defined(GL_D3D9) || defined(GL_OPENGL)
	for (s32 i = glShaderType_MAX - 1; i >= 0; i--)
		if (ctx->m_shaderInfos[i].m_dataSize > 0)
			GL_STACK_ALLOC->Free(ctx->m_shaderInfos[i].m_data);
#endif
}

#if !defined(GL_D3D11) && !defined(GL_MARMALADE)

void glCtx_Base_Reset(glCtx* ctx, u32 flags)
{
	const glDeviceStartupParams* params = glDevice_GetStartupParams();

	if (flags & glResetFlags_Streams)
	{
		for (u32 i = 0; i < GL_MAX_BOUND_VERTEX_STREAMS; i++)
			glCtx_SetStream(ctx, i, NULL, NULL);
		glCtx_SetIndices(ctx, NULL);
	}

	if (flags & glResetFlags_RenderTargets)
		glCtx_SetRenderGroup(ctx, NULL);

	if (flags & glResetFlags_Samplers)
	{
#if defined(GL_MARMALADE)
		// Do nothing
#elif defined(GL_OPENGL)
		for (u32 i = 0; i < ctx->m_numSamplers; i++)
			glCtx_SetTexture_Immediate(ctx, glShaderType_Fragment /* anything here */, i, NULL);
#else
		for (u32 i = 0; i < glShaderType_MAX; i++)
			for (u32 j = 0; j < ctx->m_numSamplers[i]; j++)
				glCtx_SetTexture_Immediate(ctx, (glShaderType) i, j, NULL);
#endif

#if !defined(GL_FIXED_PIPELINE)
		for (u32 j = 0; j < params->m_maxGlobalSamplerConstants; j++)
			ctx->m_samplerShaderConstantsLocal[j].m_textureBuffer = NULL;
#endif
	}

#if !defined(GL_FIXED_PIPELINE)

	if (flags & glResetFlags_Shaders)
	{
		glCtx_SetProgram(ctx, NULL);

#if defined(GL_D3D9) || defined(GL_OPENGL)
#if defined(UE_DEBUG)
		for (u32 i = 0; i < glShaderType_MAX; i++)
			if (ctx->m_shaderInfos[i].m_dataSize > 0)
				ueMemZero(ctx->m_shaderInfos[i].m_data, ctx->m_shaderInfos[i].m_dataSize);
#endif
#endif
	}

#endif // !defined(GL_FIXED_PIPELINE)
}

#endif

glRenderGroup* glDevice_GetMainRenderGroup()
{
	return GLDEV->m_mainRenderGroup;
}

glRenderGroup* glCtx_GetCurrentRenderGroup(glCtx* ctx)
{
	return ctx->m_renderGroup;
}

#if !defined(GL_FIXED_PIPELINE)

void glCtx_SetSamplerConstant(glCtx* ctx, const glConstantHandle handle, glTextureBuffer* tb, const glSamplerParams* samplerParams)
{
	UE_ASSERT(handle.IsValid());
	UE_ASSERT_MSG(!tb || glTextureBuffer_IsTexture(tb), "Texture buffer to be set is not a texture.");
	const glConstant* constant = handle.GetConstant();
	glCtx_Base::SamplerConstantInfo* info = &ctx->m_samplerShaderConstantsLocal[constant->m_sampler.m_index];
	info->m_textureBuffer = tb;
	if (samplerParams)
		info->m_samplerParams = *samplerParams;
	else
		info->m_samplerParams.SetDefaults();
}

void glCtx_SetNumericConstant(glCtx* ctx, const glConstantHandle handle, glConstantType dataType, const void* srcData, u32 srcStride, u32 count, u32 dstIndexOffset)
{
	UE_ASSERT(handle.IsValid());
	const u32 elementSize = glUtils_GetShaderConstantSize(dataType);
	const glConstant* constant = handle.GetConstant();
	u8* dstData = (u8*) ctx->m_numericShaderConstantsLocal + constant->m_numeric.m_cacheOffset + dstIndexOffset * elementSize;
	if (srcStride == elementSize)
		ueMemCpy(dstData, srcData, count * srcStride);
	else
		for (u32 i = 0; i < count; i++)
		{
			ueMemCpy(dstData, srcData, elementSize);
			srcData = (u8*) srcData + srcStride;
		}
}

// glTextureBuffer

glTextureBuffer* glTextureBuffer_Create(const glTextureBufferDesc* desc, const glTextureInitData* initData)
{
	// Validate format support

	const ueBool isTexture = (desc->m_flags & glTextureBufferFlags_IsTexture) != 0;
	const ueBool isRT = (desc->m_flags & glTextureBufferFlags_IsRenderTarget) != 0;

	const glCaps* caps = glDevice_GetCaps();

	if (isTexture && isRT)
	{
		if (!caps->m_formatInfos[desc->m_format].m_supportedAsRTAndTexture)
		{
			ueLogE("Failed to create texture buffer, reason: unsupported texture and render target (at the same time) format '%s'", glBufferFormat_ToString(desc->m_format));
			return NULL;
		}
	}
	else if (isTexture)
	{
		if (!caps->m_formatInfos[desc->m_format].m_supportedAsTextureOnly)
		{
			ueLogE("Failed to create texture buffer, reason: unsupported texture format '%s'", glBufferFormat_ToString(desc->m_format));
			return NULL;
		}
	}
	else if (isRT) 
		if (!caps->m_formatInfos[desc->m_format].m_supportedAsRTOnly)
		{
			ueLogE("Failed to create texture buffer, reason: unsupported render target format '%s'", glBufferFormat_ToString(desc->m_format));
			return NULL;
		}

	if (isTexture && desc->m_enableAutoMips)
	{
		if (!caps->m_formatInfos[desc->m_format].m_supportsAutoMips)
		{
			ueLogE("Failed to create texture buffer, reason: format '%s' doesn't support automatic mipmap generation", glBufferFormat_ToString(desc->m_format));
			return NULL;
		}
	}

	glTextureBuffer* tb = new(GLDEV->m_textureBuffersPool) glTextureBuffer;
	if (!tb)
	{
		ueLogE("Failed to create texture buffer, reason: texture pool empty - increase pool size while creating device");
		return NULL;
	}
	tb->m_desc = *desc;
	tb->m_isMainFrameBuffer = UE_FALSE;

	if (tb->m_desc.m_numLevels == 0)
		tb->m_desc.m_numLevels = glUtils_CalcNumMipLevels(tb->m_desc.m_width, tb->m_desc.m_height, tb->m_desc.m_depth);

	glTextureBuffer_CreateInPlace(tb, initData);
	return tb;
}

void glTextureBuffer_Destroy(glTextureBuffer* tb)
{
	glTextureBuffer_DestroyInPlace(tb);
	GLDEV->m_textureBuffersPool.Free(tb);
}

#endif // !defined(GL_FIXED_PIPELINE)

const glTextureBufferDesc* glTextureBuffer_GetDesc(glTextureBuffer* tb)
{
	return &tb->m_desc;
}

ueBool glTextureBuffer_IsRenderTarget(glTextureBuffer* tb)
{
	return (tb->m_desc.m_flags & glTextureBufferFlags_IsRenderTarget) != 0;
}

ueBool glTextureBuffer_IsTexture(glTextureBuffer* tb)
{
	return (tb->m_desc.m_flags & glTextureBufferFlags_IsTexture) != 0;
}

#if !defined(GL_MARMALADE)

// glVertexBuffer

glVertexBuffer* glVertexBuffer_Create(const glVertexBufferDesc* desc, const void* initData)
{
	glVertexBuffer* vb = new(GLDEV->m_vertexBuffersPool) glVertexBuffer;
	UE_ASSERT(vb);
	vb->m_desc = *desc;
	glVertexBuffer_CreateInPlace(vb, initData);
	return vb;
}

void glVertexBuffer_Destroy(glVertexBuffer* vb)
{
	glVertexBuffer_DestroyInPlace(vb);
	GLDEV->m_vertexBuffersPool.Free(vb);
}

const glVertexBufferDesc* glVertexBuffer_GetDesc(glVertexBuffer* vb)
{
	return &vb->m_desc;
}

// glIndexBuffer

glIndexBuffer* glIndexBuffer_Create(const glIndexBufferDesc* desc, const void* initData)
{
	glIndexBuffer* ib = new(GLDEV->m_indexBuffersPool) glIndexBuffer;
	UE_ASSERT(ib);
	ib->m_desc = *desc;
	glIndexBuffer_CreateInPlace(ib, initData);
	return ib;
}

void glIndexBuffer_Destroy(glIndexBuffer* ib)
{
	glIndexBuffer_DestroyInPlace(ib);
	GLDEV->m_indexBuffersPool.Free(ib);
}

const glIndexBufferDesc* glIndexBuffer_GetDesc(glIndexBuffer* ib)
{
	return &ib->m_desc;
}

// glConstantHandle

ueBool glConstantHandle::Init(const char* name, glConstantType type, u32 count)
{
	glShaderConstantDesc desc;
	desc.m_name = name;
	desc.m_type = type;
	desc.m_count = count;
	m_constant = glShaderConstantMgr_GetConstant(&desc);
	return m_constant != NULL;
}

const char* glConstantHandle::GetName() const
{
	UE_ASSERT(IsValid());
	return m_constant->m_desc.m_name;
}

glConstantType glConstantHandle::GetType() const
{
	UE_ASSERT(IsValid());
	return (glConstantType) m_constant->m_desc.m_type;
}

u32 glConstantHandle::GetCount() const
{
	UE_ASSERT(IsValid());
	return m_constant->m_desc.m_count;
}

// glStreamFormat

glStreamFormat*	glStreamFormat_Create(const glStreamFormatDesc* desc)
{
	// Try to get existing one

	glStreamFormat temp;
	temp.m_stride = desc->m_stride;
	temp.m_numElements = desc->m_numElements;
	for (u32 i = 0; i < desc->m_numElements; i++)
		temp.m_elements[i] = desc->m_elements[i];

	glStreamFormat** sfPtr = s_dev->m_streamFormats.Find(&temp);
	if (sfPtr)
	{
		(*sfPtr)->m_refCount++;
		return *sfPtr;
	}

	// Validate for OpenGL usage

#if defined(GL_OPENGL)
	for (u32 i = 0; i < desc->m_numElements; i++)
	{
		const glVertexElement& elem = desc->m_elements[i];
		if (elem.m_type == ueNumType_F16 && !GLEW_ARB_half_float_vertex)
		{
			ueLogE("Failed to create vertex stream, reason: no support for half-floats (elem index = %u; semantic = %s; type = %s; count = %u; offset = %u)", i, ueEnumToString(glSemantic, (glSemantic) elem.m_semantic), ueEnumToString(ueNumType, (ueNumType) elem.m_type), (u32) elem.m_count, (u32) elem.m_offset);
			return NULL;
		}
	}
#endif

	// Create new stream format

	glStreamFormat* sf = new(s_dev->m_streamFormatsPool) glStreamFormat;
	UE_ASSERT(sf);
	sf->m_stride = desc->m_stride;
	sf->m_numElements = desc->m_numElements;
	for (u32 i = 0; i < desc->m_numElements; i++)
		sf->m_elements[i] = desc->m_elements[i];
	sf->m_refCount = 1;

	s_dev->m_streamFormats.Insert(sf);

	return sf;
}

void glStreamFormat_Destroy(glStreamFormat* sf)
{
	UE_ASSERT(sf->m_refCount > 0);
	if (--sf->m_refCount)
		return;
	s_dev->m_streamFormats.Remove(sf);
	s_dev->m_streamFormatsPool.Free(sf);
}

#endif // !defined(GL_MARMALADE)

// glSamplerParams

void glSamplerDesc_InitializeDefaults()
{
	glSamplerParams::DefaultClamp.m_addressU = glTexAddr_Clamp;
	glSamplerParams::DefaultClamp.m_addressV = glTexAddr_Clamp;
	glSamplerParams::DefaultClamp.m_addressW = glTexAddr_Clamp;

	glSamplerParams::DefaultMirror.m_addressU = glTexAddr_Mirror;
	glSamplerParams::DefaultMirror.m_addressV = glTexAddr_Mirror;
	glSamplerParams::DefaultMirror.m_addressW = glTexAddr_Mirror;

	// Disable filtering for post-processing
	glSamplerParams::DefaultPP.m_minFilter = glTexFilter_Nearest;
	glSamplerParams::DefaultPP.m_magFilter = glTexFilter_Nearest;
	glSamplerParams::DefaultPP.m_mipFilter = glTexFilter_None;
	glSamplerParams::DefaultPP.m_addressU = glTexAddr_Clamp;
	glSamplerParams::DefaultPP.m_addressV = glTexAddr_Clamp;
	glSamplerParams::DefaultPP.m_addressW = glTexAddr_Clamp;
}

#if !defined(GL_MARMALADE)

// glVertexShaderInput

glVertexShaderInput* glVertexShaderInput_Create(const glVertexShaderInputDesc* desc)
{
	// Try to get existing one

	glVertexShaderInput temp;
	temp.m_numAttrs = desc->m_numAttrs;
	ueMemCpy(temp.m_attrs, desc->m_attrs, desc->m_numAttrs * sizeof(glShaderAttr));

	glVertexShaderInput** vsiPtr = GLDEV->m_vertexShaderInputs.Find(&temp);
	if (vsiPtr)
	{
		(*vsiPtr)->m_refCount++;
		return *vsiPtr;
	}

	// Not found - create new one

	glVertexShaderInput* vsi = new(GLDEV->m_vertexShaderInputsPool) glVertexShaderInput;
	UE_ASSERT(vsi);
	vsi->m_numAttrs = desc->m_numAttrs;
	ueMemCpy(vsi->m_attrs, desc->m_attrs, desc->m_numAttrs * sizeof(glShaderAttr));
	vsi->m_refCount = 1;

#if defined(GL_D3D9) || defined(GL_X360)
	vsi->m_entries = NULL;
#endif

	GLDEV->m_vertexShaderInputs.Insert(vsi);

	return vsi;
}

void glVertexShaderInput_Destroy(glVertexShaderInput* vsi)
{
	UE_ASSERT(vsi->m_refCount > 0);

	if (--vsi->m_refCount)
		return;

#if defined(GL_D3D9) || defined(GL_X360)

	glVertexDeclarationEntry* entry = vsi->m_entries;
	while (entry)
	{
		glVertexDeclarationEntry* nextEntry = entry->m_next;

		glVertexDeclaration_Destroy(entry->m_decl);

		for (u32 i = 0; i < entry->m_numFormats; i++)
			if (entry->m_formats[i])
				glStreamFormat_Destroy(entry->m_formats[i]);
		GLDEV->m_vertexShaderDecls.Remove(entry);
		GLDEV->m_vertexShaderDeclsPool.Free(entry);

		entry = nextEntry;
	}

#endif // defined(GL_D3D9) || defined(GL_X360)

	GLDEV->m_vertexShaderInputs.Remove(vsi);
	GLDEV->m_vertexShaderInputsPool.Free(vsi);
}

#endif // !defined(GL_MARMALADE)

glTextureBuffer* glTextureBuffer_Create(const glTextureFileDesc* fileDesc)
{
	glTextureBufferDesc desc;
	desc.m_type = (glTexType) fileDesc->m_type;
	desc.m_format = (glBufferFormat) fileDesc->m_format;
	desc.m_width = fileDesc->m_width;
	desc.m_height = fileDesc->m_height;
	desc.m_depth = fileDesc->m_depth;
	desc.m_numLevels = fileDesc->m_numLevels;
	desc.m_enableAutoMips = fileDesc->m_enableAutoMips;
	desc.m_flags |= fileDesc->m_isDynamic ? glTextureBufferFlags_IsDynamic : 0;

	glTextureInitData initData;
	initData.m_storage = (glTexStorage) fileDesc->m_storage;
	initData.m_dataArrayLength = fileDesc->m_dataArrayLength;
	initData.m_dataArray = fileDesc->m_dataArray;

	return glTextureBuffer_Create(&desc, &initData);
}

void glTextureBuffer_SetDebugName(glTextureBuffer* tb, const char* name)
{
#ifndef UE_FINAL
	ueStrCpyS(tb->m_debugName, name);
#endif
}

const char* glTextureBuffer_GetDebugName(const glTextureBuffer* tb)
{
#ifdef UE_FINAL
	return NULL;
#else
	return tb->m_debugName;
#endif
}

f32 glDevice_GetBufferAspectRatio()
{
	const glDeviceStartupParams* params = glDevice_GetStartupParams();
	return (f32) params->m_width / (f32) params->m_height;
}

u32 glStreamFormat_GetStride(glStreamFormat* sf)
{
	return sf->m_stride;
}

const glVertexElement* glStreamFormat_GetElement(glStreamFormat* sf, glSemantic semantic, u32 usageIndex)
{
	for (u32 i = 0; i < sf->m_numElements; i++)
	{
		const glVertexElement& e = sf->m_elements[i];
		if (e.m_semantic == semantic && e.m_usageIndex == usageIndex)
			return &e;
	}
	return NULL;
}

void glVertexElement_GetDataAsF32(const glVertexElement* element, f32* dst, u32 dstCount, const void* srcData)
{
	ueNumType_ToF32((ueNumType) element->m_type, (u32) element->m_count, element->m_isNormalized ? UE_TRUE : UE_FALSE, srcData, dstCount, dst);
}

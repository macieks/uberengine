#include "Base/ueBase.h"

#if defined(GL_D3D11)

#include "Utils/ueWindow.h"
#include "Graphics/glLib_Private.h"

static glDevice s_dev;

glDeviceStartupParams::glDeviceStartupParams() :
	m_maxVertexDecls(256),
	m_maxVertexShaderDecls(1024),
	m_fullscreen(UE_FALSE),
	m_hardwareVertexProcessing(UE_TRUE),
	m_multithreaded(UE_FALSE),
	m_window(NULL),
	m_adapterIndex(0),
	m_driverType(D3D_DRIVER_TYPE_HARDWARE),
	m_maxSamplerStates(512),
	m_maxBlendStates(128),
	m_maxRasterStates(128),
	m_maxDepthStencilStates(128),
	m_MSAAQuality(0)
{}

ID3D11Device* glDevice_GetD3Dev()
{
	return GLDEV->m_d3dev;
}

ID3D11DeviceContext* glDevice_GetCtx()
{
	return GLDEV->m_d3dCtx;
}

void glDevice_PrintStats()
{
	ueLogI("Display info (D3D11):");

	IDXGIAdapter1* adapter;
	UE_ASSERT_FUNC(s_dev.m_dxgiFactory->EnumAdapters1(s_dev.m_params.m_adapterIndex, &adapter) == S_OK);

	DXGI_ADAPTER_DESC1 desc;
	UE_ASSERTWinCall(adapter->GetDesc1(&desc));

	adapter->Release();
	adapter = NULL;

	ueLogI("\tAdapter: %S", desc.Description);
	ueLogI("\tDriver version: %u.%u.%u.%u", desc.VendorId, desc.DeviceId, desc.SubSysId, desc.Revision);

	ueLogI("\tAvailable SYS / GPU / SHARED memory: %u / %u / %u MB", desc.DedicatedSystemMemory >> 20, desc.DedicatedVideoMemory >> 20, desc.SharedSystemMemory >> 20);

	ueLogI("\tPixel shader version: %u.%u", 0, 0);
	ueLogI("\tVertex shader version: %u.%u", 0, 0);
}

void glDevice_CalcMonitorAspectRatio(const glDeviceStartupParams* params)
{
	s_dev.m_monitorAspectRatio = 16.0f / 9.0f;

	u32 largestSize = 0;

	glAdapterInfo info;
	u32 numModes = glUtils_GetAdapterInfo(params->m_adapterIndex, &info);
	for (u32 i = 0; i < info.m_numModes; i++)
	{
		glOutputMode mode;
		glUtils_GetOutputMode(params->m_adapterIndex, i, &mode);

		const u32 size = mode.m_width * mode.m_height;
		if (size > largestSize)
		{
			largestSize = size;
			s_dev.m_monitorAspectRatio = (f32) mode.m_width / (f32) mode.m_height;
		}
	}
}

f32 glDevice_GetMonitorAspectRatio()
{
	return s_dev.m_monitorAspectRatio;
}

f32 glDevice_GetAspectRatio()
{
	return s_dev.m_params.m_fullscreen ? glDevice_GetMonitorAspectRatio() : glDevice_GetBufferAspectRatio();
}

ueBool glDevice_ChangeMode(const glDeviceStartupParams* params)
{
	if (!glDevice_Reset(params))
		return UE_FALSE;

	if (!ueWindow_ChangeMode(
		s_dev.m_params.m_window,
		s_dev.m_params.m_fullscreen,
		s_dev.m_params.m_width, s_dev.m_params.m_height,
		s_dev.m_params.m_frequency))
		return UE_FALSE;

	return UE_TRUE;
}

void glDevice_PreStartup()
{
	UE_ASSERT(!glDevice_IsSet());

	glDevice_Set(&s_dev);

	// Create DXGI factory object

	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**) &s_dev.m_dxgiFactory);
	if (FAILED(hr))
		ueFatalErrorP("Failed to create D3D11 device, reason: CreateDXGIFactory1 failed with hr = 0x%x", hr);

	// Count available adapters

	s_dev.m_numAdapters = 0;

	IDXGIAdapter1* adapter;
	while (s_dev.m_dxgiFactory->EnumAdapters1(s_dev.m_numAdapters, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		ueLogI("D3D11 adapter [%u]: %S", s_dev.m_numAdapters, desc.Description);
		adapter->Release();

		s_dev.m_numAdapters++;
	}
}

void glDevice_AcquireBackBuffer()
{
	glDeviceStartupParams* params = &s_dev.m_params;

	// Get back buffer and wrap it into engine render target

	ID3D11Texture2D* backBufferHandle = NULL;
	UE_ASSERTWinCall(s_dev.m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &backBufferHandle));

	glTextureBufferDesc backBufferDesc;
	backBufferDesc.m_width = params->m_width;
	backBufferDesc.m_height = params->m_height;
	backBufferDesc.m_format = params->m_colorFormat;
	backBufferDesc.m_numLevels = 1;
	backBufferDesc.m_flags = glTextureBufferFlags_IsRenderTarget;
	backBufferDesc.m_rt.m_MSAALevel = params->m_MSAALevel;

	s_dev.m_backBuffer = new(s_dev.m_textureBuffersPool) glTextureBuffer;
	s_dev.m_backBuffer->m_desc = backBufferDesc;
	s_dev.m_backBuffer->m_textureHandle = backBufferHandle;
	s_dev.m_backBuffer->m_isMainFrameBuffer = UE_TRUE;

	glTextureBuffer_CreateViews(s_dev.m_backBuffer);

	// Put render target to pool

	glRenderBufferPool_ReleaseTextureBuffer(s_dev.m_backBuffer);

	// Create main render group

	UE_ASSERT(!s_dev.m_mainRenderGroup);

	glRenderGroupDesc rgDesc;
	rgDesc.m_width = params->m_width;
	rgDesc.m_height = params->m_height;
	rgDesc.m_MSAALevel = params->m_MSAALevel;
	rgDesc.m_numColorBuffers = 1;
	rgDesc.m_colorDesc[0].m_format = params->m_colorFormat;
	rgDesc.m_depthStencilDesc.m_format = params->m_depthStencilFormat;
	rgDesc.m_hasDepthStencil = UE_TRUE;
	rgDesc.m_needColorContent = UE_FALSE;
	rgDesc.m_needDepthStencilContent = UE_FALSE;
	rgDesc.m_isMainFrameBuffer = UE_TRUE;
	s_dev.m_mainRenderGroup = s_dev.m_defCtx->m_renderGroup = glRenderGroup_Create(&rgDesc);

	// Reset viewport

	glViewportDesc viewport;
	viewport.m_left = 0;
	viewport.m_top = 0;
	viewport.m_width = params->m_width;
	viewport.m_height = params->m_height;
	viewport.m_minZ = 0.0f;
	viewport.m_maxZ = 1.0f;
	glCtx_SetViewport(glDevice_GetDefaultContext(), viewport);
}

UE_INLINE ueBool ueFlags_CheckAll(u32 value, u32 flags)
{
	return (value & flags) == flags;
}

void glDevice_Startup(const glDeviceStartupParams* params)
{
	UE_ASSERT_MSG(glDevice_IsSet(), "PreStartup was not called before.");

	s_dev.m_params = *params;

	glDevice_CalcMonitorAspectRatio(params);

	glSamplerDesc_InitializeDefaults();
	glDevice_Base_Startup(params);

	UE_ASSERT_FUNC(GLDEV->m_vertexDeclsPool.Init(GL_STACK_ALLOC, sizeof(glVertexDeclaration), params->m_maxVertexDecls));
	UE_ASSERT_FUNC(GLDEV->m_vertexDecls.Init(GL_STACK_ALLOC, params->m_maxVertexDecls));

	UE_ASSERT_FUNC(GLDEV->m_vertexShaderDeclsPool.Init(GL_STACK_ALLOC, sizeof(glVertexDeclarationEntry), params->m_maxVertexShaderDecls));
	UE_ASSERT_FUNC(GLDEV->m_vertexShaderDecls.Init(GL_STACK_ALLOC, params->m_maxVertexShaderDecls));

	UE_ASSERT_FUNC(GLDEV->m_programsPool.Init(GL_STACK_ALLOC, sizeof(glProgram), params->m_maxPrograms));

	UE_ASSERT_FUNC(GLDEV->m_samplerStateCache.Init(GL_STACK_ALLOC, params->m_maxSamplerStates));
	UE_ASSERT_FUNC(GLDEV->m_blendStateCache.Init(GL_STACK_ALLOC, params->m_maxBlendStates));
	UE_ASSERT_FUNC(GLDEV->m_rasterStateCache.Init(GL_STACK_ALLOC, params->m_maxRasterStates));
	UE_ASSERT_FUNC(GLDEV->m_depthStencilStateCache.Init(GL_STACK_ALLOC, params->m_maxDepthStencilStates));

	// Get adapter

	IDXGIAdapter1* adapter;
	if (FAILED(s_dev.m_dxgiFactory->EnumAdapters1(s_dev.m_params.m_adapterIndex, &adapter)))
		ueFatalErrorP("Failed to get adapter from DXGI factory at index %d", s_dev.m_params.m_adapterIndex);

	// Create swap chain description

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ueMemZeroS(swapChainDesc);
    swapChainDesc.BufferCount = 1;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.BufferDesc.Width = s_dev.m_params.m_width;
	swapChainDesc.BufferDesc.Height = s_dev.m_params.m_height;
	if (!glUtils_ToD3DFORMAT(s_dev.m_params.m_colorFormat, swapChainDesc.BufferDesc.Format))
		ueFatalErrorP("Invalid main swap chain color format (glBufferFormat = %s)", ueEnumToString(glBufferFormat, s_dev.m_params.m_colorFormat));
	swapChainDesc.BufferDesc.RefreshRate.Numerator = s_dev.m_params.m_frequency;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutpueWindow = ueWindow_GetHWND(s_dev.m_params.m_window);
	swapChainDesc.SampleDesc.Count = s_dev.m_params.m_MSAALevel;
    swapChainDesc.SampleDesc.Quality = s_dev.m_params.m_MSAAQuality;
	swapChainDesc.Windowed = s_dev.m_params.m_fullscreen ? FALSE : TRUE;

    const D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    UINT createDeviceFlags = 0;
#if defined(UE_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create device and swap chain

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL,//adapter, FIXME: wtf
		s_dev.m_params.m_driverType,
		NULL,
		createDeviceFlags,
		featureLevels,
		UE_ARRAY_SIZE(featureLevels),
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&s_dev.m_swapChain,
		&s_dev.m_d3dev,
		&s_dev.m_featureLevel,
		&s_dev.m_d3dCtx);

	adapter->Release();
	adapter = NULL;

	if (FAILED(hr))
		ueFatalErrorP("Failed to create D3D11 device and swap chain, hr = 0x%x", hr);

	// Determine available MSAA levels

	s_dev.m_caps.m_numMSAALevels = 0;
	s_dev.m_caps.m_MSAALevels[s_dev.m_caps.m_numMSAALevels++] = 1;

	UINT numQualityLevels;
	for (u32 i = 2; i <= 32; i++)
		if (SUCCEEDED(s_dev.m_d3dev->CheckMultisampleQualityLevels(swapChainDesc.BufferDesc.Format, i, &numQualityLevels)) && numQualityLevels)
			s_dev.m_caps.m_MSAALevels[s_dev.m_caps.m_numMSAALevels++] = i;

	// Set up caps

	glDevice_PrintStats();

	s_dev.m_caps.m_supportsTwoSidedStencil = UE_TRUE;
	s_dev.m_caps.m_hasHalfPixelOffset = UE_FALSE;

	s_dev.m_caps.m_maxColorRenderTargets = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
	s_dev.m_caps.m_maxSamplers[glShaderType_Vertex] = 16;
	s_dev.m_caps.m_maxSamplers[glShaderType_Fragment] = 16;
	s_dev.m_caps.m_maxSamplers[glShaderType_Geometry] = 16;
	s_dev.m_caps.m_maxVertexStreams = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;

	s_dev.m_caps.m_shaderVersion[glShaderType_Fragment] = 4;
	s_dev.m_caps.m_shaderVersion[glShaderType_Vertex] = 4;
	s_dev.m_caps.m_shaderVersion[glShaderType_Geometry] = 4;

	s_dev.m_caps.m_maxShaderRegisters[glShaderType_Fragment] = 0;
	s_dev.m_caps.m_maxShaderRegisters[glShaderType_Vertex] = 0;
	s_dev.m_caps.m_maxShaderRegisters[glShaderType_Geometry] = 0;

	s_dev.m_caps.m_maxAnisotropicFilteringlevel = D3D11_MAX_MAXANISOTROPY;

	s_dev.m_caps.m_maxUserClippingPlanes = 0;

	s_dev.m_caps.m_maxPrimitives = U32_MAX;

	s_dev.m_caps.m_supportsOcclusionQuery = UE_TRUE;
	s_dev.m_caps.m_supportsOcclusionQueryAnySample = UE_TRUE;
	s_dev.m_caps.m_supportsOcclusionPredicateQuery = UE_TRUE;

	s_dev.m_caps.m_supportsIndependentColorWriteMasks = UE_TRUE;

	for (u32 i = 0; i < glBufferFormat_MAX; i++)
	{
		glBufferFormat format = (glBufferFormat) i;
		if (format == glBufferFormat_Unknown)
			continue;

		DXGI_FORMAT d3dFormat;
		if (!glUtils_ToD3DFORMAT(format, d3dFormat))
			continue;

		const ueBool isDepthFormat = glUtils_IsDepthFormat(format);

		glCaps::FormatInfo& info = s_dev.m_caps.m_formatInfos[i];

		UINT support;
		SUCCEEDED(s_dev.m_d3dev->CheckFormatSupport(d3dFormat, &support));

		info.m_supportedAsRTOnly = ueFlags_CheckAll(support, D3D11_FORMAT_SUPPORT_TEXTURE2D | (isDepthFormat ? D3D11_FORMAT_SUPPORT_DEPTH_STENCIL : D3D11_FORMAT_SUPPORT_RENDER_TARGET));
		info.m_supportedAsRTAndTexture = ueFlags_CheckAll(support, D3D11_FORMAT_SUPPORT_TEXTURE2D | D3D11_FORMAT_SUPPORT_SHADER_SAMPLE | (isDepthFormat ? D3D11_FORMAT_SUPPORT_DEPTH_STENCIL : D3D11_FORMAT_SUPPORT_RENDER_TARGET));
		info.m_supportedAsTextureOnly = ueFlags_CheckAll(support, D3D11_FORMAT_SUPPORT_TEXTURE2D | D3D11_FORMAT_SUPPORT_SHADER_SAMPLE);
		info.m_supportsBlending = ueFlags_CheckAll(support, D3D11_FORMAT_SUPPORT_BLENDABLE);
		info.m_supportsFiltering = UE_TRUE;
		info.m_supportsAutoMips = ueFlags_CheckAll(support, D3D11_FORMAT_SUPPORT_TEXTURE2D);
	}

	glUtils_ListSupportedFormats();

	// Create default render context

	s_dev.m_defCtx = new(s_dev.m_contextsPool) glCtx;
	s_dev.m_defCtx->m_handle = s_dev.m_d3dCtx;
	glCtx_Init(s_dev.m_defCtx);

	// Startup necessary subsystems

	glShaderConstantMgr_Startup();
	glRenderBufferPool_Startup();

	// Retrieve back buffer and encapsulate it by texture buffer

	glDevice_AcquireBackBuffer();

	// Reset default context

	glCtx_Reset(s_dev.m_defCtx, glResetFlags_All);
}

void glDevice_Shutdown()
{
	UE_ASSERT(glDevice_IsSet());

	// Release all D3D resources

	glCtx_Reset(s_dev.m_defCtx, glResetFlags_Resources);

	// Destroy all graphics subsystems

	glRenderGroup_Destroy(s_dev.m_mainRenderGroup);
	s_dev.m_mainRenderGroup = NULL;

	glRenderBufferPool_Shutdown();
	glShaderConstantMgr_Shutdown();

	for (ueHashMap<D3D11_DEPTH_STENCIL_DESC, ID3D11DepthStencilState*>::Iterator i(s_dev.m_depthStencilStateCache); i.Next(); )
		(*i.Value())->Release();
	for (ueHashMap<D3D11_RASTERIZER_DESC, ID3D11RasterizerState*>::Iterator i(s_dev.m_rasterStateCache); i.Next(); )
		(*i.Value())->Release();
	for (ueHashMap<D3D11_BLEND_DESC, ID3D11BlendState*>::Iterator i(s_dev.m_blendStateCache); i.Next(); )
		(*i.Value())->Release();
	for (ueHashMap<D3D11_SAMPLER_DESC, ID3D11SamplerState*>::Iterator i(s_dev.m_samplerStateCache); i.Next(); )
		(*i.Value())->Release();

	UINT refCount;

	refCount = s_dev.m_swapChain->Release();
	UE_ASSERT(refCount == 0);
	s_dev.m_swapChain = NULL;

	glCtx_Deinit(s_dev.m_defCtx);
	s_dev.m_contextsPool.Free(s_dev.m_defCtx);

	refCount = s_dev.m_d3dev->Release();
	UE_ASSERT_MSGP(refCount == 0, "D3D11 device ref count is %u on exit", refCount);
	s_dev.m_d3dev = NULL;

	refCount = s_dev.m_dxgiFactory->Release();
	UE_ASSERT(refCount == 0);
	s_dev.m_dxgiFactory = NULL;

	GLDEV->m_depthStencilStateCache.Deinit();
	GLDEV->m_rasterStateCache.Deinit();
	GLDEV->m_blendStateCache.Deinit();
	GLDEV->m_samplerStateCache.Deinit();

	UE_ASSERT_MSGP(GLDEV->m_programsPool.Size() == 0, "Unfreed glProgram instances (count = %u).", GLDEV->m_programsPool.Size());
	GLDEV->m_programsPool.Deinit();

	GLDEV->m_vertexShaderDecls.Deinit();
	GLDEV->m_vertexShaderDeclsPool.Deinit();

	GLDEV->m_vertexDecls.Deinit();
	GLDEV->m_vertexDeclsPool.Deinit();

	glDevice_Base_Shutdown();

	glDevice_Set(NULL);
}

ueBool glDevice_Reset(const glDeviceStartupParams* newParams)
{
	ueBool needsToReaquireBackBuffer = UE_FALSE;

	// Change resolution and back buffer format

	if (newParams->m_width != s_dev.m_params.m_width ||
		newParams->m_height != s_dev.m_params.m_height ||
		newParams->m_frequency != s_dev.m_params.m_frequency ||
		newParams->m_colorFormat != s_dev.m_params.m_colorFormat)
	{
		DXGI_FORMAT d3dFormat;
		if (!glUtils_ToD3DFORMAT(newParams->m_colorFormat, d3dFormat))
			return UE_FALSE;

		glCtx_SetRenderGroup(s_dev.m_defCtx, NULL);
		glRenderBufferPool_DestroyAllBuffers(UE_TRUE);
		glRenderGroup_Destroy(s_dev.m_mainRenderGroup);
		s_dev.m_mainRenderGroup = NULL;

#if 1
		if (FAILED(GLDEV->m_swapChain->ResizeBuffers(1, newParams->m_width, newParams->m_height, d3dFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)))
#else
		DXGI_MODE_DESC modeDesc;
		ueMemZeroS(modeDesc);
		modeDesc.Width = newParams->m_width;
		modeDesc.Height = newParams->m_height;
		modeDesc.Format = d3dFormat;
		modeDesc.RefreshRate.Numerator = newParams->m_frequency;
		modeDesc.RefreshRate.Denominator = 1;
		if (FAILED(GLDEV->m_swapChain->ResizeTarget(&modeDesc)))
#endif
		{
			glDevice_AcquireBackBuffer();
			return UE_FALSE;
		}

		s_dev.m_params.m_width = newParams->m_width;
		s_dev.m_params.m_height = newParams->m_height;
		s_dev.m_params.m_colorFormat = newParams->m_colorFormat;
		//s_dev.m_params.m_frequency = newParams->m_frequency;

		needsToReaquireBackBuffer = UE_TRUE;
	}

	// Change fullscreen state

	if (newParams->m_fullscreen != s_dev.m_params.m_fullscreen)
	{
		glCtx_SetRenderGroup(s_dev.m_defCtx, NULL);
		glRenderBufferPool_DestroyAllBuffers(UE_TRUE);
		glRenderGroup_Destroy(s_dev.m_mainRenderGroup);
		s_dev.m_mainRenderGroup = NULL;

		if (FAILED(GLDEV->m_swapChain->SetFullscreenState(newParams->m_fullscreen ? TRUE : FALSE, NULL)))
		{
			glDevice_AcquireBackBuffer();
			return UE_FALSE;
		}

		s_dev.m_params.m_fullscreen = newParams->m_fullscreen;

		needsToReaquireBackBuffer = UE_TRUE;
	}

	if (needsToReaquireBackBuffer)
		glDevice_AcquireBackBuffer();

	return UE_TRUE;
}

#else // defined(GL_D3D11)
	UE_NO_EMPTY_FILE
#endif
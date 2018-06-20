#include "Base/ueBase.h"

#if defined(GL_D3D9)

#include "Base/ueWindow.h"
#include "Graphics/glLib_Private.h"

static glDevice s_dev;

glDeviceStartupParams::glDeviceStartupParams() :
	m_fullscreen(UE_FALSE),
	m_hardwareVertexProcessing(UE_TRUE),
	m_multithreaded(UE_FALSE),
	m_window(NULL),
	m_adapterIndex(D3DADAPTER_DEFAULT),
	m_deviceType(D3DDEVTYPE_HAL)
{}

void glDevice_PrintStats(const D3DCAPS9& caps)
{
	ueLogI("Display info (D3D9):");

	D3DADAPTER_IDENTIFIER9 adapter;
	s_dev.m_d3d9->GetAdapterIdentifier(s_dev.m_params.m_adapterIndex, 0, &adapter);

	const u32 driverProduct = HIWORD(adapter.DriverVersion.HighPart);
	const u32 driverVersion = LOWORD(adapter.DriverVersion.HighPart);
	const u32 driverSubVersion = HIWORD(adapter.DriverVersion.LowPart);
	const u32 driverBuild = LOWORD(adapter.DriverVersion.LowPart);
	ueLogI("\tD3D9 driver: %s", adapter.Driver);
	ueLogI("\tD3D9 driver version: %u.%u.%u.%u", driverProduct, driverVersion, driverSubVersion, driverBuild);

	ueLogI("\tAdapter: %s", adapter.Description);
	ueLogI("\tDevice name: %s", adapter.DeviceName);

	ueLogI("\tAvailable GPU memory: %u MB", s_dev.m_d3dev->GetAvailableTextureMem() >> 20);

	ueLogI("\tPixel shader version: %u.%u", (u32) D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion), (u32) D3DSHADER_VERSION_MINOR(caps.PixelShaderVersion));
	ueLogI("\tVertex shader version: %u.%u", (u32) D3DSHADER_VERSION_MAJOR(caps.VertexShaderVersion), (u32) D3DSHADER_VERSION_MINOR(caps.VertexShaderVersion));
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
	UE_ASSERT(!s_dev.m_d3d9);

	glDevice_Set(&s_dev);

#if !defined(UE_TOOL_X360)

	// Create main Direct3D object

	s_dev.m_d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	UE_ASSERT(s_dev.m_d3d9);

#endif
}

f32 glDevice_GetMonitorAspectRatio()
{
	return s_dev.m_monitorAspectRatio;
}

f32 glDevice_GetAspectRatio()
{
	return s_dev.m_params.m_fullscreen ? glDevice_GetMonitorAspectRatio() : glDevice_GetBufferAspectRatio();
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

void glDevice_Startup(const glDeviceStartupParams* params)
{
	UE_ASSERT_MSG(glDevice_IsSet(), "PreStartup was not called before.");

	s_dev.m_params = *params;

	glDevice_CalcMonitorAspectRatio(params);

	glSamplerDesc_InitializeDefaults();
	glDevice_D3D9Base_Startup(params);

	// Assume RGB8 format adapter

	const D3DFORMAT adapterFormat = D3DFMT_X8R8G8B8;

	// Determine available MSAA levels

	s_dev.m_caps.m_numMSAALevels = 1;
	s_dev.m_caps.m_MSAALevels[0] = 1;
	D3DFORMAT d3d9BackBufferFormat;
	UE_ASSERT_FUNC( glUtils_ToD3DFORMAT(params->m_colorFormat, d3d9BackBufferFormat) );
	for (u32 i = 2; i <= 16; i += 1)
		if (SUCCEEDED(s_dev.m_d3d9->CheckDeviceMultiSampleType(params->m_adapterIndex, params->m_deviceType, adapterFormat, !params->m_fullscreen, (D3DMULTISAMPLE_TYPE) i, NULL)))
			s_dev.m_caps.m_MSAALevels[s_dev.m_caps.m_numMSAALevels++] = i;

	// Set up presentation parameters

	D3DPRESENT_PARAMETERS pp;
	glDevice_SetD3D9PresentationParameters(&pp, params);

	// Create the device

	HRESULT hr;
	if (FAILED(hr = s_dev.m_d3d9->CreateDevice(
			params->m_adapterIndex,
			params->m_deviceType,
			ueWindow_GetHWND(params->m_window),
			(params->m_hardwareVertexProcessing ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING) |
				(params->m_multithreaded ? D3DCREATE_MULTITHREADED : 0),
			&pp,
			&s_dev.m_d3dev)))
	{
		// Try again but without hardware vertex processing

		if (params->m_hardwareVertexProcessing)
		{
			ueLogW("Failed to create D3D device (%ux%u; hr = 0x%x), retrying without hardware vertex processing...", params->m_width, params->m_height, hr);
			if (FAILED(hr = s_dev.m_d3d9->CreateDevice(
				params->m_adapterIndex,
				params->m_deviceType,
				ueWindow_GetHWND(params->m_window),
				D3DCREATE_SOFTWARE_VERTEXPROCESSING |
					(params->m_multithreaded ? D3DCREATE_MULTITHREADED : 0),
				&pp,
				&s_dev.m_d3dev)))
				ueFatalErrorP("Failed to create D3D9 device (%ux%u) with software vertex processing, hr = 0x%x", params->m_width, params->m_height, hr);
		}
		else
			ueFatalErrorP("Failed to create D3D9 device (%ux%u), hr = 0x%x", params->m_width, params->m_height, hr);
	}

	// Set up caps

	D3DCAPS9 d3d9Caps;
	UE_ASSERTWinCall(s_dev.m_d3dev->GetDeviceCaps(&d3d9Caps));

	glDevice_PrintStats(d3d9Caps);

	s_dev.m_caps.m_supportsTwoSidedStencil = (d3d9Caps.StencilCaps & D3DSTENCILCAPS_TWOSIDED) != 0;
	s_dev.m_caps.m_hasHalfPixelOffset = UE_TRUE;

	s_dev.m_caps.m_maxColorRenderTargets = (u32) d3d9Caps.NumSimultaneousRTs;
	s_dev.m_caps.m_maxSamplers[glShaderType_Vertex] = 4;
	s_dev.m_caps.m_maxSamplers[glShaderType_Fragment] = 16; // FIXME: Deduce from PS version
	s_dev.m_caps.m_maxSamplers[glShaderType_Geometry] = 0;
	s_dev.m_caps.m_maxVertexStreams = (u32) d3d9Caps.MaxStreams;

	s_dev.m_caps.m_shaderVersion[glShaderType_Fragment] = d3d9Caps.PixelShaderVersion;
	s_dev.m_caps.m_shaderVersion[glShaderType_Vertex] = d3d9Caps.VertexShaderVersion;
	s_dev.m_caps.m_shaderVersion[glShaderType_Geometry] = -1;

	s_dev.m_caps.m_maxShaderRegisters[glShaderType_Fragment] = /*d3d9Caps.MaxPixelShaderConst*/ 1024;
	s_dev.m_caps.m_maxShaderRegisters[glShaderType_Vertex] = d3d9Caps.MaxVertexShaderConst;
	s_dev.m_caps.m_maxShaderRegisters[glShaderType_Geometry] = 0;

	s_dev.m_caps.m_maxAnisotropicFilteringlevel = (u32) ((d3d9Caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) ? d3d9Caps.MaxAnisotropy : 0);

	s_dev.m_caps.m_maxUserClippingPlanes = (u32) d3d9Caps.MaxUserClipPlanes;

	IDirect3DQuery9* tempQuery = NULL;
	s_dev.m_caps.m_supportsOcclusionQuery = SUCCEEDED(s_dev.m_d3dev->CreateQuery(D3DQUERYTYPE_OCCLUSION, &tempQuery));
	if (tempQuery)
	{
		tempQuery->Release();
		tempQuery = NULL;
	}
	s_dev.m_caps.m_supportsOcclusionQuery = s_dev.m_caps.m_supportsOcclusionQueryAnySample;

	s_dev.m_caps.m_supportsOcclusionPredicateQuery = UE_FALSE;

	s_dev.m_caps.m_supportsIndependentColorWriteMasks = (d3d9Caps.PrimitiveMiscCaps & D3DPMISCCAPS_INDEPENDENTWRITEMASKS) != 0;

	for (u32 i = 0; i < glBufferFormat_MAX; i++)
	{
		glBufferFormat format = (glBufferFormat) i;
		if (format == glBufferFormat_Unknown)
			continue;

		D3DFORMAT d3dFormat;
		if (!glUtils_ToD3DFORMAT(format, d3dFormat))
			continue;

		const ueBool isDepthFormat = glUtils_IsDepthFormat(format);

		glCaps::FormatInfo& info = s_dev.m_caps.m_formatInfos[i];

		info.m_supportedAsRTOnly = SUCCEEDED(s_dev.m_d3d9->CheckDeviceFormat(params->m_adapterIndex, params->m_deviceType, adapterFormat, isDepthFormat ? D3DUSAGE_DEPTHSTENCIL : D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, d3dFormat));
		info.m_supportedAsRTAndTexture = SUCCEEDED(s_dev.m_d3d9->CheckDeviceFormat(params->m_adapterIndex, params->m_deviceType, adapterFormat, isDepthFormat ? D3DUSAGE_DEPTHSTENCIL : D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, d3dFormat));
		info.m_supportedAsTextureOnly = SUCCEEDED(s_dev.m_d3d9->CheckDeviceFormat(params->m_adapterIndex, params->m_deviceType, adapterFormat, 0, D3DRTYPE_TEXTURE, d3dFormat));
		info.m_supportsBlending = SUCCEEDED(s_dev.m_d3d9->CheckDeviceFormat(params->m_adapterIndex, params->m_deviceType, adapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, d3dFormat));
		info.m_supportsFiltering = SUCCEEDED(s_dev.m_d3d9->CheckDeviceFormat(params->m_adapterIndex, params->m_deviceType, adapterFormat, D3DUSAGE_QUERY_FILTER, D3DRTYPE_TEXTURE, d3dFormat));
		info.m_supportsAutoMips = SUCCEEDED(s_dev.m_d3d9->CheckDeviceFormat(params->m_adapterIndex, params->m_deviceType, adapterFormat, D3DUSAGE_AUTOGENMIPMAP, D3DRTYPE_TEXTURE, d3dFormat));
	}
	glUtils_ListSupportedFormats();

	s_dev.m_caps.m_maxPrimitives = d3d9Caps.MaxPrimitiveCount;

	// Create default render context

	s_dev.m_defCtx = new(s_dev.m_contextsPool) glCtx;
	s_dev.m_defCtx->m_handle = s_dev.m_d3dev;
	glCtx_Init(s_dev.m_defCtx);

	// Startup necessary subsystems

	glShaderConstantMgr_Startup();
	glRenderBufferPool_Startup();

	// Retrieve back buffer and encapsulate it by texture buffer

	IDirect3DSurface9* backBufferHandle = NULL;
	UE_ASSERTWinCall(D3DEV->GetRenderTarget(0, &backBufferHandle));

	glTextureBufferDesc backBufferDesc;
	backBufferDesc.m_width = params->m_width;
	backBufferDesc.m_height = params->m_height;
	backBufferDesc.m_format = params->m_colorFormat;
	backBufferDesc.m_numLevels = 1;
	backBufferDesc.m_flags = glTextureBufferFlags_IsRenderTarget;
	backBufferDesc.m_rt.m_MSAALevel = params->m_MSAALevel;

	s_dev.m_backBuffer = new(s_dev.m_textureBuffersPool) glTextureBuffer;
	s_dev.m_backBuffer->m_desc = backBufferDesc;
	s_dev.m_backBuffer->m_surfaceHandle = backBufferHandle;
	s_dev.m_backBuffer->m_textureHandle = NULL;
	s_dev.m_backBuffer->m_isMainFrameBuffer = UE_TRUE;

	glRenderBufferPool_ReleaseTextureBuffer(s_dev.m_backBuffer);

	// Retrieve depth stencil buffer and encapsulate it by texture buffer

	IDirect3DSurface9* depthStencilBufferHandle = NULL;
	UE_ASSERTWinCall(D3DEV->GetDepthStencilSurface(&depthStencilBufferHandle));

	glTextureBufferDesc depthStencilBufferDesc;
	depthStencilBufferDesc.m_width = params->m_width;
	depthStencilBufferDesc.m_height = params->m_height;
	depthStencilBufferDesc.m_format = params->m_depthStencilFormat;
	depthStencilBufferDesc.m_numLevels = 1;
	depthStencilBufferDesc.m_flags = glTextureBufferFlags_IsRenderTarget;
	depthStencilBufferDesc.m_rt.m_MSAALevel = params->m_MSAALevel;

	s_dev.m_depthStencilBuffer = new(s_dev.m_textureBuffersPool) glTextureBuffer;
	s_dev.m_depthStencilBuffer->m_desc = depthStencilBufferDesc;
	s_dev.m_depthStencilBuffer->m_surfaceHandle = depthStencilBufferHandle;
	s_dev.m_depthStencilBuffer->m_textureHandle = NULL;
	s_dev.m_depthStencilBuffer->m_isMainFrameBuffer = UE_TRUE;

	glRenderBufferPool_ReleaseTextureBuffer(s_dev.m_depthStencilBuffer);

	// Create main render group

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

	// Reset default context

	glCtx_Reset(s_dev.m_defCtx, glResetFlags_All);
}

void glDevice_Shutdown()
{
	UE_ASSERT(glDevice_IsSet());

	// Release all D3D9 resources

	glCtx_Reset(s_dev.m_defCtx, glResetFlags_Resources);

	glRenderGroup_Destroy(s_dev.m_mainRenderGroup);
	s_dev.m_mainRenderGroup = NULL;

	glRenderBufferPool_Shutdown();
	glShaderConstantMgr_Shutdown();

	glCtx_Deinit(s_dev.m_defCtx);
	s_dev.m_contextsPool.Free(s_dev.m_defCtx);

	const UINT refCount = s_dev.m_d3d9->Release();
	UE_ASSERT(refCount == 0);
	s_dev.m_d3d9 = NULL;

	glDevice_D3D9Base_Shutdown();

	glDevice_Set(NULL);
}

glDeviceState glDevice_GetState()
{
	UE_ASSERT(glDevice_IsSet());

	const HRESULT hr = D3DEV->TestCooperativeLevel();
	switch (hr)
	{
		case D3D_OK: return glDeviceState_Valid;
		case D3DERR_DEVICELOST: return glDeviceState_Lost;
		case D3DERR_DEVICENOTRESET: return glDeviceState_NotReset;
		case D3DERR_DRIVERINTERNALERROR: return glDeviceState_UnknownError;
	}
	return glDeviceState_UnknownError;
}

void glDevice_OnLostDevice()
{
	UE_ASSERT(glDevice_IsSet());

	// Destroy main render group

	if (s_dev.m_mainRenderGroup)
	{
		glRenderGroup_Destroy(s_dev.m_mainRenderGroup);
		s_dev.m_mainRenderGroup = NULL;
		s_dev.m_defCtx->m_mainRenderGroup = NULL;
	}

	// Destroy non-managed resources
	
	ueGenericPool::Iterator vbIter(s_dev.m_vertexBuffersPool);
	while (glVertexBuffer* vb = (glVertexBuffer*) vbIter.Next())
		glVertexBuffer_OnLostDevice(vb);

	ueGenericPool::Iterator ibIter(s_dev.m_indexBuffersPool);
	while (glIndexBuffer* ib = (glIndexBuffer*) ibIter.Next())
		glIndexBuffer_OnLostDevice(ib);

	ueGenericPool::Iterator tbIter(s_dev.m_textureBuffersPool);
	while (glTextureBuffer* tb = (glTextureBuffer*) tbIter.Next())
		glTextureBuffer_OnLostDevice(tb);

	ueGenericPool::Iterator oqIter(s_dev.m_queriesPool);
	while (glOcclusionQuery* oq = (glOcclusionQuery*) oqIter.Next())
		glOcclusionQuery_OnLostDevice(oq);
}

void glDevice_OnResetDevice()
{
	UE_ASSERT(glDevice_IsSet());

	// Recreate non-managed resources

	ueGenericPool::Iterator vbIter(s_dev.m_vertexBuffersPool);
	while (glVertexBuffer* vb = (glVertexBuffer*) vbIter.Next())
		glVertexBuffer_OnResetDevice(vb);

	ueGenericPool::Iterator ibIter(s_dev.m_indexBuffersPool);
	while (glIndexBuffer* ib = (glIndexBuffer*) ibIter.Next())
		glIndexBuffer_OnResetDevice(ib);

	ueGenericPool::Iterator tbIter(s_dev.m_textureBuffersPool);
	while (glTextureBuffer* tb = (glTextureBuffer*) tbIter.Next())
		glTextureBuffer_OnResetDevice(tb);

	ueGenericPool::Iterator oqIter(s_dev.m_queriesPool);
	while (glOcclusionQuery* oq = (glOcclusionQuery*) oqIter.Next())
		glOcclusionQuery_OnResetDevice(oq);

	// Get back buffer and depth-stencil

	UE_ASSERTWinCall(D3DEV->GetRenderTarget(0, &s_dev.m_backBuffer->m_surfaceHandle));
	glTextureBufferDesc& backBufferDesc = s_dev.m_backBuffer->m_desc;
	backBufferDesc.m_width = s_dev.m_params.m_width;
	backBufferDesc.m_height = s_dev.m_params.m_height;
	backBufferDesc.m_format = s_dev.m_params.m_colorFormat;
	backBufferDesc.m_rt.m_MSAALevel = s_dev.m_params.m_MSAALevel;

	UE_ASSERTWinCall(D3DEV->GetDepthStencilSurface(&s_dev.m_depthStencilBuffer->m_surfaceHandle));
	glTextureBufferDesc& depthStencilBufferDesc = s_dev.m_depthStencilBuffer->m_desc;
	depthStencilBufferDesc.m_width = s_dev.m_params.m_width;
	depthStencilBufferDesc.m_height = s_dev.m_params.m_height;
	depthStencilBufferDesc.m_format = s_dev.m_params.m_depthStencilFormat;
	depthStencilBufferDesc.m_rt.m_MSAALevel = s_dev.m_params.m_MSAALevel;

	// Recreate main render group

	glRenderGroupDesc rgDesc;
	rgDesc.m_width = s_dev.m_params.m_width;
	rgDesc.m_height = s_dev.m_params.m_height;
	rgDesc.m_MSAALevel = s_dev.m_params.m_MSAALevel;
	rgDesc.m_numColorBuffers = 1;
	rgDesc.m_colorDesc[0].m_format = s_dev.m_params.m_colorFormat;
	rgDesc.m_depthStencilDesc.m_format = s_dev.m_params.m_depthStencilFormat;
	rgDesc.m_hasDepthStencil = UE_TRUE;
	rgDesc.m_needColorContent = UE_FALSE;
	rgDesc.m_needDepthStencilContent = UE_FALSE;
	rgDesc.m_isMainFrameBuffer = UE_TRUE;
	s_dev.m_mainRenderGroup = s_dev.m_defCtx->m_renderGroup = glRenderGroup_Create(&rgDesc);

	// Reset state of the default render context

	glCtx_Reset(s_dev.m_defCtx, glResetFlags_All);
}

ueBool glDevice_Reset(const glDeviceStartupParams* params)
{
	UE_ASSERT(glDevice_IsSet());

	if (!params)
		params = &s_dev.m_params; // Reset with same settings

	ueLogD("D3D9 device reset started (%s, MSAA x %u, %ux%u)...",
		params->m_fullscreen ? "fullscreen" : "windowed",
		params->m_MSAALevel,
		params->m_width, params->m_height);

	// Destroy all non-managed resources

	glDevice_OnLostDevice();

	// Set up presentation parameters

	// Reset device with new setting
	// FIXME: Fix case when after reset - in that case it's forbidden to call Reset again; instead must call TestCooperativeLevel() and then Release()

	D3DPRESENT_PARAMETERS pp;
	glDevice_SetD3D9PresentationParameters(&pp, params);

	HRESULT hr = D3DEV->Reset(&pp);
	if (FAILED(hr))
		return UE_FALSE;

	s_dev.m_params = *params;

	// Recreate all non-managed resources

	glDevice_OnResetDevice();

	ueLogD("D3D9 device reset done");

	return UE_TRUE;
}

void glDevice_SetD3D9PresentationParameters(D3DPRESENT_PARAMETERS* pp, const glDeviceStartupParams* params)
{
	// Select MSAA level that matches request best but isn't greater

	u32 MSAALevel = 1;
	for (u32 i = 0; i < s_dev.m_caps.m_numMSAALevels; i++)
		if (s_dev.m_caps.m_MSAALevels[i] <= params->m_MSAALevel &&
			ueAbs((s32) s_dev.m_caps.m_MSAALevels[i] - (s32) params->m_MSAALevel) < ueAbs((s32) MSAALevel - (s32) params->m_MSAALevel))
			MSAALevel = s_dev.m_caps.m_MSAALevels[i];

	// Set up presentation params

	ueMemSet(pp, 0, sizeof(D3DPRESENT_PARAMETERS));

	pp->Windowed = params->m_fullscreen ? FALSE : TRUE;
	pp->hDeviceWindow = ueWindow_GetHWND(params->m_window);
	pp->BackBufferWidth = params->m_width;
	pp->BackBufferHeight = params->m_height;
	pp->BackBufferCount = params->m_backBufferCount;
	UE_ASSERT_FUNC( glUtils_ToD3DFORMAT(params->m_colorFormat, pp->BackBufferFormat) );
	pp->EnableAutoDepthStencil = TRUE;
	UE_ASSERT_FUNC( glUtils_ToD3DFORMAT(params->m_depthStencilFormat, pp->AutoDepthStencilFormat) );
	pp->MultiSampleType = glUtils_ToD3DMULTISAMPLE_TYPE(MSAALevel);
	pp->SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp->Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	pp->PresentationInterval = params->m_displaySync == glDisplaySync_None ? D3DPRESENT_INTERVAL_IMMEDIATE : D3DPRESENT_INTERVAL_ONE;
	pp->FullScreen_RefreshRateInHz = params->m_fullscreen ? params->m_frequency : 0;
}

#else // defined(GL_D3D9)
	UE_NO_EMPTY_FILE
#endif
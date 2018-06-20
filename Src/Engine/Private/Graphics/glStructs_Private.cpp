#include "Graphics/glLib_Private.h"

// Device startup params

glDeviceStartupParams_Base::glDeviceStartupParams_Base() :
	m_stackAllocator(NULL),
	m_freqAllocator(NULL),
	m_width(800),
	m_height(600),
	m_frequency(60),
	m_colorFormat(glBufferFormat_Native_R8G8B8A8),
	m_depthStencilFormat(glBufferFormat_D24S8),
	m_MSAALevel(1),
	m_displaySync(glDisplaySync_None),
	m_backBufferCount(2),
	m_maxContexts(1),
	m_maxRenderGroups(64),
	m_maxPoolManagedTextureBuffers(64),
	m_maxTextureBuffers(2048),
	m_maxOcclusionQueries(1024),
	m_maxVertexBuffers(2048),
	m_maxIndexBuffers(2048),
	m_maxStreamFormats(128),
	m_maxVertexShaderInputs(128),
	m_maxPrograms(1024),
	m_maxGlobalSamplerConstants(256),
	m_maxGlobalNumericConstants(1024),
	m_maxGlobalNumericConstantsDataSize(65536),
	m_constantNameBufferSize(32768)
{}


// Sampler description

glSamplerParams glSamplerParams::DefaultWrap;
glSamplerParams glSamplerParams::DefaultClamp;
glSamplerParams glSamplerParams::DefaultMirror;
glSamplerParams glSamplerParams::DefaultPP;
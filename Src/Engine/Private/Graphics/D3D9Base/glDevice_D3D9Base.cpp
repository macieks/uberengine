#include "Base/ueBase.h"

#if defined(GL_D3D9) || defined(GL_X360)

#include "Graphics/glLib_Private.h"

glDeviceStartupParams_D3D9Base::glDeviceStartupParams_D3D9Base() :
	m_maxVertexDecls(256),
	m_maxVertexShaderDecls(1024)
{}

IDirect3D9* glDevice_GetD3D()
{
	return GLDEV->m_d3d9;
}

IDirect3DDevice9* glDevice_GetD3Dev()
{
	return GLDEV->m_d3dev;
}

void glDevice_D3D9Base_Startup(const glDeviceStartupParams* params)
{
	glDevice_Base_Startup(params);

	UE_ASSERT_FUNC(GLDEV->m_vertexDeclsPool.Init(GL_STACK_ALLOC, sizeof(glVertexDeclaration), params->m_maxVertexDecls));
	UE_ASSERT_FUNC(GLDEV->m_vertexDecls.Init(GL_STACK_ALLOC, params->m_maxVertexDecls));

	UE_ASSERT_FUNC(GLDEV->m_vertexShaderDeclsPool.Init(GL_STACK_ALLOC, sizeof(glVertexDeclarationEntry), params->m_maxVertexShaderDecls));
	UE_ASSERT_FUNC(GLDEV->m_vertexShaderDecls.Init(GL_STACK_ALLOC, params->m_maxVertexShaderDecls));

	UE_ASSERT_FUNC(GLDEV->m_programsPool.Init(GL_STACK_ALLOC, sizeof(glProgram), params->m_maxPrograms));
}

void glDevice_D3D9Base_Shutdown()
{
	UE_ASSERT(GLDEV->m_programsPool.Size() == 0);
	GLDEV->m_programsPool.Deinit();

	GLDEV->m_vertexShaderDecls.Deinit();
	GLDEV->m_vertexShaderDeclsPool.Deinit();

	GLDEV->m_vertexDecls.Deinit();
	GLDEV->m_vertexDeclsPool.Deinit();

	glDevice_Base_Shutdown();
}

#else // defined(GL_D3D9) || defined(GL_X360)
	UE_NO_EMPTY_FILE
#endif
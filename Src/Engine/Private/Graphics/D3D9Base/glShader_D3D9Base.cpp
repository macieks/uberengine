#include "Base/ueBase.h"

#if defined(GL_D3D9) || defined(GL_X360)

#include "Graphics/glLib_Private.h"

glProgram* glProgram_Create(glShader* vs, glShader* fs, glShader* gs)
{
	glDevice* dev = glDevice_Get();

	glProgram* program = new(dev->m_programsPool) glProgram();
	UE_ASSERT(program);

	program->m_shaders[glShaderType_Vertex] = vs;
	program->m_shaders[glShaderType_Fragment] = fs;
	program->m_shaders[glShaderType_Geometry] = gs;

	for (u32 i = 0; i < glShaderType_MAX; i++)
	{
		glShader* shader = program->m_shaders[i];
		if (shader)
			shader->m_numProgramRefs++;
		UE_ASSERT_MSGP(!shader || shader->m_shaderType == i, "Incorrect shader type (got %u; expected %u)", (u32) shader->m_shaderType, (u32) i);
	}

	return program;
}

void glProgram_Destroy(glProgram* program)
{
	glDevice* dev = glDevice_Get();

	for (u32 i = 0; i < glShaderType_MAX; i++)
	{
		glShader* shader = program->m_shaders[i];
		if (shader)
		{
			shader->m_numProgramRefs--;
			program->m_shaders[i] = NULL;
		}
	}
	dev->m_programsPool.Free(program);
}

#else // defined(GL_D3D9) || defined(GL_X360)
	UE_NO_EMPTY_FILE
#endif
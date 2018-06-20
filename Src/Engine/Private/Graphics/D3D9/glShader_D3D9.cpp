#include "Base/ueBase.h"

#if defined(GL_D3D9)

#include "Graphics/glLib_Private.h"

glShader* glShader_Create(glShaderDesc* desc)
{
	const u32 size = sizeof(glShader) + desc->m_numConstants * sizeof(glShaderConstantInstance);
	u8* memory = (u8*) GL_FREQ_ALLOC->Alloc(size);
	UE_ASSERT(memory);

	glShader* shader = (glShader*) memory;
	shader->m_shaderType = desc->m_type;
	shader->m_handle = NULL;
	shader->m_numConstants = desc->m_numConstants;
	shader->m_numOutputs = desc->m_numOutputs;
	shader->m_numProgramRefs = 0;
	memory += sizeof(glShader);

	shader->m_constants = (glShaderConstantInstance*) memory;
	for (u32 i = 0; i < desc->m_numConstants; i++)
	{
		glShaderConstantInstanceDesc* constantInstanceDesc = &desc->m_constants[i];
		glShaderConstantInstance* constantInstance = &shader->m_constants[i];

		constantInstance->m_constant = glShaderConstantMgr_GetConstant(&constantInstanceDesc->m_desc);
		constantInstance->m_isColumnMajor = constantInstanceDesc->m_isColumnMajor;
		constantInstance->m_offset = constantInstanceDesc->m_offset;
	}

	switch (desc->m_type)
	{
		case glShaderType_Vertex:
		{
			UE_ASSERT(desc->m_VSInputDesc);
			shader->m_input = glVertexShaderInput_Create(desc->m_VSInputDesc);
			UE_ASSERT(shader->m_input);
			UE_ASSERTWinCall(D3DEV->CreateVertexShader((const DWORD*) desc->m_code, (IDirect3DVertexShader9**) &shader->m_handle));
			return shader;
		}
		case glShaderType_Fragment:
		{
			UE_ASSERTWinCall(D3DEV->CreatePixelShader((const DWORD*) desc->m_code, (IDirect3DPixelShader9**) &shader->m_handle));
			return shader;
		}
		UE_INVALID_CASE(desc->m_type)
	}
	return NULL;
}

void glShader_Destroy(glShader* shader)
{
	UE_ASSERT(shader->m_numProgramRefs == 0);

	if (shader->m_shaderType == glShaderType_Vertex)
		glVertexShaderInput_Destroy(shader->m_input);

	const UINT refCount = shader->m_handle->Release();
	UE_ASSERT(refCount == 0);
	shader->m_handle = NULL;

	GL_FREQ_ALLOC->Free(shader);
}

#else // defined(GL_D3D9)
	UE_NO_EMPTY_FILE
#endif
#include "Base/ueBase.h"

#if defined(GL_D3D11)

#include "Graphics/glLib_Private.h"

glShader* glShader_Create(glShaderDesc* desc)
{
	// Allocate memory

	u32 totalBuffersSize = 0;
	for (u32 i = 0; i < desc->m_numBuffers; i++)
		totalBuffersSize += desc->m_bufferDescs[i].m_size;

	const u32 size = sizeof(glShader) +
		desc->m_numConstants * sizeof(glShaderConstantInstance) +
		desc->m_numBuffers * (sizeof(ID3D11Buffer*) + sizeof(glShader::BufferData)) +
		totalBuffersSize +
		desc->m_codeSize;
	u8* memory = (u8*) GL_FREQ_ALLOC->Alloc(size);
	UE_ASSERT(memory);

	// Create engine resource

	glShader* shader = (glShader*) memory;
	shader->m_shaderType = desc->m_type;
	shader->m_handle = NULL;
	shader->m_numConstants = desc->m_numConstants;
	shader->m_numOutputs = desc->m_numOutputs;
	shader->m_numBuffers = desc->m_numBuffers;
	shader->m_numProgramRefs = 0;
	shader->m_vertexDeclEntries = NULL;
	memory += sizeof(glShader);

	shader->m_buffers = (ID3D11Buffer**) memory;
	memory += desc->m_numBuffers * sizeof(ID3D11Buffer*);

	shader->m_bufferDatas = (glShader::BufferData*) memory;
	memory += desc->m_numBuffers * sizeof(glShader::BufferData);
	for (u32 i = 0; i < desc->m_numBuffers; i++)
	{
		glShader::BufferData& data = shader->m_bufferDatas[i];

		data.m_size = desc->m_bufferDescs[i].m_size;
		data.m_memory = memory;
		memory += data.m_size;
		data.m_minDirtyOffset = U32_MAX;
		data.m_maxDirtyOffset = 0;

#if defined(UE_DEBUG)
		ueMemZero(data.m_memory, data.m_size);
#endif

		// Create GPU constant buffer

		D3D11_BUFFER_DESC cbDesc;
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = 0;
		cbDesc.MiscFlags = 0;
		cbDesc.ByteWidth = data.m_size;
		UE_ASSERTWinCall(D3DEV->CreateBuffer(&cbDesc, NULL, &shader->m_buffers[i]));
	}

	shader->m_constants = (glShaderConstantInstance*) memory;
	for (u32 i = 0; i < desc->m_numConstants; i++)
	{
		glShaderConstantInstanceDesc* constantInstanceDesc = &desc->m_constants[i];
		glShaderConstantInstance* constantInstance = &shader->m_constants[i];

		constantInstance->m_constant = glShaderConstantMgr_GetConstant(&constantInstanceDesc->m_desc);
		constantInstance->m_isColumnMajor = constantInstanceDesc->m_isColumnMajor;
		constantInstance->m_offset = constantInstanceDesc->m_offset;
	}
	memory += sizeof(glShaderConstantInstance) * desc->m_numConstants;

	shader->m_codeSize = desc->m_codeSize;
	shader->m_code = (const char*) memory;
	ueMemCpy(const_cast<char*>(shader->m_code), desc->m_code, desc->m_codeSize);
	memory += desc->m_codeSize;

	// Create GPU resource

	switch (desc->m_type)
	{
		case glShaderType_Vertex:
		{
			UE_ASSERT(desc->m_VSInputDesc);

			shader->m_input = glVertexShaderInput_Create(desc->m_VSInputDesc);
			UE_ASSERT(shader->m_input);

			UE_ASSERTWinCall(D3DEV->CreateVertexShader((const VOID*) desc->m_code, desc->m_codeSize, NULL, (ID3D11VertexShader**) &shader->m_handle));

			UE_ASSERTWinCall(D3DGetInputSignatureBlob((const VOID*) desc->m_code, desc->m_codeSize, &shader->m_inputSignature));

			return shader;
		}
		case glShaderType_Fragment:
		{
			UE_ASSERTWinCall(D3DEV->CreatePixelShader((const VOID*) desc->m_code, desc->m_codeSize, NULL, (ID3D11PixelShader**) &shader->m_handle));
			return shader;
		}
		case glShaderType_Geometry:
		{
			UE_ASSERTWinCall(D3DEV->CreateGeometryShader((const VOID*) desc->m_code, desc->m_codeSize, NULL, (ID3D11GeometryShader**) &shader->m_handle));
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
	{
		glVertexShaderInput_Destroy(shader->m_input);
		shader->m_inputSignature->Release();

		// Destroy vertex decl entries

		glVertexDeclarationEntry* entry = shader->m_vertexDeclEntries;
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
	}

	for (u32 i = 0; i < shader->m_numBuffers; i++)
		shader->m_buffers[i]->Release();

	const UINT refCount = shader->m_handle->Release();
	UE_ASSERT(refCount == 0);
	shader->m_handle = NULL;

	GL_FREQ_ALLOC->Free(shader);
}

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

#else // defined(GL_D3D11)
	UE_NO_EMPTY_FILE
#endif
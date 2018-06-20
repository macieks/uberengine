#include "Base/ueBase.h"

#if defined(GL_OPENGL)

#include "Graphics/glLib_Private.h"

glShader* glShader_Create(glShaderDesc* desc)
{
	// Create shader object

	GLenum shaderType;
	switch (desc->m_type)
	{
		case glShaderType_Vertex: shaderType = GL_VERTEX_SHADER_ARB; break;
		case glShaderType_Fragment: shaderType = GL_FRAGMENT_SHADER_ARB; break;
#ifndef GL_OPENGL_ES
		case glShaderType_Geometry: shaderType = GL_GEOMETRY_SHADER_ARB; break;
#endif
		UE_INVALID_CASE(desc->m_type)
	}

	GLhandleARB handle = GLC(glCreateShaderObjectARB(shaderType));
	UE_ASSERT(handle);

	// Compile shader

	const GLchar* sourceCode = (const GLchar*) desc->m_code;

	GL(glShaderSourceARB(handle, 1, &sourceCode, NULL));
	GL(glCompileShaderARB(handle));

	GLint compileResult;
	GL(glGetShaderiv(handle, GL_OBJECT_COMPILE_STATUS_ARB, &compileResult));
	if (!compileResult)
	{
		GLsizei logSize;
		GLcharARB log[1 << 16];
#ifdef GL_OPENGL_ES
		GL(glGetShaderInfoLog(handle, UE_ARRAY_SIZE(log), &logSize, log));
#else
		GL(glGetInfoLogARB(handle, UE_ARRAY_SIZE(log), &logSize, log));
#endif
		ueLogE("Failed to compile shader program, reason:\n%s\nGLSL source code:\n%s", log, sourceCode);
		return NULL;
	}

	// Set up engine shader

	const u32 size = sizeof(glShader) + desc->m_numConstants * sizeof(glShaderConstantInstance);
	u8* memory = (u8*) GL_FREQ_ALLOC->Alloc(size);
	UE_ASSERT(memory);

	glShader* shader = (glShader*) memory;
	shader->m_shaderType = desc->m_type;
	shader->m_handle = handle;
	shader->m_numConstants = desc->m_numConstants;
	shader->m_numOutputs = desc->m_numOutputs;
	shader->m_numProgramRefs = 0;
#if defined(GL_ENABLE_SHADER_PREVIEW)
	shader->m_debugSrc = ueStrDup(GL_FREQ_ALLOC, sourceCode);
#endif
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

	if (desc->m_type == glShaderType_Vertex)
	{
		shader->m_input = glVertexShaderInput_Create(desc->m_VSInputDesc);
		UE_ASSERT(shader->m_input);
	}

	return shader;
}

void glShader_Destroy(glShader* shader)
{
	glDevice* dev = glDevice_Get();

	UE_ASSERT_MSG(shader->m_numProgramRefs == 0, "There are still valid programs that reference this shader.");

#if defined(GL_ENABLE_SHADER_PREVIEW)
	GL_FREQ_ALLOC->Free(shader->m_debugSrc);
#endif

	GL(glDeleteShader(shader->m_handle));
	shader->m_handle = 0;

	if (shader->m_shaderType == glShaderType_Vertex)
		glVertexShaderInput_Destroy(shader->m_input);

	GL_FREQ_ALLOC->Free(shader);
}

glProgram* glProgram_Create(glShader* vs, glShader* fs, glShader* gs)
{
	glDevice* dev = glDevice_Get();

	glShader* shaders[glShaderType_MAX] = {vs, fs, gs};
#if defined(UE_ENABLE_ASSERTION)
	for (u32 i = 0; i < glShaderType_MAX; i++)
	{
		glShader* shader = shaders[i];
		UE_ASSERT_MSGP(!shader || shader->m_shaderType == i, "Invalid shader type (got %u; expected %u)", (u32) shader->m_shaderType, i);
	}
#endif // defined(UE_ENABLE_ASSERTION)

	// Find existing program

	glProgram desc;
	for (u32 i = 0; i < glShaderType_MAX; i++)
		desc.m_shaders[i] = shaders[i];

	glProgram* program = dev->m_programs.Find(desc);
	if (program)
	{
		program->m_refCount++;
		return program;
	}

	// Create new program

	GLhandleARB handle = GLC(glCreateProgramObjectARB());
	UE_ASSERT(handle);

	// Attach shaders to program

	for (u32 i = 0; i < glShaderType_MAX; i++)
		if (shaders[i])
			GL(glAttachObjectARB(handle, shaders[i]->m_handle));

	// Link shaders into program

	GL(glLinkProgramARB(handle));

	GLint linkResult;
	GL(glGetProgramiv(handle, GL_OBJECT_LINK_STATUS_ARB, &linkResult));
	if (!linkResult)
	{
		GLsizei logSize;
		GLcharARB log[1 << 16];
#ifdef GL_OPENGL_ES
		GL(glGetProgramInfoLog(handle, UE_ARRAY_SIZE(log), &logSize, log));
#else
		GL(glGetInfoLogARB(handle, UE_ARRAY_SIZE(log), &logSize, log));
#endif
		ueLogE("Failed to link shader program, reason: %s", log);
		return NULL;
	}

	// Bind program

	GLint oldProgram;
	GL(glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram));

	GL(glUseProgram(handle));

	// Get attribute & uniform counts

	GLint attrCount, maxAttrNameLength;
	GL(glGetProgramiv(handle, GL_OBJECT_ACTIVE_ATTRIBUTES_ARB, &attrCount));
	GL(glGetProgramiv(handle, GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB, &maxAttrNameLength));

	glShader* vs = shaders[glShaderType_Vertex];
	UE_ASSERT(vs);
	UE_ASSERT(attrCount == vs->m_input->m_numAttrs);

	GLint uniformCount, maxUniformNameLength;
	GL(glGetProgramiv(handle, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &uniformCount));
	GL(glGetProgramiv(handle, GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB, &maxUniformNameLength));

	// Add program

	program = dev->m_programs.Insert(desc);
	UE_ASSERT(program);

	program->m_handle = handle;
	program->m_refCount = 1;
	program->m_numUniforms = uniformCount;
	program->m_uniforms = (glProgram::Uniform*) GL_FREQ_ALLOC->Alloc(
		sizeof(glProgram::Uniform) * uniformCount +
		sizeof(glProgram::Attr) * attrCount);
	UE_ASSERT(program->m_uniforms);
	program->m_numAttrs = attrCount;
	program->m_attrs = (glProgram::Attr*) (program->m_uniforms + uniformCount);

	// Collect attribs

	for (s32 i = 0; i < attrCount; i++)
		program->m_attrs[i].m_location = 0;

	GLenum attrType;
	GLint attrNameLength;
	GLint attrSize;
	GLchar attrName[128];
	UE_ASSERT(maxAttrNameLength + 1 < UE_ARRAY_SIZE(attrName));

	for (s32 i = 0; i < attrCount; i++)
	{
		// Figure out attribute semantic and usage index

		GL(glGetActiveAttribARB(handle, i, UE_ARRAY_SIZE(attrName), &attrNameLength, &attrSize, &attrType, attrName));

		const GLint location = GLC(glGetAttribLocationARB(handle, attrName));
		UE_ASSERT(location >= 0);

		u8 semantic;
		u8 usageIndex;
		u8 isGeneric;
		glUtils_GetUsageAndSemanticForCgAttrib(attrName, attrNameLength, semantic, usageIndex, isGeneric);

		UE_ASSERT(isGeneric); // All built-in attributes are converted to generic ones in Cg shader compiler

		// Find matching vertex shader attr input index

		s32 index = S32_MAX;
		for (s32 j = 0; j < attrCount; j++)
		{
			if (program->m_attrs[j].m_location)
				continue;
			
			const glShaderAttr& vsAttr = vs->m_input->m_attrs[j];
			if (vsAttr.m_semantic == semantic && vsAttr.m_usageIndex == usageIndex)
			{
				index = j;
				break;
			}
		}
		UE_ASSERT(index != S32_MAX);

		glProgram::Attr& attr = program->m_attrs[index];
		attr.m_location = location;
		attr.m_semantic = semantic;
		attr.m_usageIndex = usageIndex;
		attr.m_isGeneric = isGeneric;
	}

	// Collect uniforms

	GLenum uniformType;
	GLint uniformNameLength;
	GLint uniformSize;
	GLchar uniformName[128];
	UE_ASSERT(maxUniformNameLength + 1 < UE_ARRAY_SIZE(uniformName));

	u32 samplerIndex = 0;

	for (s32 i = 0; i < uniformCount; i++)
	{
		glProgram::Uniform& uniform = program->m_uniforms[i];
		GL(glGetActiveUniformARB(handle, i, UE_ARRAY_SIZE(uniformName), &uniformNameLength, &uniformSize, &uniformType, uniformName));

		// Get uniform location

		uniform.m_location = GLC(glGetUniformLocationARB(handle, uniformName));
		UE_ASSERT(uniform.m_location >= 0);

		// Find matching constant in any of the bound shaders

		char constantName[128];
		ueStrCpyS(constantName, uniformName[0] == '_' ? &uniformName[1] : uniformName);

		if (char* hasArrayInName = ueStrStr(constantName, "[0]"))
			*hasArrayInName = 0;

		uniform.m_constant = NULL;
		for (u32 j = 0; (j < glShaderType_MAX && !uniform.m_constant); j++)
		{
			glShader* shader = shaders[j];
			if (!shader)
				continue;

			for (u32 k = 0; k < shader->m_numConstants; k++)
			{
				glShaderConstantInstance& constantInstance = shader->m_constants[k];
				const glConstant* constant = constantInstance.m_constant;
				if (!ueStrCmp(constantName, constant->m_desc.m_name))
				{
					uniform.m_constant = constant;

					// Assign consecutive sampler index to sampler uniform

					if (glUtils_IsSampler((glConstantType) constant->m_desc.m_type))
					{
						GL(glUniform1i(uniform.m_location, samplerIndex));
						samplerIndex++;
					}
					break;
				}
			}
		}
		UE_ASSERT_MSGP(uniform.m_constant, "Failed to find uniform (name = %s, GL type = %d, GL size = %d)", uniformName, (s32) uniformType, (s32) uniformSize);
	}

	// Restore previous program

	GL(glUseProgram(oldProgram));

	return program;
}

void glProgram_Destroy(glProgram* program)
{
	UE_ASSERT(program->m_handle);

	if (--program->m_refCount)
		return;

#ifdef GL_OPENGL_ES
	GL(glDeleteProgram(program->m_handle));
#else
	GL(glDeleteProgramsARB(1, &program->m_handle));
#endif
	program->m_handle = 0;

	GL_FREQ_ALLOC->Free(program->m_uniforms);
	glDevice_Get()->m_programs.Remove(*program);
}

#else // defined(GL_OPENGL)
	UE_NO_EMPTY_FILE
#endif
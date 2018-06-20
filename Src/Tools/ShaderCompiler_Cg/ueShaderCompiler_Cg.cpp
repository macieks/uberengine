#include "ShaderCompiler_Common/ueToolShader.h"
#include "ueShaderCompiler_Cg.h"

CGcontext s_context;
CgType s_cgType;

bool CheckForCgError(const char* situation)
{
	CGerror error;
	const char* string = cgGetLastErrorString(&error);

	if (error == CG_NO_ERROR)
		return true;

	ueLogE("Cg error: %s: %s", situation, string);
	if (error == CG_COMPILER_ERROR)
		ueLogE("Cg compiler error:\n%s", cgGetLastListing(s_context));
	return false;
}

const char* cgGetParameterGpuSemantic(CGparameter p)
{
	CGprogram program = cgGetParameterProgram(p);
	const char* programString = cgGetProgramString(program, CG_COMPILED_PROGRAM);

	const char* semantic = cgGetParameterSemantic(p);

	char semanticString[256];
	ueStrFormatS(semanticString, "$vin.%s : ", semantic);

	const char* semanticStart = strstr(programString, semanticString);
	if (!semanticStart)
		return "<NONE>";
	const char* attrStart = semanticStart + strlen(semanticString);
	if (*attrStart == '$') attrStart++;
	const char* attrEnd = strstr(attrStart, " ");

	static char buffer[256];
	memcpy(buffer, attrStart, attrEnd - attrStart);
	buffer[attrEnd - attrStart] = 0;

	return buffer;
}

bool GetSemanticAndUsageFromString(glSemantic& semantic, u32& usageIndex, const char* semanticFullName, const char* semanticName, glSemantic candidateSemantic)
{
	const u32 semanticNameLength = ueStrLen(semanticName);

	// Exact match? - default to usage 0

	if (!ueStrCmp(semanticFullName, semanticName))
	{
		semantic = candidateSemantic;
		usageIndex = 0;
		return true;
	}

	// Prefix match? - figure out the usage from the following number

	if (ueStrNCmp(semanticFullName, semanticName, semanticNameLength)) return false;

	if (!ueStrToU32(semanticFullName + semanticNameLength, usageIndex)) return false;
	if (usageIndex > 32) return false;

	semantic = candidateSemantic;

	return true;
}

bool CompileShader(ueToolShader& shader, const ueToolShader::CompileSettings& settings, const ueAssetParams& params)
{
	// Recognize shader type

	bool isGLES = false;

	if (settings.m_profile == "glslf")
	{
		s_cgType = CgType_OpenGL;
		shader.m_shaderType = glShaderType_Fragment;
	}
	else if (settings.m_profile == "glslv")
	{
		s_cgType = CgType_OpenGL;
		shader.m_shaderType = glShaderType_Vertex;
	}
	else if (settings.m_profile == "glslesf")
	{
		isGLES = true;
		s_cgType = CgType_OpenGL;
		shader.m_shaderType = glShaderType_Fragment;
	}
	else if (settings.m_profile == "glslesv")
	{
		isGLES = true;
		s_cgType = CgType_OpenGL;
		shader.m_shaderType = glShaderType_Vertex;
	}
	else if (settings.m_profile == "ps_4_0")
	{
		s_cgType = CgType_D3D11;
		shader.m_shaderType = glShaderType_Fragment;
	}
	else if (settings.m_profile == "vs_4_0")
	{
		s_cgType = CgType_D3D11;
		shader.m_shaderType = glShaderType_Vertex;
	}
	else
	{
		ueLogE("Unsupported profile '%s' (the only supported ones are: glslf and glslv for OpenGL and d3d11_vs_4_0 and d3d11_ps_4_0 for D3D11)", settings.m_profile.c_str());
		return false;
	}

	// Minimal graphics API startup

	switch (s_cgType)
	{
	case CgType_OpenGL:
		ShaderCompiler_Cg_Init_OpenGL(isGLES);
		break;
	case CgType_D3D11:
		ShaderCompiler_Cg_Init_D3D11();
		break;
	}

	// Create Cg context

	s_context = cgCreateContext();
	if (!CheckForCgError("creating context")) return false;

	switch (s_cgType)
	{
	case CgType_OpenGL:
		ShaderCompiler_Cg_PostInit_OpenGL();
		break;
	case CgType_D3D11:
		ShaderCompiler_Cg_PostInit_D3D11();
		break;
	}

	// Get suitable profile

	CGprofile profile;
	switch (s_cgType)
	{
	case CgType_OpenGL:
		profile = ShaderCompiler_Cg_SelectProfile_OpenGL(shader.m_shaderType);
		break;
	case CgType_D3D11:
		profile = ShaderCompiler_Cg_SelectProfile_D3D11(shader.m_shaderType);
		break;
	}
	if (!CheckForCgError("Selecting profile")) return false;

	// Build macros

	const u32 numMacros = settings.m_macros.size();

	std::vector<std::string> macroStrings(numMacros);
	for (u32 i = 0; i < numMacros; i++)
	{
		const ueToolShader::Macro& macro = settings.m_macros[i];

		std::string macroString = "-D";
		macroString += macro.m_name.c_str();
		if (macro.m_value.length() > 0)
		{
			macroString += "=";
			macroString += macro.m_value.c_str();
		}

		macroStrings[i] = macroString;
	}

	std::vector<const char*> macroStringPtrs(numMacros + 1);
	for (u32 i = 0; i < numMacros; i++)
		macroStringPtrs[i] = macroStrings[i].c_str();
	macroStringPtrs[numMacros] = NULL;

	// Compile program

	CGprogram program =
		cgCreateProgramFromFile(
			s_context,						// Cg runtime context
			CG_SOURCE,						// Program in human-readable form
			settings.m_sourcePath.c_str(),	// Name of file containing program
			profile,						// Profile: OpenGL ARB vertex program
			settings.m_entry.c_str(),		// Entry function name
			&macroStringPtrs[0]);			// No extra compiler options
	if (!program)
	{
		std::string cgArgs;
		for (u32 i = 0; i < macroStringPtrs.size() - 1; i++)
		{
			cgArgs += macroStringPtrs[i];
			if (i < macroStringPtrs.size() - 2)
				cgArgs += " ";
		}

		const char* shaderTypeName = "???";
		switch (shader.m_shaderType)
		{
			case glShaderType_Vertex: shaderTypeName = "vertex"; break;
			case glShaderType_Fragment: shaderTypeName = "fragment"; break;
			case glShaderType_Geometry: shaderTypeName = "geometry"; break;
		}

		ueLogE("Failed to compile Cg %s shader (source = '%s'; entry = '%s'; Cg profile = '%s'; Cg args = '%s')",
			shaderTypeName, settings.m_sourcePath.c_str(), settings.m_entry.c_str(), settings.m_profile.c_str(), cgArgs.c_str());
		CheckForCgError("Creating program from file");
		return false;
	}

	// Retrieve constants information

	CGparameter param = cgGetFirstParameter(program, CG_GLOBAL);
	while (param)
	{
		if (!cgIsParameterUsed(param, program))
		{
			param = cgGetNextParameter(param);
			continue;
		}

		ueToolShader::ConstantInstance constant;
		constant.m_name = cgGetParameterName(param);
		constant.m_offset = 0; // This will be set at runtime (after linking shaders into program)

		const CGtype type = cgGetParameterType(param);

		switch (type)
		{
			case CG_ARRAY:
			{
				const u32 numArrayDims = cgGetArrayDimension(param);
				if (numArrayDims != 1)
				{
					ueLogE("Multidimensional arrays not supported (parameter name = '%s', dimensions = %u)", constant.m_name.c_str(), numArrayDims);
					return false;
				}

				constant.m_count = cgGetArraySize(param, 0);
				UE_ASSERT(constant.m_count);

				const CGtype arrayType = cgGetArrayType(param);
				switch (arrayType)
				{
					case CG_FLOAT:
					case CG_FLOAT1:
						constant.m_type = glConstantType_Float; break;
					case CG_FLOAT2: constant.m_type = glConstantType_Float2; break;
					case CG_FLOAT3: constant.m_type = glConstantType_Float3; break;
					case CG_FLOAT4: constant.m_type = glConstantType_Float4; break;
					case CG_FLOAT3x3: constant.m_type = glConstantType_Float3x3; break;
					case CG_FLOAT4x3: constant.m_type = glConstantType_Float4x3; break;
					case CG_FLOAT4x4: constant.m_type = glConstantType_Float4x4; break;
					default:
					{
						ueLogE("Unsupported type for array parameter (name = %s; type = %s)", constant.m_name.c_str(), cgGetTypeString(arrayType));
						break;
					}
				}

				shader.m_constants.push_back(constant);
				break;
			}
			case CG_FLOAT:
			case CG_FLOAT1:
			case CG_FLOAT2:
			case CG_FLOAT3:
			case CG_FLOAT4:
			case CG_FLOAT3x3:
			case CG_FLOAT4x3:
			case CG_FLOAT4x4:
			{
				switch (type)
				{
					case CG_FLOAT:
					case CG_FLOAT1:
						constant.m_type = glConstantType_Float; break;
					case CG_FLOAT2: constant.m_type = glConstantType_Float2; break;
					case CG_FLOAT3: constant.m_type = glConstantType_Float3; break;
					case CG_FLOAT4: constant.m_type = glConstantType_Float4; break;
					case CG_FLOAT3x3: constant.m_type = glConstantType_Float3x3; break;
					case CG_FLOAT4x3: constant.m_type = glConstantType_Float4x3; break;
					case CG_FLOAT4x4: constant.m_type = glConstantType_Float4x4; break;
				}
				constant.m_count = 1;

				shader.m_constants.push_back(constant);
				break;
			}

			case CG_SAMPLER1D:
				constant.m_type = glConstantType_Sampler1D;
				constant.m_count = 1;
				shader.m_constants.push_back(constant);
				break;
			case CG_SAMPLER2D:
				constant.m_type = glConstantType_Sampler2D;
				constant.m_count = 1;
				shader.m_constants.push_back(constant);
				break;
			case CG_SAMPLER3D:
				constant.m_type = glConstantType_Sampler3D;
				constant.m_count = 1;
				shader.m_constants.push_back(constant);
				break;
			case CG_SAMPLERCUBE:
				constant.m_type = glConstantType_SamplerCube;
				constant.m_count = 1;
				shader.m_constants.push_back(constant);
				break;
			default:
				ueLogE("Unsupported shader constant type (type = %s; name = '%s')", cgGetTypeString(type), constant.m_name.c_str());
				return false;
		}

		param = cgGetNextParameter(param);
	}

	// Extract output count and input attributes if it's vertex shader

	std::vector<std::string> semanticNames;

	shader.m_numOutputs = 0;

	for (int i = 0; i < 2; i++)
	{
		if (shader.m_shaderType == glShaderType_Vertex && i > 0)
			break;

		param = cgGetFirstParameter(program, i == 0 ? CG_PROGRAM : CG_GLOBAL);
		while (param)
		{
			// Skip unused

			if (!cgIsParameterUsed(param, program))
			{
				param = cgGetNextParameter(param);
				continue;
			}

			// Check direction

			const CGenum dir = cgGetParameterDirection(param);
			if (dir == CG_OUT)
			{
				shader.m_numOutputs++;
				param = cgGetNextParameter(param);
				continue;
			}
			else if (dir != CG_IN)
			{
				ueLogE("Unsupported parameter direction (name = %s; dir = %u)", cgGetParameterName(param), (u32) dir);
				return false;
			}

			// Only add attributes for vertex shader (needed to match vertex streams with vertex streams at run-time)

			if (shader.m_shaderType != glShaderType_Vertex)
			{
				param = cgGetNextParameter(param);
				continue;
			}

			// Check type

			const CGtype type = cgGetParameterType(param);
			switch (type)
			{
				case CG_INT:
				case CG_INT1:
				case CG_INT2:
				case CG_INT3:
				case CG_INT4:
				case CG_FLOAT:
				case CG_FLOAT1:
				case CG_FLOAT2:
				case CG_FLOAT3:
				case CG_FLOAT4:
					break;
				default:
				{
					ueLogE("Unsupported vertex shader input type (parameter name = %s; type = %s)", cgGetParameterName(param), cgGetTypeString(type));
					return false;
				}
			}

			// Figure out semantic - this isn't used at runtime, it's just for verification purposes
			// That's because the actual semantic is determined at runtime after linking shaders into program

			const char* semanticFullName = cgGetParameterSemantic(param);

			glSemantic semantic;
			u32 usageIndex;

			bool semanticFound = false;
			switch (s_cgType)
			{
			case CgType_OpenGL:
				semanticFound = ShaderCompiler_Cg_GetSemanticAndUsageFromString_OpenGL(semantic, usageIndex, semanticFullName);
				break;
			case CgType_D3D11:
				semanticFound = ShaderCompiler_Cg_GetSemanticAndUsageFromString_D3D11(semantic, usageIndex, semanticFullName);
				break;
			}

			if (!semanticFound)
			{
				ueLogE("Unsupported vertex shader semantic (param name = %s; semantic = %s)", cgGetParameterName(param), semanticFullName);
				return false;
			}

			semanticNames.push_back(semanticFullName);

#if 0 // This isn't useful at all

			// Figure out GPU semantic

			const char* gpuSemanticFullName = cgGetParameterGpuSemantic(param);

			glSemantic gpuSemantic;
			u32 gpuUsageIndex;
			if (!GetSemanticAndUsageFromString(gpuSemantic, gpuUsageIndex, gpuSemanticFullName))
			{
				ueLogE("Unsupported vertex shader GPU semantic (param name = %s; semantic = %s)", cgGetParameterName(param), gpuSemanticFullName);
				return false;
			}

#endif
			// Add attribute

			glShaderAttr attribute;
			attribute.m_semantic = (u8) semantic;
			attribute.m_usageIndex = (u8) usageIndex;

			shader.m_inputs.push_back(attribute);

			param = cgGetNextParameter(param);
		}
	}

	// Get compiler program buffer

	const char* programString = cgGetProgramString(program, CG_COMPILED_PROGRAM);

	// Preprocess program string

	const char* preprocessedProgramString = NULL;
	switch (s_cgType)
	{
	case CgType_OpenGL:
		preprocessedProgramString = ShaderCompiler_Cg_PreprocessProgram_OpenGL(shader.m_shaderType, semanticNames, programString);
		break;
	case CgType_D3D11:
		preprocessedProgramString = ShaderCompiler_Cg_PreprocessProgram_D3D11(shader.m_shaderType, semanticNames, programString);
		break;
	}

	shader.m_buffer = (void*) preprocessedProgramString;
	return true;
}

bool ueToolShader::SerializePlatformSpecific(ioSegmentWriter* sw)
{
	const u32 codeSize = ueStrLen((const char*) m_buffer) + 1;

	// Remainder of glShaderDesc

	sw->WriteNumber<u32>(codeSize);			// u32 m_codeSize
	ioPtr codePtr = sw->WritePtr();			// void* m_code

	// Code

	sw->BeginPtr(codePtr);
	sw->WriteData(m_buffer, codeSize);

	return true;
}

void ueToolShader::DumpSettingsPlatformSpecific()
{
}
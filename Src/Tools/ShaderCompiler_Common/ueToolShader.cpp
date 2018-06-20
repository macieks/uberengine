#include "IO/ioPackageWriter.h"
#include "ContentPipeline/ueContentPipeline.h"
#include "ShaderCompiler_Common/ueToolShader.h"

void ueToolShader::DumpSettings()
{
	DumpSettingHeader();
	DumpSettingString("source", 0, "Source shader file name relative to root source directory ('root_src'), e.g. 'my_shaders/simple_shader.hlsl'");
	DumpSettingString("profile", 0, "Profile name, e.g. 'glslf', 'glslv', 'ps_3_0', 'vs_3_0'");
	DumpSettingString("macros", 0, "String containing ';' separated macros to be defined during shader compilation, e.g. 'SKINNING=TRUE;NO_SPECULAR;NO_FOG'");
	DumpSettingString("entry", 0, "Name of the shader entry function, e.g. 'main' or 'my_vertex_shader'");

	DumpSettingsPlatformSpecific();
}

bool ueToolShader::ParseCompileSettings(CompileSettings& settings, ueAssetParams& params)
{
	settings.m_sourcePath = params.GetParam("source");
	settings.m_profile = params.GetParam("profile");

	// Create list of macros

	const std::string macrosParam = params.GetParam("macros");
	std::vector<std::string> macrosStrings;
	string_split(macrosStrings, macrosParam, ";");
	for (u32 i = 0; i < macrosStrings.size(); i++)
	{
		std::vector<std::string> nameAndValue;
		string_split(nameAndValue, macrosStrings[i], "=");

		Macro macro;
		macro.m_name = nameAndValue[0].c_str();
		macro.m_value = nameAndValue.size() == 2 ? nameAndValue[1].c_str() : "";
		settings.m_macros.push_back(macro);
	}

	settings.m_entry = params.GetParam("entry");

	return true;
}

bool ueToolShader::Serialize(ioPackageWriter* pw)
{
	ioSegmentParams segmentParams;
	segmentParams.m_symbol = UE_BE_4BYTE_SYMBOL('s','h','d','r');
	ioSegmentWriter sw;
	pw->BeginSegment(sw, segmentParams);

	// glShaderDesc

	sw.WriteNumber<u32>(m_shaderType);				// glShaderType m_type

	sw.WriteNumber<u32>((u32) m_constants.size());  // u32 m_numConstants
	ioPtr constantsPtr = sw.WritePtr();				// glShaderConstantInstanceDesc* m_constants

	ioPtr inputDescPtr;
	if (m_shaderType == glShaderType_Vertex)
		inputDescPtr = sw.WritePtr();					// glVertexShaderInputDesc* m_VSInputDesc
	else
		sw.WriteNullPtr();

	sw.WriteNumber<u32>(m_numOutputs);				// u32 m_numOutputs

	// Write platform specific data

	if (!SerializePlatformSpecific(&sw))
		return false;

	// Array of glShaderConstantInstanceDesc

	sw.BeginPtr(constantsPtr, 4);
	for (u32 i = 0; i < m_constants.size(); i++)
	{
		const ConstantInstance& inst = m_constants[i];

		// glShaderConstantDesc m_desc

		sw.WritePooledString(inst.m_name.c_str());		// const char* m_name
		sw.WriteNumber<u32>(inst.m_type);				// u32 m_type
		sw.WriteNumber<u32>(inst.m_count);				// u32 m_count

		sw.WriteNumber<u32>(inst.m_offset);				// u32 m_offset
		sw.WriteBool(inst.m_isColumnMajor);				// ueBool m_isColumnMajor
		sw.WriteAlign(4);
	}

	// glVertexShaderInputDesc

	if (m_shaderType == glShaderType_Vertex)
	{
		sw.BeginPtr(inputDescPtr);

		sw.WriteNumber<u32>((u32) m_inputs.size());	// u32 m_numAttrs
		ioPtr inputsPtr = sw.WritePtr();			// glShaderAttr* m_attrs

		// Array of glShaderAttr

		sw.BeginPtr(inputsPtr, 4);
		for (u32 i = 0; i < m_inputs.size(); i++)
		{
			const glShaderAttr& attr = m_inputs[i];
			sw.WriteNumber<u8>(attr.m_semantic);	// u8 m_semantic
			sw.WriteNumber<u8>(attr.m_usageIndex);	// u8 m_usageIndex
		}
	}

	return true;
}
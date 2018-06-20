#pragma once

#include "ContentPipeline/ueContentPipeline.h"
#include "GL/glStructs_Shared.h" // We need just a header
#include "IO/ioPackageWriter.h"

struct ueToolShader
{
	struct Macro
	{
		std::string m_name;
		std::string m_value;
	};

	struct CompileSettings
	{
		std::string m_sourcePath;
		std::string m_profile;
		std::vector<Macro> m_macros;
		std::string m_entry;
	};

	// Corresponds with glShaderConstantInstance
	struct ConstantInstance
	{
		std::string m_name;
		glConstantType m_type;
		u32 m_count;

		u32 m_offset;
		bool m_isColumnMajor;
	};

	struct ConstantBuffer
	{
		u32 m_size;
	};

	std::string m_name;
	std::string m_sourcePath;

	glShaderType m_shaderType;
	std::vector<ConstantInstance> m_constants;
	std::vector<ConstantBuffer> m_buffers;
	std::vector<glShaderAttr> m_inputs;
	u32 m_numOutputs;

	// Platform / configuration specific buffer

	void* m_buffer;

	bool Serialize(ioPackageWriter* pw);
	bool SerializePlatformSpecific(ioSegmentWriter* sw);

	static void DumpSettings();
	static void DumpSettingsPlatformSpecific();
	static bool ParseCompileSettings(CompileSettings& settings, ueAssetParams& params);
};
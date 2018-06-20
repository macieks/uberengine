#include "ShaderCompiler_Common/ueToolShader.h"
#include "d3dx9.h"

bool ueToolShader::SerializePlatformSpecific(ioSegmentWriter* sw)
{
	ID3DXBuffer* d3dBuffer = (ID3DXBuffer*) m_buffer;
	const u32 codeSize = (u32) d3dBuffer->GetBufferSize();

	// Remainder of glShaderDesc

	sw->WriteNumber<u32>(codeSize);			// u32 m_codeSize
	ioPtr codePtr = sw->WritePtr();			// void* m_code

	// Code

	sw->BeginPtr(codePtr);
	sw->WriteData(d3dBuffer->GetBufferPointer(), codeSize);

	return true;
}

void ueToolShader::DumpSettingsPlatformSpecific()
{
}
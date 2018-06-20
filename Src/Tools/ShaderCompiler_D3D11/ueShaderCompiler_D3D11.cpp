#include "ShaderCompiler_Common/ueToolShader.h"

#include <d3dx11.h>
#include <d3dcompiler.h>

ueBool glUtils_FromD3DSemanticName(glSemantic& semantic, const char* name)
{
	if (!ueStrCmp(name, "POSITION")) semantic = glSemantic_Position;
	else if (!ueStrCmp(name, "BLENDWEIGHT")) semantic = glSemantic_BoneWeights;
	else if (!ueStrCmp(name, "BLENDINDICES")) semantic = glSemantic_BoneIndices;
	else if (!ueStrCmp(name, "NORMAL")) semantic = glSemantic_Normal;
	else if (!ueStrCmp(name, "TEXCOORD")) semantic = glSemantic_TexCoord;
	else if (!ueStrCmp(name, "TANGENT")) semantic = glSemantic_Tangent;
	else if (!ueStrCmp(name, "BINORMAL")) semantic = glSemantic_Binormal;
	else if (!ueStrCmp(name, "COLOR")) semantic = glSemantic_Color;
	else if (!ueStrCmp(name, "DEPTH")) semantic = glSemantic_Depth;
	else if (!ueStrCmp(name, "SAMPLE")) semantic = glSemantic_Sample;
	else
		return UE_FALSE;
	return UE_TRUE;
}

bool CompileShader(ueToolShader& shader, const ueToolShader::CompileSettings& settings, const ueAssetParams& params)
{
	if (settings.m_profile == "ps_4_0" ||
		settings.m_profile == "ps_5_0")
		shader.m_shaderType = glShaderType_Fragment;
	else if (settings.m_profile == "vs_4_0" ||
		settings.m_profile == "vs_5_0")
		shader.m_shaderType = glShaderType_Vertex;
	else if (settings.m_profile == "gs_4_0" ||
		settings.m_profile == "gs_5_0")
		shader.m_shaderType = glShaderType_Geometry;
	else
	{
		ueLogE("Unsupported profile '%s'", settings.m_profile.c_str());
		return false;
	}

	// Create list of macros

	std::vector<D3D10_SHADER_MACRO> macros;
	for (u32 i = 0; i < settings.m_macros.size(); i++)
	{
		D3D10_SHADER_MACRO macro;
		macro.Name = settings.m_macros[i].m_name.c_str();
		macro.Definition = settings.m_macros[i].m_value.c_str();
		macros.push_back(macro);
	}

	D3D10_SHADER_MACRO endMacro;
	ueMemSet(&endMacro, 0, sizeof(D3D10_SHADER_MACRO));
	macros.push_back(endMacro);

	// Compile the shader

	UINT compileFlags = 0;

	bool enableDX9Compatibility;
	if (params.GetBoolParam("enableDX9Compatibility", enableDX9Compatibility) && enableDX9Compatibility)
		compileFlags |= D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY;

	HRESULT hr;

	ID3DBlob* shaderBlob;
	ID3DBlob* errorBlob;

	hr = D3DX11CompileFromFile(
		settings.m_sourcePath.c_str(),
		&macros[0],
		NULL,
		settings.m_entry.c_str(),
		settings.m_profile.c_str(),
		compileFlags, // Flags1
		0, // Flags2
		NULL, // ThreadPump
		&shaderBlob,
		&errorBlob,
		NULL);
	if (FAILED(hr))
	{
		ueLogE("Failed to compile shader code (path = '%s'), reason:\n%s", shader.m_sourcePath.c_str(), errorBlob ? (const char*) errorBlob->GetBufferPointer() : "Unknown (make sure source file exists)");
		return false;
	}

	shader.m_buffer = shaderBlob;

	// Get shader reflection interface

	ID3D11ShaderReflection* reflection;
	hr = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &reflection);
	if (FAILED(hr))
	{
		ueLogE("Failed to get D3D11 shader reflection interface, hr = 0x%x", hr);
		return false;
	}

	D3D11_SHADER_DESC desc;
	hr = reflection->GetDesc(&desc);
	if (FAILED(hr))
	{
		ueLogE("Failed to get D3D11 shader description, hr = 0x%x", hr);
		return false;
	}

	// Process constant buffers

	for (u32 i = 0; i < desc.ConstantBuffers; i++)
	{
		ID3D11ShaderReflectionConstantBuffer* buffer = reflection->GetConstantBufferByIndex(i);

		// Get buffer description

		D3D11_SHADER_BUFFER_DESC bufferDesc;
		hr = buffer->GetDesc(&bufferDesc);
		if (FAILED(hr))
		{
			ueLogE("Failed to get D3D11 shader buffer description, hr = 0x%x", hr);
			return false;
		}

		// Add buffer info

		ueToolShader::ConstantBuffer info;
		info.m_size = bufferDesc.Size;
		
		shader.m_buffers.push_back(info);

		// Get shader constants

		for (u32 j = 0; j < bufferDesc.Variables; j++)
		{
			ID3D11ShaderReflectionVariable* var = buffer->GetVariableByIndex(j);

			D3D11_SHADER_VARIABLE_DESC varDesc;
			hr = var->GetDesc(&varDesc);
			if (FAILED(hr))
			{
				ueLogE("Failed to get D3D11 shader buffer variable description, hr = 0x%x", hr);
				return false;
			}

			ueToolShader::ConstantInstance constant;
			constant.m_name = varDesc.Name;
			constant.m_offset = /* Buffer index */ (i << 16) | /* Offset within buffer */ varDesc.StartOffset;

			ID3D11ShaderReflectionType* varType = var->GetType();

			D3D11_SHADER_TYPE_DESC varTypeDesc;
			hr = varType->GetDesc(&varTypeDesc);
			if (FAILED(hr))
			{
				ueLogE("Failed to get D3D11 shader buffer variable type description, hr = 0x%x", hr);
				return false;
			}

			constant.m_count = varTypeDesc.Elements == 0 ? 1 : varTypeDesc.Elements;

			switch (varTypeDesc.Type)
			{
			case D3D_SVT_BOOL:
				if (varTypeDesc.Class != D3D_SVC_SCALAR)
					return false;
				constant.m_type = glConstantType_Bool;
				break;
			case D3D_SVT_INT:
				if (varTypeDesc.Class != D3D_SVC_SCALAR)
					return false;
				constant.m_type = glConstantType_Int;
				break;
			case D3D_SVT_FLOAT:
				constant.m_isColumnMajor = varTypeDesc.Class == D3D_SVC_MATRIX_COLUMNS;

				switch (varTypeDesc.Class)
				{
				case D3D_SVC_SCALAR:
					constant.m_type = glConstantType_Float;
					break;
				case D3D_SVC_VECTOR:
					switch (varTypeDesc.Columns)
					{
						case 2: constant.m_type = glConstantType_Float2; break;
						case 3: constant.m_type = glConstantType_Float3; break;
						case 4: constant.m_type = glConstantType_Float4; break;
						default: return false;
					}
					break;
				case D3D_SVC_MATRIX_COLUMNS:
				case D3D_SVC_MATRIX_ROWS:
					if (varTypeDesc.Rows == 4 && varTypeDesc.Columns == 4)
						constant.m_type = glConstantType_Float4x4;
					else if (varTypeDesc.Rows == 4 && varTypeDesc.Columns == 3)
						constant.m_type = glConstantType_Float4x3;
					else
						return false;
					break;
				default:
					return false;
				}
				break;
			default:
				ueLogE("Unsupported shader constant type (constant name = '%s'; shader path= '%s')", constant.m_name, shader.m_sourcePath.c_str());
				return false;
			}

			shader.m_constants.push_back(constant);
		}
	}

	// Get resource bindings

	for (u32 i = 0; i < desc.BoundResources; i++)
	{
		D3D11_SHADER_INPUT_BIND_DESC siDesc;
		hr = reflection->GetResourceBindingDesc(i, &siDesc);
		if (FAILED(hr))
		{
			ueLogE("Failed to get D3D11 shader buffer variable type description, hr = 0x%x", hr);
			return false;
		}

		ueToolShader::ConstantInstance constant;
		constant.m_name = siDesc.Name;
		constant.m_offset = siDesc.BindPoint;
		constant.m_count = siDesc.BindCount;

		switch (siDesc.Type)
		{
		case D3D_SIT_CBUFFER:
		case D3D_SIT_SAMPLER:
			continue;
		case D3D_SIT_TEXTURE:
			switch (siDesc.Dimension)
			{
			case D3D_SRV_DIMENSION_TEXTURE2D:
				constant.m_type = glConstantType_Sampler2D;
				break;
			case D3D_SRV_DIMENSION_TEXTURE3D:
				constant.m_type = glConstantType_Sampler3D;
				break;
			case D3D_SRV_DIMENSION_TEXTURECUBE:
				constant.m_type = glConstantType_SamplerCube;
				break;
			default:
				ueLogE("Unsupported shader texture type (name = '%s'; shader path= '%s')", siDesc.Name, shader.m_sourcePath.c_str());
				return false;
			}
			break;
		default:
			ueLogE("Unsupported shader resource binding type (name = '%s'; shader path= '%s')", siDesc.Name, shader.m_sourcePath.c_str());
			return false;
		}

		shader.m_constants.push_back(constant);
	}

	// Extract input attributes if it's vertex shader

	if (shader.m_shaderType == glShaderType_Vertex)
		for (u32 i = 0; i < desc.InputParameters; i++)
		{
			D3D11_SIGNATURE_PARAMETER_DESC inputDesc;
			hr = reflection->GetInputParameterDesc(i, &inputDesc);
			if (FAILED(hr))
			{
				ueLogE("Failed to get D3D11 shader input parameter description, hr = 0x%x", hr);
				return false;
			}

			glSemantic semantic;
			if (!glUtils_FromD3DSemanticName(semantic, inputDesc.SemanticName))
			{
				ueLogE("Shader constant has unsupported usage (D3D semantic name = %s)", inputDesc.SemanticName);
				return false;
			}
			glShaderAttr attribute;
			attribute.m_semantic = semantic;
			attribute.m_usageIndex = inputDesc.SemanticIndex;

			shader.m_inputs.push_back(attribute);
		}

	// Get number of output semantics

	shader.m_numOutputs = desc.OutputParameters;

	return true;
}

bool ueToolShader::SerializePlatformSpecific(ioSegmentWriter* sw)
{
	ID3DBlob* shaderBlob = (ID3DBlob*) m_buffer;
	const u32 codeSize = (u32) shaderBlob->GetBufferSize();

	// Remainder of glShaderDesc

	sw->WriteNumber<u32>(codeSize);			// u32 m_codeSize
	ioPtr codePtr = sw->WritePtr();			// void* m_code

	sw->WriteNumber<u32>((u32) m_buffers.size());	// u32 m_numBuffers
	ioPtr buffersPtr = sw->WritePtr();				// BufferDesc* m_bufferDescs

	// Code

	sw->BeginPtr(codePtr, 4);
	sw->WriteData(shaderBlob->GetBufferPointer(), codeSize);

	// Array of glShaderDesc::BufferDesc

	sw->BeginPtr(buffersPtr, 4);
	for (u32 i = 0; i < m_buffers.size(); i++)
	{
		const ConstantBuffer& info = m_buffers[i];
		sw->WriteNumber<u32>(info.m_size);
	}

	return true;
}

void ueToolShader::DumpSettingsPlatformSpecific()
{
	DumpSettingBool("enableDX9Compatibility", Setting_Optional, "Enables DirectX 9 shader model compatibility");
}
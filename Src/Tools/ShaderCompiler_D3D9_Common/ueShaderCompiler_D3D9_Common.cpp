#include "ShaderCompiler_Common/ueToolShader.h"
#include "d3dx9.h"

template <class IUnknownSubclass>
class IUnknownDisposer
{
protected:
	IUnknownSubclass* m_unknown;
public:
	IUnknownDisposer(IUnknownSubclass* unknown = NULL) : m_unknown(unknown) {}
	~IUnknownDisposer() { if (m_unknown) m_unknown->Release(); }
	operator IUnknownSubclass* () const { return m_unknown; }
	IUnknownSubclass* operator -> () const { return m_unknown; }
	void operator = (IUnknownSubclass* unknown) { if (m_unknown) m_unknown->Release(); m_unknown = unknown; }
};

// Implemented similarly to GL/glUtils.cpp but with the difference that we can see all Win32 & X360 semantics at the same time
UE_INLINE D3DDECLUSAGE glUtils_ToD3DDECLUSAGE(glSemantic semantic)
{
	switch (semantic)
	{
		case glSemantic_Position: return D3DDECLUSAGE_POSITION;
		case glSemantic_BoneWeights: return D3DDECLUSAGE_BLENDWEIGHT;
		case glSemantic_BoneIndices: return D3DDECLUSAGE_BLENDINDICES;
		case glSemantic_Normal: return D3DDECLUSAGE_NORMAL;
		case glSemantic_PointSize: return D3DDECLUSAGE_PSIZE;
		case glSemantic_TexCoord: return D3DDECLUSAGE_TEXCOORD;
		case glSemantic_Tangent: return D3DDECLUSAGE_TANGENT;
		case glSemantic_Binormal: return D3DDECLUSAGE_BINORMAL;
		case glSemantic_TessFactor: return D3DDECLUSAGE_TESSFACTOR;
		case glSemantic_PositionT: return D3DDECLUSAGE_POSITIONT;
		case glSemantic_Color: return D3DDECLUSAGE_COLOR;
		case glSemantic_Fog: return D3DDECLUSAGE_FOG;
		case glSemantic_Depth: return D3DDECLUSAGE_DEPTH;
		case glSemantic_Sample: return D3DDECLUSAGE_SAMPLE;
		UE_INVALID_CASE(semantic)
	};
	return D3DDECLUSAGE_SAMPLE;
}

UE_INLINE ueBool glUtils_FromD3DDECLUSAGE(glSemantic& semantic, D3DDECLUSAGE d3dDeclUsage)
{
	for (u32 i = 0; i < glSemantic_MAX; i++)
		if (glUtils_ToD3DDECLUSAGE((glSemantic) i) == d3dDeclUsage)
		{
			semantic = (glSemantic) i;
			return UE_TRUE;
		}
	return UE_FALSE;
}

bool CompileShader(ueToolShader& shader, const ueToolShader::CompileSettings& settings, const ueAssetParams& params)
{
	if (settings.m_profile == "ps_3_0" ||
		settings.m_profile == "ps_2_0" ||
		settings.m_profile == "ps_1_4")
		shader.m_shaderType = glShaderType_Fragment;
	else if (settings.m_profile == "vs_3_0" ||
		settings.m_profile == "vs_2_0" ||
		settings.m_profile == "vs_1_1")
		shader.m_shaderType = glShaderType_Vertex;
	else
	{
		ueLogE("Unsupported profile '%s'", settings.m_profile.c_str());
		return false;
	}

	// Create list of macros

	std::vector<D3DXMACRO> macros;
	for (u32 i = 0; i < settings.m_macros.size(); i++)
	{
		D3DXMACRO macro;
		macro.Name = settings.m_macros[i].m_name.c_str();
		macro.Definition = settings.m_macros[i].m_value.c_str();
		macros.push_back(macro);
	}

	D3DXMACRO endMacro;
	ueMemSet(&endMacro, 0, sizeof(D3DXMACRO));
	macros.push_back(endMacro);

	// Compile the shader

	IUnknownDisposer<ID3DXConstantTable> constantsTable;
	IUnknownDisposer<ID3DXBuffer> errorMessage;
	ID3DXBuffer* d3dBuffer = NULL;

	HRESULT hr = 
		D3DXCompileShaderFromFile(
			settings.m_sourcePath.c_str(),
			&macros[0],
			NULL,
			settings.m_entry.c_str(),
			settings.m_profile.c_str(),
			0, // Flags
			&d3dBuffer,
			&(ID3DXBuffer*&) errorMessage,
			&(ID3DXConstantTable*&) constantsTable);
	if (FAILED(hr))
	{
		ueLogE("Failed to compile shader code (path = '%s'), reason:\n%s", shader.m_sourcePath.c_str(), errorMessage ? (const char*) errorMessage->GetBufferPointer() : "Unknown (make sure source file exists)");
		return false;
	}

	shader.m_buffer = d3dBuffer;

	// Retrieve constants information

	D3DXCONSTANTTABLE_DESC d3dConstantTableDesc;
	hr = constantsTable->GetDesc(&d3dConstantTableDesc);
	UE_ASSERT(SUCCEEDED(hr));

	// Extract constants

	for (u32 constantIndex = 0; constantIndex < d3dConstantTableDesc.Constants; ++constantIndex)
	{
		// Retrieve constant info

		D3DXHANDLE constantHandle = constantsTable->GetConstant(NULL, constantIndex);
		UE_ASSERT(constantHandle != NULL);

		D3DXCONSTANT_DESC constantsArray[128];
		u32 constantsArraySize = UE_ARRAY_SIZE(constantsArray);
		hr = constantsTable->GetConstantDesc(constantHandle, constantsArray, &constantsArraySize);
		UE_ASSERT(SUCCEEDED(hr));

		// Create constant info

		ueToolShader::ConstantInstance constant;
		constant.m_name = constantsArray[0].Name;

		switch (constantsArray[0].Type)
		{
		case D3DXPT_BOOL:
		case D3DXPT_INT:
			ueLogE("Unsupported shader constant type (int or bool; name = '%s')", constant.m_name.c_str());
			return false;

		case D3DXPT_FLOAT:
		{
			const u32 bytesPerElement = constantsArray[0].Rows * constantsArray[0].Columns * 4;
			switch (bytesPerElement)
			{
				case 4: constant.m_type = glConstantType_Float; break;
				case 8: constant.m_type = glConstantType_Float2; break;
				case 12: constant.m_type = glConstantType_Float3; break;
				case 16: constant.m_type = glConstantType_Float4; break;
				case 48:
					constant.m_isColumnMajor = (constantsArray[0].Class != D3DXPC_MATRIX_ROWS);
					constant.m_type = glConstantType_Float4x3;
					break;
				case 64:
					constant.m_isColumnMajor = (constantsArray[0].Class != D3DXPC_MATRIX_ROWS);
					constant.m_type = glConstantType_Float4x4;
					break;
				default:
					UE_ASSERT(0);
					return false;
			}

			constant.m_offset = constantsArray[0].RegisterIndex << 4;

			constant.m_count = constantsArray[0].Bytes / bytesPerElement;
			UE_ASSERT(constant.m_count == constantsArray[0].Elements);

			shader.m_constants.push_back(constant);
			break;
		}
		case D3DXPT_STRING:
			// Ignore strings
			continue;

		case D3DXPT_SAMPLER1D:
			constant.m_type = glConstantType_Sampler1D;
			constant.m_count = 1;
			constant.m_offset = constantsArray[0].RegisterIndex;
			shader.m_constants.push_back(constant);
			break;
		case D3DXPT_SAMPLER2D:
			constant.m_type = glConstantType_Sampler2D;
			constant.m_count = 1;
			constant.m_offset = constantsArray[0].RegisterIndex;
			shader.m_constants.push_back(constant);
			break;
		case D3DXPT_SAMPLER3D:
			constant.m_type = glConstantType_Sampler3D;
			constant.m_count = 1;
			constant.m_offset = constantsArray[0].RegisterIndex;
			shader.m_constants.push_back(constant);
			break;
		case D3DXPT_SAMPLERCUBE:
			constant.m_type = glConstantType_SamplerCube;
			constant.m_count = 1;
			constant.m_offset = constantsArray[0].RegisterIndex;
			shader.m_constants.push_back(constant);
			break;
		}
	}

	// Extract input attributes if it's vertex shader

	D3DXSEMANTIC semantics[32];
	UINT semanticsCount;

	if (shader.m_shaderType == glShaderType_Vertex)
	{
		semanticsCount = UE_ARRAY_SIZE(semantics);
		HRESULT hr = D3DXGetShaderInputSemantics((DWORD*) d3dBuffer->GetBufferPointer(), semantics, &semanticsCount);
		UE_ASSERT(SUCCEEDED(hr));

		for (u32 i = 0; i < semanticsCount; ++i)
		{
			glSemantic semantic;
			if (!glUtils_FromD3DDECLUSAGE(semantic, (D3DDECLUSAGE) semantics[i].Usage))
			{
				ueLogE("Shader constant has unsupported usage (D3DDECLUSAGE = %u)", (u32) semantics[i].Usage);
				return false;
			}
			glShaderAttr attribute;
			attribute.m_semantic = semantic;
			attribute.m_usageIndex = semantics[i].UsageIndex;

			shader.m_inputs.push_back(attribute);
		}
	}

	// Extract the number of output semantics

	semanticsCount = UE_ARRAY_SIZE(semantics);
	hr = D3DXGetShaderOutputSemantics((DWORD*) d3dBuffer->GetBufferPointer(), semantics, &semanticsCount);
	UE_ASSERT(SUCCEEDED(hr));

	shader.m_numOutputs = semanticsCount;

	return true;
}
#include "Graphics/glLib_Private.h"
#include "Base/Containers/ueHashSet.h"

struct glShaderConstantCalcHash
{
	u32 operator () (const glConstant& value) const
	{
		const u32 hashes[3] =
		{
			ueCalcHash(value.m_desc.m_name),
			ueCalcHash(value.m_desc.m_type),
			ueCalcHash(value.m_desc.m_count)
		};
		return ueCalcHash(hashes, sizeof(hashes));
	}
};

struct glShaderConstantCmp
{
	s32 operator () (const glConstant& a, const glConstant& b) const
	{
		if (a.m_desc.m_count != b.m_desc.m_count)
			return (s32) (a.m_desc.m_count - b.m_desc.m_count);
		if (a.m_desc.m_type != b.m_desc.m_type)
			return (s32) a.m_desc.m_type - (s32) b.m_desc.m_type;

		return ueStrCmp(a.m_desc.m_name, b.m_desc.m_name);
	}
};

struct glShaderConstantManagerData
{
	ueBool m_isInitialized;

	u32 m_numericConstantsSize;
	u32 m_numSamplerConstants;

	ueHashSet<glConstant, glShaderConstantCalcHash, glShaderConstantCmp> m_constants;

	u32 m_nameBufferSize;
	u32 m_nameBufferCapacity;
	char* m_nameBuffer;

	glShaderConstantManagerData() : m_isInitialized(UE_FALSE) {}
};

static glShaderConstantManagerData s_data;

void glShaderConstantMgr_Startup()
{
	UE_ASSERT(!s_data.m_isInitialized);

	const glDeviceStartupParams* params = glDevice_GetStartupParams();

	s_data.m_numericConstantsSize = 0;
	s_data.m_numSamplerConstants = 0;
	UE_ASSERT_FUNC(s_data.m_constants.Init(GL_STACK_ALLOC, params->m_maxGlobalNumericConstants + params->m_maxGlobalSamplerConstants));

	s_data.m_nameBuffer = (char*) GL_STACK_ALLOC->Alloc(params->m_constantNameBufferSize);
	UE_ASSERT(s_data.m_nameBuffer);
	s_data.m_nameBufferSize = 0;
	s_data.m_nameBufferCapacity = params->m_constantNameBufferSize;

	s_data.m_isInitialized = UE_TRUE;
}

void glShaderConstantMgr_Shutdown()
{
	UE_ASSERT(s_data.m_isInitialized);
	GL_STACK_ALLOC->Free(s_data.m_nameBuffer);
	s_data.m_constants.Deinit();
	s_data.m_isInitialized = UE_FALSE;
}

const glConstant* glShaderConstantMgr_GetConstant(const glShaderConstantDesc* desc)
{
	UE_ASSERT(s_data.m_isInitialized);

	// Search in existing constants

	glConstant constant;
	constant.m_desc = *desc;

	glConstant* constantPtr = s_data.m_constants.Find(constant);
	if (constantPtr)
		return constantPtr;

	// Not found - create new constant...

	// Store name in name buffer

	const u32 nameLength = ueStrLen(desc->m_name);
	UE_ASSERT(s_data.m_nameBufferSize + nameLength + 1 < s_data.m_nameBufferCapacity);
	char* name = (char*) s_data.m_nameBuffer + s_data.m_nameBufferSize;
	ueMemCpy(name, desc->m_name, nameLength + 1);
	s_data.m_nameBufferSize += nameLength + 1;

	constant.m_desc.m_name = name;

	// Assign location in cache

	const glDeviceStartupParams* params = glDevice_GetStartupParams();

	if (glUtils_IsSampler((glConstantType) desc->m_type))
	{
		UE_ASSERT(s_data.m_numSamplerConstants < params->m_maxGlobalSamplerConstants);
		constant.m_sampler.m_index = s_data.m_numSamplerConstants;
		s_data.m_numSamplerConstants++;
	}
	else
	{
		constant.m_numeric.m_cacheOffset = s_data.m_numericConstantsSize;
		constant.m_numeric.m_size = glUtils_GetShaderConstantSize((glConstantType) desc->m_type) * desc->m_count;
		s_data.m_numericConstantsSize = ueAlignPow2(s_data.m_numericConstantsSize + constant.m_numeric.m_size, (u32) 16);
		UE_ASSERT(s_data.m_numericConstantsSize <= params->m_maxGlobalNumericConstantsDataSize);
	}

	return s_data.m_constants.Insert(constant);
}

void glCtx_PrintShaderConstantValues(glCtx* ctx, glShaderType shaderType)
{
	if (!ctx->m_program)
	{
		ueLogI("No shader program bound");
		return;
	}

	const glShader* shader = ctx->m_program->m_shaders[shaderType];
	if (!shader)
	{
		ueLogI("No %s shader bound", glShaderType_ToString(shaderType));
		return;
	}

	ueLogI("Shader constants for %s shader [%u]:", glShaderType_ToString(shaderType), shader->m_numConstants);
	for (u32 i = 0; i < shader->m_numConstants; i++)
	{
		const glShaderConstantInstance& constantInstance = shader->m_constants[i];
		const glConstant& constant = *constantInstance.m_constant;
		const glConstantType constantType = (glConstantType) constant.m_desc.m_type;

		const u8* data = !glUtils_IsSampler(constantType) ? &ctx->m_numericShaderConstantsLocal[constant.m_numeric.m_cacheOffset] : NULL;
		const f32* f32Data = (const f32*) data;
		const s32* s32Data = (const s32*) data;
		const ueBool* boolData = (const ueBool*) data;

		u32 registerIndex = 0;
		if (glUtils_IsSampler(constantType)) registerIndex = constantInstance.m_offset;
		else if (glUtils_IsFloat(constantType) || glUtils_IsInt(constantType)) registerIndex = (constantInstance.m_offset + 15) >> 4;
		else if (glUtils_IsBool(constantType)) registerIndex = constantInstance.m_offset / sizeof(ueBool);

		char buffer[128];
		ueStrFormatS(buffer, " [%u] %s (reg = %u) %s =",
			i,
			glConstantType_ToString((glConstantType) constant.m_desc.m_type),
			registerIndex,
			constant.m_desc.m_name);

		switch (constant.m_desc.m_type)
		{
			case glConstantType_Sampler1D:
			case glConstantType_Sampler2D:
			case glConstantType_Sampler3D:
			case glConstantType_SamplerCube:
			{
				const glCtx::SamplerConstantInfo& samplerInfo = ctx->m_samplerShaderConstantsLocal[constant.m_sampler.m_index];
				glTextureBuffer* tb = samplerInfo.m_textureBuffer;
				const glTextureBufferDesc* tbDesc = tb ? glTextureBuffer_GetDesc(tb) : NULL;

				char samplerDescBuffer[64];
				if (!tb)
					ueStrCpyS(samplerDescBuffer, "<NONE>");
				else
					ueStrFormatS(samplerDescBuffer, "%s %ux%u %s mips:%u rt:%s", glTexType_ToString(tbDesc->m_type), tbDesc->m_width, tbDesc->m_height, glBufferFormat_ToString(tbDesc->m_format), tbDesc->m_numLevels, glTextureBuffer_IsRenderTarget(tb) ? "TES" : "NO");

				ueLogI("%s %s", buffer, samplerDescBuffer);
				break;
			}
			case glConstantType_Float: ueLogI("%s %f", buffer, f32Data[0]); break;
			case glConstantType_Float2: ueLogI("%s %f %f", buffer, f32Data[0], f32Data[1]); break;
			case glConstantType_Float3: ueLogI("%s %f %f %f", buffer, f32Data[0], f32Data[1], f32Data[2]); break;
			case glConstantType_Float4: ueLogI("%s %f %f %f %f", buffer, f32Data[0], f32Data[1], f32Data[2], f32Data[3]); break;
			case glConstantType_Float3x3: ueLogI("%s %f %f %f %f %f %f %f %f %f", buffer, f32Data[0], f32Data[1], f32Data[2], f32Data[3], f32Data[4], f32Data[5], f32Data[6], f32Data[7], f32Data[8], f32Data[9]); break;
			case glConstantType_Float4x3: ueLogI("%s %f %f %f %f %f %f %f %f %f %f %f %f", buffer, f32Data[0], f32Data[1], f32Data[2], f32Data[3], f32Data[4], f32Data[5], f32Data[6], f32Data[7], f32Data[8], f32Data[9], f32Data[10], f32Data[11]); break;
			case glConstantType_Float4x4: ueLogI("%s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", buffer, f32Data[0], f32Data[1], f32Data[2], f32Data[3], f32Data[4], f32Data[5], f32Data[6], f32Data[7], f32Data[8], f32Data[9], f32Data[10], f32Data[11], f32Data[12], f32Data[13], f32Data[14], f32Data[15]); break;
			case glConstantType_Bool: ueLogI("%s %s", buffer, ueStrFromBool(boolData[0])); break;
			case glConstantType_Bool2: ueLogI("%s %s %s", buffer, ueStrFromBool(boolData[0]), ueStrFromBool(boolData[1])); break;
			case glConstantType_Bool3: ueLogI("%s %s %s %s", buffer, ueStrFromBool(boolData[0]), ueStrFromBool(boolData[1]), ueStrFromBool(boolData[2])); break;
			case glConstantType_Bool4: ueLogI("%s %s %s %s %s", buffer, ueStrFromBool(boolData[0]), ueStrFromBool(boolData[1]), ueStrFromBool(boolData[2]), ueStrFromBool(boolData[3])); break;
			case glConstantType_Int: ueLogI("%s %d", buffer, s32Data[0]); break;
			case glConstantType_Int2: ueLogI("%s %d %d", buffer, s32Data[0], s32Data[1]); break;
			case glConstantType_Int3: ueLogI("%s %d %d %d", buffer, s32Data[0], s32Data[1], s32Data[2]); break;
			case glConstantType_Int4: ueLogI("%s %d %d %d %d", buffer, s32Data[0], s32Data[1], s32Data[2], s32Data[3]); break;
			default: UE_ASSERT_MSGP(0, "Unknown constant type (type = %u)", constant.m_desc.m_type);
		}
	}
}

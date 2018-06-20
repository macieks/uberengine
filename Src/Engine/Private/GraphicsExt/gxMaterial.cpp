#include "GraphicsExt/gxMaterial.h"
#include "Base/ueResource_Private.h"

#define GX_MAX_MATERIAL_PARAMS 64

//! Base material
struct gxMaterial : ueResource
{
};

//! Master material resource
struct gxMasterMaterial : gxMaterial
{
	ueHashSet<gxMaterialConfig>	m_configs;	//!< Configurations
	u32							m_numParams;//!< Number of parameters
	gxMaterialParam*			m_params;	//!< Params array (sorted by name)
};

//! Override material (overrides selected parameters within master material)
struct gxOverrideMaterial : gxMaterial
{
	ueResourceHandle<gxMasterMaterial> m_masterMaterial;	//!< Referenced master material
	u32 m_numParams;			//!< Number of overridden parameters
	gxMaterialParam* m_params;	//!< Overridden params array (sorted by name)
};

struct gxMaterialParamValue_Sampler_Internal : gxMaterialParamValue_Sampler
{
	ueResourceHandle<gxTexture> m_texture;
};

void gxMaterialRenderState_Set(glCtx* ctx, const gxMaterialRenderState* state)
{
	glCtx_SetDepthTest(ctx, state->m_enableDepthTest ? UE_TRUE : UE_FALSE);
	glCtx_SetDepthFunc(ctx, (glCmpFunc) state->m_depthTestFunc);

	glCtx_SetAlphaTest(ctx, state->m_enableAlphaTest ? UE_TRUE : UE_FALSE);
	glCtx_SetAlphaFunc(ctx, (glCmpFunc) state->m_alphaFunc, state->m_alphaRef);

	glCtx_SetBlending(ctx, state->m_enableBlending ? UE_TRUE : UE_FALSE);
	if (state->m_enableBlending)
	{
		glCtx_SetBlendOp(ctx, (glBlendOp) state->m_colorBlendOp, (glBlendOp) state->m_alphaBlendOp);
		glCtx_SetBlendFunc(ctx,
			(glBlendingFunc) state->m_colorSrcBlend, (glBlendingFunc) state->m_colorDstBlend,
			(glBlendingFunc) state->m_alphaSrcBlend, (glBlendingFunc) state->m_alphaDstBlend);
	}

	glCtx_SetCullMode(ctx, (glCullMode) state->m_cullMode);
}

ueBool gxMaterialInstance::PreRender(glCtx* ctx, u32 configId)
{
	static gxMaterialParam** nullParams

	// Get master material and parameters

	gxMasterMaterial* masterMaterial;
	gxMaterialParam* masterParams;
	gxMaterialParam* overrideParams = nullParams;

	if (ueResource_GetTypeId(m_material) == ueResourceType<gxMasterMaterial>::ID)
	{
		masterMaterial = static_cast<gxMasterMaterial*>(m_material);
		overrideParams = NULL;
	}
	else
	{
		overrideParams = static_cast<gxOverrideMaterial*>(m_material)->m_params;
		masterMaterial = *static_cast<gxOverrideMaterial*>(m_material)->m_masterMaterial;
	}

	masterParams = masterMaterial->m_params;

	// Get config

	gxMaterialConfig configKey;
	configKey.m_descriptionId = configId;
	gxMaterialConfig* config = masterMaterial->m_configs.Find(configKey);
	if (!config)
		return UE_FALSE;

	// Set render states

	gxMaterialRenderState_Set(ctx, &config->m_renderState);

	// Set program

	glCtx_SetProgram(ctx, config->m_program.GetProgram());

	// Set shader parameters for current config
	// Parameters' priority:
	//   first - material instance
	//	 then - override material
	//   then - master material

	for (u32 i = 0; i < masterMaterial->m_numParams; i++)
	{
		if (!(config->m_usedParamsMask & UE_POW2(i)))
			continue;

		// Get parameter

		gxMaterialParam* param = m_params[i] ? m_params[i] : ((overrideParams && overrideParams[i]) ? overrideParams[i] : &masterParams[i]);
		gxMaterialParamValue* value = param->m_type == gxMaterialParamType_Generated ? param->m_generator.m_func(param->m_generator.m_userData) : param->m_value;
		UE_ASSERT(value);

		// Set parameter value to shader constant

		const glConstantType constantType = param->m_handle.GetConstantType();
		switch (constantType)
		{
			case glConstantType_Bool:	glCtx_SetBoolConstant(ctx, param->m_handle, value->GetBool(), value->m_count); break;
			case glConstantType_Int:	glCtx_SetIntConstant(ctx, param->m_handle, value->GetS32(), value->m_count); break;
			case glConstantType_Float:	glCtx_SetFloatConstant(ctx, param->m_handle, value->GetF32(), value->m_count); break;
			case glConstantType_Float2:	glCtx_SetFloat2Constant(ctx, param->m_handle, value->GetF32(), value->m_count); break;
			case glConstantType_Float3:	glCtx_SetFloat3Constant(ctx, param->m_handle, value->GetF32(), value->m_count); break;
			case glConstantType_Float4:	glCtx_SetFloat4Constant(ctx, param->m_handle, value->GetF32(), value->m_count); break;
			case glConstantType_Float4x4:glCtx_SetFloat4x4Constant(ctx, param->m_handle, value->GetF32(), value->m_count); break;
			case glConstantType_Sampler1D:
			case glConstantType_Sampler2D:
			case glConstantType_Sampler3D:
			case glConstantType_SamplerCube:
			{
				gxMaterialParamValue_Sampler* sampler = (gxMaterialParamValue_Sampler*) value;
				glCtx_SetSamplerConstant(ctx, param->m_handle, sampler->m_textureBuffer, &sampler->m_sampler);
				break;
			}
			UE_INVALID_CASE(constantType);
		}
	}

	return UE_TRUE;
}
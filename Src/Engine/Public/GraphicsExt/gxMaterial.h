#ifndef GX_MATERIAL_H
#define GX_MATERIAL_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Base/ueResource.h"
#include "Base/Containers/ueHashSet.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxProgram.h"

UE_DEFINE_RESOURCE_TYPE(gxMasterMaterial, UE_BE_4BYTE_SYMBOL('m','t','r','l'));
UE_DEFINE_RESOURCE_TYPE(gxOverrideMaterial, UE_BE_4BYTE_SYMBOL('m','t','r','o'));

struct gxMaterialParamValue;

//! Material render state
struct gxMaterialRenderState
{
	u32 m_enableDepthTest		: 1;
	u32 m_depthTestFunc			: ueCalcBitsRequired<glCmpFunc_MAX>::VALUE;

	u32 m_enableAlphaTest		: 1;
	u32 m_alphaRef				: 8;
	u32 m_alphaFunc				: ueCalcBitsRequired<glCmpFunc_MAX>::VALUE;

	u32 m_enableBlending		: 1;
	u32 m_colorBlendOp			: ueCalcBitsRequired<glBlendOp_MAX>::VALUE;
	u32 m_colorSrcBlend			: ueCalcBitsRequired<glBlendingFunc_MAX>::VALUE;
	u32 m_colorDstBlend			: ueCalcBitsRequired<glBlendingFunc_MAX>::VALUE;
	u32 m_alphaBlendOp			: ueCalcBitsRequired<glBlendOp_MAX>::VALUE;
	u32 m_alphaSrcBlend			: ueCalcBitsRequired<glBlendingFunc_MAX>::VALUE;
	u32 m_alphaDstBlend			: ueCalcBitsRequired<glBlendingFunc_MAX>::VALUE;

	u32 m_cullMode				: ueCalcBitsRequired<glCullMode_MAX>::VALUE;
};

//! Material parameter value generator
typedef gxMaterialParamValue* (*gxMaterialParamGeneratorFunc)(void* userData);

//! Material parameter value
struct gxMaterialParamValue
{
	u32 m_count; //!< Elements count

	inline ueBool* GetBool() const { return (ueBool*) (this + 1); }
	inline s32* GetS32() const { return (s32*) (this + 1); }
	inline f32* GetF32() const { return (f32*) (this + 1); }
};

//! Sampler parameter
struct gxMaterialParamValue_Sampler : gxMaterialParamValue
{
	glTextureBuffer* m_textureBuffer;	//!< Texture buffer
	glSamplerParams m_sampler;			//!< Sampler parameters
};

//! Material parameter type
enum gxMaterialParamType
{
	gxMaterialParamType_Constant,	//!< For example SomeDiffuseConstant (may be the only difference between materials using same shaders); never changed on run-time; yes, we could avoid constant numerical params altogether if we compiled shader constants into the shader code, but this might increase shader count too much
	gxMaterialParamType_Custom,		//!< For example Redness (specific to surface); value is stored under m_value which points into user supplied location
	gxMaterialParamType_Generated,	//!< For example MovieSampler (may sample different texture every frame); generated via generator called every time parameter needs to be set

	gxMaterialParamType_MAX
};

struct gxMaterialParamGenerator
{
	gxMaterialParamGeneratorFunc m_func;	//!< Generator function
	void* m_userData;						//!< Generator user data
};

//! Generator or custom param initialization data (so, can for example automatically hook up video playback for a specific sampler)
struct gxMaterialParamInitData
{
	u32 m_typeSymbol; //!< Unique symbol of the param type

	// Parame type specific data follows here
};

//! Material parameter
struct gxMaterialParam
{
	gxMaterialParamType m_type;		//!< Parameter type
	glConstantHandle m_handle;		//!< Shader constant handle

	union
	{
		gxMaterialParamValue* m_value;			//!< Value of, constant or custom, parameter
		gxMaterialParamGenerator m_generator;	//!< Parameter generator
	};

	UE_INLINE const char* GetName() const { return m_handle.GetName(); }
	UE_INLINE glConstantType GetConstantType() const { return m_handle.GetType(); }
	UE_INLINE u32 GetCount() const { return m_handle.GetCount(); }
};

typedef void (*gxMaterialParamInitFunc)(gxMaterialParam* paramToInitialize, gxMaterialParamInitData* initData, void* userData);
typedef void (*gxMaterialParamDeinitFunc)(gxMaterialParam* paramToInitialize, void* userData);

//! Material configuration
struct gxMaterialConfig
{
	u32 m_descriptionId;			//!< Hash id calculated from configuration description i.e. gxMaterialConfigDesc
	u32 m_usedParamsMask;			//!< Bit mask indicating which parameters (gxMasterMaterial::m_params) are used
	gxProgram m_program;			//!< Shader program
	gxMaterialRenderState m_renderState; //!< Render state
};

//UE_DEFINE_RESOURCE_TYPE(gxMasterMaterial, UE_BE_4BYTE_SYMBOL('m','s','m','l'));
//UE_DEFINE_RESOURCE_TYPE(gxOverrideMaterial, UE_BE_4BYTE_SYMBOL('o','r','m','l'));

struct gxMaterial;

//! Material instance
class gxMaterialInstance
{
public:
	//! Gets parameter index by name; returns U32_MAX if not found
	u32 GetParamIndex(const char* name) const;

	//! Sets boolean paramater value
	void SetBool(u32 index, const ueBool* values, u32 count = 1);
	//! Sets integer paramater value
	void SetS32(u32 index, const s32* values, u32 count = 1);
	//! Sets float paramater value
	void SetF32(u32 index, const f32* values, u32 count = 1);
	//! Sets vector-2 paramater value
	void SetVec2(u32 index, const ueVec2* values, u32 count = 1);
	//! Sets vector-3 paramater value
	void SetVec3(u32 index, const ueVec3* values, u32 count = 1);
	//! Sets vector-4 paramater value
	void SetVec4(u32 index, const ueVec4* values, u32 count = 1);
	//! Sets 4x4 matrix paramater value
	void SetMat44(u32 index, const ueMat44* values, u32 count = 1);
	//! Sets 4x3 matrix paramater value
	void SetMat43(u32 index, const ueMat44* values, u32 count = 1);

	//! Sets external parameter value
	void SetExternalValue(u32 index, gxMaterialParamValue* value);
	//! Sets custom parameter value generator
	void SetGenerator(u32 index, gxMaterialParamGeneratorFunc func, void* userData);

private:
	ueBool PreRender(glCtx* ctx, u32 config);

	gxMaterial* m_material;		//!< Material reference (master or override)
	gxMaterialParam** m_params;	//!< Params array of length gxMasterMaterial::m_numParams
};

void gxMaterialSys_RegisterParamType(u32 typeSymbol, gxMaterialParamInitFunc initFunc, gxMaterialParamDeinitFunc deinitFunc, void* userData);
void gxMaterialSys_UnregisterParamType(u32 typeSymbol);

// @}

#endif // GX_MATERIAL_H

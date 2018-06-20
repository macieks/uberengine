#ifndef GX_MATERIAL_CONFIG_COMMON_H
#define GX_MATERIAL_CONFIG_COMMON_H

#include "Base/ueBase.h"

enum gxShadowMapType
{
	gxShadowMapType_Standard = 0,
	gxShadowMapType_PCF,
	gxShadowMapType_Variance,
	gxShadowMapType_Exponential,

	gxShadowMapType_MAX
};

UE_ENUM_BEGIN(gxShadowMapType)
UE_ENUM_VALUE(gxShadowMapType, Standard)
UE_ENUM_VALUE(gxShadowMapType, PCF)
UE_ENUM_VALUE(gxShadowMapType, Variance)
UE_ENUM_VALUE(gxShadowMapType, Exponential)
UE_ENUM_END(gxShadowMapType)

enum gxShadowMapWarping
{
	gxShadowMapWarping_Standard = 0,
	gxShadowMapWarping_Trapezoidal,
	gxShadowMapWarping_Perspective,
	gxShadowMapWarping_LightSpacePerspective,
	gxShadowMapWarping_Cube,
	gxShadowMapWarping_DualParaboidal,

	gxShadowMapWarping_MAX
};

UE_ENUM_BEGIN(gxShadowMapWarping)
UE_ENUM_VALUE(gxShadowMapWarping, Standard)
UE_ENUM_VALUE(gxShadowMapWarping, Trapezoidal)
UE_ENUM_VALUE(gxShadowMapWarping, Perspective)
UE_ENUM_VALUE(gxShadowMapWarping, LightSpacePerspective)
UE_ENUM_VALUE(gxShadowMapWarping, Cube)
UE_ENUM_VALUE(gxShadowMapWarping, DualParaboidal)
UE_ENUM_END(gxShadowMapWarping)

enum gxShadowMapSplitting
{
	gxShadowMapSplitting_None = 0,
	gxShadowMapSplitting_ViewAligned,
	gxShadowMapSplitting_Cascaded,

	gxShadowMapSplitting_MAX
};

UE_ENUM_BEGIN(gxShadowMapSplitting)
UE_ENUM_VALUE(gxShadowMapSplitting, None)
UE_ENUM_VALUE(gxShadowMapSplitting, ViewAligned)
UE_ENUM_VALUE(gxShadowMapSplitting, Cascaded)
UE_ENUM_END(gxShadowMapSplitting)

struct gxShadowMapDesc
{
	gxShadowMapType		m_type;
	gxShadowMapWarping	m_warping;
	gxShadowMapSplitting m_splitting;
	u8					m_numSplits;
	ueBool				m_hasFadeoutDistance;
};

enum gxLightMapType
{
	gxLightMapType_None = 0,
	gxLightMapType_Vertex,
	gxLightMapType_Texture,

	gxLightMapType_MAX
};

enum gxLightMapEncoding
{
	gxLightMapEncoding_Normal = 0,
	gxLightMapEncoding_TriNormal,
	gxLightMapEncoding_SphericalHarmonics,
	gxLightMapEncoding_HaarWavelets,

	gxLightMapEncoding_MAX
};

enum gxLightMapColor
{
	gxLightMapColor_Mono = 0,
	gxLightMapColor_Color,

	gxLightMapColor_MAX
};

struct gxLightMapDesc
{
	gxLightMapType		m_type;
	gxLightMapEncoding	m_encoding;
	gxLightMapColor		m_color;
};

enum gxLightType
{
	gxLightType_Ambient = 0,
	gxLightType_Omni,
	gxLightType_Directional,
	gxLightType_Spot,

	gxLightType_MAX
};

enum gxLightAttenuation
{
	gxLightAttenuation_None = 0,
	gxLightAttenuation_Linear,
	gxLightAttenuation_Quadratic,

	gxLightAttenuation_MAX
};

enum gxShadowType
{
	gxShadowType_None = 0,
	gxShadowType_ShadowMask,
	gxShadowType_ShadowMap,
	gxShadowType_ShadowVolumes,

	gxShadowType_MAX
};

struct gxLightDesc
{
	gxLightType		m_type;
	gxLightAttenuation	m_attenuation;
	ueBool			m_usesProjectionTexture;
	gxShadowType	m_shadowType;
	gxShadowMapDesc	m_shadowMap;
};

enum gxFogMode
{
	gxFogMode_None = 0,
	gxFogMode_Distance,
	gxFogMode_Height,

	gxFogMode_MAX
};

enum gxFogAttenuation
{
	gxFogAttenuation_None = 0,
	gxFogAttenuation_Linear,
	gxFogAttenuation_Quadratic,

	gxFogAttenuation_MAX
};

struct gxFogDesc
{
	gxFogMode			m_mode;
	gxFogAttenuation	m_attenuation;
};

struct gxSkinningDesc
{
	u8 m_numBones;			//!< 0 indicates no skinning; 1 indicates rigid skinning
	ueBool m_usesTexture;	//!< Only valid if m_numBones >= 1
};

enum gxToneMapping
{
	gxToneMapping_None = 0,
	gxToneMapping_Lookup3DTexture,

	gxToneMapping_MAX
};

enum gxColorEncoding
{
	gxColorEncoding_RGBA = 0,
	gxColorEncoding_RGBE,

	gxColorEncoding_MAX
};

struct gxMaterialConfigDesc
{
	u32 m_typeId;

	UE_INLINE gxMaterialConfigDesc(u32 typeId) : m_typeId(typeId) {}
	virtual u32 CalculateId() const = 0;
};

struct gxMaterialConfig_DepthPrepass : gxMaterialConfigDesc
{
	UE_INLINE gxMaterialConfig_DepthPrepass() : gxMaterialConfigDesc('dpre') {}
	virtual u32 CalculateId() const;
};

#endif // GX_MATERIAL_CONFIG_COMMON_H
#ifndef GX_MATERIAL_CONFIG_SHADOW_MAPPING_H
#define GX_MATERIAL_CONFIG_SHADOW_MAPPING_H

#include "GraphicsExt/gxMaterialConfig_Common.h"

struct gxMaterialConfig_ShadowMapping_DepthPass : gxMaterialConfigDesc
{
	gxShadowMapDesc m_shadowMap;
	gxSkinningDesc m_skinning;

	UE_INLINE gxMaterialConfig_ShadowMapping_DepthPass() : gxMaterialConfigDesc('smdp') {}
	virtual u32 CalculateId() const;
};

struct gxMaterialConfig_ShadowMapping_ShadowMaskPass : gxMaterialConfigDesc
{
	gxShadowMapDesc m_shadowMap;
	gxSkinningDesc m_skinning;

	UE_INLINE gxMaterialConfig_ShadowMapping_ShadowMaskPass() : gxMaterialConfigDesc('smsm') {}
	virtual u32 CalculateId() const;
};

#endif // GX_MATERIAL_CONFIG_SHADOW_MAPPING_H
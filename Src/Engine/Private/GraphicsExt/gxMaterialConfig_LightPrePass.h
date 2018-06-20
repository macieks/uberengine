#ifndef GX_MATERIAL_CONFIG_LIGHT_PRE_PASS_H
#define GX_MATERIAL_CONFIG_LIGHT_PRE_PASS_H

#include "GraphicsExt/gxMaterialConfig_Common.h"

struct gxMaterialConfig_LightPrePass_GeomPass : gxMaterialConfigDesc
{
	gxSkinningDesc m_skinning;

	UE_INLINE gxMaterialConfig_LightPrePass_GeomPass() : gxMaterialConfigDesc('lpgp') {}
	virtual u32 CalculateId() const;
};

struct gxMaterialConfig_LightPrePass_LightPass : gxMaterialConfigDesc
{
	gxLightDesc m_light;

	UE_INLINE gxMaterialConfig_LightPrePass_LightPass() : gxMaterialConfigDesc('lplp') {}
	virtual u32 CalculateId() const;
};

struct gxMaterialConfig_LightPrePass_CompositePass : gxMaterialConfigDesc
{
	gxSkinningDesc m_skinning;

	UE_INLINE gxMaterialConfig_LightPrePass_CompositePass() : gxMaterialConfigDesc('lpcp') {}
	virtual u32 CalculateId() const;
};

#endif // GX_MATERIAL_CONFIG_LIGHT_PRE_PASS_H
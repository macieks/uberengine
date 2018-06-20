#ifndef GX_MATERIAL_CONFIG_DEFERRED_SHADING_H
#define GX_MATERIAL_CONFIG_DEFERRED_SHADING_H

#include "GraphicsExt/gxMaterialConfig_Common.h"

struct gxMaterialConfig_DeferredShading_GeomPass : gxMaterialConfigDesc
{
	gxSkinningDesc m_skinning;

	UE_INLINE gxMaterialConfig_DeferredShading_GeomPass() : gxMaterialConfigDesc('dsgp') {}
	virtual u32 CalculateId() const;
};

struct gxMaterialConfig_DeferredShading_LightPass : gxMaterialConfigDesc
{
	gxLightDesc m_light;

	UE_INLINE gxMaterialConfig_DeferredShading_LightPass() : gxMaterialConfigDesc('dslp') {}
	virtual u32 CalculateId() const;
};

#endif // GX_MATERIAL_CONFIG_DEFERRED_SHADING_H
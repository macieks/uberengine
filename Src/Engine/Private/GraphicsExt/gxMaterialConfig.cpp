#include "gxMaterialConfig_Common.h"
#include "gxMaterialConfig_DeferredShading.h"
#include "gxMaterialConfig_ForwardShading.h"
#include "gxMaterialConfig_LightPrePass.h"
#include "gxMaterialConfig_ShadowMapping.h"
#include "Base/ueHash.h"

// Deferred shading

u32 gxMaterialConfig_DeferredShading_GeomPass::CalculateId() const
{
	return ueCalcHash(this, sizeof(gxMaterialConfig_DeferredShading_GeomPass));
}

u32 gxMaterialConfig_DeferredShading_LightPass::CalculateId() const
{
	return ueCalcHash(this, sizeof(gxMaterialConfig_DeferredShading_LightPass));
}

// Forward shading

u32 gxMaterialConfig_ForwardShading::CalculateId() const
{
	return ueCalcHash(this, sizeof(gxMaterialConfig_ForwardShading));
}

// Shadow mapping

u32 gxMaterialConfig_ShadowMapping_DepthPass::CalculateId() const
{
	return ueCalcHash(this, sizeof(gxMaterialConfig_ShadowMapping_DepthPass));
}

u32 gxMaterialConfig_ShadowMapping_ShadowMaskPass::CalculateId() const
{
	return ueCalcHash(this, sizeof(gxMaterialConfig_ShadowMapping_ShadowMaskPass));
}
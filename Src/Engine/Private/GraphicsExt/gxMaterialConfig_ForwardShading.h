#ifndef GX_MATERIAL_CONFIG_FORWARD_SHADING_H
#define GX_MATERIAL_CONFIG_FORWARD_SHADING_H

#include "GraphicsExt/gxMaterialConfig_Common.h"

struct gxMaterialConfig_ForwardShading : gxMaterialConfigDesc
{
	gxLightDesc*	m_lights;
	gxLightMapDesc	m_lightMap;
	gxSkinningDesc	m_skinning;
	gxFogDesc		m_fog;
	u8				m_numLights;
	gxToneMapping	m_toneMapping;
	gxColorEncoding	m_colorEncoding;

	UE_INLINE gxMaterialConfig_ForwardShading() : gxMaterialConfigDesc('fwsh') {}
	virtual u32 CalculateId() const;
};

#endif // GX_MATERIAL_CONFIG_FORWARD_SHADING_H
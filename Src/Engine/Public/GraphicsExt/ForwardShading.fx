#include "Shading.fx"

// Light computation helpers
// -------------------------

#define ADD_LIGHT(index)
{
	float3 lightColor = 0;
	
	// Lighting model
	
	// Light type

#if LIGHT##index##_TYPE == LIGHT_TYPE_OMNI
#elif LIGHT##index##_TYPE == LIGHT_TYPE_DIR
#elif LIGHT##index##_TYPE == LIGHT_TYPE_SPOT
#endif

	// Light color
	
	lightColor *= 

	// Attenuation

#if LIGHT##index##_TYPE == LIGHT_TYPE_OMNI || LIGHT##index##_TYPE == LIGHT_TYPE_DIR
	#if LIGHT##index##_ATTENUATION == LIGHT_ATTENUATION_LINEAR
		lightColor *= Attenuation_Linear(LightPos[index], LightNearFarRadius[index].x, LightNearFarRadius[index].y, WorldPos);
	#elif LIGHT##index##_ATTENUATION == LIGHT_ATTENUATION_QUADRATIC
		lightColor *= Attenuation_Quadratic(LightPos[index], LightNearFarRadius[index].x, LightNearFarRadius[index].y, WorldPos);
	#endif
#endif

	OUT_Color += lightColor;
}

struct VS_INPUT
{
	float3 Pos;
#if defined(NEED_NORMAL)
	float3 Normal;
#endif
#if defined(NEED_TEX0)
	float2 Tex0;
#endif
#if defined(NEED_TEX1)
	float2 Tex1;
#endif
#if defined(SKINNED)
	float4 BoneWeights;
	float4 BoneIndices;
#endif
};

void ForwardShading_VS(in VS_INPUT IN, out VS_OUTPUT OUT)
{
	float intensity;
#if NUM_LIGHTS >= 1
	ADD_LIGHT(0)
#endif
#if NUM_LIGHTS >= 2
	ADD_LIGHT(1)
#endif

}
#include "Shading.fx"

struct VS_INPUT
{
	float3 Pos : POSITION0;
#if defined(NEED_NORMAL)
	float3 Normal : NORMAL0;
#endif
#if defined(NEED_TEX0)
	float2 Tex0 : TEXCOORD0;
#endif
#if defined(NEED_TEX1)
	float2 Tex1 : TEXCOORD1;
#endif
#if defined(SKINNED)
	float4 BoneWeights : BLENDWEIGHTS;
	float4 BoneIndices : BLENDINDICES;
#endif
};

struct VS_OUTPUT
{
							// Depth 24Z, Stencil S8
	float4 Color0 : COLOR0;	// Lighting accumulation RGB, Intensity A
	float4 Color1 : COLOR1; // Normal F16 X, F16 Y (normal.Z = sqrt(1 - X * X - Y * Y))
	float4 Color2 : COLOR2; // Motion XY vec, specular power, specular intensity
	float4 Color3 : COLOR3; // Albedo RGB, sun-occlusion
};

void DeferredShading_GeomPass_VS(in VS_INPUT IN, out VS_OUTPUT OUT)
{
	float4 worldPos = mul(WVP, IN.Pos);
}
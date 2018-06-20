#include "Shading.fx"

#if defined(HAS_COLOR_MAP) || defined(HAS_NORMAL_MAP) || defined(HAS_SPECULAR_MAP)
	#define NEEDS_TEX0
#endif

#if defined(HAS_NORMAL_MAP)
	#define NEEDS_TANGENT_SPACE
#endif

struct VS_INPUT
{
	float3 Pos : POSITION0;
#if defined(NEEDS_NORMAL)
	float3 Normal : NORMAL0;
#endif
#if defined(NEEDS_TANGENT_SPACE)
	float4 Tangent : TANGENT0;
#endif
#if defined(NEEDS_TEX0)
	float2 Tex0 : TEXCOORD0;
#endif
#if defined(SKINNED)
	float4 BoneWeights : BLENDWEIGHTS;
	float4 BoneIndices : BLENDINDICES;
#endif
};

struct VS_OUTPUT
{
	float4 Pos : POSITION0;
	float DepthVS : TEXCOORD0;
	float3 NormalVS : TEXCOORD1;
#if defined(NEEDS_TEX0)
	float2 Tex0 : TEXCOORD2;
#endif
#if defined(NEEDS_TANGENT_SPACE)
	float3 TangentVS : TEXCOORD3;
	float3 BinormalVS : TEXCOORD4;
#endif
};

struct FS_OUTPUT
{
	float DepthVS : COLOR0;
	float4 NormalVS_SpecularPower : COLOR1;
};

void DeferredShading_LPP_Geom_VS(in VS_INPUT IN, out VS_OUTPUT OUT)
{
#if defined(SKINNED)
	float4x4 WorldView = mul(View, GetSkinningTransform(IN.BoneWeights, IN.BoneIndices));
#endif

	float4 posVS = mul(WorldView, float4(IN.Pos, 1));
	OUT.NormalVS = mul((float3x3) WorldView, IN.Normal * 2 - 1);
#if defined(NEEDS_TANGENT_SPACE)
	OUT.TangentVS = mul((float3x3) WorldView, IN.Tangent.xyz);
	OUT.BinormalVS = cross(OUT.TangentVS, OUT.NormalVS) * IN.Tangent.w;
#endif

	OUT.Pos = mul(Proj, posVS);
	OUT.DepthVS = posVS.z;

#if defined(NEEDS_TEX0)	
	OUT.Tex0 = IN.Tex0;
#endif
}

void DeferredShading_LPP_Geom_FS(in VS_OUTPUT IN, out FS_OUTPUT OUT)
{
	// Store VS depth

	OUT.DepthVS = IN.DepthVS / GetFarPlane();
	
	// Store VS normal

#if defined(HAS_NORMAL_MAP)
	float3x3 tbnTransform = float3x3(IN.TangentVS, IN.BinormalVS, IN.NormalVS);
	float3 normalTex = tex2D(NormalMap, IN.Tex0);

	float3 normal = mul(tbnTransform, normalTex);
	OUT.NormalVS_SpecularPower.rgb = normalize(normal) * 0.5 + 0.5;
#else
	OUT.NormalVS_SpecularPower.rgb = normalize(IN.NormalVS) * 0.5 + 05;
#endif

	// Store specular power

#if defined(HAS_SPECULAR_MAP)
	OUT.NormalVS_SpecularPower.a = tex2D(SpecularMap, IN.Tex0).a;
#else
	OUT.NormalVS_SpecularPower.a = SpecularColor.a;
#endif

	// Perform alpha test

#if defined(HAS_ALPHA_MAP)

	float alpha = tex2D(ColorMap, IN.Tex0).a;
	if (alpha < AlphaTestMinThreshold)
		discard;
#endif
}
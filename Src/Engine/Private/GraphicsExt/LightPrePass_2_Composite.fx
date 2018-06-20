#include "Shading.fx"

#if defined(HAS_COLOR_MAP) || defined(HAS_SPECULAR_MAP)
	#define NEEDS_TEX0
#endif

struct VS_INPUT
{
	float3 Pos : POSITION0;
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
	float4 PosCS : POSITION0;
	float2 TexTS : TEXCOORD0;
#if defined(NEEDS_TEX0)
	float2 Tex0 : TEXCOORD1;
#endif
};

struct FS_OUTPUT
{
	float4 Color : COLOR0;
};

void LightPrePass_LPP_Composite_VS(in VS_INPUT IN, out VS_OUTPUT OUT)
{
#if defined(SKINNED)
	float4x4 WorldViewProj = mul(ViewProj, GetSkinningTransform(IN.BoneWeights, IN.BoneIndices));
#endif

	OUT.PosCS = mul(WorldViewProj, float4(IN.Pos, 1));
	OUT.TexTS = OUT.PosCS.xy / OUT.PosCS.w * 0.5 + 0.5;
	
#if defined(NEEDS_TEX0)
	OUT.Tex0 = IN.Tex0;
#endif
}

void LightPrePass_LPP_Composite_FS(in VS_OUTPUT IN, out FS_OUTPUT OUT)
{
	float4 lighting = tex2D(SceneLighting, IN.TexTS);

	float3 color = 0;

#if defined(HAS_COLOR_MAP)
	color += lighting.rgb * tex2D(ColorMap, IN.Tex0).rgb;
#else
	color += lighting.rgb;
#endif

#if defined(HAS_SPECULAR_MAP)
	color += lighting.a * tex2D(SpecularMap, IN.Tex0).rgb;
#else
	color += lighting.a;
#endif

	OUT.Color.rgb = color;
	OUT.Color.a = 1;
}
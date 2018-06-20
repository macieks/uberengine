#include "Shading.fx"

struct VS_INPUT
{
	float3 Pos : POSITION0;
};

struct VS_OUTPUT
{
	float4 PosCS : POSITION0;
	float2 TexTS : TEXCOORD0;
};

struct FS_OUTPUT
{
	float4 Color : COLOR0;
};

void LightPrePass_LPP_Composite_VS(in VS_INPUT IN, out VS_OUTPUT OUT)
{
	OUT.PosCS = mul(WorldViewProj, float4(IN.Pos, 1));
	OUT.TexTS = OUT.PosCS.xy / OUT.PosCS.w * 0.5 + 0.5;
}

void LightPrePass_LPP_Composite_FS(in VS_OUTPUT IN, out FS_OUTPUT OUT)
{
	float4 texColor = tex2D(SceneColor, IN.TexTS);
	float4 lighting = tex2D(SceneLighting, IN.TexTS);

	// Lighting = ( N.L + R.V^power ) * att * LightColor
	
	OUT.Color.rgb = texColor.rgb * (lighting.rgb + lighting.aaa);
	OUT.Color.a = 1;
}
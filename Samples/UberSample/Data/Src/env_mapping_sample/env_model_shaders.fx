float4x4 World;
float4x4 WorldView;
float4x4 ViewProj;

samplerCUBE EnvMap;

void model_vs(
		in float3 IN_Pos : POSITION0,
		in float3 IN_Normal : NORMAL0,
		in float2 IN_Tex : TEXCOORD0,
		out float3 OUT_ViewNormal : TEXCOORD0,
		out float2 OUT_Tex : TEXCOORD1,
		out float3 OUT_ViewPos : TEXCOORD2,
		out float4 OUT_Pos : POSITION)
{
	float4 pos = float4(IN_Pos, 1);
	OUT_Pos = mul(ViewProj, mul(World, pos));
	OUT_ViewPos = mul(WorldView, pos);

	OUT_ViewNormal = mul((float3x3) WorldView, IN_Normal * 2 - 1);

	OUT_Tex = IN_Tex;
}

float4 model_fs(
	in float3 IN_ViewNormal : TEXCOORD0,
	in float2 IN_Tex : TEXCOORD1,
	in float3 IN_ViewPos : TEXCOORD2) : COLOR0
{
	float3 viewDir = normalize(IN_ViewPos);

	float3 normal = normalize(IN_ViewNormal);
	float3 refl = reflect(viewDir, normal);

	return texCUBE(EnvMap, refl);
}
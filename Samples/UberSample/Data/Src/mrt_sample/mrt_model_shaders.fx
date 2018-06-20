float4x4 World;
float4x4 ViewProj;

sampler2D ColorMap;

void mrt_model_vs(
		in float3 IN_Pos : POSITION0,
		in float3 IN_Normal : NORMAL0,
		in float2 IN_Tex : TEXCOORD0,
		out float2 OUT_Tex : TEXCOORD0,
		out float3 OUT_Normal : TEXCOORD1,
		out float4 OUT_Pos2 : TEXCOORD2,
		out float4 OUT_Pos : POSITION)
{
	OUT_Pos = mul(ViewProj, mul(World, float4(IN_Pos, 1)));
	OUT_Pos2 = OUT_Pos;
	OUT_Normal = mul((float3x3) World, IN_Normal * 2 - 1);
	OUT_Tex = IN_Tex;
}

void mrt_model_fs(
	in float2 IN_Tex : TEXCOORD0,
	in float3 IN_Normal : TEXCOORD1,
	in float4 IN_Pos : TEXCOORD2,
	out float4 OUT_Color : COLOR0,
	out float4 OUT_Lighting : COLOR1,
	out float4 OUT_Depth : COLOR2,
	out float4 OUT_Normal : COLOR3)
{
	float3 LightDir = normalize(float3(1, 1, 1));

	float3 normal = normalize(IN_Normal);

	OUT_Color = tex2D(ColorMap, IN_Tex);
	OUT_Lighting = max(0, dot(normal, LightDir));
	OUT_Depth = IN_Pos.z / IN_Pos.w;
	OUT_Normal = float4(normal * 0.5 + 0.5, 1);
}
float4x4 ViewProj;

sampler2D ColorMap;
sampler2D DetailMap;

sampler2D TerrainMap0;
sampler2D TerrainMap1;
sampler2D TerrainMap2;

/*----------------------- Terrain -----------------------------*/

#define L0 0.2
#define L1 0.4
#define L2 0.6
#define L3 0.8

void Terrain_VS(
		in float3 IN_Pos : POSITION0,
		in float2 IN_Tex : TEXCOORD0,
		in float3 IN_Normal : NORMAL0,
		in float4 IN_Color : COLOR0,
		out float2 OUT_Tex : TEXCOORD0,
		out float3 OUT_Normal : TEXCOORD1,
		out float3 OUT_Blend : TEXCOORD2,
		out float4 OUT_Color : COLOR0,
		out float4 OUT_Pos : POSITION)
{
	OUT_Pos = mul(ViewProj, float4(IN_Pos, 1));
	OUT_Normal = IN_Normal * 2 - 1;
	OUT_Tex = IN_Tex;
	OUT_Color = IN_Color;
	
	float h = IN_Pos.y;
	if (h <= L0)		OUT_Blend = float3(1, 0, 0);
	else if (h <= L1)	OUT_Blend = float3(1 - (h - L0) / (L1 - L0), (h - L0) / (L1 - L0), 0);
	else if (h <= L2)	OUT_Blend = float3(0, 1, 0);
	else if (h <= L3)	OUT_Blend = float3(0, 1 - (h - L2) / (L3 - L2), (h - L2) / (L3 - L2));
	else				OUT_Blend = float3(0, 0, 1);
}

float4 Terrain_FS(
	in float2 IN_Tex : TEXCOORD0,
	in float3 IN_Normal : TEXCOORD1,
	in float3 IN_Blend : TEXCOORD2,
	in float4 IN_Color : COLOR0) : COLOR0
{
	float4 t0 = tex2D(TerrainMap0, IN_Tex);
	float4 t1 = tex2D(TerrainMap1, IN_Tex);
	float4 t2 = tex2D(TerrainMap2, IN_Tex);
	float4 tex = t0 * IN_Blend.x + t1 * IN_Blend.y + t2 * IN_Blend.z;

	float4 d = tex2D(DetailMap, IN_Tex * 14);
	float3 n = normalize(IN_Normal + d.xyz * 0.4);
	
	const float3 LightDir = normalize(float3(-0.6, 1, -0.6));

	float diff = saturate(dot(n, LightDir));
	return IN_Color * float4(tex.rgb * diff, 1);
}
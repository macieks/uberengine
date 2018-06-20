float4x4 ViewProj;
float4x4 World;
float3 RightVec;
float3 UpVec;
float2 ParticleScale;
float4 Color;

sampler2D ColorMap;

void quad_vs(
	float3	IN_Pos				: POSITION,
	float4	IN_Color			: COLOR0,
	float4	IN_Size2_Tex2		: TEXCOORD0,
	float4	IN_Velocity3_Rotation: TEXCOORD1,
	
	out float4	OUT_Pos			: POSITION,
	out float4	OUT_Color		: COLOR0,
	out float2	OUT_Tex			: TEXCOORD0)
{
	// Copy texture coords

	OUT_Tex = IN_Size2_Tex2.zw;

	// 2D corner position (size & rotation)

	float2 quadPos = (IN_Size2_Tex2.zw - float2(0.5, 0.5)) * IN_Size2_Tex2.xy;

	float rotSin, rotCos;
	sincos(IN_Velocity3_Rotation.w, rotSin, rotCos);

	quadPos = float2(
		rotCos * quadPos.x - rotSin * quadPos.y,
		rotSin * quadPos.x + rotCos * quadPos.y);

	// World position

	float4 position =
		mul(World, float4(IN_Pos, 1)) +
		quadPos.x * float4(RightVec, 0) * ParticleScale.x +
		quadPos.y * float4(UpVec, 0) * ParticleScale.y;

	// Final position

	OUT_Pos	= mul(ViewProj, position);
	
	// Color

   	OUT_Color = IN_Color * Color;
}

void default_fs(
	float4		IN_Color	: COLOR0,
	float2		IN_Tex		: TEXCOORD0,
	out float4	OUT_Color	: COLOR0)
{
    float4 tex = tex2D(ColorMap, IN_Tex);
    OUT_Color = IN_Color * tex;
}

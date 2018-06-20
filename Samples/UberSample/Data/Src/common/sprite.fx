float4x4 ViewProj;

sampler2D ColorMap;
sampler2D ColorMap2;

void sprite_vs(
		in float3 IN_Pos : POSITION,
		in float4 IN_Tex : TEXCOORD0,
		in float4 IN_Color : COLOR0,
		out float4 OUT_Pos : POSITION,
		out float4 OUT_Tex : TEXCOORD0,
		out float OUT_Lerp : TEXCOORD1,
		out float4 OUT_Color : COLOR0)
{
	OUT_Pos = mul(ViewProj, float4(IN_Pos.xy, 0, 1));
	OUT_Tex = IN_Tex;
	OUT_Lerp = IN_Pos.z;
	OUT_Color = IN_Color;
}

float4 sprite_fs(
	in float4 IN_Tex : TEXCOORD0,
	in float4 IN_Color : COLOR0) : COLOR0
{
	float4 tex = tex2D(ColorMap, IN_Tex.xy);
	return 
		//float4(tex.rgb, 1);
		//float4(IN_Tex.xy, 0, 1);
		IN_Color * tex.rgba;
}

float4 animated_sprite_fs(
	in float4 IN_Tex : TEXCOORD0,
	in float IN_Lerp : TEXCOORD1,
	in float4 IN_Color : COLOR0) : COLOR0
{
	float4 tex0 = tex2D(ColorMap, IN_Tex.xy);
	float4 tex1 = tex2D(ColorMap2, IN_Tex.zw);
	return IN_Color * lerp(float4(tex0.rgb * tex0.a, tex0.a), float4(tex1.rgb * tex1.a, tex1.a), IN_Lerp);
}
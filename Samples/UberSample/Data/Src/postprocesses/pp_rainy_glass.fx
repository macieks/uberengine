sampler2D ColorMap;
sampler2D NormalMap;

float4 Color;
float DistortionScale;

float MaxEvaporation;

float4 pp_rainy_glass_evaporation_fs(in float2 IN_Tex : TEXCOORD0) : COLOR0
{
	const float4 targetDirAndAlpha = float4(0.5, 0.5, 1.0, 0.0);

	float4 color = tex2D(ColorMap, IN_Tex);
	float4 evaporationDelta = clamp(targetDirAndAlpha - color, -MaxEvaporation, MaxEvaporation);
	return color + evaporationDelta;
}

float4 pp_rainy_glass_distortion_fs(in float2 IN_Tex : TEXCOORD0) : COLOR0
{
	float4 normal = tex2D(NormalMap, IN_Tex);
	normal.xyz = normal.xyz * 2.0 - 1.0;
	float3 color = tex2D(ColorMap, IN_Tex + normal.xy * DistortionScale).rgb;
	return float4(saturate(color + Color.rgb * normal.w), 1);
}
float BlendFactor;
sampler2D ColorMap;
sampler2D BlurMap;

float4 pp_bloom_composite_fs(in float2 IN_Tex : TEXCOORD0) : COLOR0
{
	float4 c0 = tex2D(ColorMap, IN_Tex);
	float4 c1 = tex2D(BlurMap, IN_Tex);
	return lerp(c0, c1, BlendFactor);
}
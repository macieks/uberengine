sampler2D ColorMap;
sampler2D DepthMap;
sampler2D BlurMap;

float4 DOFParams;

void pp_depth_of_field_fs(
	in float2 IN_Tex : TEXCOORD0,
	out float4 OUT_Color : COLOR)
{
    float depth = tex2D(DepthMap, IN_Tex).r;
    float3 color = tex2D(ColorMap, IN_Tex).rgb;
    float3 blur = tex2D(BlurMap, IN_Tex).rgb;

    float nearBlurAmount = saturate(depth * DOFParams.x + DOFParams.y);
    float farBlurAmount = saturate(depth * DOFParams.z + DOFParams.w);
    float blurAmount = max(nearBlurAmount, farBlurAmount);

    OUT_Color = float4(lerp(color, blur, blurAmount), 1);
}
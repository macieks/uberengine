sampler2D ColorMap;
sampler2D DepthMap;

float2 DepthParams;
float MaxMotionBlur;
float4x4 ViewProjInv;
float4x4 PrevViewProj;

float GetProjDepth(float2 tex)
{
    float viewDepth = tex2D(DepthMap, tex).r;
    return (-DepthParams.x * DepthParams.y + viewDepth * DepthParams.y) / viewDepth;
}

float4 GetProjPos(float2 tex)
{
	return float4(tex.x * 2.0 - 1.0, (1.0 - tex.y) * 2.0 - 1.0, GetProjDepth(tex), 1.0);
}

void pp_camera_motion_blur_fs(
	in float2 IN_Tex : TEXCOORD0,
	out float4 OUT_Color : COLOR)
{
	// Get clip space position

    float4 projPos = GetProjPos(IN_Tex);

	// Get world space position

    float4 worldPos = mul(ViewProjInv, projPos);
    worldPos /= worldPos.w;

    // Get previous clip space position

    float4 oldProjPos = mul(PrevViewProj, worldPos);
    oldProjPos /= oldProjPos.w;

    // Determine velocity
    
    float2 velocity = (projPos.xy - oldProjPos.xy) * 0.5;
    
    // Clamp velocity

    float velocityLength = length(velocity);
    if (velocityLength > MaxMotionBlur)
		velocity = velocity / velocityLength * MaxMotionBlur;

    // Sample along velocity

    int NumSamples = 10;
    float NumSamplesInv = 1.0 / (float) NumSamples;

    velocity *= NumSamplesInv;

	float4 color = 0;
	for (int i = 0; i < NumSamples; ++i, IN_Tex += velocity)
		color += tex2D(ColorMap, IN_Tex);

    OUT_Color = color * NumSamplesInv;
}
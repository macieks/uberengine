sampler2D ColorMap;
sampler2D DepthMap;
sampler2D BlurMap;
sampler2D SmallBlurMap;
sampler2D LargeBlurMap;

float4 ColorMapSize;
float4 RenderTargetSize;
float2 PixelOffset;

float2 DOFNearParams;
float3 DOFFarParams;

float4 DOFLerpScale;
float4 DOFLerpBias;

// Stage 1: Downsample and calculate COC
// ---------------------------------

void pp_depth_of_field_hq_downsample_vs(
		in float2 IN_Pos : POSITION0,
		in float2 IN_Tex : TEXCOORD0,
		out float4 OUT_Tex0 : TEXCOORD0,
		out float4 OUT_Tex1 : TEXCOORD1,
		out float4 OUT_Tex2 : TEXCOORD2,
		out float4 OUT_Pos : POSITION)
{
	OUT_Pos = float4(IN_Pos + PixelOffset, 0, 1);

	OUT_Tex0.xy = IN_Tex + float2(-1.0, -1.0) * ColorMapSize.zw;
	OUT_Tex0.zw = IN_Tex + float2( 1.0, -1.0) * ColorMapSize.zw;
	OUT_Tex1.xy = IN_Tex + float2(-1.5, -1.5) * ColorMapSize.zw;
	OUT_Tex1.zw = IN_Tex + float2(-0.5, -1.5) * ColorMapSize.zw;
	OUT_Tex2.xy = IN_Tex + float2( 0.5, -1.5) * ColorMapSize.zw;
	OUT_Tex2.zw = IN_Tex + float2( 1.5, -1.5) * ColorMapSize.zw;
}

void pp_depth_of_field_hq_downsample_fs(
	in float4 IN_Tex0 : TEXCOORD0,
	in float4 IN_Tex1 : TEXCOORD1,
	in float4 IN_Tex2 : TEXCOORD2,
	out float4 OUT_Color : COLOR0)
{
	float2 rowOfs[3];
	rowOfs[0] = float2(0, ColorMapSize.w);
	rowOfs[1] = rowOfs[0] * 2;
	rowOfs[2] = rowOfs[0] * 3;
 
	// Use bilinear filtering to average 4 color samples for free
	
	float3 color = 0;
	color += tex2D(ColorMap, IN_Tex0.xy).rgb;
	color += tex2D(ColorMap, IN_Tex0.zw).rgb;
	color += tex2D(ColorMap, IN_Tex0.xy + rowOfs[1]).rgb;
	color += tex2D(ColorMap, IN_Tex0.zw + rowOfs[1]).rgb;
	color /= 4;
	
	// Process 4 samples at a time to use vector hardware efficiently

	float4 depth;
	float4 coc;

	depth[0] = tex2D(DepthMap, IN_Tex1.xy).r;
	depth[1] = tex2D(DepthMap, IN_Tex1.zw).r;
	depth[2] = tex2D(DepthMap, IN_Tex2.xy).r;
	depth[3] = tex2D(DepthMap, IN_Tex2.zw).r;
	coc = saturate(DOFNearParams.x * depth + DOFNearParams.y);
	
	depth[0] = tex2D(DepthMap, IN_Tex1.xy + rowOfs[0]).r;
	depth[1] = tex2D(DepthMap, IN_Tex1.zw + rowOfs[0]).r;
	depth[2] = tex2D(DepthMap, IN_Tex2.xy + rowOfs[0]).r;
	depth[3] = tex2D(DepthMap, IN_Tex2.zw + rowOfs[0]).r;
	coc = max(coc, saturate(DOFNearParams.x * depth + DOFNearParams.y));

	depth[0] = tex2D(DepthMap, IN_Tex1.xy + rowOfs[1]).r;
	depth[1] = tex2D(DepthMap, IN_Tex1.zw + rowOfs[1]).r;
	depth[2] = tex2D(DepthMap, IN_Tex2.xy + rowOfs[1]).r;
	depth[3] = tex2D(DepthMap, IN_Tex2.zw + rowOfs[1]).r;
	coc = max(coc, saturate(DOFNearParams.x * depth + DOFNearParams.y));

	depth[0] = tex2D(DepthMap, IN_Tex1.xy + rowOfs[2]).r;
	depth[1] = tex2D(DepthMap, IN_Tex1.zw + rowOfs[2]).r;
	depth[2] = tex2D(DepthMap, IN_Tex2.xy + rowOfs[2]).r;
	depth[3] = tex2D(DepthMap, IN_Tex2.zw + rowOfs[2]).r;
	coc = max(coc, saturate(DOFNearParams.x * depth + DOFNearParams.y));
	
	// Output color in RGB and max coc in Alpha
	
	OUT_Color.rgb = color;
	OUT_Color.a = max(max(coc[0], coc[1]), max(coc[2], coc[3]));
}

// Stage 2: Near COC
// ---------------------------------

void pp_depth_of_field_hq_near_coc_fs
(
	in float2 IN_Tex : TEXCOORD0,
	out float4 OUT_Color : COLOR0
)
{
	float4 shrunk = tex2D(ColorMap, IN_Tex);
	float4 blurred = tex2D(BlurMap, IN_Tex);
	
	OUT_Color.rgb = shrunk.rgb;
	OUT_Color.a = 2 * max(blurred.a, shrunk.a) - shrunk.a;
}

// Stage 3: Small blur
// ---------------------------------

void pp_depth_of_field_hq_small_blur_vs(
		in float2 IN_Pos : POSITION0,
		in float2 IN_Tex : TEXCOORD0,
		out float4 OUT_Tex : TEXCOORD0,
		out float4 OUT_Pos : POSITION)
{
	OUT_Pos = float4(IN_Pos + PixelOffset, 0, 1);
	OUT_Tex = IN_Tex.xxyy + float4(-0.5, 0.5, -0.5, 0.5) * ColorMapSize.zzww;
}

void pp_depth_of_field_hq_small_blur_fs
(
	in float4 IN_Tex : TEXCOORD0,
	out float4 OUT_Color : COLOR0
)
{
	float4 color = 0;
	color += tex2D(ColorMap, IN_Tex.xz);  
	color += tex2D(ColorMap, IN_Tex.yz);  
	color += tex2D(ColorMap, IN_Tex.xw);  
	color += tex2D(ColorMap, IN_Tex.yw);
	OUT_Color = color / 4;
}

// Stage 4: Composite DOF
// ---------------------------------

float3 GetSmallBlurSample(float2 tc)
{
	float3 sum;
	const float weight = 4.0 / 17.0;
	
	sum = 0; // Unblurred sample done by alpha blending
	sum += weight * tex2D(ColorMap, tc + float2( 0.5, -1.5) * ColorMapSize.zw).rgb;
	sum += weight * tex2D(ColorMap, tc + float2(-1.5, -0.5) * ColorMapSize.zw).rgb;
	sum += weight * tex2D(ColorMap, tc + float2(-0.5,  1.5) * ColorMapSize.zw).rgb;
	sum += weight * tex2D(ColorMap, tc + float2( 1.5,  0.5) * ColorMapSize.zw).rgb;

	return sum;
}

float4 InterpolateDof(float3 small, float3 med, float3 large, float t)
{
	float4 weights;
	float3 color;
	float alpha;
	
	// Efficiently calculate the cross-blend weights for each sample.
	// Let the unblurred sample to small blur fade happen over distance
	// d0, the small to medium blur over distance d1, and the medium to
	// large blur over distance d2, where d0 + d1 + d2 = 1.
	// dofLerpScale = float4(-1 / d0, -1 / d1, -1/ d2, 1 / d2);
	// dofLerpBias = float4(1, (1 - d2) / d1, 1 / d2, (d2 - 1) / d2);

	weights = saturate(t * DOFLerpScale + DOFLerpBias);
	weights.yz = min(weights.yz, 1 - weights.xy);
	
	// Unblurred sample with weight "weights.x" done by alpha blending

	color = weights.y * small + weights.z * med + weights.w * large;
	alpha = dot(weights.yzw, float3(16.0 / 17.0, 1.0, 1.0));
	
	return float4(color, alpha);
}

void pp_depth_of_field_hq_composite_fs
(
	in float2 IN_Tex : TEXCOORD0,
	out float4 OUT_Color : COLOR0
)
{
	float3 small = GetSmallBlurSample(IN_Tex);
	float4 med = tex2D(SmallBlurMap, IN_Tex);
	float3 large = tex2D(LargeBlurMap, IN_Tex).rgb;
	float nearCoc = med.a;
	
	float depth = tex2D(DepthMap, IN_Tex).r;
	
	float coc;
/*
	// Don't blur background sky
	if (depth > 1.0e6)
	{
		coc = nearCoc;
	}
	else
*/
	{
		// DOFFarParams.x and DOFFarParams.y specify the linear ramp to convert
		// to depth for the distant out-of-focus region.
		// DOFFarParams.z is the ratio of the far to the near blur radius
		float farCoc = saturate(DOFFarParams.x * depth + DOFFarParams.y);
		coc = max(nearCoc, farCoc * DOFFarParams.z);
	}

	float4 dof = InterpolateDof(small, med.rgb, large, coc);

#if 1
	OUT_Color.rgb = lerp(tex2D(ColorMap, IN_Tex).rgb, dof.rgb, dof.a);
	OUT_Color.a = 1;
#else
	if (IN_Tex.x < 0.33)
		OUT_Color.rgb = small;
	else if (IN_Tex.x < 0.34)
		OUT_Color.rgb = 1;
	else if (IN_Tex.x < 0.66)
		OUT_Color.rgb = med.rgb;
	else if (IN_Tex.x < 0.67)
		OUT_Color.rgb = 1;
	else
		OUT_Color.rgb = large;
#endif
}
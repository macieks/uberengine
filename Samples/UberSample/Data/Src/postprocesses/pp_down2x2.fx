sampler2D ColorMap;
float4 RenderTargetSize;
float2 PixelOffset;

void pp_down2x2_vs(
		in float2 IN_Pos : POSITION0,
		in float2 IN_Tex : TEXCOORD0,
		out float4 OUT_Tex0 : TEXCOORD0,
		out float4 OUT_Tex1 : TEXCOORD1,
		out float4 OUT_Pos : POSITION)
{
	OUT_Pos = float4(IN_Pos + PixelOffset, 0, 1);

	// Calculate offset coordinates
	OUT_Tex0.xy = IN_Tex + float2( RenderTargetSize.z,  RenderTargetSize.w);
	OUT_Tex0.zw = IN_Tex + float2( RenderTargetSize.z, -RenderTargetSize.w);
	OUT_Tex1.xy = IN_Tex + float2(-RenderTargetSize.z, -RenderTargetSize.w);
	OUT_Tex1.zw = IN_Tex + float2(-RenderTargetSize.z,  RenderTargetSize.w);
}

float4 pp_down2x2_fs(
	in float4 IN_Tex0 : TEXCOORD0,
	in float4 IN_Tex1 : TEXCOORD1) : COLOR0
{
	float4 tex = 0;
	tex += tex2D(ColorMap, IN_Tex0.xy);
	tex += tex2D(ColorMap, IN_Tex0.zw);
	tex += tex2D(ColorMap, IN_Tex1.xy);
	tex += tex2D(ColorMap, IN_Tex1.zw);
	return tex * 0.25f;
}
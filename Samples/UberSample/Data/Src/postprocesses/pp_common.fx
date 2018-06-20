float4 RenderTargetSize;
float2 PixelOffset;

void pp_default_vs(
		in float2 IN_Pos : POSITION0,
		in float2 IN_Tex : TEXCOORD0,
		out float2 OUT_Tex : TEXCOORD0,
		out float4 OUT_Pos : POSITION)
{
	OUT_Pos = float4(IN_Pos + PixelOffset, 0, 1);
	OUT_Tex = IN_Tex;
}
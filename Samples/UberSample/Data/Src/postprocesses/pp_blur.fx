sampler2D ColorMap;
float4 RenderTargetSize;
float2 PixelOffset;
float BlurKernel;

#define NUM_BLUR_SAMPLES 13

static const float PixelKernel[NUM_BLUR_SAMPLES] =
{
    -6,
    -5,
    -4,
    -3,
    -2,
    -1,
    0,
    1,
    2,
    3,
    4,
    5,
    6
};

static const float BlurWeights[NUM_BLUR_SAMPLES] = 
{
    0.002216,
    0.008764,
    0.026995,
    0.064759,
    0.120985,
    0.176033,
    0.199471,
    0.176033,
    0.120985,
    0.064759,
    0.026995,
    0.008764,
    0.002216,
};

float4 pp_vertical_blur_fs(in float2 IN_Tex : TEXCOORD0) : COLOR0
{
	float blurOffsetScale = BlurKernel * RenderTargetSize.w;

	float4 color = 0;
    for (int i = 0; i < NUM_BLUR_SAMPLES; i++)
        color += tex2D(ColorMap, IN_Tex + float2(0, PixelKernel[i] * blurOffsetScale)) * BlurWeights[i];
	return color;
}

float4 pp_horizontal_blur_fs(in float2 IN_Tex : TEXCOORD0) : COLOR0
{
	float blurOffsetScale = BlurKernel * RenderTargetSize.z;

	float4 color = 0;
    for (int i = 0; i < NUM_BLUR_SAMPLES; i++)
        color += tex2D(ColorMap, IN_Tex + float2(PixelKernel[i] * blurOffsetScale, 0)) * BlurWeights[i];
	return color;
}

/*

sampler SceneColor  : register (s0);
float4 ViewportSize : register(c34);
float4 GaussKernelA : register(c0); // 0-3 sampler
float4 GaussKernelB : register(c1); // 4-6 samples

struct Default_VS_IN
{
	float4 Pos      : POSITION;
	float2 Tex		: TEXCOORD0;
};

struct Default_VS_OUT
{
	float4	Pos		: POSITION;
	float4	Tex0	: TEXCOORD0;
	float4	Tex1	: TEXCOORD1;
	float4	Tex2	: TEXCOORD2;
	float4	Tex3	: TEXCOORD3;
	float2	Tex4	: TEXCOORD4;	
};


void vs_main(Default_VS_IN In, out Default_VS_OUT Out)
{
	// Copy position
	Out.Pos = In.Pos;
	
	// Offets
	Out.Tex0 = In.Tex.xyyy + float4( 0.0f, 1.3366f, 3.4295f, 5.4264f ) * ViewportSize.w;
	Out.Tex1 = In.Tex.xyyy + float4( 0.0f, 7.4359f, 9.4436f, 11.4401f ) * ViewportSize.w;
	Out.Tex2 = In.Tex.xyyy + float4( 0.0f, -1.3366f, -3.4295f, -5.4264f ) * ViewportSize.w;
	Out.Tex3 = In.Tex.xyyy + float4( 0.0f, -7.4359f, -9.4436f, -11.4401f ) * ViewportSize.w;
	Out.Tex4 = In.Tex;
}

float4 ps_main(Default_VS_OUT In) : COLOR0
{
    float4 tex = 0;
    tex += tex2D( SceneColor, In.Tex4.xy ) * GaussKernelA.x;
    tex += tex2D( SceneColor, In.Tex0.xy ) * GaussKernelA.y;
    tex += tex2D( SceneColor, In.Tex0.xz ) * GaussKernelA.z;
    tex += tex2D( SceneColor, In.Tex0.xw ) * GaussKernelA.w;
    tex += tex2D( SceneColor, In.Tex1.xy ) * GaussKernelB.x;
    tex += tex2D( SceneColor, In.Tex1.xz ) * GaussKernelB.y;
    tex += tex2D( SceneColor, In.Tex1.xw ) * GaussKernelB.z;
    tex += tex2D( SceneColor, In.Tex2.xy ) * GaussKernelA.y;
    tex += tex2D( SceneColor, In.Tex2.xz ) * GaussKernelA.z;
    tex += tex2D( SceneColor, In.Tex2.xw ) * GaussKernelA.w;
    tex += tex2D( SceneColor, In.Tex3.xy ) * GaussKernelB.x;
    tex += tex2D( SceneColor, In.Tex3.xz ) * GaussKernelB.y;
    tex += tex2D( SceneColor, In.Tex3.xw ) * GaussKernelB.z;
    return tex;
}

*/
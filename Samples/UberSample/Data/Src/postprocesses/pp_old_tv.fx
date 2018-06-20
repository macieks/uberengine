sampler2D ColorMap;
float Time;
float4 RenderTargetSize;
float2 PixelOffset;

float OverExposureAmount <
	string UIName = "Overexposure amount";
	string UIWidget = "slider";
	half UIMin = 0.0;
	half UIMax = 10.0;
	half UIStep = 0.1;
> = 0.1; // 1.5
//----------------------------------------------------------------------------

float DustAmount <
	string UIName = "Dust amount";
	string UIWidget = "slider";
	half UIMin = 0.0;
	half UIMax = 4.0;
	half UIStep = 1;
> = 4.0;
//----------------------------------------------------------------------------

float FrameJitterFrequency <
	string UIName = "Frame Jitter Frequency";
	string UIWidget = "slider";
	half UIMin = 0.0;
	half UIMax = 6.0;
	half UIStep = 0.1;
> = 3.0; // 4.7
//----------------------------------------------------------------------------

float MaxFrameJitter <
	string UIName = "Max Frame Jitter Amount";
	string UIWidget = "slider";
	half UIMin = 0.0;
	half UIMax = 10.0;
	half UIStep = 0.1;
> = 1.4;
//----------------------------------------------------------------------------

float4 FilmColor <
	string UIWidget = "Color";
> = {1.0, 0.7559052, 0.58474624, 1.0};
//----------------------------------------------------------------------------

float GrainThicknes <
	string UIName = "Grain Thicknes";
	string UIWidget = "slider";
	half UIMin = 0.1;
	half UIMax = 4.0;
	half UIStep = 0.1;
> = 1.0;
//----------------------------------------------------------------------------

float GrainAmount <
	string UIName = "Grain Amount";
	string UIWidget = "slider";
	half UIMin = 0.0;
	half UIMax = 1.0;
	half UIStep = 0.1;
> = 0.8;
//----------------------------------------------------------------------------

float ScratchesAmount <
	string UIName = "Num vert. scratches";
	string UIWidget = "slider";
	half UIMin = 0.0;
	half UIMax = 3.0;
	half UIStep = 1;
> = 3.0;
//----------------------------------------------------------------------------

float ScratchesLevel <
	string UIName = "Num vert. scratches";
	string UIWidget = "slider";
	half UIMin = 0.0;
	half UIMax = 1.0;
	half UIStep = 0.1;
> = 0.7;

/*
texture DustMap;
sampler DustSamp = sampler_state 
{
	texture = <DustMap>;
	AddressU  = BORDER;        
	AddressV  = BORDER;
	AddressW  = CLAMP;
	MIPFILTER = NONE;
	MINFILTER = LINEAR;
	MAGFILTER = LINEAR;
	BorderColor = {1.0, 1.0, 1.0, 1.0};
};

texture LineMap;
sampler LineSamp = sampler_state 
{
	texture = <LineMap>;
	AddressU  = BORDER;        
	AddressV  = CLAMP;
	AddressW  = CLAMP;
	MIPFILTER = NONE;
	MINFILTER = NONE;
	MAGFILTER = NONE;
	BorderColor = {1.0, 1.0, 1.0, 1.0};
};

texture TvAndNoiseMap;
sampler TvSamp = sampler_state 
{
	texture = <TvAndNoiseMap>;
	AddressU  = CLAMP;        
	AddressV  = CLAMP;
	AddressW  = CLAMP;
	MIPFILTER = NONE;
	MINFILTER = LINEAR;
	MAGFILTER = LINEAR;
};

sampler NoiseSamp = sampler_state 
{
	texture = <TvAndNoiseMap>;
	AddressU  = WRAP;        
	AddressV  = WRAP;
	AddressW  = WRAP;
	MIPFILTER = NONE;
	MINFILTER = LINEAR;
	MAGFILTER = LINEAR;
};
*/

sampler2D LineMap;
sampler2D DustMap;
sampler2D TvMap;
sampler2D NoiseMap;

struct OldTV_VS_OUTPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
	
	half4 Dust0And1Coords : TEXCOORD1;
	half4 Dust2And3Coords : TEXCOORD2;

	half2 TvCoords        : TEXCOORD3;
	half2 NoiseCoords     : TEXCOORD4;

	half4 Line0VertCoords : TEXCOORD5;
	half4 Line1VertCoords : TEXCOORD6;
	half4 Line2VertCoords : TEXCOORD7;

	half4 OverExposure    : COLOR0;
};

void pp_old_tv_vs(
		in float2 IN_Pos : POSITION0,
		in float2 IN_Tex : TEXCOORD0,
		out OldTV_VS_OUTPUT Out)
{
	// some pseudo-random numbers
	float Random0 = fmod(Time, 7.000);
	float Random1 = fmod(Time, 3.300);
	float Random2 = fmod(Time, 0.910);
	float Random3 = fmod(Time, 0.110);
	float Random4 = fmod(Time, 0.070);
	float Random5 = fmod(Time, 0.013);

	float4 pos = float4(IN_Pos + PixelOffset, 0, 1);
	Out.Pos = pos;

	// compute vert. frame jitter
	float frameJitterY = 40 * MaxFrameJitter * Random2 * Random0 * Random3;
	if (frameJitterY < (6 - FrameJitterFrequency) * 10)
		frameJitterY = 0;
 	frameJitterY *= RenderTargetSize.w;

	// add jitter to the original coords.
	Out.Tex = IN_Tex + half2(0, frameJitterY);

	// compute overexposure amount
	Out.OverExposure = OverExposureAmount * Random3;
	
	// pass original screen coords (border rendering)
	Out.TvCoords = IN_Tex;

	// compute noise coords.
	half2 NoiseCoordsTmp = (RenderTargetSize.xy / (GrainThicknes * half2(128.0, 128.0)))
		* IN_Tex.xy;
	NoiseCoordsTmp += half2(100 * Random3 * Random1 - Random0, 
		Random4 + Random1 * Random2);
	Out.NoiseCoords = NoiseCoordsTmp.xy;

	// dust section (turns on or off particular dust texture)
	if (DustAmount > 0)
	{
		Out.Dust0And1Coords.xy = 
			2.0 *
			 pos.xy 
			+ 200 * float2(Random1 * Random4, fmod(Time, 0.03))
			;
	}
	else
	{
		Out.Dust0And1Coords.xy = 0;
	}

	if (DustAmount > 1)
	{
		Out.Dust0And1Coords.zw = 2.3 * pos.yx 
			- 210 * float2(Random4 * 0.45, Random5 * 2);
	}
	else
	{
		Out.Dust0And1Coords.zw = 0;
	}

	if (DustAmount > 2)
	{
		Out.Dust2And3Coords.xy = 1.4 * pos.xy 
			+ float2(700, +100) * float2(Random2 * Random4, Random2);
	}
	else
	{
		Out.Dust2And3Coords.xy = 0;
	}

	if (DustAmount > 3)
	{
		Out.Dust2And3Coords.zw = 1.7 * pos.yx 
			+ float2(-100, 130) * float2(Random2 * Random4, Random1 * Random4);
	}
	else
	{
		Out.Dust2And3Coords.zw = 0;
	}
	
	// vert lines section
	Out.Line0VertCoords   = 0.5 * pos.xxxx * RenderTargetSize.xxxx * 0.3;
	Out.Line1VertCoords = Out.Line0VertCoords;
	Out.Line2VertCoords = Out.Line0VertCoords;

	// first line
	if (ScratchesAmount > 0)
	{
		Out.Line0VertCoords.x += 0.15 - ((Random1 - Random3 + Random2) * 0.1) 
			* RenderTargetSize.x;
	}
	else
	{
		Out.Line0VertCoords.x = -1;
	}

	// second line
	if (ScratchesAmount > 1)
	{
		Out.Line1VertCoords.x += 0.55 + ((Random0 - Random2 + Random4) * 0.1) 
			* RenderTargetSize.x;
	}
	else
	{
		Out.Line1VertCoords.x = -1;
	}

	// third line
	if (ScratchesAmount > 2)
	{
		Out.Line2VertCoords.x += 0.31 + ((Random1 - Random2 + Random4) * 0.2) 
			* RenderTargetSize.x;
	}
	else
	{
		Out.Line2VertCoords.x = -1;
	}
}

float4 pp_old_tv_fs(OldTV_VS_OUTPUT In) : COLOR0
{
    // sample scene
	half4 img = tex2D(ColorMap, In.Tex.xy);

	// compute sepia (or other color) image
	half4 img2 =
		lerp(
			dot(img, half4(0.30, 0.59, 0.11, 0.3)),
			img * half4(0.50, 0.59, 0.41, 0.3),
			0.5)
		* FilmColor;

	// sample dust textures
	half4 dust0 = tex2D(DustMap, In.Dust0And1Coords.xy);
	half4 dust1 = tex2D(DustMap, In.Dust0And1Coords.wz);
	half4 dust2 = tex2D(DustMap, In.Dust2And3Coords.xy);
	half4 dust3 = tex2D(DustMap, In.Dust2And3Coords.wz);

	// sample line textures
	half4 line0 = tex2D(LineMap, In.Line0VertCoords.xy);
	half4 line1 = tex2D(LineMap, In.Line1VertCoords.xy);
	half4 line2 = tex2D(LineMap, In.Line2VertCoords.xy);

	// sample border texture
	half4 tv = tex2D(TvMap, In.TvCoords.xy);

	// sample noise values
	half4 noiseVal = tex2D(NoiseMap, In.NoiseCoords.xy);
	noiseVal = lerp(half4(1.0, 1.0, 1.0, 1.0), noiseVal, GrainAmount);

	// "accumulate" dust
	half finalDust = dust0.x * dust1.y * dust2.z * dust3.w;

	// "accumulate" lines
	half finalLines = line0.x * line1.x * line2.x;
	//finalLines = lerp(half4(1.0, 1.0, 1.0, 1.0), finalLines, ScratchesLevel);
	
	// final composition
	return
		tv * (In.OverExposure + finalDust * finalLines * img2)
		* noiseVal.wwww;
}
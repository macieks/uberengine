float4x4 ViewProj;
float4x4 World;
float4x4 WorldViewProj;
float4x4 View;

sampler2D ColorMap;
float4 ColorMapSize;

float2 PixelOffset;

float4 Color;

/*----------------------- Position 2D No-Transform -----------------------------*/

void pos_no_transform_vs(
		in float2 IN_Pos : POSITION0,
		out float4 OUT_Color : COLOR0,
		out float4 OUT_Pos : POSITION)
{
	OUT_Pos = float4(IN_Pos + PixelOffset, 0, 1);
	OUT_Color = Color;
}

/*----------------------- Position + Tex0 2D No-Transform -----------------------------*/

void pos_tex_no_transform_vs(
		in float2 IN_Pos : POSITION0,
		in float2 IN_Tex0 : TEXCOORD0,
		out float2 OUT_Tex0 : TEXCOORD0,
		out float4 OUT_Color : COLOR0,
		out float4 OUT_Pos : POSITION)
{
	OUT_Pos = float4(IN_Pos + PixelOffset, 0, 1);
	OUT_Tex0 = IN_Tex0;
	OUT_Color = Color;
}

/*--------------------- Position + Color -------------------------------*/

void pos_col_vs(
		in float3 IN_Pos : POSITION0,
		in float4 IN_Color : COLOR0,
		out float4 OUT_Color : COLOR0,
		out float4 OUT_Pos : POSITION)
{
	OUT_Pos = mul(WorldViewProj, float4(IN_Pos, 1));
	OUT_Color = IN_Color;
}

float4 pos_col_fs(float4 IN_Color : COLOR0) : COLOR0
{
	return IN_Color;
}

/*----------------------- Position + Tex0 -----------------------------*/

void pos_tex_vs(
		in float3 IN_Pos : POSITION0,
		in float2 IN_Tex : TEXCOORD0,
		out float2 OUT_Tex : TEXCOORD0,
		out float4 OUT_Pos : POSITION)
{
	OUT_Pos = mul(WorldViewProj, float4(IN_Pos, 1));
	OUT_Tex = IN_Tex;
}

float4 pos_tex_fs(
	in float2 IN_Tex : TEXCOORD0) : COLOR0
{
	return tex2D(ColorMap, IN_Tex);
}

/*----------------------- Position + Color + Tex0 -----------------------------*/

void pos_col_tex_vs(
		in float3 IN_Pos : POSITION0,
		in float2 IN_Tex : TEXCOORD0,
		in float4 IN_Color : COLOR0,
		out float2 OUT_Tex : TEXCOORD0,
		out float4 OUT_Color : COLOR0,
		out float4 OUT_Pos : POSITION)
{
	OUT_Pos = mul(WorldViewProj, float4(IN_Pos, 1));
	OUT_Tex = IN_Tex;
	OUT_Color = IN_Color;
}

float4 pos_col_tex_fs(
	in float2 IN_Tex : TEXCOORD0,
	in float4 IN_Color : COLOR0) : COLOR0
{
	return IN_Color * tex2D(ColorMap, IN_Tex);
}

/*---------------------- Font (with color) ------------------------------*/

void font_vs(
		in float2 IN_Pos : POSITION0,
		in float2 IN_Tex : TEXCOORD0,
		in float4 IN_Color : COLOR0,
		in float4 IN_ChannelMask : COLOR1,
		out float2 OUT_Tex : TEXCOORD0,
		out float4 OUT_ChannelMask : TEXCOORD1,
		out float4 OUT_Color : COLOR0,
		out float4 OUT_Pos : POSITION)
{
	OUT_Pos = mul(WorldViewProj, float4(IN_Pos, 0, 1));
	OUT_Tex = IN_Tex;
	OUT_ChannelMask = IN_ChannelMask;
	OUT_Color = IN_Color;
}

float4 font_fs(
	in float2 IN_Tex : TEXCOORD0,
	in float4 IN_ChannelMask : TEXCOORD1,
	in float4 IN_Color : COLOR0) : COLOR0
{
	float font = dot(tex2D(ColorMap, IN_Tex).argb, IN_ChannelMask);
	return float4(IN_Color.rgb, IN_Color.a * font);
}

/*----------------------- Debug normals --------------------------*/

float4 DebugNormals_FS(in float3 IN_Normal : TEXCOORD2) : COLOR0
{
	float3 normal = normalize(IN_Normal);
	return float4(normal * 0.5 + 0.5, 1);
}

/*--------------------- Depth Only (view space) -------------------------------*/

void depth_view_space_vs(
		in float3 IN_Pos : POSITION0,
		out float OUT_Pos2 : TEXCOORD0,
		out float4 OUT_Pos : POSITION)
{
	float4 worldPos = mul(World, float4(IN_Pos, 1));
	OUT_Pos = mul(ViewProj, worldPos);
	
	float4 viewPos = mul(View, worldPos);
	OUT_Pos2 = viewPos.z;
}

float4 depth_view_space_fs(float IN_Pos2 : TEXCOORD0) : COLOR0
{
	return IN_Pos2;
}

/*---------------------- Debug mip-levels ---------------------------*/

// Calculates mip level based on texture coordinates and Color Map dimensions
float CalcMipLevel(float2 uv, float2 textureSize)
{
    float2 dx = 1;//ddx(uv * textureSize.x);
    float2 dy = 1;//ddy(uv * textureSize.y);
    float d = max( dot(dx, dx), dot(dy, dy) );

    return 0.5 * log2(d);
}

// Number of visualized mip levels; all levels below are displayed in some color
#define NUM_MIP_LEVELS 6

// Colors for few highest mip levels (from highest, to highest considered)
static const float3 MipColors[NUM_MIP_LEVELS + 1] =
{
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1},
    {1, 1, 0},
    {0, 1, 1},
    {1, 0, 1},
    {0.3, 0.3, 0.3}
};

// Determines mip level color based on mip level
float3 CalcMipLevelColor(int mipLevel)
{
    return MipColors[mipLevel];
}

float4 DebugMipLevels_FS(float2 IN_Tex : TEXCOORD0) : COLOR
{
    // Determine color of mip-level that would be used
    float mipLevel = CalcMipLevel(IN_Tex, ColorMapSize.xy);
    int mipLevelInt = (int) clamp(mipLevel, 0, NUM_MIP_LEVELS);
    float3 mipLevelColor = CalcMipLevelColor(mipLevelInt);

    // Draw gridlines corresponding to 64 texels at incoming UV resolution
    float gridDensity = pow(2, mipLevelInt);
    float gridStep = 1.0 / (16.0 * gridDensity);
    float inGrid = step(0.1f, frac(IN_Tex.x * ColorMapSize.x * gridStep)) * step(0.1f, frac(IN_Tex.y * ColorMapSize.y * gridStep));
    mipLevelColor *= inGrid;

    return float4(mipLevelColor, 1);
}
#ifndef GL_STRUCTS_SHARED_H
#define GL_STRUCTS_SHARED_H

#include "Base/ueMath.h"

/**
 *	@addtogroup gl
 *	@{
 */

//! Shader constant types
enum glConstantType
{
	glConstantType_Sampler1D = 0,	//!< 1D sampler
	glConstantType_Sampler2D,		//!< 2D sampler
	glConstantType_Sampler3D,		//!< 3D sampler
	glConstantType_SamplerCube,		//!< Cube sampler
	glConstantType_Float,			//!< float
	glConstantType_Float2,			//!< float2
	glConstantType_Float3,			//!< float3
	glConstantType_Float4,			//!< float4
	glConstantType_Float3x3,		//!< float3x3
	glConstantType_Float4x3,		//!< float4x3
	glConstantType_Float4x4,		//!< float4x4
	glConstantType_Bool,			//!< bool
	glConstantType_Bool2,			//!< bool2
	glConstantType_Bool3,			//!< bool3
	glConstantType_Bool4,			//!< bool4
	glConstantType_Int,				//!< int
	glConstantType_Int2,			//!< int2
	glConstantType_Int3,			//!< int3
	glConstantType_Int4,			//!< int4

	glConstantType_MAX
};

UE_ENUM_BEGIN(glConstantType)
UE_ENUM_VALUE(glConstantType, Sampler1D)
UE_ENUM_VALUE(glConstantType, Sampler2D)
UE_ENUM_VALUE(glConstantType, Sampler3D)
UE_ENUM_VALUE(glConstantType, SamplerCube)
UE_ENUM_VALUE(glConstantType, Float)
UE_ENUM_VALUE(glConstantType, Float2)
UE_ENUM_VALUE(glConstantType, Float3)
UE_ENUM_VALUE(glConstantType, Float4)
UE_ENUM_VALUE(glConstantType, Float3x3)
UE_ENUM_VALUE(glConstantType, Float4x3)
UE_ENUM_VALUE(glConstantType, Float4x4)
UE_ENUM_VALUE(glConstantType, Bool)
UE_ENUM_VALUE(glConstantType, Bool2)
UE_ENUM_VALUE(glConstantType, Bool3)
UE_ENUM_VALUE(glConstantType, Bool4)
UE_ENUM_VALUE(glConstantType, Int)
UE_ENUM_VALUE(glConstantType, Int2)
UE_ENUM_VALUE(glConstantType, Int3)
UE_ENUM_VALUE(glConstantType, Int4)
UE_ENUM_END(glConstantType)

//! Blending functions
enum glBlendingFunc
{
	glBlendingFunc_One = 0,		//!< One
	glBlendingFunc_Zero,		//!< Zero
	glBlendingFunc_SrcAlpha,	//!< Source alpha
	glBlendingFunc_InvSrcAlpha,	//!< One minus source alpha
	glBlendingFunc_SrcColor,	//!< Source color
	glBlendingFunc_InvSrcColor,	//!< One minus source color
	glBlendingFunc_DstAlpha,	//!< Destination alpha
	glBlendingFunc_InvDstAlpha,	//!< One minus destination alpha
	glBlendingFunc_DstColor,	//!< Destination color
	glBlendingFunc_InvDstColor,	//!< One minus destination color

	glBlendingFunc_MAX
};

//! Compare functions
enum glCmpFunc
{
	glCmpFunc_Less = 0,	//!< Less
	glCmpFunc_LEqual,	//!< Less or equal
	glCmpFunc_Equal,	//!< Equal
	glCmpFunc_GEqual,	//!< Greater or equal
	glCmpFunc_Greater,	//!< Greater
	glCmpFunc_Never,	//!< Never
	glCmpFunc_Always,	//!< Always

	glCmpFunc_MAX
};

//! Shader types
enum glShaderType
{
	glShaderType_Vertex = 0,	//!< Vertex shader
	glShaderType_Fragment,		//!< Fragment shader
	glShaderType_Geometry,		//!< Geometry shader

	glShaderType_MAX
};

UE_ENUM_BEGIN(glShaderType)
UE_ENUM_VALUE(glShaderType, Vertex)
UE_ENUM_VALUE(glShaderType, Fragment)
UE_ENUM_VALUE(glShaderType, Geometry)
UE_ENUM_END(glShaderType)

//! Buffer formats
enum glBufferFormat
{
	glBufferFormat_A8 = 0,		//!< Alpha-8
	glBufferFormat_A16,			//!< Alpha-16
	glBufferFormat_B8G8R8,		//!< Blue-8 Green-8 Red-8
	glBufferFormat_R8G8B8,		//!< Red-8 Green-8 Blue-8
	glBufferFormat_B5G6R5,		//!< Blue-5 Green-6 Red-5
	glBufferFormat_R5G6B5,		//!< Red-5 Green-6 Blue-5
	glBufferFormat_B8G8R8A8,	//!< Alpha-8 Red-8 Green-8 Blue-8
	glBufferFormat_R8G8B8A8,	//!< Alpha-8 Blue-8 Green-8 Red-8
	glBufferFormat_B4G4R4A4,	//!< Alpha-4 Blue-4 Green-4 Red-4
	glBufferFormat_R4G4B4A4,	//!< Alpha-4 Red-4 Green-4 Blue-4
	glBufferFormat_B5G5R5A1,	//!< Blue-5 Green-5 Red-5 Alpha-1
	glBufferFormat_R5G5B5A1,	//!< Alpha-1 Red-5 Green-5 Blue-5

	glBufferFormat_D16,			//!< Depth-16
	glBufferFormat_D24,			//!< Depth-24
	glBufferFormat_D24S8,		//!< Depth-24 Stencil-8
	glBufferFormat_D32,			//!< Depth-32
	glBufferFormat_Depth,		//!< Depth (default supported depth format)

	glBufferFormat_DXT1,		//!< DXT1 (4 bits per pixel; RGB)
	glBufferFormat_DXT1a,		//!< DXT1a (4 bits per pixel; 1-bit alpha)
	glBufferFormat_DXT3,		//!< DXT3 (8 bits per pixel; RGB)
	glBufferFormat_DXT5,		//!< DXT5 (8 bits per pixel; 8-bit alpha and RGB)
	glBufferFormat_DXTN,		//!< DXTN (8 bits per pixel; used for normals)

	glBufferFormat_PVRTC2_RGB,	//!< PVRTC 2-bit RGB (iOS only)
	glBufferFormat_PVRTC2_RGBA,	//!< PVRTC 2-bit RGBA (iOS only)
	glBufferFormat_PVRTC4_RGB,	//!< PVRTC 4-bit RGB (iOS only)
	glBufferFormat_PVRTC4_RGBA,	//!< PVRTC 4-bit RGBA (iOS only)

	glBufferFormat_R16F,		//!< Red-16F
	glBufferFormat_R16G16F,		//!< Green-16F Red-16F
	glBufferFormat_R16G16B16A16F,//!< Alpha-16F Blue-16F Green-16F Red-16F

	glBufferFormat_R32F,		//!< Red-32F
	glBufferFormat_R32G32F,		//!< Green-32F Red-32F
	glBufferFormat_R32G32B32A32F,//!< Alpha-32F Blue-32F Green-32F Red-32F

	glBufferFormat_Unknown,		//!< Unknown

	glBufferFormat_MAX,

	//! Natively supported RGB format (could be RGB or BGR)
	glBufferFormat_Native_R8G8B8 =
#ifdef GL_OPENGL_ES
		glBufferFormat_R8G8B8
#else
		glBufferFormat_B8G8R8
#endif
	,

	//! Natively supported RGBA format (could be RGBA or BGRA or ARGB)
	glBufferFormat_Native_R8G8B8A8 =
#ifdef GL_OPENGL_ES
		glBufferFormat_R8G8B8A8
#else
		glBufferFormat_B8G8R8A8
#endif
};

UE_ENUM_BEGIN(glBufferFormat)
UE_ENUM_VALUE(glBufferFormat, A8)
UE_ENUM_VALUE(glBufferFormat, A16)
UE_ENUM_VALUE(glBufferFormat, B8G8R8)
UE_ENUM_VALUE(glBufferFormat, R8G8B8)
UE_ENUM_VALUE(glBufferFormat, B5G6R5)
UE_ENUM_VALUE(glBufferFormat, R5G6B5)
UE_ENUM_VALUE(glBufferFormat, B8G8R8A8)
UE_ENUM_VALUE(glBufferFormat, R8G8B8A8)
UE_ENUM_VALUE(glBufferFormat, B4G4R4A4)
UE_ENUM_VALUE(glBufferFormat, R4G4B4A4)
UE_ENUM_VALUE(glBufferFormat, B5G5R5A1)
UE_ENUM_VALUE(glBufferFormat, R5G5B5A1)

UE_ENUM_VALUE(glBufferFormat, D16)
UE_ENUM_VALUE(glBufferFormat, D24)
UE_ENUM_VALUE(glBufferFormat, D24S8)
UE_ENUM_VALUE(glBufferFormat, D32)
UE_ENUM_VALUE(glBufferFormat, Depth)

UE_ENUM_VALUE(glBufferFormat, DXT1)
UE_ENUM_VALUE(glBufferFormat, DXT1a)
UE_ENUM_VALUE(glBufferFormat, DXT3)
UE_ENUM_VALUE(glBufferFormat, DXT5)
UE_ENUM_VALUE(glBufferFormat, DXTN)

UE_ENUM_VALUE(glBufferFormat, PVRTC2_RGB)
UE_ENUM_VALUE(glBufferFormat, PVRTC2_RGBA)
UE_ENUM_VALUE(glBufferFormat, PVRTC4_RGB)
UE_ENUM_VALUE(glBufferFormat, PVRTC4_RGBA)

UE_ENUM_VALUE(glBufferFormat, R16F)
UE_ENUM_VALUE(glBufferFormat, R16G16F)
UE_ENUM_VALUE(glBufferFormat, R16G16B16A16F)

UE_ENUM_VALUE(glBufferFormat, R32F)
UE_ENUM_VALUE(glBufferFormat, R32G32F)
UE_ENUM_VALUE(glBufferFormat, R32G32B32A32F)

UE_ENUM_VALUE(glBufferFormat, Unknown)
UE_ENUM_END(glBufferFormat)

//! Serialized texture storage
enum glTexStorage
{
	glTexStorage_GPU = 0,	//!< GPU format (e.g. DXT1, ARGB8)

	glTexStorage_Original,	//!< Original source format; this is used by tools only

	glTexStorage_DDS,		//!< DDS format
	glTexStorage_JPG,		//!< JPG format
	glTexStorage_PNG,		//!< PNG format
	glTexStorage_HDR,		//!< HDR format
	glTexStorage_TGA,		//!< TGA format
	glTexStorage_PVR,		//!< PVR format

	glTexStorage_MAX
};

UE_ENUM_BEGIN(glTexStorage)
UE_ENUM_VALUE(glTexStorage, GPU)
UE_ENUM_VALUE(glTexStorage, Original)
UE_ENUM_VALUE(glTexStorage, DDS)
UE_ENUM_VALUE(glTexStorage, JPG)
UE_ENUM_VALUE(glTexStorage, PNG)
UE_ENUM_VALUE(glTexStorage, HDR)
UE_ENUM_VALUE(glTexStorage, TGA)
UE_ENUM_VALUE(glTexStorage, PVR)
UE_ENUM_END(glTexStorage)

//! Texture types
enum glTexType
{
	glTexType_2D = 0,	//!< 2D texture
	glTexType_3D,		//!< 3D texture
	glTexType_Cube,		//!< Cube texture

	glTexType_MAX
};

UE_ENUM_BEGIN(glTexType)
UE_ENUM_VALUE(glTexType, 2D)
UE_ENUM_VALUE(glTexType, 3D)
UE_ENUM_VALUE(glTexType, Cube)
UE_ENUM_END(glTexType)

//! Shader attribute semantic
enum glSemantic
{
	glSemantic_Position = 0,	//!< Position
	glSemantic_BoneWeights,		//!< Bone weights
	glSemantic_BoneIndices,		//!< Bone indices
	glSemantic_Normal,			//!< Normal vector
	glSemantic_PointSize,		//!< Point size
	glSemantic_TexCoord,		//!< Texture coordinates
	glSemantic_Tangent,			//!< Tangent vector
	glSemantic_Binormal,		//!< Binormal vector
	glSemantic_TessFactor,		//!< Tesselation factor
	glSemantic_PositionT,		//!< Transformed position within (0,0) -> (viewport width, viewport height) range
	glSemantic_Color,			//!< Color
	glSemantic_Fog,				//!< Fog
	glSemantic_Depth,			//!< Depth
	glSemantic_Sample,			//!< Sample
	glSemantic_Generic,			//!< Generic

	glSemantic_MAX
};

UE_ENUM_BEGIN(glSemantic)
UE_ENUM_VALUE(glSemantic, Position)
UE_ENUM_VALUE(glSemantic, BoneWeights)
UE_ENUM_VALUE(glSemantic, BoneIndices)
UE_ENUM_VALUE(glSemantic, Normal)
UE_ENUM_VALUE(glSemantic, PointSize)
UE_ENUM_VALUE(glSemantic, TexCoord)
UE_ENUM_VALUE(glSemantic, Tangent)
UE_ENUM_VALUE(glSemantic, Binormal)
UE_ENUM_VALUE(glSemantic, TessFactor)
UE_ENUM_VALUE(glSemantic, PositionT)
UE_ENUM_VALUE(glSemantic, Color)
UE_ENUM_VALUE(glSemantic, Fog)
UE_ENUM_VALUE(glSemantic, Depth)
UE_ENUM_VALUE(glSemantic, Sample)
UE_ENUM_VALUE(glSemantic, Generic)
UE_ENUM_END(glSemantic)

//! Shader attribute description
struct glShaderAttr
{
	u8 m_semantic;		//!< Shader semantic; @see glSemantic
	u8 m_usageIndex;	//!< Usage index starting from 0
};

//! Texture filtering modes
enum glTexFilter
{
	glTexFilter_None = 0,	//!< No filtering
	glTexFilter_Nearest,	//!< Nearest filtering
	glTexFilter_Linear,		//!< Linear filtering
	glTexFilter_Anisotropic,//!< Anisotropic filtering

	glTexFilter_MAX
};

//! Texture addressing mode
enum glTexAddr
{
	glTexAddr_Wrap = 0,		//!< Wrapping
	glTexAddr_Clamp,		//!< Clamp
	glTexAddr_ClampToBorder,//!< Clamp to specific border color
	glTexAddr_Mirror,		//!< Mirror
	glTexAddr_MirrorOnce,	//!< Mirro once only

	glTexAddr_MAX
};

//! Sampler render state description
struct glSamplerParams
{
	glTexAddr m_addressU; //!< Texture addressing mode along U coordinate
	glTexAddr m_addressV; //!< Texture addressing mode along V coordinate
	glTexAddr m_addressW; //!< Texture addressing mode along W coordinate (used only by volume and array textures)
	glTexFilter m_minFilter; //!< Filter to be used when in minimization
	glTexFilter m_magFilter; //!< Filter to be used when in maximization
	glTexFilter m_mipFilter; //!< Filter to be used to filter between mip levels

	u32 m_maxAnisotropicFilteringLevel; //!< Maximal anisotropic filtering level; defaults to 1 which means no anisotropic filtering

	u32 m_maxMipLevel; //!< Maximal LOD level to be used; defaults to max 0

	ueColor32 m_borderColor; //!< Texture border color; used only when border clamping mode is used

	//! Defaults

	UE_INLINE glSamplerParams() { SetDefaults(); }

	void SetDefaults()
	{
		m_addressU = glTexAddr_Wrap;
		m_addressV = glTexAddr_Wrap;
		m_addressW = glTexAddr_Wrap;
		m_minFilter = glTexFilter_Linear;
		m_magFilter = glTexFilter_Linear;
		m_mipFilter = glTexFilter_Nearest;
		m_maxAnisotropicFilteringLevel = 1;
		m_maxMipLevel = 0;
		m_borderColor = ueColor32::White;
	}

	UE_INLINE ueBool operator != (const glSamplerParams& other) const
	{
		return ueMemCmp(this, &other, sizeof(glSamplerParams)) != 0;
	}

	// Defaults

	//! Default wrapping state; tri-linear filtering
	static glSamplerParams DefaultWrap;
	//! Default clamped state; tri-linear filtering
	static glSamplerParams DefaultClamp;
	//! Default mirrored state; tri-linear filtering
	static glSamplerParams DefaultMirror;
	//! Default state used for post-processes; clamped, nearest filtering
	static glSamplerParams DefaultPP;
};

//! Render primitive types
enum glPrimitive
{
	glPrimitive_PointList = 0,	//!< List of points
	glPrimitive_LineList,		//!< List of lines
	glPrimitive_LineStrip,		//!< Line strip
	glPrimitive_TriangleList,	//!< Triangle list
	glPrimitive_TriangleStrip,	//!< Triangle strip

	glPrimitive_MAX
};

//! Description of vertex stream element
struct glVertexElement
{
	u8 m_semantic;		//!< Shader semantic; @see glSemantic
	u8 m_usageIndex;	//!< Usage index within 0..N range
	u8 m_type;			//!< Type; @see ueNumType
	u8 m_count;			//!< Elements count within 1..4 range
	u8 m_isNormalized;	//!< 1 for normalized, 0 otherwise
	u8 m_offset;		//!< Offset in bytes within stream
};

// @}

#endif // GL_STRUCTS_SHARED_H

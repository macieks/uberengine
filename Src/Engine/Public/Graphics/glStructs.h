#ifndef GL_STRUCTS_H
#define GL_STRUCTS_H

#include "Graphics/glStructs_Shared.h"

/**
 *	@addtogroup gl
 *	@{
 */

// Types

struct glDevice;
struct glCtx;
struct glTextureBuffer;
struct glIndexBuffer;
struct glVertexBuffer;
struct glShader;
struct glProgram;
struct glStreamFormat;
struct glRenderGroup;
struct glSwapChain;
struct glOcclusionQuery;

#define GL_MAX_TEXTURE_BUFFER_DEBUG_NAME 128

// Enumerations
// ------------------------------

//! Blending operations
enum glBlendOp
{
	glBlendOp_Add = 0,	//!< Add
	glBlendOp_Subtract,	//!< Subtract
	glBlendOp_Min,		//!< Minimum
	glBlendOp_Max,		//!< Maximum

	glBlendOp_MAX
};

//! Cull modes
enum glCullMode
{
	glCullMode_CW = 0,	//!< Clockwise
	glCullMode_CCW,		//!< Counter-clockwise
	glCullMode_None,	//!< None (disabled)

	glCullMode_MAX
};

//! Fill modes
enum glFillMode
{
	glFillMode_Points = 0,	//!< Draw points
	glFillMode_Wire,		//!< Draw lines
	glFillMode_Solid,		//!< Draw polygons (default)

	glFillMode_MAX
};

//! Clear flags
enum glClearFlag
{
	glClearFlag_Color	= UE_POW2(0),	//!< Color buffer
	glClearFlag_Depth	= UE_POW2(1),	//!< Depth buffer
	glClearFlag_Stencil	= UE_POW2(2),	//!< Stencil buffer

	glClearFlag_All		= glClearFlag_Color | glClearFlag_Depth | glClearFlag_Stencil
};

//! Display synchronization modes
enum glDisplaySync
{
	glDisplaySync_None = 0,		//!< None
	glDisplaySync_Vertical,		//!< Vertical synchronization
	glDisplaySync_Horizontal,	//!< Horizontal synchronization

	glDisplaySync_MAX
};

//! Stencil operations
enum glStencilOperation
{
	glStencilOp_Zero = 0,	//!< Zeroe value
	glStencilOp_Keep,		//!< Keep value
	glStencilOp_Replace,	//!< Replace value
	glStencilOp_Incr,		//!< Increment value
	glStencilOp_Decr,		//!< Decrement value
	glStencilOp_Invert,		//!< Invert value

	glStencilOp_MAX
};

//! Cube texture faces
enum glCubeFace
{
	glCubeFace_PosX = 0,	//!< +X
	glCubeFace_NegX,		//!< -X
	glCubeFace_PosY,		//!< +Y
	glCubeFace_NegY,		//!< -Y
	glCubeFace_PosZ,		//!< +Z
	glCubeFace_NegZ,		//!< -Z

	glCubeFace_MAX
};

//! Color buffer masks
enum glColorMask
{
	glColorMask_Red		= UE_POW2(0),	//!< Red
	glColorMask_Green	= UE_POW2(1),	//!< Green
	glColorMask_Blue	= UE_POW2(2),	//!< Blue
	glColorMask_Alpha	= UE_POW2(3),	//!< Alpha

	glColorMask_All = glColorMask_Red | glColorMask_Green | glColorMask_Blue | glColorMask_Alpha
};

//! Context reset flags; @see glCtx_Reset
enum glResetFlags
{
	// Base groups

	glResetFlags_RenderStates	= UE_POW2(0),	//!< Resets all render states
	glResetFlags_Streams		= UE_POW2(1),	//!< Resets index & vertex buffers
	glResetFlags_Samplers		= UE_POW2(2),	//!< Resets samplers
	glResetFlags_Shaders		= UE_POW2(3),	//!< Resets shaders
	glResetFlags_RenderTargets	= UE_POW2(4),	//!< Resets render targets

	// Combo-groups

	glResetFlags_Resources		= glResetFlags_Streams | glResetFlags_Samplers | glResetFlags_Shaders | glResetFlags_RenderTargets,
	glResetFlags_All			= glResetFlags_RenderStates | glResetFlags_Resources
};

//! Device query type
enum glQueryType
{
	glQueryType_OcclusionNumSamples = UE_POW2(0),
	glQueryType_OcclusionAnySample = UE_POW2(1),
	glQueryType_OcclusionPredicate = UE_POW2(2),
	glQueryType_MAX
};

//! Base device capabilities
struct glCaps_Base
{
	//! Information about specific format
	struct FormatInfo
	{
		ueBool m_supportedAsRTOnly;			//!< Is supported as render target only?
		ueBool m_supportedAsTextureOnly;	//!< Is supported as texture only?
		ueBool m_supportedAsRTAndTexture;	//!< Is supported as render target and texture?
		ueBool m_supportsBlending;			//!< Supports blending?
		ueBool m_supportsFiltering;			//!< Supports linear filtering?
		ueBool m_supportsAutoMips;			//!< Supports automatic mipmaps?

		FormatInfo();
	};

	ueBool m_supportsOcclusionQuery;			//!< Supports occlusion query (number of samples)?
	ueBool m_supportsOcclusionQueryAnySample;	//!< Supports occlusion query (any samples)?
	ueBool m_supportsOcclusionPredicateQuery;	//!< Supports predicate query? (conditional rendering)
	ueBool m_supportsIndependentColorWriteMasks;//!< Supports independent color write masks?
	ueBool m_supportsTwoSidedStencil;			//!< Supports two-sided stencil operations?

	ueBool m_hasHalfPixelOffset;				//!< Indicates whether half-pixel offset is enabled (e.g. D3D9)

	u32 m_maxVertexStreams;						//!< Max. number of vertex streams
	u32 m_maxColorRenderTargets;				//!< Max. number of color render targets
	u32 m_maxSamplers[glShaderType_MAX];		//!< Max. number of simultaneous samplers

	u32 m_shaderVersion[glShaderType_MAX];		//!< Max. supported shader version
	u32 m_maxShaderRegisters[glShaderType_MAX];	//!< Max. supported of shader registers

	u32 m_numMSAALevels;		//!< Number of supported MSAA levels
	u32 m_MSAALevels[64];		//!< Supported MSAA levels
	u32 m_maxAnisotropicFilteringlevel;			//!< Max. supported anisotropic filtering levels

	FormatInfo m_formatInfos[glBufferFormat_MAX];//!< Support info for all buffer formats

	u32 m_maxPrimitives;						//!< Max. number of primitives per draw call
	u32 m_maxUserClippingPlanes;				//!< Max. number of user clipping planes

	glCaps_Base();
};

//! Base device startup params
struct glDeviceStartupParams_Base
{
	ueAllocator* m_stackAllocator;		//!< Prefarably stack allocator
	ueAllocator* m_freqAllocator;		//!< Random access allocator

	u32 m_width;						//!< Back buffer width
	u32 m_height;						//!< Back buffer depth
	u32 m_frequency;					//!< Display frequency
	glBufferFormat m_colorFormat;		//!< Back buffer color format
	glBufferFormat m_depthStencilFormat;//!< Back buffer depth-stencil format
	glDisplaySync m_displaySync;		//!< Display synchronization mode
	u32 m_backBufferCount;				//!< Back buffer count (2 by default)
	u32 m_MSAALevel;			//!< MSAA level (1 by default = no MSAA)

	// Pool sizes for various resources

	u32 m_maxContexts;					//!< Max. number of contexts
	u32 m_maxRenderGroups;				//!< Max. number of render groups (@see struct glRenderGroup)
	u32 m_maxPoolManagedTextureBuffers;	//!< Max. number of texture buffers managed by pool (glRenderBufferPool)
	u32 m_maxTextureBuffers;			//!< Max. number of texture buffer (@see struct glTextureBuffer)
	u32 m_maxOcclusionQueries;			//!< Max number of occlusion queries
	u32 m_maxVertexBuffers;				//!< Max. number of vertex buffer (@see struct glVertexBuffer)
	u32 m_maxIndexBuffers;				//!< Max. number of index buffer (@see struct glIndexBuffer)
	u32 m_maxStreamFormats;				//!< Max. number of different stream formats (@see struct glStreamFormat)
	u32 m_maxVertexShaderInputs;		//!< Max. number of different vertex shader inputs
	u32 m_maxPrograms;					//!< Max. number of shader programs (@see struct glProgram)

	u32 m_maxGlobalSamplerConstants;	//!< Max. number of global sampler constants
	u32 m_maxGlobalNumericConstants;	//!< Max. number of global numerical constants
	u32 m_maxGlobalNumericConstantsDataSize;//!< Max. total size in bytes of all global numerical constants
	u32 m_constantNameBufferSize;		//!< Max. total size in bytes of names of all global sampler constants

	//! Sets up defaults
	glDeviceStartupParams_Base();
};

//! Texture buffer description base
struct glTextureBufferDesc_Base
{
	glTexType m_type;			//!< Texture type
	glBufferFormat m_format;	//!< Texture format
	u32 m_width;				//!< Width
	u32 m_height;				//!< Height
	u32 m_depth;				//!< Depth; 1 for non-3D textures
	u32 m_numLevels;			//!< 0 indicates all mip-levels
	ueBool m_enableAutoMips;	//!< Whether mip levels are to be automatically regenerated when modified
	ueBool m_enableManualMips;	//!< Whether mip levels generation via glTextureBuffer_GenerateMips is to be supported

	glTextureBufferDesc_Base() :
		m_type(glTexType_2D),
		m_format(glBufferFormat_Native_R8G8B8A8),
		m_width(64),
		m_height(64),
		m_depth(1),
		m_numLevels(0),
		m_enableAutoMips(UE_FALSE),
		m_enableManualMips(UE_FALSE)
	{}

	UE_INLINE ueBool operator == (const glTextureBufferDesc_Base& other) const
	{
		return
			m_type == other.m_type &&
			m_format == other.m_format &&
			m_width == other.m_width &&
			m_height == other.m_height &&
			m_depth == other.m_depth &&
			m_numLevels == other.m_numLevels &&
			m_enableAutoMips == other.m_enableAutoMips &&
			m_enableManualMips == other.m_enableManualMips;
	}
};

//! Description of a texture block
struct glTextureArea
{
	u32 m_x;		//!< X coordinate
	u32 m_y;		//!< Y coordinate
	u32 m_z;		//!< Z coordinate; only used by 3D texture
	u32 m_width;	//!< Block width
	u32 m_height;	//!< Block height
	u32 m_depth;	//!< Block depth; only used by 3D texture

	UE_INLINE glTextureArea() :
		m_x(0), m_y(0), m_z(0),
		m_width(0), m_height(0), m_depth(0)
	{}
};

//! Locked texture block
struct glTextureData
{
	u32 m_rowPitch;		//!< Row pitch (or data size if non-GPU format, e.g. PNG or JPG)
	u32 m_slicePitch;	//!< Slice pitch; only used by 3D textures
	void* m_data;		//!< Pointer to start of the first row of the block

	UE_INLINE glTextureData() :
		m_rowPitch(0),
		m_slicePitch(0),
		m_data(NULL)
	{}
};

//! Texture initialization data
struct glTextureInitData
{
	glTexStorage m_storage;		//!< Data storage; for non-GPU data is stored under first m_dataArray entry (m_data is data and m_rowSize is total data size)

	u32 m_dataArrayLength;		//!< Length of the data array
	glTextureData* m_dataArray;	//!< Data array; one entry per sub-resource (face's mip-level)

	glTextureInitData() :
		m_storage(glTexStorage_GPU),
		m_dataArrayLength(0),
		m_dataArray(NULL)
	{}
};

//! Serialized texture description
struct glTextureFileDesc
{
	u32 m_storage;		//!< glTexStorage
	u32 m_type;			//!< glTexType
	u32 m_format;		//!< glBufferFormat
	u32 m_width;		//!< Width in pixels
	u32 m_height;		//!< Height in pixels
	u32 m_depth;		//!< Depth in pixels
	u32 m_numLevels;	//!< Number of levels this texture should have (either loaded precomputed ones or generated)
	ueBool m_enableAutoMips;	//!< Indicates whether automatic mip-map generation shall be enabled for this texture
	ueBool m_isDynamic;	//!< Indicates whether texture is to be dynamically modifiable

	u32 m_dataArrayLength;		//!< Number of elements in the texture
	glTextureData* m_dataArray;	//!< Texture elements (one per mip-level and cube/array texture face)
};

// Shader constant handle
// -----------------------------

struct glConstant;

//! Global shader constant handle
class glConstantHandle
{
public:
	UE_INLINE glConstantHandle() : m_constant(NULL) {}
	//! Initializes handle with constant as described by name, type and elements count
	ueBool Init(const char* name, glConstantType type, u32 count = 1);
	//! Tells whether handle is valid
	UE_INLINE ueBool IsValid() const { return m_constant != NULL; }
	//! Gets managed constant
	UE_INLINE const glConstant* GetConstant() const { return m_constant; }
	//! Gets constant name
	const char* GetName() const;
	//! Gets constant type
	glConstantType GetType() const;
	//! Gets constant value count
	u32 GetCount() const;

private:
	const glConstant* m_constant; //!< Managed constant
};

// Vertex format
// -----------------------------

//! Description of a vertex stream format
struct glStreamFormatDesc
{
	u32 m_stride;						//!< Vertex stride
	u32 m_numElements;					//!< Number of elements
	const glVertexElement* m_elements;	//!< An array of elements
};

//! Viewport description
struct glViewportDesc
{
	s32 m_left;		//!< Left
	s32 m_top;		//!< Top
	s32 m_width;	//!< Width
	s32 m_height;	//!< Height
	f32 m_minZ;		//!< Min. Z coordinate
	f32 m_maxZ;		//!< Max. Z coordinate
};

//! Render buffer group description
struct glRenderGroupDesc
{
	//! Description of the buffer within render buffer group
	struct BufferDesc
	{
		glBufferFormat m_format;	//!< Buffer format
		u32 m_numLevels;			//!< Number of texture mip levels; 0 indicates all
		ueBool m_enableAutoMips;	//!< Indicates whether to enable automatic mip-map generation

		BufferDesc() :
			m_format(glBufferFormat_Native_R8G8B8A8),
			m_numLevels(1),
			m_enableAutoMips(UE_FALSE)
		{}
	};

	u32 m_width;						//!< Width of all render targets
	u32 m_height;						//!< Height of all render targets
	u32 m_MSAALevel;			//!< MSAA level for all (where applicable, e.g. excluding depth buffer) render targets

	ueBool m_hasDepthStencil;			//!< Indicates whether group has depth-stencil buffer
	ueBool m_needDepthStencilContent;	//!< Indicates whether depth-stencil content of the rendering is needed
	BufferDesc m_depthStencilDesc;		//!< Description of the depth-stencil buffer

	ueBool m_needColorContent;			//!< Indicates whether color content of the rendering is needed (for all color render targets used)
	u32 m_numColorBuffers;				//!< Number of color render targets
	BufferDesc m_colorDesc[GL_MAX_COLOR_RENDER_TARGETS]; //!< Description of all color render buffers

	ueBool m_isMainFrameBuffer;			//!< Private use: indicates whether render group is main frame buffer

	glRenderGroupDesc() :
		m_width(256),
		m_height(256),
		m_hasDepthStencil(UE_FALSE),
		m_needDepthStencilContent(UE_FALSE),
		m_needColorContent(UE_FALSE),
		m_numColorBuffers(1),
		m_MSAALevel(1),
		m_isMainFrameBuffer(UE_FALSE)
	{
		m_depthStencilDesc.m_format = glBufferFormat_Depth;
	}
};

//! Shader constant description
struct glShaderConstantDesc
{
	const char* m_name;	//!< Unique constant name
	u32 m_type;			//!< Constant type (glConstantType)
	u32 m_count;		//!< Elements count
};

//! Shader constant instance description
struct glShaderConstantInstanceDesc
{
	glShaderConstantDesc m_desc;	//!< Description
	u32 m_offset;					//!< Offset within shader registers / samplers
	ueBool m_isColumnMajor;			//!< Indicates whether value is column major; used for matrices only
};

//! Vertex shader input description
struct glVertexShaderInputDesc
{
	u32 m_numAttrs;			//!< Number of attributes
	glShaderAttr* m_attrs;	//!< An array of attributes
};

//! Shader description
struct glShaderDesc
{
	glShaderType m_type;		//!< Shader type

	u32 m_numConstants;			//!< Number of shader constants
	glShaderConstantInstanceDesc* m_constants;	//!< An array of shader constants

	glVertexShaderInputDesc* m_VSInputDesc;	//!< Input description (only used for vertex shaders)
	u32 m_numOutputs;			//!< Number of output attributes

	u32 m_codeSize;				//!< Shader code size
	void* m_code;				//!< Shader code buffer

#if defined(GL_D3D11)

	struct BufferDesc
	{
		u32 m_size;
	};

	u32 m_numBuffers;			//!< Number of constant buffers
	BufferDesc* m_bufferDescs;	//!< Constant buffers descriptions

#endif

	glShaderDesc(glShaderType type) :
		m_type(type),
		m_codeSize(0),
		m_code(NULL),
		m_numConstants(0),
		m_constants(NULL),
		m_VSInputDesc(NULL),
		m_numOutputs(0)
#if defined(GL_D3D11)
		, m_numBuffers(0),
		m_bufferDescs(NULL)
#endif
	{}
};

// Index & vertex buffer
// ------------------------------

//! Index or vertex buffer flags
enum glBufferFlags
{
	glBufferFlags_Dynamic		= UE_POW2(0),	//!< Buffer is dynamic i.e. will be modified frequently
	glBufferFlags_WriteOnly		= UE_POW2(1)	//!< Buffer is write-only (no read operations will be allowed)
};

//! Index or vertex buffer lock flags
enum glBufferLockFlags
{
	glBufferLockFlags_Read			= UE_POW2(0),	//!< Want to read
	glBufferLockFlags_Write			= UE_POW2(1),	//!< Want to write
	glBufferLockFlags_NoOverwrite	= UE_POW2(2),	//!< Will not overwrite any in-use part of the buffer
	glBufferLockFlags_Discard		= UE_POW2(3)	//!< May discard content of the whole buffer - it's not needed anymore
};

//! Index buffer description
struct glIndexBufferDesc
{
	u32 m_flags;		//!< Index buffer flags; see glBufferFlags
	u32 m_numIndices;	//!< Number of indices
	u32 m_indexSize;	//!< Size of the index value; 2 for 16-bit shorts or 4 for 32-bit ints

	glIndexBufferDesc() :
		m_flags(glBufferFlags_WriteOnly),
		m_numIndices(0),
		m_indexSize(sizeof(u32))
	{}
};

//! Vertex buffer description
struct glVertexBufferDesc
{
	u32 m_flags;	//!< Vertex buffer flags; see glBufferFlags
	u32 m_size;		//!< Size of the vertex buffer data

	glVertexBufferDesc() :
		m_flags(glBufferFlags_WriteOnly),
		m_size(0)
	{}
};

//! Display adapter description
struct glAdapterInfo
{
	//! Display adapter flags
	enum Flags
	{
		Flags_IsPrimary			= UE_POW2(0),
		Flags_HasMonitor		= UE_POW2(1),
		Flags_HasPrimaryMonitor	= UE_POW2(2)
	};

	u32 m_flags;			        //!< Adapter flags; see Flags
	u32 m_numModes;					//!< Number of display modes

	char m_driver[256];				//!< Driver name
	char m_description[128];		//!< Adapter description
	char m_deviceName[32];			//!< Device name

	u32 m_monitorWidth;		        //!< Monitor width in pixels
	u32 m_monitorHeight;	        //!< Monitor height in pixels
	f32 m_aspectRatio;				//!< Monitor aspect ratio
};

//! Display output mode
struct glOutputMode
{
	u32 m_width;			//!< Width in pixels
	u32 m_height;			//!< Height in pixels
	u32 m_frequency;		//!< Refresh rate in Hz
	u32 m_bitsPerPixel;		//!< Bits per color buffer pixel (e.g. ARGB8 is 32 bits)
};

// @}

#endif // GL_STRUCTS_H

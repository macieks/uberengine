#pragma once

struct TexLib_Context;
struct TexLib_Header;
struct TexLib_LevelData;

//! Allocation callback
typedef void* (*TexLib_AllocFunc)(TexLib_Context* ctx, int size);
//! Deallocation callback
typedef void (*TexLib_FreeFunc)(TexLib_Context* ctx, void* ptr);

//! Data reading callback; returns true on success, false otherwise
typedef bool (*TexLib_ReadFunc)(TexLib_Context* ctx, void* data, int size);
//! On texture header callback; returns true to continue loading the texture or false otherwise
typedef bool (*TexLib_OnHeaderFunc)(TexLib_Context* ctx, const TexLib_Header* header);
//! On texture face level callback; fills out data; return true on success, false otherwise
typedef bool (*TexLib_OnBeginFaceLevelFunc)(TexLib_Context* ctx, const TexLib_Header* header, int faceIndex, int levelIndex, TexLib_LevelData* data);
//! On texture face level end callback
typedef void (*TexLib_OnEndFaceLevelFunc)(TexLib_Context* ctx, const TexLib_Header* header, int faceIndex, int levelIndex);

//! Default memory allocator
extern void* TexLib_DefaultAlloc(TexLib_Context* ctx, int size);
//! Default memory deallocator
extern void TexLib_DefaultFree(TexLib_Context* ctx, void* ptr);

//! Available input texture types
enum TexLib_Type
{
	TexLib_Type_PNG = 0,
	TexLib_Type_TGA,
	TexLib_Type_JPG,
	TexLib_Type_DDS,

	TexLib_Type_Unknown
};

//! Available output texture formats
enum TexLib_Format
{
	TexLib_Format_A = 0,	// Not yet supported
	TexLib_Format_RGB,
	TexLib_Format_ARGB,
	TexLib_Format_DXT1,
	TexLib_Format_DXT1a,	// Not yet supported
	TexLib_Format_DXT3,		// Only supported for DDS textures
	TexLib_Format_DXT5,

	TexLib_Format_Unknown
};

//! Texture header
struct TexLib_Header
{
	int m_width;			//!< Image width
	int m_height;			//!< Image height; 0 for 1D textures
	int m_depth;			//!< Image depth; 0 for 2D and cube textures
	int m_numFaces;			//!< Number of faces the image has
	int m_numLevels;		//!< Number of levels the image has
	bool m_hasColor;		//!< Indicates whether texture has RGB color channel
	bool m_hasAlpha;		//!< Indicates whether texture has alpha channel
	TexLib_Format m_format; //!< Nearest format; for DXT textures the output format has to match this one
};

//! Texture level data supplied by the user
struct TexLib_LevelData
{
	TexLib_Format m_format;	//!< Format to convert texture to
	unsigned char* m_data;	//!< Pointer to output level data to
	int m_dataSize;			//!< Data size
	int m_rowPitch;			//!< Row pitch
	int m_slicePitch;		//!< Slice pitch
};

//! User supplied texture context
struct TexLib_Context
{
	bool m_loadFromFile;							//<! Indicates whether to load image from file; if so, m_path must be set to valid path

	TexLib_AllocFunc m_alloc;						//!< Allocation function; defaults to malloc
	TexLib_FreeFunc m_free;							//!< Free function; defaults to free

	TexLib_OnHeaderFunc m_onHeader;					//!< Callback function for when texture header information is known
	TexLib_OnBeginFaceLevelFunc m_onBeginFaceLevel;	//!< Callback function for when texture level face reading begins
	TexLib_OnEndFaceLevelFunc m_onEndFaceLevel;		//!< Callback function for when texture level face reading is finished

	TexLib_Type m_type;								//!< [optional] Texture type; must be set if reading from stream; only used when m_loadFromFile is set to false
	TexLib_ReadFunc m_read;							//!< [optional] Read function; only used when m_loadFromFile is set to false

	const char* m_path;								//!< Only used when m_loadFromFile is set to true

	void* m_userData;								//!< User data

	void* m_privateData;							//!< Private; do not use

	TexLib_Context() :
		m_loadFromFile(true),
		m_alloc(TexLib_DefaultAlloc),
		m_free(TexLib_DefaultFree),
		m_onHeader(NULL),
		m_onBeginFaceLevel(NULL),
		m_onEndFaceLevel(NULL),
		m_type(TexLib_Type_Unknown),
		m_read(NULL),
		m_path(NULL),
		m_userData(NULL),
		m_privateData(NULL)
	{}
};

//! Loads texture using given context; return true on success, false otherwise
bool TexLib_Load(TexLib_Context* ctx);
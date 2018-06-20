#pragma once

#include "ContentPipeline/ueContentPipeline.h"
#include "GL/glStructs_Shared.h"
#include "Math/ueShapes.h"
#include <d3dx9.h>

class ioPackageWriter;
class ioSegmentWriter;

enum ueTextureRescaleMethod
{
	ueTextureRescaleMethod_ToPreviousPowerOf2 = 0,
	ueTextureRescaleMethod_ToNextPowerOf2,
	ueTextureRescaleMethod_ToNearestPowerOf2,
	ueTextureRescaleMethod_None,

	ueTextureRescaleMethod_MAX
};

UE_ENUM_BEGIN(ueTextureRescaleMethod)
UE_ENUM_ADD(ueTextureRescaleMethod, ToPreviousPowerOf2)
UE_ENUM_ADD(ueTextureRescaleMethod, ToNextPowerOf2)
UE_ENUM_ADD(ueTextureRescaleMethod, ToNearestPowerOf2)
UE_ENUM_ADD(ueTextureRescaleMethod, None)
UE_ENUM_END()

class ueToolTexture
{
public:
	struct LoadSettings
	{
		std::vector<std::string> m_sourceFileNames;
		bool m_isAtlas;
		std::string m_atlasDescriptionFilePath;
		u32 m_borderInAtlas;
		glTexType m_type;
		glTexStorage m_storage;
		glBufferFormat m_format;
		u32 m_width;
		u32 m_height;
		u32 m_depth;
		u32 m_maxWidth;
		u32 m_maxHeight;
		u32 m_maxDepth;
		u32 m_numLevels;
		ueTextureRescaleMethod m_rescaleMethod;
		bool m_isNormalMap;
		bool m_isDynamic;
		bool m_makePow2;

		LoadSettings() :
			m_isAtlas(false),
			m_borderInAtlas(1),
			m_storage(glTexStorage_GPU),
			m_type(glTexType_2D),
			m_format(glBufferFormat_DXT1),
			m_width(0),
			m_height(0),
			m_depth(0),
			m_maxWidth(0),
			m_maxHeight(0),
			m_maxDepth(0),
			m_numLevels(0),
			m_rescaleMethod(ueTextureRescaleMethod_ToPreviousPowerOf2),
			m_isNormalMap(false),
			m_isDynamic(false),
			m_makePow2(false)
		{}
	};

	struct Level
	{
		bool m_ownsMemory;
		void* m_data;
		u32 m_dataSize;
		u32 m_width;
		u32 m_height;
		u32 m_depth;

		Level() : m_ownsMemory(false), m_data(NULL) {}
	};

	struct Image
	{
		std::string m_name;
		std::vector<Level> m_levels;
	};

	struct AtlasEntry
	{
		std::string m_name;
		ueRectI m_rectInAtlas;
		f32 m_scale[2];
		f32 m_translation[2];
	};

	LoadSettings m_settings;

	ioPath m_outputPath;

	glTexStorage m_storage;		//!< Current storage
	glBufferFormat m_format;	//!< Current format

	glTexType m_type;
	bool m_enableAutoMips;

	bool m_isAtlas;
	bool m_atlasIsReady;		//!< Indicates that the texture atlas is already built (only used to track current texture/atlas build state)
	std::vector<AtlasEntry> m_atlasEntries; //!< Texture atlas entries

	std::vector<Image> m_faces; //!< An image per face (1D, 2D & 3D textures have 1; cube texture has 6; array texture has N)

	ueToolTexture();
	~ueToolTexture();
	void Destroy();

	bool Create2D(u32 width, u32 height, u32 numLevels, glBufferFormat format, u8 initByteValue = 0);
	void FillColor(ueColor32 color);
	bool Serialize(ioPackageWriter* pw);

	UE_INLINE u32 GetWidth() const { return m_faces[0].m_levels[0].m_width; }
	UE_INLINE u32 GetHeight() const { return m_faces[0].m_levels[0].m_height; }
	UE_INLINE u32 GetDepth() const { return m_faces[0].m_levels[0].m_depth; }
	UE_INLINE u32 GetNumLevels() const { return (u32) m_faces[0].m_levels.size(); }
	UE_INLINE u32 GetNumFaces() const { return (u32) m_faces.size(); }

	// Utils

	static void DumpSettings();
	static bool ParseSettings(LoadSettings& s, const ueAssetParams& params);

	void MakeSingleTextureAtlas(u32 totalWidth, u32 totalHeight, u32 actualWidth, u32 actualHeight);
	bool BuildAtlas();
	bool ConvertToOutputFormat();

protected:
	void SerializeGPU(ioSegmentWriter* sw);
	void SerializeAtlasInfo(ioPackageWriter* pw);
	void ReorderChannels();
	bool PackAtlas();
	bool ParseAtlasDescriptionFromFile();

	bool ConvertToOutputFormat(u32 faceIndex);
	bool ConvertToOutputFormatD3DX(u32 faceIndex);
	bool ConvertToOutputFormatPVRTC(u32 faceIndex);

	void SaveImageDebugD3DX();

	bool SaveAsNonGPU(Level& level, const char* path, D3DXIMAGE_FILEFORMAT format, ID3DXBuffer** buffer = NULL);
};
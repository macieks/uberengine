#include "TextureCompiler_Common/ueToolTexture.h"
#include "Misc/ueTextureRectPacker.h"
#include "IO/ioPackageWriter.h"
#include "IO/ioPackageWriterUtils.h"
#include "GL/glLib_Shared.h"

extern ueBool glUtils_FormatRequiresSquareDimensions(glBufferFormat format);

ueToolTexture::ueToolTexture() :
	m_enableAutoMips(false),
	m_isAtlas(false),
	m_atlasIsReady(false)
{}

ueToolTexture::~ueToolTexture()
{
	Destroy();
}

void ueToolTexture::Destroy()
{
	for (u32 i = 0; i < m_faces.size(); i++)
		for (u32 j = 0; j < m_faces[i].m_levels.size(); j++)
		{
			Level& level = m_faces[i].m_levels[j];
			if (level.m_ownsMemory && level.m_data)
				free(level.m_data);
		}
	m_faces.clear();
}

bool ueToolTexture::Create2D(u32 width, u32 height, u32 numLevels, glBufferFormat format, u8 initByteValue)
{
	m_faces.clear();

	if (numLevels == 0)
		numLevels = glUtils_CalcNumMipLevels(width, height, 1);

	m_type = glTexType_2D;
	m_storage = glTexStorage_GPU;
	m_format = format;

	Image& face = vector_push(m_faces);
	for (u32 i = 0; i < numLevels; i++)
	{
		Level& level = vector_push(face.m_levels);
		level.m_width = ueMax(1U, width >> i);
		level.m_height = ueMax(1U, height >> i);
		level.m_depth = 1;
		level.m_dataSize = glUtils_CalcSurfaceSize(glTexType_2D, level.m_width, level.m_height, level.m_depth, m_format);
		level.m_data = new u8[level.m_dataSize];
		UE_ASSERT(level.m_data);

		ueMemSet(level.m_data, initByteValue, level.m_dataSize);
	}
	return true;
}

void ueToolTexture::FillColor(ueColor32 color)
{
	const u32 pixelSize = glUtils_GetFormatBytes(m_format);

	u32 r, g, b, a;
	glUtils_GetChannelIndices(m_format, r, g, b, a);

	u8 pixel[16] = {0};
	if (r != 0xFFFFFFFF) pixel[r] = color.r;
	if (g != 0xFFFFFFFF) pixel[g] = color.g;
	if (b != 0xFFFFFFFF) pixel[b] = color.b;
	if (a != 0xFFFFFFFF) pixel[a] = color.a;

	for (u32 i = 0; i < m_faces.size(); i++)
	{
		Image& face = m_faces[i];
		for (u32 j = 0; j < face.m_levels.size(); j++)
		{
			Level& level = face.m_levels[j];
			const u32 numPixels = level.m_width * level.m_height;
			u8* dst = (u8*) level.m_data;
			for (u32 p = 0; p < numPixels; p++, dst += pixelSize)
				ueMemCpy(dst, pixel, pixelSize);
		}
	}
}

bool ueToolTexture::PackAtlas()
{
	const bool is4x4Compression = glUtils_Is4x4CompressedFormat(m_format);

	// Prepare input data for atlas generator

	std::vector<ueTextureRectPacker::Rect> rects;
	for (u32 i = 0; i < m_faces.size(); i++)
	{
		Image& srcImg = m_faces[i];
		ueTextureRectPacker::Rect& dstRect = vector_push(rects);

		dstRect.w = srcImg.m_levels[0].m_width;
		dstRect.h = srcImg.m_levels[0].m_height;
		dstRect.m_userData = &srcImg;

		// 1st step of compression fix up - set 1/4th of the dimensions to properly handle compressions with 4x4 pixel blocks (e.g. DXT, PVRTC)

		if (is4x4Compression)
		{
			dstRect.w = ueMax(1U, dstRect.w / 4);
			dstRect.h = ueMax(1U, dstRect.w / 4);
		}
	}

	// Run atlas generator

	u32 totalWidth, totalHeight;
	ueTextureRectPacker::Solve(m_settings.m_borderInAtlas * 2, 1, true, rects, totalWidth, totalHeight);

	// 2nd step of compression fix up - fix back positions/dimensions to what they should be

	if (is4x4Compression)
	{
		for (u32 i = 0; i < m_faces.size(); i++)
		{
			ueTextureRectPacker::Rect& rect = rects[i];
			const Image& img = *(const Image*) rect.m_userData;

			rect.x *= 4;
			rect.y *= 4;
			rect.w = img.m_levels[0].m_width;
			rect.h = img.m_levels[0].m_height;
		}

		totalWidth *= 4;
		totalHeight *= 4;
	}

	// Check for max width/height

	if (totalWidth > m_settings.m_maxWidth || totalHeight > m_settings.m_maxHeight)
	{
		ueLogE("Generated texture (%ux%u) atlas exceeded max specified dimensions (%ux%u)", totalWidth, totalHeight, m_settings.m_maxWidth, m_settings.m_maxHeight);
		return false;
	}

	// Make square if needed

	if (glUtils_FormatRequiresSquareDimensions(m_settings.m_format))
		totalWidth = totalHeight = ueMax(totalWidth, totalHeight);

	// Create atlas texture

	Image atlasImg;
	atlasImg.m_name = "<atlas>";
	ueToolTexture::Level& atlasLevel0 = vector_push(atlasImg.m_levels);
	atlasLevel0.m_width = totalWidth;
	atlasLevel0.m_height = totalHeight;
	atlasLevel0.m_depth = 1;
	atlasLevel0.m_ownsMemory = true;

	u32 atlasRowSize, atlasNumRows;
	glUtils_CalcSurfaceRowParams(totalWidth, totalHeight, 1, m_format, atlasRowSize, atlasNumRows);
	atlasLevel0.m_dataSize = atlasRowSize * atlasNumRows;

	atlasLevel0.m_data = malloc(atlasLevel0.m_dataSize);
	if (!atlasLevel0.m_data)
	{
		ueLogE("Failed to allocate %u bytes for texture atlas.", atlasLevel0.m_dataSize);
		return false;
	}
	ueMemZero(atlasLevel0.m_data, atlasLevel0.m_dataSize);

	// Create atlas entries and copy them into atlas

	for (u32 i = 0; i < rects.size(); i++)
	{
		const ueTextureRectPacker::Rect& rect = rects[i];
		const Image& img = *(const Image*) rect.m_userData;

		// Create atlas entry info

		AtlasEntry& entry = vector_push(m_atlasEntries);
		entry.m_name = img.m_name.substr(0, img.m_name.length() - 4); // Auto-determine resource name based on source file name (just getting rid of 3-characters long extension)
		entry.m_rectInAtlas = ueRectI(rect.x, rect.y, rect.w, rect.h);
		entry.m_translation[0] = (f32) entry.m_rectInAtlas.m_left / (f32) totalWidth;
		entry.m_translation[1] = (f32) entry.m_rectInAtlas.m_top / (f32) totalHeight;
		entry.m_scale[0] = (f32) entry.m_rectInAtlas.m_width / (f32) totalWidth;
		entry.m_scale[1] = (f32) entry.m_rectInAtlas.m_height / (f32) totalHeight;

		// Get source image params

		u32 srcRowSize, srcNumRows;
		glUtils_CalcSurfaceRowParams(rect.w, rect.h, 1, m_format, srcRowSize, srcNumRows);

		// Copy texture content

		glUtils_CopyTextureRect(m_format, atlasLevel0.m_data, atlasRowSize, entry.m_rectInAtlas, img.m_levels[0].m_data, srcRowSize, srcNumRows);

		// Create border around the texture

		if (m_settings.m_borderInAtlas)
			glUtils_MakeTextureBorder(m_format, atlasLevel0.m_data, totalWidth, totalHeight, entry.m_rectInAtlas, m_settings.m_borderInAtlas);
	}

#if 0
	u8* ptr = (u8*) atlasLevel0.m_data;
	for (u32 y = 0; y < 200; y++)
	{
		for (u32 x = 0; x < 60; x++)
		{
			if (ptr[3] > 128)
				printf("X");
			else
				printf(" ");
			ptr += 4;
		}
		ptr += 4 * (totalWidth - 60);
		printf("\n");
	}
#endif

	// Release individual images and set atlas image

	Destroy();
	m_faces.push_back(atlasImg);

	return true;
}

void ueToolTexture::MakeSingleTextureAtlas(u32 totalWidth, u32 totalHeight, u32 actualWidth, u32 actualHeight)
{
	UE_ASSERT(!m_isAtlas);

	m_isAtlas = true;
	m_atlasIsReady = true;

	// Assume entry in top-left corner

	AtlasEntry& entry = vector_push(m_atlasEntries);
	entry.m_name = ""; // Leave name empty, so the runtime knows this is single-texture atlas
	entry.m_rectInAtlas = ueRectI(0, 0, actualWidth, actualHeight);
	entry.m_translation[0] = (f32) entry.m_rectInAtlas.m_left / (f32) totalWidth;
	entry.m_translation[1] = (f32) entry.m_rectInAtlas.m_top / (f32) totalHeight;
	entry.m_scale[0] = (f32) entry.m_rectInAtlas.m_width / (f32) totalWidth;
	entry.m_scale[1] = (f32) entry.m_rectInAtlas.m_height / (f32) totalHeight;
}

bool ueToolTexture::BuildAtlas()
{
	if (!m_isAtlas)
		return true;

	if (m_atlasIsReady)
		return true;

	// Just load atlas description from text file

	if (m_settings.m_atlasDescriptionFilePath.length())
		return ParseAtlasDescriptionFromFile();

	// Pack all textures into single texture

	return PackAtlas();
}

void ueToolTexture::SerializeAtlasInfo(ioPackageWriter* pw)
{
	// Start segment

	ioSegmentParams segmentParams;
	segmentParams.m_symbol = UE_BE_4BYTE_SYMBOL('t','x','t','a');
	ioSegmentWriter sw;
	pw->BeginSegment(sw, segmentParams);

	// gxTextureAtlasInfo

	sw.WriteNumber<u32>(m_atlasEntries.size());		// m_numEntries
	ioPtr entriesPtr = sw.WritePtr();				// m_entries
	sw.WriteNullPtr();								// m_texture

	// Array of gxTextureAtlasEntry

	std::vector<ioPtr> entryNameArray;

	sw.BeginPtr(entriesPtr);
	for (u32 i = 0; i < m_atlasEntries.size(); i++)
	{
		const AtlasEntry& entry = m_atlasEntries[i];

		// gxTextureAtlasEntry

		entryNameArray.push_back( sw.WritePtr() );	// m_name
		WriteRectI(&sw, entry.m_rectInAtlas);		// m_rect
		sw.WriteNumber<f32>(entry.m_scale[0]);		// m_scale[0]
		sw.WriteNumber<f32>(entry.m_scale[1]);		// m_scale[1]
		sw.WriteNumber<f32>(entry.m_translation[0]);// m_translation[0]
		sw.WriteNumber<f32>(entry.m_translation[1]);// m_translation[1]
		sw.WriteNullPtr();							// m_texture
	}

	// Array of gxTextureAtlasEntry::m_name contents

	for (u32 i = 0; i < m_atlasEntries.size(); i++)
	{
		const AtlasEntry& entry = m_atlasEntries[i];
		sw.BeginPtr(entryNameArray[i]);
		sw.WriteData(entry.m_name.c_str(), entry.m_name.length() + 1);
	}
}

bool ueToolTexture::Serialize(ioPackageWriter* pw)
{
	// Begin writing out

	ioSegmentParams segmentParams;
	segmentParams.m_symbol = UE_BE_4BYTE_SYMBOL('t','x','t','r');
	ioSegmentWriter sw;
	pw->BeginSegment(sw, segmentParams);

	// glTextureFileDesc

	sw.WriteNumber<u32>(m_storage);
	sw.WriteNumber<u32>(m_type);
	sw.WriteNumber<u32>(m_format);
	sw.WriteNumber<u32>(GetWidth());
	sw.WriteNumber<u32>(GetHeight());
	sw.WriteNumber<u32>(GetDepth());
	sw.WriteNumber<u32>(GetNumLevels());
	sw.WriteBool(m_enableAutoMips);
	sw.WriteBool(m_settings.m_isDynamic);

	if (m_storage == glTexStorage_GPU)
		SerializeGPU(&sw);
	else
	{
		sw.WriteNumber<u32>(m_faces.size());		// u32 m_dataArrayLength
		ioPtr dataArrayPtr = sw.WritePtr();			// glTextureData* m_dataArray

		// Array of glTextureData

		std::vector<ioPtr> dataPtrs;

		sw.BeginPtr(dataArrayPtr);
		for (u32 i = 0; i < m_faces.size(); i++)
		{
			const Level& topLevel = m_faces[i].m_levels[0];

			// glTextureData

			sw.WriteNumber<u32>(topLevel.m_dataSize);			// u32 m_rowPitch (data size)
			sw.WriteNumber<u32>(0);								// u32 m_slicePitch
			dataPtrs.push_back(sw.WritePtr());					// void* m_data
		}

		// Array of texture data

		for (u32 i = 0; i < m_faces.size(); i++)
		{
			const Level& topLevel = m_faces[i].m_levels[0];

			sw.BeginPtr(dataPtrs[i]);
			sw.WriteData(topLevel.m_data, topLevel.m_dataSize);
		}
	}

	// Optional: Write atlas info

	if (m_isAtlas)
		SerializeAtlasInfo(pw);

	return true;
}

#if defined(UE_TOOLS_WIN32)

void ueToolTexture::SerializeGPU(ioSegmentWriter* sw)
{
	const u32 numLevelsToSerialize = m_enableAutoMips ? 1 : (s32) GetNumLevels();
	const u32 dataArrayLength = m_faces.size() * numLevelsToSerialize;

	// Remainder of glTextureFileDesc

	sw->WriteNumber<u32>(dataArrayLength);		// u32 m_dataArrayLength
	ioPtr dataArrayPtr = sw->WritePtr();		// glTextureData* m_dataArray

	// Array of glTextureData

	std::vector<ioPtr> dataPtrs;

	sw->BeginPtr(dataArrayPtr);
	for (u32 i = 0; i < m_faces.size(); i++)
		for (u32 j = 0; j < numLevelsToSerialize; j++)
		{
			const Level& level = m_faces[i].m_levels[j];

			// Get level info

			u32 rowSize, numRows;
			glUtils_CalcSurfaceRowParams(level.m_width, level.m_height, level.m_depth, m_format, rowSize, numRows);

			// glTextureData

			sw->WriteNumber<u32>(rowSize);						// u32 m_rowPitch
			sw->WriteNumber<u32>(rowSize * numRows);			// u32 m_slicePitch
			dataPtrs.push_back(sw->WritePtr());					// void* m_data
		}

	// Array of texture data (the actual bytes)

	for (u32 i = 0; i < m_faces.size(); i++)
		for (u32 j = 0; j < numLevelsToSerialize; j++)
		{
			const Level& level = m_faces[i].m_levels[j];

			sw->BeginPtr(dataPtrs[i * numLevelsToSerialize + j]);
			sw->WriteData(level.m_data, level.m_dataSize);
		}
}

#elif defined(UE_TOOLS_X360)

bool ueToolTexture::SerializeData(ueArchive& ar, bool saveAsBufferOnly)
{
	XGTEXTURE_DESC d3dLevelDesc;

	// Determine X360 texture format
	D3DFORMAT d3dFormat;
	ueCheck( ueD3D9Constants::ToD3DFORMAT(m_format, d3dFormat) );
	const D3DFORMAT d3dFormatUntiled = (D3DFORMAT) (d3dFormat & ~D3DFORMAT_TILED_MASK);
	const DWORD gpuFormat = XGGetGpuFormat(d3dFormat);

	// Create X360 texture header
	UINT baseSize, mipSize;
	D3DTexture d3dTexture;
	XGSetTextureHeaderEx(
		GetWidth(), GetHeight(), GetNumLevels(),
		0, // Flags
		d3dFormat,
		0, 0, // Exp Bias, Flags
		0, XGHEADER_CONTIGUOUS_MIP_OFFSET, // Base offset, Mip offset
		0, // Pitch
		&d3dTexture, &baseSize, &mipSize);

	// Determine first tail mip-level index
	const ueInt32 firstTailLevel = (ueInt32) XGGetMipTailBaseLevel(GetWidth(), GetHeight(), FALSE);

	// Calculate and serialize mip-level offsets as stored in file
	ueTextureBuffer::MipLevelFileOffsets offsets(GetNumLevels());

	ueInt offset = 0;
	if (firstTailLevel < GetNumLevels())
	{
		for (ueInt i = firstTailLevel; i < GetNumLevels(); i++)
			offsets[i] = 0;

		XGGetTextureDesc(&d3dTexture, firstTailLevel, &d3dLevelDesc);
		offset += (ueInt32) d3dLevelDesc.SlicePitch;
	}

	for (ueInt i = ueMin(GetNumLevels(), firstTailLevel) - 1; i >= 0; i--)
	{
		offsets[i] = offset;
		XGGetTextureDesc(&d3dTexture, i, &d3dLevelDesc);
		offset += (ueInt32) d3dLevelDesc.SlicePitch;
	}

	ar << offsets;

	// Pack, tile and serialize mip-tail (if there is any)
	if (firstTailLevel < GetNumLevels())
	{
		// Get size of packed mip-tail
		XGGetTextureDesc(&d3dTexture, firstTailLevel, &d3dLevelDesc);
		const ueInt32 mipTailSize = (ueInt32) d3dLevelDesc.SlicePitch;

		// Pack all mip-tail levels
		ueTempPtr<ueChar> tailData = new(UE_GLOBAL_ALLOC) ueChar[mipTailSize];
		ueTempPtr<ueChar> tempData = new(UE_GLOBAL_ALLOC) ueChar[mipTailSize];

		for (ueInt i = 0; i < m_images.Size(); i++)
			for (ueInt j = firstTailLevel; j < GetNumLevels(); j++)
			{
				const Level& level = m_images[i].m_levels[j];

				// Get description of this mip-level
				XGGetTextureDesc(&d3dTexture, j, &d3dLevelDesc);

				ueInt32 rowPitch, numRows;
				ueRenderUtils::CalcSurfaceRowPitchAndRowCount(d3dLevelDesc.Width, d3dLevelDesc.Height, m_format, rowPitch, numRows);

				// Swap endianess from little to big
				XGEndianSwapSurface(
					*tempData,					// Dest data
					d3dLevelDesc.RowPitch,		// Dest pitch
					level.m_data,				// Src data
					rowPitch,					// Src pitch
					d3dLevelDesc.Width,
					d3dLevelDesc.Height,
					d3dFormatUntiled);

				// Get offset to mip-level of the mip-tail
				const ueInt32 tailLevelOffset = (ueInt32) XGGetMipTailLevelOffset(
					GetWidth(), GetHeight(), GetDepth(),
					j,
					gpuFormat,
					TRUE,	// Tiled
					FALSE);	// No border

				// Tile mip-level of the mip-tail
				XGTileTextureLevel(
					GetWidth(), GetHeight(),
					j,
					gpuFormat,
					0,								// Flags
					*tailData + tailLevelOffset,	// Dest data
					NULL,							// Start point at (0, 0)
					*tempData,						// Src data
					d3dLevelDesc.RowPitch,			// Src pitch
					NULL);							// Whole rectangle
			}

		// Serialize packed mip-tail
		ar << AR_DATA(*tailData, mipTailSize);
	}

	// Tile and serialize all non-tail mip-levels
	ueTempPtr<ueChar> mipLevelData = new(UE_GLOBAL_ALLOC) ueChar[baseSize];
	ueTempPtr<ueChar> tempData = new(UE_GLOBAL_ALLOC) ueChar[baseSize];

	for (ueInt i = 0; i < m_images.Size(); i++)
		for (ueInt j = ueMin(GetNumLevels(), firstTailLevel) - 1; j >= 0; j--)
		{
			const Level& level = m_images[i].m_levels[j];

			// Get description of this mip-level
			XGGetTextureDesc(&d3dTexture, j, &d3dLevelDesc);

			ueInt32 rowPitch, numRows;
			ueRenderUtils::CalcSurfaceRowPitchAndRowCount(d3dLevelDesc.Width, d3dLevelDesc.Height, m_format, rowPitch, numRows);

			// Swap endianess from little to big
			XGEndianSwapSurface(
				*tempData,					// Dest data
				d3dLevelDesc.RowPitch,		// Dest pitch
				level.m_data,				// Src data
				rowPitch,					// Src pitch
				d3dLevelDesc.Width,
				d3dLevelDesc.Height,
				d3dFormatUntiled);

			// Tile mip-level
			XGTileTextureLevel(
				GetWidth(), GetHeight(),
				j,
				gpuFormat,
				0,								// Flags
				*mipLevelData,					// Dest
				NULL,							// Start point at (0, 0)
				*tempData,						// Src data
				d3dLevelDesc.RowPitch,			// Src pitch
				NULL);							// Whole rectangle

			// Serialize mip-level
			ar << AR_DATA(*mipLevelData, d3dLevelDesc.SlicePitch);
		}

	return !ar.IsError();
}

#endif

void ueToolTexture::DumpSettings()
{
	DumpSettingHeader();
	DumpSettingString("source", Setting_Optional, "Source texture file name relative to root source directory (\"root_src\"), e.g. 'ui_sample/invader.tga'");
	DumpSettingString("source[N]", Setting_Optional | Setting_Array, "Array of source texture file names; for use with cube (source must be specified for N from 0 to 5), array textures and texture atlases");
	DumpSettingBool("isAtlas", Setting_Optional, "Indicates whether the texture is actually texture atlas to be composed from multiple images (specified via an array of source[] file names)");
	DumpSettingBool("makePow2", Setting_Optional, "Make texture/atlas dimensions power of 2 without rescaling (redundant pixels won't be used)");
	DumpSettingString("atlasDescriptionFile", Setting_Optional, "Path to TXT file describing atlas content; each line should contain <name> <left> <top> <width> <height>");
	DumpSettingU32("borderInAtlas", Setting_Optional, "Width in pixels of the texture in atlas (only used if the texture ends up in atlas)");
	DumpSettingEnum("textureType", 0, glTexType, "Texture type");
	DumpSettingEnum("format", 0, glBufferFormat, "Texture format on run-time side");
	DumpSettingU32("width", 0, "Texture width; 0 indicates original source texture width");
	DumpSettingU32("height", 0, "Texture height; 0 indicates original source texture height");
	DumpSettingU32("depth", 0, "Texture depth (only used for 3D textures); 0 indicates original source texture depth");
	DumpSettingU32("maxWidth", 0, "Max texture width");
	DumpSettingU32("maxHeight", 0, "Max texture depth");
	DumpSettingU32("maxDepth", 0, "Max texture height");
	DumpSettingU32("numLevels", 0, "Number of texture mip levels; 0 indicates all mip levels");
	DumpSettingEnum("storage", 0, glTexStorage, "Texture storage; GPU indicates format ready for GPU to be loaded (no conversion required); for best compression consider using JPG or PNG");
	DumpSettingEnum("rescaleMethod", 0, ueTextureRescaleMethod, "Texture rescale method");
	DumpSettingBool("isNormalMap", 0, "Indicates whether texture is to be treated as normal map which impacts filtering during mip tail generation");
	DumpSettingBool("enableStreaming", 0, "Enables mip-level streaming for the texture");
	DumpSettingU32("minLoadedLevels", 0, "Minimum number of mip-levels that must always be loaded at runtime. Only used when enableStreaming is enabled");
	DumpSettingBool("isDynamic", Setting_Optional, "Indicates whether the texture is to be dynamically modifiable at runtime");
}

bool ueToolTexture::ParseAtlasDescriptionFromFile()
{
	FILE* f = fopen(m_settings.m_atlasDescriptionFilePath.c_str(), "r");
	if (!f)
	{
		ueLogE("Failed to open texture atlas description file at '%s'", m_settings.m_atlasDescriptionFilePath.c_str());
		return false;
	}

	const f32 atlasWidth = (f32) m_faces[0].m_levels[0].m_width;
	const f32 atlasHeight = (f32) m_faces[0].m_levels[0].m_height;

	u32 line = 1;
	while (!feof(f))
	{
		char name[512];
		s32 left, top, width, height;

		const s32 numScannedValues = fscanf(f, "%s %d %d %d %d", &name, &left, &top, &width, &height);
		if (numScannedValues <= 0)
			break;
		if (numScannedValues != 5)
		{
			ueLogE("Texture atlas description file '%s' contains invalid entry at line %u; should be: <name> <left> <top> <width> <height>", m_settings.m_atlasDescriptionFilePath.c_str(), line);
			fclose(f);
			return false;
		}

		AtlasEntry& entry = vector_push(m_atlasEntries);
		entry.m_name = name;
		entry.m_rectInAtlas = ueRectI(left, top, width, height);
		entry.m_scale[0] = (f32) width / atlasWidth;
		entry.m_scale[1] = (f32) height / atlasHeight;
		entry.m_translation[0] = (f32) left / atlasWidth;
		entry.m_translation[1] = (f32) top / atlasHeight;

		line++;
	}

	fclose(f);
	return true;
}

bool ueToolTexture::ParseSettings(LoadSettings& s, const ueAssetParams& params)
{
	params.GetBoolParam("isAtlas", s.m_isAtlas);
	if (s.m_isAtlas)
		params.GetParam("atlasDescriptionFile", s.m_atlasDescriptionFilePath);

	params.GetU32Param("borderInAtlas", s.m_borderInAtlas);

	params.GetBoolParam("makePow2", s.m_makePow2);

	const char* sourceFileName = params.GetParam("source");
	if (sourceFileName && ueStrCmp(sourceFileName, "<none>"))
		s.m_sourceFileNames.push_back(sourceFileName);
	else // Multiple source files?
	{
		u32 index = 0;
		do
		{
			std::string paramName;
			string_format(paramName, "source[%u]", index);

			std::string sourceFileName;
			if (!params.GetParam(paramName.c_str(), sourceFileName))
				break;

			s.m_sourceFileNames.push_back(sourceFileName);
			index++;
		} while (1);

		if (s.m_sourceFileNames.size() == 0)
		{
			ueLogE("Missing source file path parameter(s) 'source' or 'source[0..N]'");
			return false;
		}
	}

	const char* textureType = params.GetParam("textureType");
	if (!ueEnumFromStringSafe(glTexType, textureType, s.m_type))
	{
		ueLogE("Unknown texture type '%s'", textureType);
		return false;
	}

	switch (s.m_type)
	{
		case glTexType_2D:
		case glTexType_3D:
			if (s.m_sourceFileNames.size() > 1 && !s.m_isAtlas)
				ueLogW("More than one source file name specified as source (source[0] = '%s', source[1] = '%s', [...]). All but first one will be ignored.",
					s.m_sourceFileNames[0].c_str(),
					s.m_sourceFileNames[1].c_str());
			break;
		case glTexType_Cube:
			if (s.m_sourceFileNames.size() == 1)
			{
				if (!string_ends_with(s.m_sourceFileNames[0], ".dds", false))
				{
					ueLogE("Cube texture requires either single DDS cube texture file or 6 2D images. Got 1 non-DDS image '%s'", s.m_sourceFileNames[0].c_str());
					return false;
				}
			}
			else if (s.m_sourceFileNames.size() != 6)
			{
				ueLogE("Cube texture requires either single DDS cube texture file or 6 2D images. Got %d images (source[0] = '%s', source[1] = '%s', [...])", s.m_sourceFileNames[0].c_str(), s.m_sourceFileNames[1].c_str());
				return false;
			}
			break;
	}

	const char* formatName = params.GetParam("format");
	if (!ueEnumFromStringSafe(glBufferFormat, formatName, s.m_format))
	{
		ueLogE("Unknown texture format '%s'", formatName);
		return false;
	}

	params.GetBoolParam("isDynamic", s.m_isDynamic);

	UE_ASSERT_FUNC( params.GetU32Param("width", s.m_width) );
	UE_ASSERT_FUNC( params.GetU32Param("height", s.m_height) );
	UE_ASSERT_FUNC( params.GetU32Param("depth", s.m_depth) );

	UE_ASSERT_FUNC( params.GetU32Param("maxWidth", s.m_maxWidth) );
	UE_ASSERT_FUNC( params.GetU32Param("maxHeight", s.m_maxHeight) );
	UE_ASSERT_FUNC( params.GetU32Param("maxDepth", s.m_maxDepth) );

	UE_ASSERT_FUNC( params.GetU32Param("numLevels", s.m_numLevels) );

	const char* storage = params.GetParam("storage");
	if (!ueEnumFromStringSafe(glTexStorage, storage, s.m_storage))
	{
		ueLogE("Unknown texture storage '%s'", storage);
		return false;
	}

	const char* rescaleMethod = params.GetParam("rescaleMethod");
	if (!ueEnumFromStringSafe(ueTextureRescaleMethod, rescaleMethod, s.m_rescaleMethod))
	{
		ueLogE("Unknown texture rescale method '%s'", rescaleMethod);
		return false;
	}

	if (s.m_isAtlas && s.m_rescaleMethod != ueTextureRescaleMethod_None)
	{
		ueLogE("Texture atlas must have rescale method set to '%s'", ueEnumToString(ueTextureRescaleMethod, ueTextureRescaleMethod_None));
		return false;
	}

	params.GetBoolParam("isNormalMap", s.m_isNormalMap);

	return true;
}

void ueToolTexture::ReorderChannels()
{
	if (m_settings.m_format == glBufferFormat_R8G8B8 && m_format == glBufferFormat_B8G8R8)
	{
		m_format = m_settings.m_format;
	}
	else
		return;

	const u32 pixelSize = glUtils_GetFormatBytes(m_format);

	for (u32 i = 0; i < m_faces.size(); i++)
		for (u32 j = 0; j < m_faces[i].m_levels.size(); j++)
		{
			Level& level = m_faces[i].m_levels[j];
			
			u8* pixels = (u8*) level.m_data;
			for (u32 p = 0; p < level.m_dataSize; p += pixelSize, pixels += pixelSize)
			{
				const u8 tmp = pixels[0];
				pixels[0] = pixels[2];
				pixels[2] = tmp;
			}
		}
}

bool ueToolTexture::ConvertToOutputFormat(u32 faceIndex)
{
	switch (m_settings.m_storage)
	{
		case glTexStorage_PVR:
			return ConvertToOutputFormatPVRTC(faceIndex);
		default:
			return ConvertToOutputFormatD3DX(faceIndex);
	}

	switch (m_settings.m_format)
	{
	case glBufferFormat_PVRTC2_RGB:
	case glBufferFormat_PVRTC2_RGBA:
	case glBufferFormat_PVRTC4_RGB:
	case glBufferFormat_PVRTC4_RGBA:
		return ConvertToOutputFormatPVRTC(faceIndex);
	}

	ueLogE("Failed to convert to output storage (%s) and/or format (%s)", ueEnumToString(glTexStorage, m_settings.m_storage), ueEnumToString(glBufferFormat, m_settings.m_format));
	return false;
}

bool ueToolTexture::ConvertToOutputFormat()
{
	// Reorder channels if needed

	if (m_storage == glTexStorage_GPU && m_settings.m_storage == glTexStorage_GPU)
		ReorderChannels();

	// Convert all faces to output storage/format

	if (m_storage != m_settings.m_storage || m_format != m_settings.m_format)
	{
		for (u32 i = 0; i < m_faces.size(); i++)
			if (!ConvertToOutputFormat(i))
				return false;
	}

	// Output texture atlas for debugging purposes

	else if (m_isAtlas)
		SaveImageDebugD3DX();

	return true;
}
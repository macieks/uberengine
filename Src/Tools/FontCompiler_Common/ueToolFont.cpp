#include "FontCompiler_Common/ueToolFont.h"
#include "IO/ioPackageWriter.h"
#include "GL/glLib_Shared.h"

ueToolFont::ueToolFont()
{}

ueToolFont::~ueToolFont()
{
	Destroy();
}

void ueToolFont::Destroy()
{
	m_glyphs.clear();
	m_texture.Destroy();
}

bool ueToolFont::Serialize(ioPackageWriter* pw)
{
	ioSegmentParams segmentParams;
	segmentParams.m_symbol = UE_BE_4BYTE_SYMBOL('f','o','n','t');
	ioSegmentWriter sw;
	pw->BeginSegment(sw, segmentParams);

	sw.WriteNumber<u32>(m_sizeInPixels);
	sw.WriteNumber<u32>(m_options);

	sw.WriteNumber<u32>((u32) m_glyphs.size());
	for (std::map<u32, gxFontGlyph>::const_iterator i = m_glyphs.begin(); i != m_glyphs.end(); i++)
	{
		// Map key

		sw.WriteNumber<u32>(i->first);

		// Map value (gxFont::Glyph)

		sw.WriteNumber<u32>(i->second.m_channelMask);
		sw.WriteNumber<f32>(i->second.m_left);
		sw.WriteNumber<f32>(i->second.m_top);
		sw.WriteNumber<f32>(i->second.m_right);
		sw.WriteNumber<f32>(i->second.m_bottom);
		sw.WriteNumber<f32>(i->second.m_widthInPixels);
		sw.WriteNumber<f32>(i->second.m_heightInPixels);
		sw.WriteNumber<f32>(i->second.m_offsetX);
		sw.WriteNumber<f32>(i->second.m_offsetY);
		sw.WriteNumber<f32>(i->second.m_advanceX);
	}

	return m_texture.Serialize(pw);
}

void ueToolFont::DumpSettings()
{
	DumpSettingHeader();
	DumpSettingString("source", 0, "Source font file name relative to root source directory ('root_src'), e.g. 'my_fonts\bold_funny.ttf'");
	DumpSettingU32("sizeInPixels", 0, "Size of the font in pixels");
	DumpSettingBool("bold", 0, "Enables bold font");
	DumpSettingBool("italic", 0, "Enables italic font");
	DumpSettingBool("underlined", 0, "Enables underlined font");
	DumpSettingEnum("textureFormat", 0, glBufferFormat, "Output glyph texture format");
	DumpSettingBool("packIntoSeparateChannels", 0, "Indicates to pack glyphs into separate layers of the image (e.g. ARGB)");
	DumpSettingBool("makePow2Texture", 0, "Indicates to make texture dimensions power of 2 (e.g. OpenGL 1.x based platforms require it)");
	DumpSettingU32("numTextureLevels", 0, "Number of texture mip-levels to be used for the font; defaults to 1");
	DumpSettingString("characterSetSource", 0, "Characters source type, either 'file' or 'string'");
	DumpSettingString("characterString", 0, "String containing characters to be generated for the font; only used when 'characterSetSource' is set to 'string'");
	DumpSettingString("characterFilePath", 0, "File containing characters to be generated for the font; only used when 'characterSetSource' is set to 'file'");
}

bool ueToolFont::ParseBuildSettings(BuildSettings& settings, ueAssetParams& params)
{
	settings.m_sourcePath = params.GetParam("source");

	UE_ASSERT_FUNC( params.GetU32Param("sizeInPixels", settings.m_sizeInPixels) );

	UE_ASSERT_FUNC( params.GetBoolParam("packIntoSeparateChannels", settings.m_packIntoSeparateChannels) );
	UE_ASSERT_FUNC( params.GetEnumParam("textureFormat", settings.m_textureFormat) );

	params.GetBoolParam("makePow2Texture", settings.m_makePow2Texture);

	bool bold, italic, underlined;
	UE_ASSERT_FUNC( params.GetBoolParam("bold", bold) );
	UE_ASSERT_FUNC( params.GetBoolParam("italic", italic) );
	UE_ASSERT_FUNC( params.GetBoolParam("underlined", underlined) );
	settings.m_options = 0;
	if (bold) settings.m_options |= gxFontFlags_Bold;
	if (italic) settings.m_options |= gxFontFlags_Italic;
	if (underlined) settings.m_options |= gxFontFlags_Underlined;

	UE_ASSERT_FUNC( params.GetU32Param("numTextureLevels", settings.m_numTextureLevels) );

	std::string characterSetSource;
	std::string characterString;
	std::string characterFilePath;
	UE_ASSERT_FUNC( params.GetParam("characterSetSource", characterSetSource) );
	UE_ASSERT_FUNC( params.GetParam("characterString", characterString) );
	UE_ASSERT_FUNC( params.GetParam("characterFilePath", characterFilePath) );

	if (characterSetSource == "file")
	{
		UE_NOT_IMPLEMENTED();
	}
	else if (characterSetSource == "string")
	{
		// Do nothing
	}
	else
	{
		fprintf(stderr, "Invalid character set source '%s'.", characterSetSource.c_str());
		return false;
	}

	// Convert UTF8 to UTF32

	settings.m_codes.resize(characterString.size());
	u32 numCharactersUTF32 = (u32) characterString.size();
	if (!ueStrUTF8ToUTF32(
		(const u8*) &characterString[0], (u32) characterString.size(),
		&settings.m_codes[0], numCharactersUTF32))
	{
		fprintf(stderr, "Invalid UTF-8 string containing characters to be converted (characters = '%s').", characterString.c_str());
		return false;
	}
	settings.m_codes.resize(numCharactersUTF32);

	return true;
}

void ueToolFont::CopyFrom(const ueTextureRectPacker::Rect& rect, const u8* buffer, u32 pitch)
{
	UE_ASSERT(m_texture.GetNumFaces() == 1);
	UE_ASSERT(m_texture.GetNumLevels() >= 1);

	const u32 numChannels = glUtils_GetFormatBytes(m_texture.m_format);

	const ueToolTexture::Level& dstLevel = m_texture.m_faces[0].m_levels[0];
	UE_ASSERT(rect.x + rect.w <= dstLevel.m_width);
	UE_ASSERT(rect.y + rect.h <= dstLevel.m_height);

	u32 dstRowSize, dstNumRows;
	glUtils_CalcSurfaceRowParams(dstLevel.m_width, dstLevel.m_height, dstLevel.m_depth, m_texture.m_format, dstRowSize, dstNumRows);

	for (u32 y = 0; y < rect.h; y++)
	{
		const u8* srcRow = buffer + pitch * y;
		u8* dstRow = (u8*) dstLevel.m_data + (rect.y + y) * dstRowSize + rect.x * numChannels;
		dstRow += rect.m_layer;
		for (u32 x = 0; x < rect.w; x++)
			dstRow[x * numChannels] = srcRow[x];
	}
}
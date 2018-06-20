#include "Test.h"

Tex s_tex;

bool on_header(TexLib_Context* ctx, const TexLib_Header* header)
{
	s_tex.m_width = header->m_width;
	s_tex.m_height = header->m_height;
	s_tex.m_format = header->m_format;
	return true;
}

bool on_begin_face_level(TexLib_Context* ctx, const TexLib_Header* header, int faceIndex, int levelIndex, TexLib_LevelData* data)
{
	data->m_data = (unsigned char*) s_tex.Lock(faceIndex, levelIndex);
	data->m_dataSize = s_tex.m_lockedSize;
	data->m_format = header->m_format;
	data->m_rowPitch = s_tex.m_lockedRowPitch;
	data->m_slicePitch = s_tex.m_lockedSlicePitch;
	return true;
}

void on_begin_end_level(TexLib_Context* ctx, const TexLib_Header* header, int faceIndex, int levelIndex)
{
	s_tex.Unlock(faceIndex, levelIndex);
}

void load_texture_sample()
{
	TexLib_Context ctx;
	ctx.m_loadFromFile = true;
	ctx.m_onHeader = on_header;
	ctx.m_onBeginFaceLevel = on_begin_face_level;
	ctx.m_onEndFaceLevel = on_begin_end_level;
	ctx.m_path =
		//"C:/Misc/Coding/image_tests/test_tex.png";
		//"C:/Misc/Coding/image_tests/test_tex.tga";
		//"C:/Misc/Coding/image_tests/test_tex_cmp.tga";
		//"C:/Misc/Coding/image_tests/test_tex_8bit.tga";
		//"C:/Misc/Coding/image_tests/test_tex.jpg";
		//"C:/Misc/Coding/image_tests/test_tex_dxt5.dds";
		"C:/Misc/Coding/image_tests/test_tex_dxt1.dds";

	bool result = TexLib_Load(&ctx);
	int test = 0;
}
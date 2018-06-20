#include "TextureCompiler_Common/ueToolTexture.h"
#include "Misc/ueTextureRectPacker.h"
#include "IO/ioPackageWriter.h"
#include "IO/ioPackageWriterUtils.h"
#include "GL/glLib_Shared.h"
#include "GL/D3D9/ueD3D9_NULLREF.h"

ueBool glUtils_HasAlpha(D3DFORMAT format)
{
	return
		format == D3DFMT_A8R8G8B8 ||
		format == D3DFMT_A1R5G5B5 ||
		format == D3DFMT_A4R4G4B4 ||
		format == D3DFMT_A8 ||
		format == D3DFMT_A8R3G3B2 ||
		format == D3DFMT_A2B10G10R10 ||
		format == D3DFMT_A8B8G8R8 ||
		format == D3DFMT_A2R10G10B10 ||
		format == D3DFMT_A16B16G16R16 ||
		format == D3DFMT_A8P8 ||
		format == D3DFMT_A8L8 ||
		format == D3DFMT_A4L4 ||
		format == D3DFMT_A2W10V10U10 ||
		format == D3DFMT_DXT2 ||
		format == D3DFMT_DXT4 ||
		format == D3DFMT_DXT5;
}

ueBool glUtils_ToStorage(glTexStorage& dst, const D3DXIMAGE_FILEFORMAT& src)
{
	switch (src)
	{
		case D3DXIFF_DDS: dst = glTexStorage_DDS; return UE_TRUE;
		case D3DXIFF_JPG: dst = glTexStorage_JPG; return UE_TRUE;
		case D3DXIFF_PNG: dst = glTexStorage_PNG; return UE_TRUE;
		case D3DXIFF_HDR: dst = glTexStorage_HDR; return UE_TRUE;
		case D3DXIFF_TGA: dst = glTexStorage_TGA; return UE_TRUE;
	}
	return UE_FALSE;
}

ueBool glUtils_ToD3DFORMAT(glBufferFormat format, glBufferFormat& loadFormat, D3DFORMAT& d3dFormat)
{
	loadFormat = format;
	switch (format)
	{
		case glBufferFormat_A8: d3dFormat = D3DFMT_A8; break;
		case glBufferFormat_B4G4R4A4: d3dFormat = D3DFMT_A4R4G4B4; break;
		case glBufferFormat_B5G6R5: d3dFormat = D3DFMT_R5G6B5; break;
		case glBufferFormat_B8G8R8: d3dFormat = D3DFMT_R8G8B8; break;
		case glBufferFormat_R8G8B8: d3dFormat = D3DFMT_R8G8B8; loadFormat = glBufferFormat_R8G8B8; break;
		case glBufferFormat_B8G8R8A8: d3dFormat = D3DFMT_A8R8G8B8; break;
		case glBufferFormat_R8G8B8A8: d3dFormat = D3DFMT_A8B8G8R8; break;
		case glBufferFormat_D16: d3dFormat = D3DFMT_D16; break;
		case glBufferFormat_D24S8: d3dFormat = D3DFMT_D24S8; break;
		case glBufferFormat_D32: d3dFormat = D3DFMT_D32; break;
		case glBufferFormat_DXT1: d3dFormat = D3DFMT_DXT1; break;
		case glBufferFormat_DXT3: d3dFormat = D3DFMT_DXT3; break;
		case glBufferFormat_DXT5: d3dFormat = D3DFMT_DXT5; break;
		case glBufferFormat_R16F: d3dFormat = D3DFMT_R16F; break;
		case glBufferFormat_R16G16F: d3dFormat = D3DFMT_G16R16F; break;
		case glBufferFormat_R16G16B16A16F: d3dFormat = D3DFMT_A16B16G16R16F; break;
		case glBufferFormat_R32F: d3dFormat = D3DFMT_R32F; break;
		case glBufferFormat_R32G32F: d3dFormat = D3DFMT_G32R32F; break;
		case glBufferFormat_R32G32B32A32F: d3dFormat = D3DFMT_A32B32G32R32F; break;
		case glBufferFormat_PVRTC4_RGB: d3dFormat = D3DFMT_R8G8B8; loadFormat = glBufferFormat_B8G8R8; break;
		case glBufferFormat_PVRTC4_RGBA: d3dFormat = D3DFMT_A8R8G8B8; loadFormat = glBufferFormat_B8G8R8A8; break;
		case glBufferFormat_PVRTC2_RGB: d3dFormat = D3DFMT_R8G8B8; loadFormat = glBufferFormat_B8G8R8; break;
		case glBufferFormat_PVRTC2_RGBA: d3dFormat = D3DFMT_A8R8G8B8; loadFormat = glBufferFormat_B8G8R8A8; break;
		case glBufferFormat_Unknown: d3dFormat = D3DFMT_UNKNOWN; break;
		default:
			return UE_FALSE;
	}
	return UE_TRUE;
}

bool ueToolTexture::SaveAsNonGPU(Level& level, const char* path, D3DXIMAGE_FILEFORMAT format, ID3DXBuffer** buffer)
{
	IDirect3DTexture9* d3dTexture2D = NULL;

#if 0
	if (FAILED(D3DXCreateTextureFromFileInMemoryEx(
			D3DEV_NULLREF,
			data,
			dataSize,
			0,	// Original width
			0,	// Original height
			1,
			0, // Usage
			D3DFMT_UNKNOWN,
			D3DPOOL_SYSTEMMEM,
			D3DX_FILTER_NONE,
			D3DX_DEFAULT, // Mip-filter
			0, // Color key
			NULL, // Image info
			NULL, // Palette entry
			(IDirect3DTexture9**) &d3dTexture2D)))
	{
		ueLogE("Failed to load texture from memory.");
		return false;
	}

#else

	glBufferFormat loadFormat;
	D3DFORMAT d3dFormat;
	if (!glUtils_ToD3DFORMAT(m_format, loadFormat, d3dFormat))
	{
		ueLogE("Failed to convert to D3D format.");
		return false;
	}

	// Create texture

	if (FAILED(D3DEV_NULLREF->CreateTexture(level.m_width, level.m_height, 1, 0, d3dFormat, D3DPOOL_SYSTEMMEM, &d3dTexture2D, NULL)))
	{
		ueLogE("Failed to load texture from memory.");
		return false;
	}

	// Lock texture

	D3DLOCKED_RECT d3dLockedRect;
	if (FAILED(d3dTexture2D->LockRect(0, &d3dLockedRect, NULL, 0)))
		return false;

	// Copy data row by row

	u32 rowSize, numRows;
	glUtils_CalcSurfaceRowParams(level.m_width, level.m_height, level.m_depth, m_format, rowSize, numRows);

	for (u32 y = 0; y < numRows; y++)
		memcpy(
			(u8*) d3dLockedRect.pBits + y * d3dLockedRect.Pitch,
			(u8*) level.m_data + y * rowSize,
			rowSize);

	// Unlock mip-level

	if (FAILED(d3dTexture2D->UnlockRect(0)))
		return false;

#endif

	// Save to file or memory

	if (buffer && FAILED(D3DXSaveTextureToFileInMemory(buffer, format, d3dTexture2D, NULL)))
	{
		ueLogE("Failed to save texture to memory.", path);
		return false;
	}

	if (path && FAILED(D3DXSaveTextureToFile(path, format, d3dTexture2D, NULL)))
	{
		ueLogE("Failed to save texture to file (path = '%s').", path);
		return false;
	}

	// Release texture

	d3dTexture2D->Release();

	return true;
}

const char* glUtils_GetTexStorageExtension(glTexStorage src)
{
	switch (src)
	{
	case glTexStorage_JPG: return "jpg";
	case glTexStorage_PNG: return "png";
	case glTexStorage_TGA: return "tga";
	case glTexStorage_DDS: return "dds";
	case glTexStorage_HDR: return "hdr";
	}
	return NULL;
}

bool glUtils_GetD3DXIMAGE_FILEFORMAT(D3DXIMAGE_FILEFORMAT& dst, glTexStorage src)
{
	switch (src)
	{
	case glTexStorage_JPG: dst = D3DXIFF_JPG; return true;
	case glTexStorage_PNG: dst = D3DXIFF_PNG; return true;
	case glTexStorage_TGA: dst = D3DXIFF_TGA; return true;
	case glTexStorage_DDS: dst = D3DXIFF_DDS; return true;
	case glTexStorage_HDR: dst = D3DXIFF_HDR; return true;
	}
	return false;
}

bool ueToolTexture::ConvertToOutputFormatD3DX(u32 faceIndex)
{
	D3DXIMAGE_FILEFORMAT format;
	if (!glUtils_GetD3DXIMAGE_FILEFORMAT(format, m_settings.m_storage))
	{
		ueLogE("Unknown texture storage (%u)", (u32) m_settings.m_storage);
		return false;
	}
	const char* ext = glUtils_GetTexStorageExtension(m_settings.m_storage);

	ueToolTexture::Image& face = m_faces[faceIndex];
	ueToolTexture::Level& topLevel = face.m_levels[0];

	// Convert into D3DX buffer

	std::string path;
	if (m_isAtlas)
	{
		path = m_outputPath;
		path += ".";
		path += ext;
	}

	ID3DXBuffer* buffer = NULL;
	if (!SaveAsNonGPU(topLevel, path.length() ? path.c_str() : NULL, format, &buffer))
		return false;

	// Get rid of all remaining images

	face.m_levels.resize(1);

	// Set up new data

	m_storage = m_settings.m_storage;
	m_format = m_settings.m_format;

	topLevel.m_data = buffer->GetBufferPointer();
	topLevel.m_dataSize = buffer->GetBufferSize();
	topLevel.m_ownsMemory = false;

	return true;
}

void ueToolTexture::SaveImageDebugD3DX()
{
	std::string path = m_outputPath;
	path += ".png";

	SaveAsNonGPU(m_faces[0].m_levels[0], path.c_str(), D3DXIFF_PNG, NULL);
}
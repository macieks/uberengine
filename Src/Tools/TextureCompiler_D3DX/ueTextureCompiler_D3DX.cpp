#include "TextureCompiler_Common/ueToolTexture.h"
#include "IO/ioPackageWriter.h"
#include "GL/D3D9/ueD3D9_NULLREF.h"
#include "GL/glLib_Shared.h"

extern ueBool glUtils_HasAlpha(D3DFORMAT format);
extern ueBool glUtils_ToStorage(glTexStorage& dst, const D3DXIMAGE_FILEFORMAT& src);
extern ueBool glUtils_FormatRequiresSquareDimensions(glBufferFormat format);
extern ueBool glUtils_ToD3DFORMAT(glBufferFormat format, glBufferFormat& loadFormat, D3DFORMAT& d3dFormat);

bool CompileTextureFace(ueToolTexture& t, u32 faceIndex, const void* data, const u32 dataSize)
{
	ueToolTexture::LoadSettings& s = t.m_settings;

	// Get source information

	D3DXIMAGE_INFO info;
	if (FAILED(D3DXGetImageInfoFromFileInMemory(data, dataSize, &info)))
	{
		ueLogE("Failed to get image information using D3DXGetImageInfoFromFileInMemory (path = '%s')", s.m_sourceFileNames[faceIndex].c_str());
		return false;
	}

	glTexStorage sourceStorage;
	const bool hasSupportedSourceStorage = glUtils_ToStorage(sourceStorage, info.ImageFileFormat);

	const bool hasAlpha = glUtils_HasAlpha(info.Format);

	// Determine final dimensions and filter to apply while resizing

	DWORD filter = hasAlpha ? D3DX_FILTER_POINT : D3DX_FILTER_LINEAR; // Don't resize linearly when there's alpha - causes artifacts

	u32 width = s.m_width ? s.m_width : info.Width;
	u32 height = s.m_height ? s.m_height : info.Height;
	u32 depth = s.m_depth ? s.m_depth : info.Depth;
		
	switch (s.m_rescaleMethod)
	{
	case ueTextureRescaleMethod_ToPreviousPowerOf2:
		width = ueLEPow2(width);
		height = ueLEPow2(height);
		depth = ueLEPow2(depth);
		break;
	case ueTextureRescaleMethod_ToNextPowerOf2:
		width = ueGEPow2(width);
		height = ueGEPow2(height);
		depth = ueGEPow2(depth);
		break;
	case ueTextureRescaleMethod_ToNearestPowerOf2:
		width = ueLEPow2(width);
		height = ueLEPow2(height);
		depth = ueLEPow2(depth);
		break;
	case ueTextureRescaleMethod_None:
		break;
	}

	width = ueMin(s.m_maxWidth, width);
	height = ueMin(s.m_maxHeight, height);
	depth = ueMin(s.m_maxDepth, depth);

	// Determine whether we need to decompress the image at all

	const bool doDecompression =
		!hasSupportedSourceStorage ||												// Wouldn't be able to load it at runtime otherwise
		(s.m_storage != glTexStorage_Original && s.m_storage != sourceStorage) ||	// Source storage differs from destination storage
		s.m_isAtlas ||																// Atlas packing is only supported for decompressed formats
		(s.m_makePow2 && (!ueIsPow2(info.Width) || !ueIsPow2(info.Height))) ||		// Texture/atlas make-pow2 is only supported for decompressed formats
		width != info.Width || height != info.Height || depth != info.Depth;		// Source and destination dimensions don't match up

	// Store in original format?

	if (!doDecompression)
	{
		const glBufferFormat defaultFormat = hasAlpha ? glBufferFormat_B8G8R8A8 : glBufferFormat_B8G8R8;

		if (s.m_format == glBufferFormat_Unknown)
			s.m_format = defaultFormat;
		else if (s.m_format == glBufferFormat_R8G8B8A8 && !hasAlpha)
			s.m_format = glBufferFormat_R8G8B8;
		else if (s.m_format == glBufferFormat_B8G8R8A8 && !hasAlpha)
			s.m_format = glBufferFormat_B8G8R8;

		if (s.m_storage == glTexStorage_Original)
			s.m_storage = sourceStorage;
		t.m_storage = sourceStorage;

		t.m_format = s.m_format;
		t.m_type = s.m_type;

		ueToolTexture::Image& face = vector_push(t.m_faces);
		face.m_name = s.m_sourceFileNames[faceIndex];
		ueToolTexture::Level& topLevel = vector_push(face.m_levels);

		void* dataCopy = malloc(dataSize);
		UE_ASSERT(dataCopy);
		ueMemCpy(dataCopy, data, dataSize);

		topLevel.m_data = dataCopy;
		topLevel.m_dataSize = dataSize;
		topLevel.m_ownsMemory = true;

		topLevel.m_width = info.Width;
		topLevel.m_height = info.Height;
		topLevel.m_depth = info.Depth;
		return true;
	}

	// Get load format (we might need to convert later to different format before output)

	s.m_storage = (s.m_storage == glTexStorage_Original) ? sourceStorage : s.m_storage;
	t.m_storage = glTexStorage_GPU;

	D3DFORMAT d3dFormat;
	if (!glUtils_ToD3DFORMAT(s.m_format, t.m_format, d3dFormat))
	{
		ueLogE("Unsupported D3DX texture load format (format = %s).", ueEnumToString(glBufferFormat, s.m_format));
		return false;
	}

#if defined(UE_TOOLS_X360)
	const D3DFORMAT d3dFormatOriginal = d3dFormat;
	d3dFormat = (D3DFORMAT) MAKELINFMT(d3dFormat);
#endif

	// Optionally make it pow-2 or square dimensions

	if (s.m_rescaleMethod == ueTextureRescaleMethod_None && !s.m_isAtlas)
	{
		if (s.m_makePow2)
		{
			width = ueGEPow2(width);
			height = ueGEPow2(height);
			depth = ueGEPow2(depth);
		}
		
		if (glUtils_FormatRequiresSquareDimensions(s.m_format))
		{
			width = ueGEPow2(width);
			height = ueGEPow2(height);
			depth = ueGEPow2(depth);
			width = height = ueMax(width, height);
		}

		if (width != info.Width || height != info.Height)
		{
			filter = D3DX_FILTER_NONE;
			t.MakeSingleTextureAtlas(width, height, info.Width, info.Height);
		}
	}

	// Load texture

	IDirect3DBaseTexture9* d3dBaseTexture = NULL;

	if (s.m_type == glTexType_2D || (s.m_type == glTexType_Cube && s.m_sourceFileNames.size() != 1))
	{
		if (FAILED(D3DXCreateTextureFromFileInMemoryEx(
			D3DEV_NULLREF,
			data,
			dataSize,
			width,
			height,
			s.m_numLevels,
			0, // Usage
			d3dFormat,
			D3DPOOL_SYSTEMMEM,
			filter,
			D3DX_DEFAULT, // Mip-filter
			0, // Color key
			NULL, // Image info
			NULL, // Palette entry
			(IDirect3DTexture9**) &d3dBaseTexture)))
		{
			ueLogE("Failed to convert 2D texture to desired format (format = %u; D3D format = %u).", (u32) t.m_format, (u32) d3dFormat);
			return false;
		}

		IDirect3DTexture9* d3dTexture2D = (IDirect3DTexture9*) d3dBaseTexture;

		// Make sure the format is known by the engine

		D3DSURFACE_DESC surfaceDesc;
		d3dTexture2D->GetLevelDesc(0, &surfaceDesc);
		if (surfaceDesc.Format != d3dFormat)
		{
			ueLogE("Unknown format of the texture (D3D format = %u).", (u32) surfaceDesc.Format);
			return false;
		}

		// Retrieve data of each mip-level

		ueToolTexture::Image& face = vector_push(t.m_faces);
		face.m_name = s.m_sourceFileNames[faceIndex];

		for (u32 i = 0; i < d3dTexture2D->GetLevelCount(); i++)
		{
			d3dTexture2D->GetLevelDesc(i, &surfaceDesc);

			ueToolTexture::Level& dstLevel = vector_push(face.m_levels);
			dstLevel.m_width = surfaceDesc.Width;
			dstLevel.m_height = surfaceDesc.Height;
			dstLevel.m_depth = 1;

			dstLevel.m_dataSize = glUtils_CalcSurfaceSize(glTexType_2D, dstLevel.m_width, dstLevel.m_height, dstLevel.m_depth, t.m_format);
			dstLevel.m_data = new u8[dstLevel.m_dataSize];
			D3DLOCKED_RECT d3dLockedRect;
			if (FAILED(d3dTexture2D->LockRect(i, &d3dLockedRect, NULL, 0)))
				return false;

			// Copy data row by row

			u32 rowSize, numRows;
			glUtils_CalcSurfaceRowParams(dstLevel.m_width, dstLevel.m_height, dstLevel.m_depth, t.m_format, rowSize, numRows);

			for (u32 y = 0; y < numRows; y++)
				memcpy(
					(u8*) dstLevel.m_data + y * rowSize,
					(u8*) d3dLockedRect.pBits + y * d3dLockedRect.Pitch,
					rowSize);

			// Unlock mip-level

			if (FAILED(d3dTexture2D->UnlockRect(i)))
				return false;
		}
	}
	else if (t.m_type == glTexType_Cube)
	{
		if (FAILED(D3DXCreateCubeTextureFromFileInMemoryEx(
			D3DEV_NULLREF,
			data,
			dataSize,
			width,
			s.m_numLevels,
			0, // Usage
			d3dFormat,
			D3DPOOL_SYSTEMMEM,
			filter,
			D3DX_DEFAULT, // Mip-filter
			0, // Color key
			NULL, // Image info
			NULL, // Palette entry
			(IDirect3DCubeTexture9**) &d3dBaseTexture)))
		{
			ueLogE("Failed to convert cube texture to desired format (format = %d, D3D format = %u)", (u32) t.m_format, (u32) d3dFormat);
			return false;
		}

		IDirect3DCubeTexture9* d3dTextureCube = (IDirect3DCubeTexture9*) d3dBaseTexture;

		// Make sure the format is known by the engine

		D3DSURFACE_DESC surfaceDesc;
		d3dTextureCube->GetLevelDesc(0, &surfaceDesc);
		if (surfaceDesc.Format != d3dFormat)
		{
			ueLogE("Unknown format of the texture (D3D format = %u).", (u32) surfaceDesc.Format);
			return false;
		}

		// Retrieve data of each face's mip-level

		for (u32 i = 0; i < 6; i++)
		{
			ueToolTexture::Image& face = vector_push(t.m_faces);
			face.m_name = s.m_sourceFileNames[faceIndex];

			for (u32 j = 0; j < d3dTextureCube->GetLevelCount(); j++)
			{
				d3dTextureCube->GetLevelDesc(j, &surfaceDesc);

				ueToolTexture::Level& dstLevel = vector_push(face.m_levels);
				dstLevel.m_width = surfaceDesc.Width;
				dstLevel.m_height = surfaceDesc.Height;
				dstLevel.m_depth = 1;

				dstLevel.m_dataSize = glUtils_CalcSurfaceSize(glTexType_Cube, dstLevel.m_width, dstLevel.m_height, dstLevel.m_depth, t.m_format);
				dstLevel.m_data = new u8[dstLevel.m_dataSize];
				D3DLOCKED_RECT d3dLockedRect;
				if (FAILED(d3dTextureCube->LockRect((D3DCUBEMAP_FACES) (D3DCUBEMAP_FACE_POSITIVE_X + i), j, &d3dLockedRect, NULL, 0)))
					return false;

				// Copy data row by row

				u32 rowSize, numRows;
				glUtils_CalcSurfaceRowParams(dstLevel.m_width, dstLevel.m_height, dstLevel.m_depth, t.m_format, rowSize, numRows);

				for (u32 y = 0; y < numRows; y++)
					ueMemCpy(
						(u8*) dstLevel.m_data + y * rowSize,
						(u8*) d3dLockedRect.pBits + y * d3dLockedRect.Pitch,
						rowSize);

				// Unlock mip-level

				if (FAILED(d3dTextureCube->UnlockRect((D3DCUBEMAP_FACES) (D3DCUBEMAP_FACE_POSITIVE_X + i), j)))
					return false;
			}
		}
	}
	else if (t.m_type == glTexType_3D)
	{
		if (FAILED(D3DXCreateVolumeTextureFromFileInMemoryEx(
			D3DEV_NULLREF,
			data,
			dataSize,
			width,
			height,
			depth,
			s.m_numLevels,
			0, // Usage
			d3dFormat,
			D3DPOOL_SYSTEMMEM,
			filter,
			D3DX_DEFAULT, // Mip-filter
			0, // Color key
			NULL, // Image info
			NULL, // Palette entry
			(IDirect3DVolumeTexture9**) &d3dBaseTexture)))
		{
			ueLogE("Failed to convert 3D texture to desired format (format = %d, D3D format = %u).", (u32) t.m_format, (u32) d3dFormat);
			return false;
		}

		IDirect3DVolumeTexture9* d3dTexture3D = (IDirect3DVolumeTexture9*) d3dBaseTexture;

		// Make sure the format is known by the engine

		D3DVOLUME_DESC volumeDesc;
		d3dTexture3D->GetLevelDesc(0, &volumeDesc);
		if (volumeDesc.Format != d3dFormat)
		{
			ueLogE("Unknown format of the texture (D3D format = %u).", (u32) volumeDesc.Format);
			return false;
		}

		// Create single image

		ueToolTexture::Image& face = vector_push(t.m_faces);
		face.m_name = s.m_sourceFileNames[faceIndex];

		// Retrieve data of each mip-level

		for (u32 i = 0; i < d3dTexture3D->GetLevelCount(); i++)
		{
			d3dTexture3D->GetLevelDesc(i, &volumeDesc);

			ueToolTexture::Level& dstLevel = vector_push(face.m_levels);
			dstLevel.m_width = volumeDesc.Width;
			dstLevel.m_height = volumeDesc.Height;
			dstLevel.m_depth = 1;

			dstLevel.m_dataSize = glUtils_CalcSurfaceSize(glTexType_3D, dstLevel.m_width, dstLevel.m_height, dstLevel.m_depth, t.m_format);
			dstLevel.m_data = new u8[dstLevel.m_dataSize];
			D3DLOCKED_BOX d3dLockedBox;
			if (FAILED(d3dTexture3D->LockBox(i, &d3dLockedBox, NULL, 0)))
				return false;

			// Copy data slice by slice

			u32 rowSize, numRows;
			glUtils_CalcSurfaceRowParams(dstLevel.m_width, dstLevel.m_height, dstLevel.m_depth, s.m_format, rowSize, numRows);

			UE_NOT_IMPLEMENTED();
			for (u32 y = 0; y < numRows; y++)
				ueMemCpy(
					(u8*) dstLevel.m_data + y * rowSize,
					(u8*) d3dLockedBox.pBits + y * d3dLockedBox.RowPitch,
					rowSize);

			// Unlock mip-level

			if (FAILED(d3dTexture3D->UnlockBox(i)))
				return false;
		}
	}

	// Success!

	return true;
}

bool CompileTexture(ueToolTexture& t, const ueToolTexture::LoadSettings& s)
{
	t.Destroy();
	t.m_isAtlas = s.m_isAtlas;
	t.m_settings = s;

	// Startup D3D9 nullref device

	ueD3D9_NULLREF_Startup();

	// Load & compile all individual source images

	t.m_type = s.m_type;

	std::vector<u8> data;
	for (u32 i = 0; i < s.m_sourceFileNames.size(); i++)
	{
		const char* fileName = s.m_sourceFileNames[i].c_str();

		if (!vector_u8_load_file(fileName, data))
		{
			ueLogE("Failed to load the image source file (path = '%s'; index = %u).", fileName, i);
			return false;
		}

		if (!CompileTextureFace(t, i, &data[0], (u32) data.size()))
		{
			ueLogE("Failed to load-from-data the image source file (path = '%s'; index = %u).", fileName, i);
			return false;
		}
	}

	// Optionally build atlas

	if (!t.BuildAtlas())
		return false;

	// Convert to output format

	if (!t.ConvertToOutputFormat())
		return false;

	return true;
}
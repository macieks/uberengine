#include "GL/Marmalade/glLib_Private_Marmalade.h"
#include "GL/glLib_Private.h"
#include "IO/ioFile.h"

extern bool IwImageUtil_LoadJPG(const char* buf, int len, CIwImage& img, bool downscale2x, u32 maxWidth, u32 maxHeight, u32& originalWidth, u32& originalHeight);
extern void IwImageUtil_Downsize2x(CIwImage* dst, CIwImage* src);
extern void IwImageUtil_Downsize2x(u8* pixels, u32& width, u32& height, u32& rowPitch, glBufferFormat format);

extern glDevice s_dev;

u32 glTextureBuffer_CalcGPUDataSize(glTextureBuffer* tb)
{
	const glTextureBufferDesc& desc = tb->m_desc;

	u32 dataSize = ueGEPow2(desc.m_width) * ueGEPow2(desc.m_height) * glUtils_GetFormatBits(desc.m_format) / 8;
	if (desc.m_numLevels != 1)
		dataSize = dataSize * 4 / 3;
	return dataSize;
}

struct PVRTexHeaderV3
{
    u32    version;            
    u32    flags;          
    u64    pixelFormat;        
    u32    colourSpace;        
    u32    channelType;        
    u32    height;         
    u32    width;          
    u32    depth;          
    u32    numSurfaces;        
    u32    numFaces;       
    u32    numMipmaps;     
    u32    metaDataSize;   
};

glTextureBuffer* glTextureBuffer_Create(const glTextureBufferDesc* desc, const glTextureInitData* initData)
{
	ueAssert(desc->m_type == glTexType_2D);

	// Create texture buffer

	glTextureBuffer* tb = new(GLDEV->m_textureBuffersPool) glTextureBuffer();
	ueAssert(tb);
	tb->m_desc = *desc;

	// Set up texture format / state

	if (!ueIsPow2(desc->m_width) || !ueIsPow2(desc->m_height))
		ueLogW("Clamping may not work with non-power of 2 textures (dims = %ux%u)", desc->m_width, desc->m_height);

	tb->m_handle.SetClamping(true);
	tb->m_handle.SetMipMapping(desc->m_numLevels == 1);
	tb->m_handle.SetModifiable((desc->m_flags & glTextureBufferFlags_IsDynamic) ? true : false);

	// Set initial texture data

	if (initData)
	{
		ueAssert(initData->m_dataArrayLength == 1);
		const glTextureData* data = &initData->m_dataArray[0];

		switch (initData->m_storage)
		{
			// Load GPU format

			case glTexStorage_GPU:
			{
				if (!ueIsPow2(desc->m_width) || !ueIsPow2(desc->m_height))
					ueLogW("Non-power of 2 texture will have to be upscaled at run-time: %ux%u", desc->m_width, desc->m_height);

				u32 realWidth = desc->m_width;
				u32 realHeight = desc->m_height;
				u32 realRowPitch = data->m_rowPitch;
				if (GLDEV->m_fakeDownsizeTextures2x)
					IwImageUtil_Downsize2x((u8*) data->m_data, realWidth, realHeight, realRowPitch, desc->m_format);

#if 1
				tb->m_handle.CopyFromBuffer(realWidth, realHeight, glUtils_ToMarmaladeFormat(desc->m_format), realRowPitch, (uint8*) data->m_data, NULL);
				tb->m_handle.Upload();

//				while (xxx) {} //make all marmalade textures square and pow2

#else // GL2 codepath for NPOT2 textures
				// Upload to VRAM

				if (IwGxGetHWType() == IW_GX_HWTYPE_GL2)
				{
					short dummyTextureData = 0x00ff;
					tb->m_handle.CopyFromBuffer(1, 1, CIwImage::RGB_565, 2, (uint8*) &dummyTextureData, 0);
					tb->m_handle.Upload();

					glBindTexture(GL_TEXTURE_2D, tb->m_handle.m_HWID);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// format
#define GL_BGR 0x80E0
#define GL_BGRA 0x80E1
#define GL_ABGR 0x80E1

// internal format
#define GL_RGB8 0x1907
#define GL_RGBA8 0x1908

// type
//#define GL_UNSIGNED_INT_8_8_8_8 0x8035
					switch (desc->m_format)
					{
					case glBufferFormat_B8G8R8:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, realWidth, realHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, data->m_data);
						break;
					case glBufferFormat_A8B8G8R8:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, realWidth, realHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);//data->m_data);
						break;
					}
				}
				else
					tb->m_handle.Upload();
#endif
				break;
			}

			// Load PVR image straight into GPU memory

			case glTexStorage_PVR:
			{
				// Create dummy texture

				short dummyTextureData = 0x00ff;
				tb->m_handle.CopyFromBuffer(1, 1, CIwImage::RGB_565, 2, (uint8*) &dummyTextureData, 0);
				tb->m_handle.Upload();

				// Get PVRTC data

				const PVRTexHeaderV3* pvrHeader = (PVRTexHeaderV3*) data->m_data;
				const u32 headerAndMetadataSize = sizeof(PVRTexHeaderV3) - sizeof(u32) /* u32 metadata size */ + pvrHeader->metaDataSize;
				u8* pvrtcData = (u8*) data->m_data + headerAndMetadataSize;
				const u32 pvrtcDataSize = data->m_rowPitch - headerAndMetadataSize;

				// Upload to VRAM (hack Marmalade)

				ueAssert(IwGxGetHWType() == IW_GX_HWTYPE_GL1 || IwGxGetHWType() == IW_GX_HWTYPE_GL2);

				glBindTexture(GL_TEXTURE_2D, tb->m_handle.m_HWID);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG	0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG	0x8C03
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG	0x8C00
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG	0x8C02

				switch (desc->m_format)
				{
				case glBufferFormat_PVRTC2_RGB:
					glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, desc->m_width, desc->m_height, 0, (desc->m_width * desc->m_height) / 4, pvrtcData);
					break;
				case glBufferFormat_PVRTC2_RGBA:
					glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, desc->m_width, desc->m_height, 0, (desc->m_width * desc->m_height) / 4, pvrtcData);
					break;
				case glBufferFormat_PVRTC4_RGB:
					glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, desc->m_width, desc->m_height, 0, (desc->m_width * desc->m_height) / 2, pvrtcData);
					break;
				case glBufferFormat_PVRTC4_RGBA:
					glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, desc->m_width, desc->m_height, 0, (desc->m_width * desc->m_height) / 2, pvrtcData);
					break;
				}

				break;
			}

			// Load PNG

			case glTexStorage_PNG:
			{
				// Very basic PNG check

				const u8* bytes = (const u8*) data->m_data;
				const u32 size = data->m_rowPitch;
				if (size < 3 || bytes[1] != 'P' || bytes[2] != 'N' || bytes[3] != 'G')
				{
					ueDelete(tb, GL_FREQ_ALLOC);
					return NULL;
				}

				// Load either straight into texture's image or into some intermediate image (and downsize to texture's image later)

				CIwImage tmpImg;
				CIwImage& img = GLDEV->m_fakeDownsizeTextures2x ? tmpImg : tb->m_handle.GetImage();

				// Load image from memory

				s3eFile* memoryFile = s3eFileOpenFromMemory(data->m_data, size);
				img.ReadFile(memoryFile);
				s3eFileClose(memoryFile);

				// Set up / check texture properties

				ueAssert(img.GetFormat() == glUtils_ToMarmaladeFormat(desc->m_format));
				tb->m_desc.m_width = img.GetWidth();
				tb->m_desc.m_height = img.GetHeight();

				// Downsize if desired

				if (GLDEV->m_fakeDownsizeTextures2x)
					IwImageUtil_Downsize2x(&tb->m_handle.GetImage(), &tmpImg);

				// Upload to VRAM

				tb->m_handle.Upload();
				break;
			}

			// Load JPG

			case glTexStorage_JPG:
			{
				CIwImage& img = tb->m_handle.GetImage();

				// Load (and optionally downscale) JPG image
				// FIXME: Use user supplied maxWidth and maxHeight

				u32 originalWidth, originalHeight;
				if (!IwImageUtil_LoadJPG((const char*) data->m_data, data->m_rowPitch, img, GLDEV->m_fakeDownsizeTextures2x, 1024, 1024, originalWidth, originalHeight))
				{
					ueDelete(tb, GL_FREQ_ALLOC);
					return NULL;
				}

				tb->m_desc.m_width = originalWidth;
				tb->m_desc.m_height = originalHeight;
				tb->m_desc.m_format = glBufferFormat_R8G8B8;

				// Upload to VRAM

				tb->m_handle.Upload();
				break;
			}
		}
	}

	// Determine GPU data size

	tb->m_gpuDataSize = glTextureBuffer_CalcGPUDataSize(tb);
	s_dev.m_textureMemory += tb->m_gpuDataSize;

#if 0
	int w;
#define GL_TEXTURE_WIDTH 0x1000
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WIDTH, &w);
	GLenum err = glGetError();
#endif

	// Create material

	const ueBool hasAlpha = glUtils_HasAlpha(desc->m_format);

	tb->m_material.SetTexture(&tb->m_handle);
	tb->m_material.SetAlphaMode(hasAlpha ? CIwMaterial::ALPHA_BLEND : CIwMaterial::ALPHA_NONE);
	tb->m_material.SetClamping(true);
	tb->m_material.SetDepthWriteMode(CIwMaterial::DEPTH_WRITE_DISABLED);
	tb->m_material.SetCullMode(CIwMaterial::CULL_NONE);

	return tb;
}

u32 glTextureBuffer_Update(glTextureBuffer* tb, u32 face, u32 level, const glTextureArea* area, const glTextureData* data)
{
	ueAssert(!area);
	tb->m_handle.CopyFromBuffer(tb->m_desc.m_width, tb->m_desc.m_height, glUtils_ToMarmaladeFormat(tb->m_desc.m_format), data->m_rowPitch, (uint8*) data->m_data, NULL);
	tb->m_handle.ChangeTexels(tb->m_handle.GetTexels());
	return 0;
}

void glTextureBuffer_Destroy(glTextureBuffer* tb)
{
	s_dev.m_textureMemory -= tb->m_gpuDataSize;
	ueDelete(tb, GLDEV->m_textureBuffersPool);
}

CIwTexture* glTextureBuffer_GetIwTexture(glTextureBuffer* tb)
{
	return &tb->m_handle;
}

CIwMaterial* glTextureBuffer_GetIwMaterial(glTextureBuffer* tb)
{
	return &tb->m_material;
}

ueBool glTextureBuffer_Lock(glTextureBuffer* tb, glTextureData* result, u32 face, u32 mipLevel, u32 lockFlags /*= glBufferLockFlags_Write*/, const glTextureArea* area /*= NULL*/)
{
	result->m_data = tb->m_handle.GetTexels();
	result->m_rowPitch = tb->m_handle.GetPitch();
	result->m_slicePitch = 0;
	return UE_TRUE;
}

void glTextureBuffer_Unlock(glTextureBuffer* tb, u32 face, u32 mipLevel)
{
	tb->m_handle.ChangeTexels(tb->m_handle.GetTexels());
}
#include "TextureCompiler_Common/ueToolTexture.h"
#include "IO/ioPackageWriter.h"

#include <nvtt/nvtt.h>
#include <nvcore/StrLib.h>
#include <nvcore/StdStream.h>
#include <nvmath/Color.h>
#include <nvimage/Image.h>
#include <nvimage/DirectDrawSurface.h>
#include <nvimage/ImageIO.h>
#include <nvimage/FloatImage.h>
#include <nvimage/Filter.h>

void* nv::mem::malloc(size_t size)
{
	return ::malloc(size);
}

void* nv::mem::malloc(size_t size, const char * file, int line)
{
	NV_UNUSED(file);
	NV_UNUSED(line);
	return ::malloc(size);
}

void nv::mem::free(const void * ptr)
{
	::free(const_cast<void *>(ptr));
}

void* nv::mem::realloc(void * ptr, size_t size)
{
	nvDebugCheck(ptr != NULL || size != 0);	// undefined realloc behavior.
	return ::realloc(ptr, size);
}

nvtt::Format ToNVTTFormat(glBufferFormat format)
{
	switch (format)
	{
		case glBufferFormat_B8G8R8A8:
		case glBufferFormat_A8:
			return nvtt::Format_RGBA;
		case glBufferFormat_DXT1: return nvtt::Format_DXT1;
//		case glBufferFormat_DXT1a: return nvtt::Format_DXT1a;
		case glBufferFormat_DXT3: return nvtt::Format_DXT3;
		case glBufferFormat_DXT5: return nvtt::Format_DXT5;
//		case glBufferFormat_DXT5n: return nvtt::Format_DXT5n;
		UE_INVALID_CASE(format);
	}
	return nvtt::Format_RGBA;
}

struct MyMessageHandler : nv::MessageHandler
{
	virtual void log(const char * str, va_list arg)
	{
		ueLogVI(str, arg);
	}
};
	
struct MyAssertHandler : nv::AssertHandler
{
	virtual int assert(const char *exp, const char *file, int line, const char *func = 0)
	{
		ueLogE("NVIDIA Texture Tools asserted:\nexp: %s\nfile: %s:%d\nfunc: %s", exp, file, line, func);
		return 0;
	}
};

struct MyErrorHandler : public nvtt::ErrorHandler
{
	virtual void error(nvtt::Error e)
	{
		ueLogE("NVTT error: %u", (u32) e);
	}
};

class MyStream : public nv::Stream
{
public:
	virtual uint serialize( void * data, uint len )
	{
		vector_push_back_array(m_data, (u8*) data, len);
		return len;
	}

	virtual void seek( uint pos ) { UE_ASSERT(0); }
	virtual uint tell() const { return m_data.size(); }
	virtual uint size() const { return m_data.size(); }
	virtual bool isError() const { return false; }
	virtual void clearError() {}
	virtual bool isAtEnd() const { return false; }
	virtual bool isSeekable() const { return false; }
	virtual bool isLoading() const { return false; }
	virtual bool isSaving() const { return true; }

	std::vector<u8> m_data;
};

struct MyOutputHandler : public nvtt::OutputHandler
{
	MyOutputHandler() :
		m_data(NULL),
		m_size(0),
		m_offset(0),
		m_numFaces(0),
		m_numLevels(0),
		m_isStarted(false)
	{}

	/// Indicate the start of a new compressed image that's part of the final texture.
	virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel)
	{
		UE_ASSERT(face < UE_ARRAY_SIZE(m_mips));
		UE_ASSERT(miplevel < UE_ARRAY_SIZE(m_mips[0]));

		//ueLogD("next image face/level size:%d dim:%dx%dx%d face:%d level:%d", size, width, height, depth, face, miplevel);

		m_isStarted = true;

		Level& level = m_mips[face][miplevel];
		level.m_data = (u8*) m_data + m_offset;
		level.m_dataSize = size;
		level.m_width = width;
		level.m_height = height;
		level.m_depth = 1;

		m_numFaces = ueMax(m_numFaces, (u32) face + 1);
		m_numLevels = ueMax(m_numLevels, (u32) miplevel + 1);
	}

	/// Output data. Compressed data is output as soon as it's generated to minimize memory allocations.
	virtual bool writeData(const void * data, int size)
	{
		if (!m_isStarted)
			return true;

		switch (m_outputFormat)
		{
			case glBufferFormat_A8:
			{
				UE_ASSERT(m_nvttOutputFormat == nvtt::Format_RGBA);
				UE_ASSERT((size & 3) == 0);
				UE_ASSERT(m_offset + size / 4 <= m_size);

				const u8* src = (const u8*) data;
				u8* dst = (u8*) m_data + m_offset;
				m_offset += size / 4;

				while (size)
				{
					dst[0] = src[3]; // Copy just the alpha
					dst++;
					size -= 4;
					src += 4;
				}

				break;
			}

			default:
				UE_ASSERT(m_offset + size <= m_size);
				memcpy((u8*) m_data + m_offset, data, size);
				m_offset += size;
				break;
		}

		return true;
	}

	virtual bool writeDataHeader(const void * data, int size)
	{
		return true;
	}

	virtual void setTotal(int totalSize)
	{
		m_data = malloc(totalSize);
		UE_ASSERT(m_data);
		m_size = totalSize;
	}

	struct Level
	{
		void* m_data;
		u32 m_dataSize;
		u32 m_width;
		u32 m_height;
		u32 m_depth;
	};

	void* m_data;
	u32 m_size;
	u32 m_offset;
	Level m_mips[6][20];
	u32 m_numLevels;
	u32 m_numFaces;
	nvtt::Format m_nvttOutputFormat;
	glBufferFormat m_outputFormat;
	bool m_isStarted;
};

bool CompileTextureFace(ueToolTexture& t, u32 faceIndex, const ueToolTexture::LoadSettings& s)
{
	const char* fileName = s.m_sourceFileNames[faceIndex].c_str();

	// Load image and set up input options

	nvtt::InputOptions inputOptions;

	nv::Image image;
	if (string_ends_with(fileName, ".dds", false))
	{
		nv::DirectDrawSurface dds(fileName);
		const bool success = dds.isValid() && dds.isSupported();
		if (!success)
		{
			ueLogE("Failed to load DDS image (path = %s)", fileName);
			return false;
		}
		UE_ASSERT(dds.isTexture2D() || dds.isTextureCube());

		if (s.m_storage != glTexStorage_GPU)
			dds.mipmap(&image, 0, 0);
		else
		{
			inputOptions.setTextureLayout(
				dds.isTexture2D() ? nvtt::TextureType_2D : nvtt::TextureType_Cube,
				/*(s.m_rescaleMethod == ueToolTexture::ueTexRescaleMethod_None && s.m_width) ? s.m_width :*/ dds.width(),
				/*(s.m_rescaleMethod == ueToolTexture::ueTexRescaleMethod_None && s.m_height) ? s.m_height :*/ dds.height());

			const u32 numFaces = dds.isTextureCube() ? 6 : 1;
			for (u32 face = 0; face < numFaces; face++)
				for (u32 level = 0; level < dds.mipmapCount(); level++)
				{
					dds.mipmap(&image, face, level);
					inputOptions.setMipmapData(image.pixels(), image.width(), image.height(), 1, face, level);
				}
		}
	}
	else
	{
		if (!image.load(fileName))
		{
			ueLogE("Failed to load source image (path = %s)", fileName);
			return false;
		}

		if (s.m_storage == glTexStorage_GPU)
		{
			inputOptions.setTextureLayout(
				nvtt::TextureType_2D,
				/*(s.m_rescaleMethod == ueToolTexture::ueTexRescaleMethod_None && s.m_width) ? s.m_width :*/ image.width(),
				/*(s.m_rescaleMethod == ueToolTexture::ueTexRescaleMethod_None && s.m_height) ? s.m_height :*/ image.height());
			inputOptions.setMipmapData(image.pixels(), image.width(), image.height());
		}
	}

	// Handle non-GPU storage

	if (s.m_storage != glTexStorage_GPU)
	{
		MyStream stream;

#if 0	// TODO: Support resizing
		nv::FloatImage fimage(&image);
		fimage.toLinear(0, 3, gamma);

		nv::BoxFilter filter();
		nv::FloatImage fresult(fimage.resize(filter, uint(image.width() * scale), uint(image.height() * scale), wrapMode));
		
		nv::Image result(fresult->createImageGammaCorrect(gamma));
		result->setFormat(nv::Image::Format_ARGB);
#endif

		bool success = false;
		switch (s.m_storage)
		{
		case glTexStorage_TGA:
			success = nv::ImageIO::saveTGA((nv::Stream&) stream, &image);
			break;
		default:
			ueLogE("No support in NVIDIA Texture Tools for serializing texture into '%s' storage (path = %s)", ueEnumToString(glTexStorage, s.m_storage), fileName);
			return false;
		}

		if (!success)
		{
			ueLogE("Failed to serialize image into '%s' storage (path = %s)", ueEnumToString(glTexStorage, s.m_storage), fileName);
			return false;
		}

		// Store non-GPU format image into single face, top mip level

		ueToolTexture::Image& face = vector_push(t.m_faces);
		face.m_name = s.m_sourceFileNames[faceIndex];
		ueToolTexture::Level& topLevel = vector_push(face.m_levels);

		topLevel.m_data = &stream.m_data[0];
		topLevel.m_dataSize = stream.m_data.size();
		topLevel.m_ownsMemory = false;
		topLevel.m_width = image.width();
		topLevel.m_height = image.height();
		topLevel.m_depth = 1;

		return true;
	}

	// Complete set up of the input parameters

	inputOptions.setMipmapFilter(nvtt::MipmapFilter_Box);

	nvtt::RoundMode nvttRoundMode;
	switch (s.m_rescaleMethod)
	{
		case ueTextureRescaleMethod_None: nvttRoundMode = nvtt::RoundMode_None; break;
		case ueTextureRescaleMethod_ToNearestPowerOf2: nvttRoundMode = nvtt::RoundMode_ToNearestPowerOfTwo; break;
		case ueTextureRescaleMethod_ToPreviousPowerOf2: nvttRoundMode = nvtt::RoundMode_ToPreviousPowerOfTwo; break;
		case ueTextureRescaleMethod_ToNextPowerOf2: nvttRoundMode = nvtt::RoundMode_ToNextPowerOfTwo; break;
		UE_INVALID_CASE(s.m_rescaleMethod);
	}
	inputOptions.setRoundMode(nvttRoundMode);

	inputOptions.setNormalMap(s.m_isNormalMap);
	if (s.m_isNormalMap)
		inputOptions.setNormalizeMipmaps(true);

	// Set up compression options

	const nvtt::Format nvttOutputFormat = ToNVTTFormat(s.m_format);
	nvtt::CompressionOptions compressionOptions;
	compressionOptions.setFormat(nvttOutputFormat);
	compressionOptions.setQuality(nvtt::Quality_Normal);

	// Set up compressor and start it

	nvtt::Compressor compressor;
	compressor.enableCudaAcceleration(false);

	// Set up our output handler

	u32 totalSize = (u32) compressor.estimateSize(inputOptions, compressionOptions);
	switch (s.m_format)
	{
		case glBufferFormat_A8:
			UE_ASSERT(nvttOutputFormat == nvtt::Format_RGBA);
			totalSize /= 4;
			break;
		default:
			break;
	}

	MyOutputHandler outputHandler;
	outputHandler.setTotal(totalSize);
	outputHandler.m_nvttOutputFormat = nvttOutputFormat;
	outputHandler.m_outputFormat = s.m_format;

	// Set up output options

	MyErrorHandler errorHandler;
	nvtt::OutputOptions outputOptions;
	outputOptions.setOutputHandler(&outputHandler);
	outputOptions.setErrorHandler(&errorHandler);
	outputOptions.setOutputHeader(false);

	// Start compressor

	if (!compressor.process(inputOptions, compressionOptions, outputOptions))
	{
		ueLogE("Failed to run NVTT compressor");
		return false;
	}
	UE_ASSERT(outputHandler.m_offset == totalSize);

	// Create texture face (with all its mip-levels)

	for (u32 i = 0; i < outputHandler.m_numFaces; i++)
	{
		ueToolTexture::Image& face = vector_push(t.m_faces);
		face.m_name = s.m_sourceFileNames[faceIndex];
		for (u32 j = 0; j < outputHandler.m_numLevels; j++)
		{
			MyOutputHandler::Level& srcLevel = outputHandler.m_mips[i][j];
			ueToolTexture::Level& dstLevel = vector_push(face.m_levels);

			dstLevel.m_data = srcLevel.m_data;
			dstLevel.m_dataSize = srcLevel.m_dataSize;
			dstLevel.m_ownsMemory = false;
			dstLevel.m_width = srcLevel.m_width;
			dstLevel.m_height = srcLevel.m_height;
			dstLevel.m_depth = srcLevel.m_depth;
		}
	}

	return true;
}

bool CompileTexture(ueToolTexture& t, const ueToolTexture::LoadSettings& s)
{
	t.Destroy();
	t.m_isAtlas = s.m_isAtlas;
	t.m_settings = s;

	MyMessageHandler messageHandler;
	nv::debug::setMessageHandler(&messageHandler);

	MyAssertHandler assertHandler;
	nv::debug::setAssertHandler(&assertHandler);

	// Load & compile all images into a texture

	t.m_storage = s.m_storage;
	t.m_format = s.m_format;
	t.m_type = s.m_type;

	for (u32 i = 0; i < s.m_sourceFileNames.size(); i++)
		if (!CompileTextureFace(t, i, s))
			return false;

	return true;
}
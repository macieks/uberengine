#include "TextureCompiler_Common/ueToolTexture.h"
#include "IO/ioPackageWriter.h"
#include "GL/glLib_Shared.h"
#include "GL/D3D9/ueD3D9_NULLREF.h"
#include "Utils/utProcess.h"
#include "Memory/ueMallocAllocator.h"

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

ueBool glUtils_ToPVRTCFormat(glBufferFormat format, std::string& pvrtcFormat, bool& hasAlpha)
{
	switch (format)
	{
		case glBufferFormat_PVRTC2_RGB:
			pvrtcFormat = "oglpvrtc2";
			hasAlpha = false;
			break;
		case glBufferFormat_PVRTC2_RGBA:
			pvrtcFormat = "oglpvrtc2";
			hasAlpha = true;
			break;
		case glBufferFormat_PVRTC4_RGB:
			pvrtcFormat = "oglpvrtc4";
			hasAlpha = false;
			break;
		case glBufferFormat_PVRTC4_RGBA:
			pvrtcFormat = "oglpvrtc4";
			hasAlpha = true;
			break;
		default:
			return UE_FALSE;
	}
	return UE_TRUE;
}

bool CompileTextureFace(ueToolTexture& t, u32 faceIndex, const ueToolTexture::LoadSettings& s)
{
	if (s.m_format != glBufferFormat_PVRTC2_RGB &&
		s.m_format != glBufferFormat_PVRTC2_RGBA &&
		s.m_format != glBufferFormat_PVRTC4_RGB &&
		s.m_format != glBufferFormat_PVRTC4_RGBA)
	{
		ueLogE("Unsupported output format %s; must be one of PVRTC formats e.g. PVRTC2_RGB, PVRTC2_RGBA, PVRTC4_RGB, PVRTC4_RGBA;(source path = '%s')", ueEnumToString(glBufferFormat, s.m_format), s.m_sourceFileNames[faceIndex].c_str());
		return false;
	}

	if (s.m_storage != glTexStorage_PVR && s.m_storage != glTexStorage_GPU)
	{
		ueLogE("Unsupported storage specified for PVR texture %s (source path = '%s')", ueEnumToString(glTexStorage, s.m_storage), s.m_sourceFileNames[faceIndex].c_str());
		return false;
	}

	// Get source image information

	D3DXIMAGE_INFO info;
	if (FAILED(D3DXGetImageInfoFromFile(s.m_sourceFileNames[faceIndex].c_str(), &info)))
	{
		ueLogE("Failed to get image information using D3DXGetImageInfoFromFileInMemory (path = '%s')", s.m_sourceFileNames[faceIndex].c_str());
		return false;
	}

	// Texture will be auto-resized (no stretch) to power of 2 square

	std::string sourcePath = s.m_sourceFileNames[faceIndex];
	bool deleteSourceWhenDone = false;
	if (!ueIsPow2(info.Width) || !ueIsPow2(info.Height) || info.Width != info.Height)
	{
		ueLogI("Source image (path = '%s') must be square and have power-of-2 dimensions in order to be coverted to PVRTC format - converting now...", s.m_sourceFileNames[faceIndex].c_str());

		const std::string resizedPath = "tmp_pow2.png";
		if (!ResizeToPow2(sourcePath, resizedPath, info))
			return false;

		sourcePath = resizedPath;
		deleteSourceWhenDone = true;
	}

	// Figure out target format

	bool hasAlpha;
	std::string pvrtcFormat;
	if (!glUtils_ToPVRTCFormat(t.m_format, pvrtcFormat, hasAlpha))
	{
		ueLogE("Unsupported PVRTC texture format (format = %u).", (u32) s.m_format);
		return false;
	}

	// Build arguments for the command line

	const std::string& inputFileName = sourcePath;
	const std::string resourceName = inputFileName.substr(0, inputFileName.length() - 4);
	const std::string outputFileName = "tmp_final.pvr";

	std::string command;
	command += "-f" + pvrtcFormat;			// Output format
	command += " -i" + inputFileName;		// Input file name
	command += " -o" + outputFileName;		// Output file name
	command += " -yflip0";					// Invert Y
//	command += " -square";					// Make it square with power-of-2 dimensions
//	command += " -border";					// Makes mirrored border around texture (4 pixels wide for PVRTC4)
	if (s.m_numLevels == 0)
		command += " -m";					// Don't generate mips
	else if (s.m_numLevels != 1)
	{
		ueLogE("Unsupported number of mip-levels for PVRTC texture compression (num levels = %u).", s.m_numLevels);
		return false;
	}

	// Run command

	char pvrTexToolPath[512];
	GetCurrentDirectoryA(512, pvrTexToolPath);
	ueStrFormatS(pvrTexToolPath, "../../../../Bin/Win32_Release/PVRTexTool.exe", );

	utProcessParams processParams;
	processParams.m_executableFileName = pvrTexToolPath;
	processParams.m_arguments = command.c_str();
	processParams.m_wait = UE_TRUE;
	processParams.m_separateWindow = UE_FALSE;

	utProcess* process = NULL;
	s32 errorCode = 0;
	if (!utProcess_Spawn(&processParams, &process, &errorCode))
	{
		ueLogE("Failed to run %s process with '%s' arguments", processParams.m_executableFileName, processParams.m_arguments);
		return false;
	}

	if (errorCode != 0)
	{
		ueLogE("%s process with '%s' arguments failed with error code %d - make sure the source texture (path = '%s') has power-of-2 dimensions", processParams.m_executableFileName, processParams.m_arguments, errorCode, inputFileName.c_str());
		return false;
	}

	// Read file into memory

	void* buffer = NULL;
	ueSize size = 0;
	ueMallocAllocator mallocAllocator;
	if (!ioFile_Load(outputFileName.c_str(), buffer, size, 0, &mallocAllocator, UE_FALSE))
	{
		ueLogE("Failed to read file '%s' output by %s", outputFileName.c_str(), processParams.m_executableFileName);
		return false;
	}

	// Delete temporary file

	ioFile_Delete(outputFileName.c_str());
	if (deleteSourceWhenDone)
		ioFile_Delete(sourcePath.c_str());

	// Add face to texture

	ueToolTexture::Image& face = vector_push(t.m_faces);
	face.m_name = resourceName;

	ueToolTexture::Level& level = vector_push(face.m_levels);
	level.m_width = ueGEPow2(info.Width);
	level.m_height = ueGEPow2(info.Height);
	level.m_depth = 1;

	// Check for one-texture atlas

	t.MakeAtlasIfRequired(info.Width, info.Height);

	// Get pointer to PVRTC data within PVR file

	const PVRTexHeaderV3* pvrHeader = (PVRTexHeaderV3*) buffer;
	const u32 headerAndMetadataSize = sizeof(PVRTexHeaderV3) - sizeof(u32) /* metadata size */ + pvrHeader->metaDataSize;
	u8* pvrtcData = (u8*) buffer + headerAndMetadataSize;
	const u32 pvrtcDataSize = size - headerAndMetadataSize;

	// Store PVR / PVRTC data

	if (s.m_storage == glTexStorage_GPU)
	{
		level.m_dataSize = pvrtcDataSize;
		level.m_data = pvrtcData;
	}
	else if (s.m_storage == glTexStorage_PVR)
	{
		t.m_storage = glTexStorage_PVR;

		level.m_dataSize = size;
		level.m_data = buffer;
	}

	return true;
}

bool CompileTexture(ueToolTexture& t, const ueToolTexture::LoadSettings& s)
{
	t.Destroy();
	t.m_isAtlas = s.m_isAtlas;
	t.m_settings = s;
	
	if (s.m_isDynamic)
	{
		ueLogE("PVRTC textures can't be dynamic");
		return false;
	}

	// Startup D3D9 nullref device but don't shut it down (in non-GPU storage mode, the image data points to D3D resource)

	ueD3D9_NULLREF_Startup();

	// Load & compile all images into a texture

	t.m_storage = s.m_storage;
	t.m_format = s.m_format;
	t.m_type = s.m_type;

	for (u32 i = 0; i < s.m_sourceFileNames.size(); i++)
		if (!CompileTextureFace(t, i, s))
			return false;

	return true;
}
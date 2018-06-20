#include "TextureCompiler_Common/ueToolTexture.h"
#include "Misc/ueTextureRectPacker.h"
#include "IO/ioPackageWriter.h"
#include "IO/ioPackageWriterUtils.h"
#include "GL/glLib_Shared.h"
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

bool ueToolTexture::ConvertToOutputFormatPVRTC(u32 faceIndex)
{
	// Get top topLevel

	ueToolTexture::Image& face = m_faces[faceIndex];
	ueToolTexture::Level& topLevel = face.m_levels[0];

	// Figure out target format

	bool hasAlpha;
	std::string pvrtcFormat;
	if (!glUtils_ToPVRTCFormat(m_settings.m_format, pvrtcFormat, hasAlpha))
	{
		ueLogE("Unsupported PVRTC texture format (format = %u).", (u32) m_settings.m_format);
		return false;
	}

	// Save file to PNG to make input for PVR Tex Tool

	const std::string inputFileName = std::string(m_outputPath) + ".png";
	const std::string outputFileName = std::string(m_outputPath) + ".pvr";

	if (!SaveAsNonGPU(topLevel, inputFileName.c_str(), D3DXIFF_PNG))
		return false;

	// Build arguments for the command line

	std::string command;
	command += "-f" + pvrtcFormat;			// Output format
	command += " -i" + inputFileName;		// Input file name
	command += " -o" + outputFileName;		// Output file name
	command += " -yflip0";					// Invert Y
//	command += " -square";					// Make it square with power-of-2 dimensions
//	command += " -border";					// Makes mirrored border around texture (4 pixels wide for PVRTC4)
	if (m_settings.m_numLevels == 0)
		command += " -m";					// Don't generate mips
	else if (m_settings.m_numLevels != 1)
	{
		ueLogE("Unsupported number of mip-levels for PVRTC texture compression (num levels = %u).", m_settings.m_numLevels);
		return false;
	}

	// Run command

	char pvrTexToolPath[512];
	GetCurrentDirectoryA(512, pvrTexToolPath);
	ueStrFormatS(pvrTexToolPath, "../../../Bin/Win32_Release/PVRTexTool.exe", );

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

	if (!m_isAtlas)
		ioFile_Delete(outputFileName.c_str()); // Leave PVR file for manual reference
	ioFile_Delete(inputFileName.c_str());

	// Check for one-texture atlas

	face.m_levels.resize(1);

	// Get pointer to PVRTC data within PVR file

	const PVRTexHeaderV3* pvrHeader = (PVRTexHeaderV3*) buffer;
	const u32 headerAndMetadataSize = sizeof(PVRTexHeaderV3) - sizeof(u32) /* metadata size */ + pvrHeader->metaDataSize;
	u8* pvrtcData = (u8*) buffer + headerAndMetadataSize;
	const u32 pvrtcDataSize = size - headerAndMetadataSize;

	// Store PVR / PVRTC data

	m_storage = m_settings.m_storage;
	m_format = m_settings.m_format;
	topLevel.m_ownsMemory = false;

	if (m_settings.m_storage == glTexStorage_GPU)
	{
		topLevel.m_dataSize = pvrtcDataSize;
		topLevel.m_data = pvrtcData;
	}
	else if (m_settings.m_storage == glTexStorage_PVR)
	{
		topLevel.m_dataSize = size;
		topLevel.m_data = buffer;
	}

	return true;
}
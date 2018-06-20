#include "ueAFSPacker.h"

int main(int argc, char** args)
{
	if (argc != 4)
	{
		fprintf(stderr, "Invalid arguments, usage: AFSPacker.exe <target_sku> <input_dir> <output_file>\n"
			"\t<target_sku> - one of: %s\n", uePlatforms_AsString());
		return 1;
	}

	uePlatform targetSKU;
	uePlatform_FromString(targetSKU, args[1]);
	if (targetSKU == uePlatform_Unknown)
	{
		fprintf(stderr, "Unknown SKU - must be one of the following: %s\n", uePlatforms_AsString());
		return 2;
	}

	// Startup

	ueToolApp_Startup();

	// Set up packing parameters

	ueAFSPacker::PackParams params;
	params.m_inputPath = args[2];
	params.m_outputPath = args[3];
	params.m_platform = targetSKU;
	params.m_silentMode = false;

	// Pack

	const bool success = ueAFSPacker().PackArchive(params);

	// Shutdown

	ueToolApp_Shutdown();

	// Return result

	return success ? 0 : 10;
}
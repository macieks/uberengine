#include "ueAFSUnpacker.h"

int main(int argc, char** args)
{
	if (argc != 3)
	{
		fprintf(stderr, "Invalid args count, usage: <input-file> <output-dir>\n");
		return 1;
	}

	ueToolApp_Startup();

	ioArcFileSysUnpacker::UnpackParams params;
	params.m_inputPath = args[1];
	params.m_outputPath = args[2];
	params.m_silentMode = false;
	const bool success = ioArcFileSysUnpacker().UnpackArchive(params);

	ueToolApp_Shutdown();

	return success ? 0 : 3;
}
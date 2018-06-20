#pragma once

#include "Base/ueToolsBase.h"
#include "IO/ioFileSys_AFS_Types.h"

struct ioFileSys;

class ioArcFileSysUnpacker
{
protected:
	std::string m_outputPath;
	bool m_silentMode;
	std::string m_errorDesc;

	ioFileSys* m_afs;

public:
	struct UnpackParams
	{
		const char* m_inputPath;
		const char* m_outputPath;
		bool m_silentMode;
	};

	ioArcFileSysUnpacker();
	bool UnpackArchive(const UnpackParams& params);

	const char* GetErrorDescription() const { return m_errorDesc.c_str(); }

private:
	bool ProcessFile(const std::string& parentPath, const ioFileInfo_AFS* fileInfo, u32 level);
	bool ProcessDir(const std::string& parentPath, const ioDirInfo_AFS* dirInfo, u32 level);

	void OutputIndent(u32 level);
};
#pragma once

//! Input arguments for asset compiler
class ueAssetCompilerAppArgSet
{
public:
	bool Init(u32 count, char** args);

	bool m_help;					//!< Help only mode

	const char* m_asset;            //!< Asset name
	const char* m_config;			//!< Export configuration
	const char* m_rootSrcDir;       //!< Root source data directory
	const char* m_rootExpDir;       //!< Root export data directory
	const char* m_rootDepDir;       //!< Root dependency data directory

	bool m_verbose;					//!< Enable verbose output

	void DumpUsage();
private:
	ueAppArgSet m_argSet;
};

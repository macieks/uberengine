#include "Base/ueBase.h"
#include "IO/ioFile.h"

struct ueAppData
{
	ueBool m_isInitialized;

	ueBool m_isDevelopmentMode;

	char m_languageSymbol[32];

	ueAppData() : m_isInitialized(UE_FALSE) {}
};

static ueAppData s_data;

void ueAssets_Startup(ueBool developmentMode)
{
	UE_ASSERT(!s_data.m_isInitialized);
	ueStrCpyS(s_data.m_languageSymbol, "EN");
	s_data.m_isInitialized = UE_TRUE;
	s_data.m_isDevelopmentMode = developmentMode;
}

void ueAssets_Shutdown()
{
	UE_ASSERT(s_data.m_isInitialized);
	s_data.m_isInitialized = UE_FALSE;
}

void ueAssets_SetLanguageSymbol(const char* symbol)
{
	UE_ASSERT(s_data.m_isInitialized);
	ueStrCpyS(s_data.m_languageSymbol, symbol);
}

void ueAssets_GetAssetPath(uePath& path, const char* assetName, const char* assetExtension, const char* assetConfig, ueAssetPath assetPathType)
{
	UE_ASSERT(s_data.m_isInitialized);

	// In development mode try paths without config first

	if (s_data.m_isDevelopmentMode)
	{
		if (assetPathType != ueAssetPath_NonlocalizedOnly)
		{
			ueStrFormatS(path, "%s_%s.%s", assetName, s_data.m_languageSymbol, assetExtension);
			if (ioFile_Exists(path))
				return;
		}

		if (assetPathType != ueAssetPath_LocalizedOnly)
		{
			ueStrFormatS(path, "%s.%s", assetName, assetExtension);
			if (ioFile_Exists(path))
				return;
		}

	}

	// Try localized asset name

	if (assetPathType != ueAssetPath_NonlocalizedOnly)
	{
		if (assetConfig)
			ueStrFormatS(path, "%s/%s_%s.%s", assetConfig, assetName, s_data.m_languageSymbol, assetExtension);
		else
			ueStrFormatS(path, "%s_%s.%s", assetName, s_data.m_languageSymbol, assetExtension);

		if (assetPathType == ueAssetPath_LocalizedOnly)
			return;

		// This actually accesses file system(s) - might be slow

		if (ioFile_Exists(path))
			return;
	}

	// Try non-localized asset name

	if (assetPathType != ueAssetPath_LocalizedOnly)
	{
		if (assetConfig)
			ueStrFormatS(path, "%s/%s.%s", assetConfig, assetName, assetExtension);
		else
			ueStrFormatS(path, "%s.%s", assetName, assetExtension);
	}
}
#ifndef UE_ASSETS_H
#define UE_ASSETS_H

/**
 *	@addtogroup ue
 *	@{
 */

//! Asset path lookup modes
enum ueAssetPath
{
	ueAssetPath_LocalizedFirst = 0,		//!< Default; checks localized file existance first; if file not found, tries non-localized
	ueAssetPath_LocalizedOnly,			//!< Localized only (for assets that must be localized)
	ueAssetPath_NonlocalizedOnly,		//!< Non-localized only (for assets that aren't non-localized)

	ueAssetPath_MAX
};

//! Starts up the assets utility; development mode enables searching for assets in source directory first
void		ueAssets_Startup(ueBool developmentMode = UE_FALSE);
//! Shuts down the assets utility
void		ueAssets_Shutdown();
//! Sets language by symbol for use by assets
void		ueAssets_SetLanguageSymbol(const char* symbol);
//! Determines full (localized and configuration specific) asset path
void		ueAssets_GetAssetPath(uePath& path, const char* assetName, const char* assetExtension, const char* assetConfig = NULL, ueAssetPath assetPathType = ueAssetPath_LocalizedFirst);

// @}

#endif // UE_ASSETS_H

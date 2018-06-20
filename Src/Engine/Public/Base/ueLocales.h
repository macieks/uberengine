#ifndef UE_LOCALES_H
#define UE_LOCALES_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ut
 *	@{
 */

//! Dynamic parameter to be inserted into localized text
struct ueLocaleParam
{
	const char* m_name;		//!< Parameter name
	const char* m_value;	//!< Parameter value; if set to NULL, m_valueInt gets used
	s32 m_valueInt;			//!< Parameter value as integer (only used if m_value is NULL)

	UE_INLINE ueLocaleParam() : m_name(NULL), m_value(NULL), m_valueInt(0) {}
	UE_INLINE ueLocaleParam(const char* name, const char* value) : m_name(name), m_value(value), m_valueInt(0) {}
	UE_INLINE ueLocaleParam(const char* name, s32 valueInt) : m_name(name), m_value(NULL), m_valueInt(valueInt) {}
};

//! Starts up locale manager
void		ueLocaleMgr_Startup(ueAllocator* allocator, u32 maxSets = 256);
//! Shuts down locale manager
void		ueLocaleMgr_Shutdown();

//! Sets root directory for localized text sets
void		ueLocaleMgr_SetRootDir(const char* rootDir);

//! Loads localization set
ueBool		ueLocaleMgr_LoadSet(const char* setName);
//! Unloads localization set
void		ueLocaleMgr_UnloadSet(const char* setName);

//! Returns (temporary i.e. valid until next call to ueLocaleMgr_Localize) pointer to localized and filled in UTF-8 text
const char*	ueLocaleMgr_Localize(const char* name, u32 numParams = 0, const ueLocaleParam* params = NULL);

// Helper macros

//! Localizes text
#define UE_LOC(name) ueLocaleMgr_Localize(name)
//! Localizes text with parameters
#define UE_LOC_P(name, params) ueLocaleMgr_Localize(name, UE_ARRAY_SIZE(params), params)
//! Localizes text with single parameter (either string or integer)
#define UE_LOC_S(name, paramName, paramValue) ueLocaleMgr_Localize(name, 1, &ueLocaleParam(paramName, paramValue))

// @}

#endif // UE_LOCALES_H
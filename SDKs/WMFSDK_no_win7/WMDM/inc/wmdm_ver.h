//+----------------------------------------------------------------------------
//
//  Microsoft Windows Media Technologies
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//
//  File:       wmdm_ver.h
//
//  Description: This file contains common definitions for verion definitions
//               for WMDM binaries.
//+----------------------------------------------------------------------------

#include <winver.h>

#ifdef _CORONA_BUILD

    #include <wmdm_build_ver.h>

    #define VER_WMDM_PRODUCTVERSION_STR	 "9.0.1." VER_WMDM_PRODUCTBUILD_STR
    #define VER_WMDM_PRODUCTVERSION	9,0,1,VER_WMDM_PRODUCTBUILD 

#else
    //Vista & Polaris Build
    #ifdef _WPD_PRE_LH_BUILD
        #include <mms_ver.h>
    #else
        #include "..\..\..\..\dmd\Crescent\core\mms_ver.h"

        #ifndef VER_PRODUCTMAJORVERSION
            #include "ntverp.h"
        #endif
    #endif
#endif //_CORONA_BUILD

#define VER_WMDM_PRODUCTNAME_STR        "Windows Media Device Manager\0"
#define VER_WMDM_COMPANYNAME_STR        "Microsoft Corporation\0"
#define VER_WMDM_LEGALCOPYRIGHT_YEARS   "1999-2001\0"
#define VER_WMDM_LEGALCOPYRIGHT_STR     "(C) Microsoft Corporation. All rights reserved.\0"
#define VER_WMDM_FILEOS                 VOS_NT_WINDOWS32

#ifdef EXPORT_CONTROLLED

#ifdef EXPORT
#define EXPORT_TAG  " (Export Version)\0"
#else
#define EXPORT_TAG  " (Domestic Use Only)\0"
#endif

#else           /* Not Export Controlled */

#define EXPORT_TAG

#endif

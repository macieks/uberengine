//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//

// This workspace contains two projects -
// 1. ProgHelp which implements the Progress Interface 
// 2. The Sample application WmdmApp. 
//
//  ProgHelp.dll needs to be registered first for the SampleApp to run.


// apppch.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__EFE21268_E11A_11D2_99F9_00C04F72D6CF__INCLUDED_)
#define AFX_STDAFX_H__EFE21268_E11A_11D2_99F9_00C04F72D6CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef STRICT
#define STRICT
#endif

#define INC_OLE2        // WIN32, get ole2 from windows.h


#include <windows.h>
#include <gdiplus.h>
#include <windowsx.h>
#include <shlobj.h>
#include <shellapi.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here

#include "util.h"
#include "appRC.h"

#include "mswmdm.h"
#include "itemdata.h"
#include "wmdevmgr.h"

#include "status.h"
#include "progress.h"
#include "devices.h"
#include "devfiles.h"
#include "wmdmapp.h"
#include "Properties.h"

#define STRSAFE_NO_DEPRECATE
#include "StrSafe.h"
#include <crtdbg.h>
//
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_STDAFX_H__EFE21268_E11A_11D2_99F9_00C04F72D6CF__INCLUDED_)


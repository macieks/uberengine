//
//  Microsoft Windows Media Technologies
//  Copyright (c) Microsoft Corporation. All rights reserved.
//

//
// This workspace contains two projects -
// 1. ProgHelp which implements the Progress Interface 
// 2. The Sample application WmdmApp. 
//
//  ProgHelp.dll needs to be registered first for the SampleApp to run.


#ifndef _UTIL_H
#define _UTIL_H

#include <wmsdk.h>
// Macros
//
#define MemAlloc( dw )        LocalAlloc( LPTR, dw )
#define MemFree( lpv )        { LocalFree( lpv ); lpv = NULL; }

#define ExitOnTrue( f )       if( f ) goto lExit;
#define ExitOnFalse( f )      if( !(f) ) goto lExit;
#define ExitOnNull( x )       if( (x) == NULL ) goto lExit;
#define ExitOnFail( hr )      if( FAILED(hr) ) goto lExit;

#define FailOnTrue( f, r )       if( f ) {hr = (r); goto lExit; }
#define FailOnFalse( f, r )      if( !(f) ) {hr = (r); goto lExit; }
#define FailOnNull( x, r )       if( (x) == NULL ) {hr = (r); goto lExit; }
#define FailOnFail( hr, r )      if( FAILED(hr) ) {hr = (r); goto lExit; }

#define SafeRelease( i )      { if( (i) ) i->Release(); i = NULL; }

// Flags for DropListToBuffer
//
typedef enum {
    LTB_NULL_TERM  = 0,
    LTB_SPACE_SEP  = 1,
} LIST_TYPE;

// Util functions
//
BOOL  CenterWindow( HWND hwnd, HWND hwndRef );
VOID  BringWndToTop( HWND hwnd );

DWORD GetTheFileSize( LPWSTR pszFile );

VOID  StripPath( LPWSTR szFullPath );
LPWSTR GetFileName( LPWSTR szFullPath );

WPARAM DoMsgLoop( BOOL fForever );
VOID   UiYield( void );

LPWSTR DropListToBuffer( HDROP hDrop, LIST_TYPE listType, UINT *uNumObjs );

VOID  SetRegistryParams( HINSTANCE hInst, HKEY hkeyRoot );
VOID  WriteRegDword(LPWSTR szPath, LPWSTR szKey, DWORD dwValue);
VOID  WriteRegDword_StrTbl(UINT uPathID,    UINT uKeyID, DWORD dwValue);
VOID  WriteRegStr(LPWSTR szPath, LPWSTR szKey, LPWSTR szValue);
VOID  WriteRegStr_StrTbl(UINT uPathID, UINT uKeyID, LPWSTR szValue);
DWORD GetRegDword(LPWSTR szPath, LPWSTR szKey, DWORD dwDefault, BOOL bStore);
DWORD GetRegDword_StrTbl(UINT uPathID, UINT uKeyID, DWORD dwDefault, BOOL bStore);

HANDLE WaitForMutex( LPWSTR pszMutexName, DWORD dwRetryTime, DWORD dwTimeout );

LPWSTR FormatBytesToSz( DWORD dwLowBytes, DWORD dwHighBytes, DWORD dwMultiplier, LPWSTR psz, size_t cbMax);
LPWSTR FormatBytesToKB_Sz( DWORD dwBytes, LPWSTR pszKB, size_t cbMax );
LPWSTR FormatKBToKB_Sz( DWORD dwKB, LPWSTR pszKB, size_t cbMax );
LPWSTR FormatKBToMB_Sz( DWORD dwKB, LPWSTR pszMB, size_t cbMax );
LPWSTR FormatSystemTimeToSz( SYSTEMTIME *pSysTime, LPWSTR pszDateTime, DWORD cchMax );

LPARAM ListView_GetLParam( HWND hwndListView, INT nItem );
LPARAM TreeView_GetLParam( HWND hwndTreeView, HTREEITEM hItem );
BOOL   TreeView_SetLParam( HWND hwndTreeView, HTREEITEM hItem, LPARAM lParam );

INT GetShellIconIndex( LPCWSTR pszItemName, LPWSTR szTypeBuffer, UINT cMaxChars );
HICON GetShellIcon( LPCWSTR pszItemName, BOOL bDirectory );

LPTSTR WStrToTStr(LPWSTR pTString, LPWSTR pWString, int iLimit);

#define hrOK		HRESULT(S_OK)
#define hrTrue		HRESULT(S_OK)
#define hrFalse		ResultFromScode(S_FALSE)
#define hrFail		ResultFromScode(E_FAIL)
#define hrNotImpl	ResultFromScode(E_NOTIMPL)
#define hrNoInterface	ResultFromScode(E_NOINTERFACE)
#define hrNoMem	ResultFromScode(E_OUTOFMEMORY)
#define hrAbort		ResultFromScode(E_ABORT)
#define hrInvalidArg	ResultFromScode(E_INVALIDARG)

#define	fFalse		0
#define fTrue		1

#define	CPRg(p)\
	do\
		{\
		if (!(p))\
			{\
			hr = hrNoMem;\
			goto Error;\
			}\
		}\
	while (fFalse)

#define	CHRg(hResult) CORg(hResult)

#define	CORg(hResult)\
	do\
		{\
		hr = (hResult);\
        if (FAILED(hr))\
            {\
            goto Error;\
            }\
		}\
	while (fFalse)

#define	CADORg(hResult)\
	do\
		{\
		hr = (hResult);\
        if (hr!=S_OK && hr!=S_FALSE)\
            {\
            hr = HRESULT_FROM_ADO_ERROR(hr);\
            DebugMessage(__FILE__, __LINE__, hr);\
            }\
		}\
	while (fFalse)

#define	CORgl(label, hResult)\
	do\
		{\
		hr = (hResult);\
        if (FAILED(hr))\
            {\
            goto label;\
            }\
		}\
	while (fFalse)



#define	CFRg(fResult)\
	{\
	if (!(fResult))\
		{\
		hr = hrFail;\
		goto Error;\
		}\
	}

#define	CFRgl(label, fResult)\
	{\
	if (!(fResult))\
		{\
		hr = hrFail;\
		goto label;\
		}\
	}

#define	CARg(p)\
	do\
		{\
		if (!(p))\
			{\
			hr = hrInvalidArg;\
			goto Error;\
			}\
		}\
	while (fFalse)

#define	CWRg(fResult)\
	{\
	if (!(fResult))\
		{\
        hr = GetLastError();\
	    if (!(hr & 0xFFFF0000)) hr = HRESULT_FROM_WIN32(hr);\
		goto Error;\
		}\
	}

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

#ifdef _DEBUG
#define _Assert(b) \
{ \
    if (!b) \
    { \
        DebugBreak(); \
    } \
}
#else
#define _Assert(b)
#endif

#endif  // _UTIL_H




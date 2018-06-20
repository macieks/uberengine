//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//

// This workspace contains two projects -
// 1. ProgHelp which implements the Progress Interface 
// 2. The Sample application WmdmApp. 
//
//  ProgHelp.dll needs to be registered first for the SampleApp to run.


//
//  Properties.h
//

#ifndef     _PROPETIES_H_
#define     _PROPETIES_H_

struct SType_String
{
    DWORD   dwType;
    WCHAR*   pszString;
};

INT_PTR CALLBACK DeviceProp_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK StoragePropSimple_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK StoragePropAlbum_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif      // _PROPETIES_H_

//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//

// This workspace contains two projects -
// 1. ProgHelp which implements the Progress Interface 
// 2. The Sample application WmdmApp. 
//
//  ProgHelp.dll needs to be registered first for the SampleApp to run.

///////////////////////////////////////////////////////////////////////////////
//
//  devices.h
//
///////////////////////////////////////////////////////////////////////////////

#ifndef     _DEVICES_H_
#define     _DEVICES_H_


class CDevices
{
    HWND    m_hwndDevices;
    HWND    m_hwndDevices_TV;

    HIMAGELIST m_himlSmall;

    BOOL InitImageList( void );

public:

    // Constructors/destructors
    //
    CDevices();
    ~CDevices();

    // Operations
    //
    BOOL Create( HWND hwndParent );
    VOID Destroy( void );

    HWND GetHwnd( void );
    HWND GetHwnd_TV( void );

    HTREEITEM GetSelectedItem( LPARAM *pLParam );
    BOOL SetSelectedItem( HTREEITEM hItem );
    INT  GetDeviceCount( VOID );
    CItemData *GetRootDevice( HTREEITEM hItem );
    BOOL HasSubFolders( HTREEITEM hItem );

    VOID UpdateStatusBar( void );
    BOOL UpdateSelection( HTREEITEM hItem, BOOL fDirty );
    
    BOOL AddItem( CItemData *pItemData );
    INT  AddChildren( HTREEITEM hItem, BOOL fDeviceItem );
    VOID RemoveAllItems( VOID );
    INT  RemoveChildren( HTREEITEM hItem );

    VOID RemoveDevices(LPCWSTR wszCanonicalNameToRemove);

    VOID OnSize( LPRECT prcMain );

    void SetDeviceIcon(HTREEITEM hItem, UINT nImageIndex);
    void SetMediaState (LPCWSTR wszCanonicalNameToRemove, bool bMediaPresent);
};


#endif      // _DEVICES_H_


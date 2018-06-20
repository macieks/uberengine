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

// Includes
//
#include <stdlib.h>

#include "appPCH.h"
#include "appRC.h"
#include "devfiles.h"

#include <atlbase.h>
#include <atlstr.h>

using namespace Gdiplus;

// Constants
//
#define _szWNDCLASS_MAIN            L"DrmXferAppWnd_Main"
#define _szMUTEX_APP                L"DrmXferApplication_Mutex"

#define MIN_MAINWND_W               400
#define SHOWBUFFER                  10

// Macros
//

// Global variables
//
HINSTANCE g_hInst                   = NULL;
HWND      g_hwndMain                = NULL;

CStatus   g_cStatus;
CDevices  g_cDevices;
CDevFiles g_cDevFiles;
CWMDM     g_cWmdm;
BOOL      g_bUseOperationInterface = FALSE;
BOOL      g_bUseInsert3            = TRUE;
BOOL      g_bUseEnumDevices2 = TRUE;

// Local variables
//
static HANDLE _hMutexDrmXfer        = NULL;

// Local functions
//
static VOID _CleanUp( void );
static VOID _InitSize( void );
static VOID _OnSize( HWND hwnd, WPARAM wParam, LPARAM lParam );
static VOID _OnMove( HWND hwnd, WPARAM wParam, LPARAM lParam );

static BOOL _InitWindow( void );
static BOOL _RegisterWindowClass( void );
static BOOL _UsePrevInstance( void );

// Local non-static functions
//
int WINAPI WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nCmdShow );
BOOL CALLBACK MainWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

// Command handlers
//
#define _nNUM_HANDLERS            10

typedef VOID (*HandleFunc) ( WPARAM wParam, LPARAM lParam );

static VOID _OnDeviceReset( WPARAM wParam, LPARAM lParam );
static VOID _OnDeviceClose( WPARAM wParam, LPARAM lParam );
static VOID _OnViewRefresh( WPARAM wParam, LPARAM lParam );
static VOID _OnFileDelete( WPARAM wParam, LPARAM lParam );
static VOID _OnCreatePlaylist( WPARAM wParam, LPARAM lParam );
static VOID _OnCreateAlbum( WPARAM wParam, LPARAM lParam );
static VOID _OnOptionsUseOperationInterface( WPARAM wParam, LPARAM lParam );
static VOID _OnOptionsUseEnumDevices2( WPARAM wParam, LPARAM lParam );
static VOID _OnOptionsUseInsert3( WPARAM wParam, LPARAM lParam );
static VOID _OnCreateMediaCast( WPARAM wParam, LPARAM lParam );

struct {
    UINT        uID;
    HandleFunc  pfnHandler;
} 
_handlers[ _nNUM_HANDLERS ] =
{
    { IDM_DEVICE_RESET,  _OnDeviceReset  },
    { IDM_CLOSE,         _OnDeviceClose  },
    { IDM_REFRESH,       _OnViewRefresh  },
    { IDM_DELETE,        _OnFileDelete   },
    { IDM_CREATEPLAYLIST, _OnCreatePlaylist },
    { IDM_CREATEALBUM, _OnCreateAlbum },
    { IDM_OPTIONS_USE_OPERATION_INTERFACE,        _OnOptionsUseOperationInterface  },
    { ID_OPTIONS_USEENUMDEVICES2,        _OnOptionsUseEnumDevices2  },
    { ID_OPTIONS_USEINSERT3,        _OnOptionsUseInsert3  },
    { IDM_CREATEMEDIACAST, _OnCreateMediaCast  }
};

struct AlbumInfo 
{
    AlbumInfo(WCHAR* szName, WCHAR* szImageFilePath, WMDM_FORMATCODE FormatCode = WMDM_FORMATCODE_IMAGE_JFIF) :
            m_FormatCode(FormatCode)
        { 
            m_szImageFilePath[0] = L'\0';
            m_szName[0] = L'\0';
            if (szName)
            {
                StringCbCopy(m_szName, sizeof(m_szName), szName); // ignore return code
            }
            if (szImageFilePath)
            {
                StringCbCopy(m_szImageFilePath, sizeof(m_szImageFilePath), szImageFilePath); // ignore return code
            }
        }
            
    WCHAR m_szName[MAX_PATH];
    WCHAR m_szImageFilePath[MAX_PATH];
    WMDM_FORMATCODE m_FormatCode;
};

struct FormatCodeConversionEntry
{
    WMDM_FORMATCODE FormatCode;
    WCHAR* wzDescription;
    CLSID clsidGdiplusCodec;
} FormatCodeConversionTable[] = 
{
//    {WMDM_FORMATCODE_IMAGE_UNDEFINED, ImageFormatUndefined},
    {WMDM_FORMATCODE_IMAGE_BMP, L"BMP", 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    {WMDM_FORMATCODE_IMAGE_JFIF, L"JPEG", 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    {WMDM_FORMATCODE_IMAGE_PNG, L"PNG",  0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    {WMDM_FORMATCODE_IMAGE_TIFF, L"TIFF", 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    {WMDM_FORMATCODE_IMAGE_GIF, L"GIF", 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    {WMDM_FORMATCODE_IMAGE_EXIF, L"EXIF", 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }
};

void SetGdiplusCodec(WMDM_FORMATCODE fmtcode, const CLSID & clsidCodec)
{
    for (UINT idxFC = 0; idxFC < sizeof(FormatCodeConversionTable) / sizeof(FormatCodeConversionTable[0]); idxFC++)
    {
        if (fmtcode == FormatCodeConversionTable[idxFC].FormatCode)
        {
            FormatCodeConversionTable[idxFC].clsidGdiplusCodec = clsidCodec;
            break;
        }        
    }            
}

void _InitFormatCodeConversionTable()
{
    Status st = Ok;
    UINT num = 0;
    UINT size = 0;
    ImageCodecInfo* pImgCodecInfo = NULL;
    
    st = GetImageEncodersSize(&num, &size);
    ExitOnFalse(Ok == st);
    
    pImgCodecInfo = (ImageCodecInfo*)malloc(size);
    ExitOnNull(pImgCodecInfo);
    
    st = GetImageEncoders(num, size, pImgCodecInfo);
    ExitOnFalse(Ok == st);

    // Set the CLSID for all loaded image encoders with a matching FormatID in the table.
    // If matching FormatID is not found in the table, the CLSID remains at the initialized value and conversion will fail. 
    // This is the desired behavior.

    for (UINT idxCodec = 0; idxCodec < num; idxCodec++)
    {
        if (IsEqualGUID(pImgCodecInfo[idxCodec].FormatID, ImageFormatBMP))
        {
            SetGdiplusCodec(WMDM_FORMATCODE_IMAGE_BMP, pImgCodecInfo[idxCodec].Clsid);
        }
        else if (IsEqualGUID(pImgCodecInfo[idxCodec].FormatID, ImageFormatJPEG))
        {
            SetGdiplusCodec(WMDM_FORMATCODE_IMAGE_JFIF, pImgCodecInfo[idxCodec].Clsid);
        }
        else if (IsEqualGUID(pImgCodecInfo[idxCodec].FormatID, ImageFormatPNG))
        {
            SetGdiplusCodec(WMDM_FORMATCODE_IMAGE_PNG, pImgCodecInfo[idxCodec].Clsid);
        }
        else if (IsEqualGUID(pImgCodecInfo[idxCodec].FormatID, ImageFormatTIFF))
        {
            SetGdiplusCodec(WMDM_FORMATCODE_IMAGE_TIFF, pImgCodecInfo[idxCodec].Clsid);
        }
        else if (IsEqualGUID(pImgCodecInfo[idxCodec].FormatID, ImageFormatGIF))
        {
            SetGdiplusCodec(WMDM_FORMATCODE_IMAGE_GIF, pImgCodecInfo[idxCodec].Clsid);
        }
        else if (IsEqualGUID(pImgCodecInfo[idxCodec].FormatID, ImageFormatEXIF))
        {
            SetGdiplusCodec(WMDM_FORMATCODE_IMAGE_EXIF, pImgCodecInfo[idxCodec].Clsid);
        }
    }

lExit:
    if (pImgCodecInfo)
    {
        free(pImgCodecInfo);
    }
}

// 
//
int WINAPI WinMain(__in HINSTANCE hInstance,
                     __in_opt HINSTANCE hPrevInstance,
                     __in_opt LPSTR     lpCmdLine,
                     __in int       nCmdShow)
{
#ifdef _DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    WPARAM wParam;
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    BOOL bGdiplusStarted = FALSE;
    
    g_hInst = hInstance;

    InitCommonControls();

    if( _UsePrevInstance() )
    {
        return 0;
    }

    // Initialize COM
    //
    ExitOnFail( CoInitialize(NULL) );

    // Initialize registry
    //
    SetRegistryParams( g_hInst, HKEY_LOCAL_MACHINE );

    // Initialize the local environment and windows
    //
    ExitOnFalse( _RegisterWindowClass() );
    ExitOnFalse( _InitWindow() );

    // Initialize the WMDM
    //
    ExitOnFail( g_cWmdm.Init());

    // Initialize GDI+
    Gdiplus::Status status = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    ExitOnFalse(Ok == status);
    
    bGdiplusStarted = TRUE; // will call GdiplusShutdown before exiting

    _InitFormatCodeConversionTable(); // returns void

    // Enter message pump until app is closed
    //
    wParam = DoMsgLoop( TRUE );
    
    g_cWmdm.Uninit();
    // Uninitialize COM
    //
    CoFreeUnusedLibraries();
    CoUninitialize();

    return (int)wParam;

lExit:
    // Only shutdown GdiPlus if GdiplusStartup succeeded
    //
    if (bGdiplusStarted)
    {
        GdiplusShutdown(gdiplusToken);
    }
    return 0;
}


LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   
    WORD wId         = LOWORD( (DWORD)wParam );
    WORD wNotifyCode = HIWORD( (DWORD)wParam );

    switch( uMsg )
    {
    case WM_CREATE:
        PostMessage( hWnd, WM_DRM_INIT, 0, 0 );
        break;

    case WM_DRM_INIT:
        _OnViewRefresh( 0, 0 );
        break;

    case WM_DRM_DELETEITEM:
        _OnFileDelete( 0, 0 );
        break;

    case WM_COMMAND:
        // Menu item selected
        if( BN_CLICKED == wNotifyCode || 0 == wNotifyCode || 1 == wNotifyCode )
        {
            INT i;

            for( i=0; i < _nNUM_HANDLERS; i++ )
            {
                if( wId == _handlers[i].uID )
                {
                    (*_handlers[i].pfnHandler)( wParam, lParam );
                    return 0;
                }
            }
        }
        break;

    case WM_ENDSESSION:
        if( (BOOL)wParam )
        {
            // shutting down
            _CleanUp();
        }
        break;

    case WM_SIZE:
        _OnSize( hWnd, wParam, lParam );
        return 0;

    case WM_SYSCOMMAND:
        if( SC_MAXIMIZE == wParam )
        {
            _OnSize( hWnd, wParam, lParam );
            return 0;
        }
        break;

    case WM_CLOSE:
        _CleanUp();
        PostQuitMessage( 0 );
        break;

    case WM_MOVE:
        _OnMove( hWnd, wParam, lParam );
        return 0;

    case WM_KEYDOWN:
        if( wParam == VK_F5 )
        {
            _OnViewRefresh( 0, 0 );
            return 0;
        }
        break;

    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam; 

            lpmmi->ptMinTrackSize.x = MIN_MAINWND_W;
        }
        return 0;

    case WM_INITMENU:
        // Enable/disable 'Delete' - command
        EnableMenuItem( (HMENU)wParam, IDM_DELETE, MF_BYCOMMAND | 
                (g_cDevFiles.OkToDelete() ? MF_ENABLED : MF_GRAYED) );
        break;

    default:
        break;
    }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}    


VOID _OnViewRefresh( WPARAM wParam, LPARAM lParam )
{
    HRESULT  hr;
    HCURSOR  hCursorPrev;

    // Show a wait cursor
    //
    hCursorPrev = SetCursor( LoadCursor(NULL, IDC_WAIT) );

    // Remove all current files
    //
    g_cDevFiles.RemoveAllItems();

    // Process messages to allow UI to refresh
    //
    UiYield();

    // Remove all devices
    //
    g_cDevices.RemoveAllItems();

    //get a new enumerator based on option EnumDevices/EnumDevices2
    IWMDMEnumDevice * pEnumDevice = NULL;
    if (g_bUseEnumDevices2)
    {
        IWMDeviceManager2 * pIWMDevMgr2 = NULL;
        hr = g_cWmdm.m_pWMDevMgr->QueryInterface (IID_IWMDeviceManager2, (void**) &pIWMDevMgr2);

        if (SUCCEEDED (hr))
        {
            hr = pIWMDevMgr2->EnumDevices2(&pEnumDevice);
            pIWMDevMgr2->Release();
        }
        else
        {
            //EnumDevices2 is not supported by the installed WMDM
            g_bUseEnumDevices2 = FALSE;
            // Uncheck and disable menu
            HMENU   hMainMenu;
            HMENU   hOptionsMenu;

            hMainMenu = GetMenu(g_hwndMain);
            hOptionsMenu = GetSubMenu( hMainMenu, 2 );

            CheckMenuItem( hOptionsMenu, ID_OPTIONS_USEENUMDEVICES2, 
                                MF_BYCOMMAND | MF_UNCHECKED);
            EnableMenuItem (hOptionsMenu, ID_OPTIONS_USEENUMDEVICES2, MF_BYCOMMAND | MF_GRAYED);
            //fall back on EnumDevices
            hr = g_cWmdm.m_pWMDevMgr->EnumDevices( &pEnumDevice );
        }
    }
    else
    {
        hr = g_cWmdm.m_pWMDevMgr->EnumDevices( &pEnumDevice );
    }
    
    ExitOnFail( hr );
    // Loop through all devices and add them to the list
    //
    while( TRUE )
    {
        IWMDMDevice *pWmdmDevice;
        CItemData   *pItemDevice;
        ULONG        ulFetched;

        hr = pEnumDevice->Next( 1, &pWmdmDevice, &ulFetched );
        if( hr != S_OK )
        {
            break;
        }
        if( ulFetched != 1 )
        {
            ExitOnFail( hr = E_UNEXPECTED );
        }

        pItemDevice = new CItemData;
        if( pItemDevice )
        {
            hr = pItemDevice->Init( pWmdmDevice );
            if( SUCCEEDED(hr) )
            {
                g_cDevices.AddItem( pItemDevice );
            }
            else
            {
                delete pItemDevice;
            }
        }

        pWmdmDevice->Release();
    }

    // Update the device portion of the status bar
    //
    g_cDevices.UpdateStatusBar();

    // Update the file portion of the status bar
    //
    g_cDevFiles.UpdateStatusBar();

    // Use the default selection
    //
    g_cDevices.UpdateSelection( NULL, FALSE );

    // Return cursor to previous state
    //
    SetCursor( hCursorPrev );

lExit:
    if (pEnumDevice)
    {
        pEnumDevice->Release();
        pEnumDevice = NULL;
    }

    return;
}

VOID _OnDeviceReset( WPARAM wParam, LPARAM lParam )
{
    CProgress  cProgress;
    CItemData *pItemDevice;
    HRESULT    hr;
    HTREEITEM  hItem;
        
    // Get the selected device to reset
    //
    hItem = g_cDevices.GetSelectedItem( (LPARAM *)&pItemDevice );
    ExitOnNull( hItem );
    ExitOnNull( pItemDevice );

    // You can only format devices, not individual folders
    //
    ExitOnFalse( pItemDevice->m_fIsDevice );

    // Create a progress dialog
    //
    ExitOnFalse( cProgress.Create(g_hwndMain) );

    // Set operation progress values
    //
    cProgress.SetOperation( L"Initializing Device..." );
    cProgress.SetDetails( pItemDevice->m_szName );
    cProgress.SetRange( 0, 100 );
    cProgress.SetCount( -1, -1 );
    cProgress.SetBytes( -1, -1 );
    cProgress.Show( TRUE );

    hr = pItemDevice->m_pStorageGlobals->Initialize( WMDM_MODE_BLOCK, NULL );

    cProgress.Show( FALSE );
    cProgress.Destroy();

     //Now refresh the dialog. Added by JDOTSON@microsoft.com 04/22/04
     _OnViewRefresh(0, 0);
     

lExit:

    // Refresh the display
    //
    g_cDevices.UpdateSelection( NULL, FALSE );
}


VOID _OnFileDelete( WPARAM wParam, LPARAM lParam )
{
    CProgress cProgress;
    HRESULT   hr;
    INT       i;
    INT      *pnSelItems = NULL;
    INT       nNumSel;

    // Get the number of selected items.
    // Exit if there are no items selected.
    //
    nNumSel = 0;

    // Call with NULL instead of pnSelItems for prefix
    g_cDevFiles.GetSelectedItems( NULL, &nNumSel );
    ExitOnTrue( 0 == nNumSel );

    // Allocate space to hold them the selected items
    //
    pnSelItems = new INT[ nNumSel ];
    ExitOnNull( pnSelItems );

    // Get the selected file(s) to delete
    //
    ExitOnTrue( -1 == g_cDevFiles.GetSelectedItems(pnSelItems, &nNumSel) );

    // Create a progress dialog
    //
    ExitOnFalse( cProgress.Create(g_hwndMain) );

    // Set operation progress values
    //
    cProgress.SetOperation( L"Deleting Files..." );
    cProgress.SetRange( 0, nNumSel );
    cProgress.SetCount( 0, nNumSel );
    cProgress.SetBytes( -1, -1 );

    for( i=nNumSel-1; i >= 0; i-- )
    {
        CItemData *pStorage;

        // Get the storage object for the current item to delete
        //
        pStorage = (CItemData *)ListView_GetLParam( g_cDevFiles.GetHwnd_LV(), pnSelItems[i] );

        if( NULL != pStorage )
        {
            IWMDMStorageControl *pStorageControl;
        
            // Set the name of the object and show the progress dialog
            //
            cProgress.SetDetails( pStorage->m_szName );
            cProgress.IncCount();
            cProgress.IncPos( 1 );
            cProgress.Show( TRUE );

            hr = pStorage->m_pStorage->QueryInterface(
                IID_IWMDMStorageControl,
                reinterpret_cast<void**>(&pStorageControl)
            );
            if( SUCCEEDED(hr) )
            {
                hr = pStorageControl->Delete( WMDM_MODE_BLOCK, NULL );

                if( SUCCEEDED(hr) )
                {
                    ListView_DeleteItem( g_cDevFiles.GetHwnd_LV(), pnSelItems[i] );
                }

                pStorageControl->Release();
            }
        }
    }

    cProgress.Show( FALSE );
    cProgress.Destroy();

lExit:

    if( pnSelItems )
    {
        delete [] pnSelItems;
    }

    // Refresh the device/devicefiles display
    //
    g_cDevices.UpdateSelection( NULL, FALSE );
}

VOID MsgBoxContainerUsage(LPCWSTR wszType)
{
    HRESULT hr = S_OK;
    size_t cchType = 0;
    hr = StringCchLength(wszType, MAX_PATH, &cchType);
    if (FAILED(hr))
    {
        return;
    }
    
    WCHAR szFmtMsg[] = L"First select one or more items from the file list (use the SHIFT or CTRL keys to select multiple files) then choose Create %s from the menu again.";
    //
    //compiler in PSDK environment doesn't like sizeof(WCHAR[])
    //const size_t cchMsg = sizeof(szFmtMsg) / sizeof(szFmtMsg[0]) + MAX_PATH;
    //
    const size_t cchMsg = 2 * MAX_PATH;
    WCHAR szMsg[cchMsg] = L"";
    hr = StringCbPrintf(szMsg, sizeof(szMsg), szFmtMsg, wszType);
    if (FAILED(hr))
    {
        return;
    }
    
    WCHAR szFmtTitle[] = L"Create %s Usage";
    //
    //compiler in PSDK environment doesn't like sizeof(WCHAR[])
    //const size_t cchTitle = sizeof(szFmtTitle) / sizeof(szFmtTitle[0]) + MAX_PATH;
    //
    const size_t cchTitle = 2 * MAX_PATH;
    WCHAR szTitle[cchTitle] = L"";
    hr = StringCbPrintf(szTitle, sizeof(szTitle), szFmtTitle, wszType);
    if (FAILED(hr))
    {
        return;
    }
    
    MessageBoxW(g_hwndMain, szMsg, szTitle, MB_OK);
}

INT_PTR CALLBACK PlaylistName_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   
    static WCHAR* szName = NULL;

    switch( uMsg )
    {
        case WM_INITDIALOG:
            szName = (WCHAR*)lParam;
            SetDlgItemTextW(hWnd, IDC_PLAYLIST_NAME, szName);
        break;

        case WM_COMMAND:
            if( GET_WM_COMMAND_ID(wParam, lParam) == IDOK ||
                GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL) 
            {
                if (szName)
                {
                    GetDlgItemTextW(hWnd, IDC_PLAYLIST_NAME, szName, MAX_PATH);
                }
                EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                return TRUE;
            }
        break;

        default:
            break;
    }
    
    return 0;
}    

INT_PTR DlgNamePlaylist(WCHAR* szName)
{
    return DialogBoxParam(g_hInst,
                MAKEINTRESOURCE(IDD_PLAYLIST_NAME),
                g_hwndMain,
                PlaylistName_DlgProc,
                (LPARAM)szName);
}

VOID MsgBoxDisplayError(DWORD dwErr)
{
    HRESULT hr = S_OK;
    WCHAR sz[MAX_PATH] = L"";
    if (SUCCEEDED(StringCbPrintf(sz, sizeof(sz), L"HRESULT = 0x%08x", dwErr)))
    {
        MessageBoxW(g_hwndMain, sz, L"Error", MB_OK);
    }
}

VOID MsgBoxImageFileError(LPCWSTR wszFilePath)
{
    HRESULT hr = S_OK;
    size_t cchFilePath = 0;
    hr = StringCchLength(wszFilePath, MAX_PATH, &cchFilePath);
    if (SUCCEEDED(hr))
    {
        
        WCHAR szFmt[] = L"Unable to open image file at %s";
        //
        //compiler in PSDK environment doesn't like sizeof(WCHAR[])
        //const size_t cchMsg = sizeof(szFmt) / sizeof(szFmt[0]) + MAX_PATH;
        //
        const size_t cchMsg = 2 * MAX_PATH;
        
        WCHAR szMsg[cchMsg] = L"";
        if (SUCCEEDED(StringCbPrintf(szMsg, sizeof(szMsg), szFmt, wszFilePath)))
        {
            MessageBoxW(g_hwndMain, szMsg, L"Error", MB_OK);
        }
    }
}

VOID _OnCreatePlaylist( WPARAM wParam, LPARAM lParam )
{
    CProgress cProgress;
    HRESULT   hr = S_OK;
    INT       i;
    INT      *pnSelItems = NULL;
    INT       nNumSel = 0;
    INT       idxStg = 0;
    WCHAR      wszName[MAX_PATH] = L"DefaultPlaylistName";
    BOOL fProgressVisible = FALSE;
    IWMDMMetaData* pIWMDMMetaData = NULL;
    IWMDMStorage3* pStg3 = NULL;
    IWMDMStorageControl3* pStorageControl = NULL;
    IWMDMStorage4* pPlaylist = NULL;
    IWMDMStorage** ppIStorage = NULL;

    // Get the number of selected items.
    // Explain usage and exit if there are no items selected.
    //
    // Call with NULL instead of pnSelItems for prefix
    g_cDevFiles.GetSelectedItems( NULL, &nNumSel );
    if( 0 == nNumSel )
    {
        MsgBoxContainerUsage(L"Playlist");
        goto lExit;
    }

    // Get, from the user, a name for the playlist
    INT_PTR nRet = DlgNamePlaylist(wszName);
    ExitOnTrue(nRet == IDCANCEL);
    
    // Allocate space to hold them the selected items
    //
    pnSelItems = new INT[ nNumSel ];
    ExitOnNull( pnSelItems );

    // Get the selected file(s) to delete
    //
    ExitOnTrue( -1 == g_cDevFiles.GetSelectedItems(pnSelItems, &nNumSel) );

    // Create a progress dialog
    //
    ExitOnFalse( cProgress.Create(g_hwndMain) );
    fProgressVisible = TRUE;

    // Set operation progress values
    //
    cProgress.SetOperation( L"Creating playlist..." );
    cProgress.SetRange( 0, nNumSel );
    cProgress.SetCount( 0, nNumSel );
    cProgress.SetBytes( -1, -1 );

    ppIStorage = new IWMDMStorage*[nNumSel];
    ExitOnNull( ppIStorage );
        
    for( i=nNumSel-1; i >= 0; i-- )
    {
        CItemData *pStorage;

        // Get the storage object for the item being added to the playlist
        //
        pStorage = (CItemData *)ListView_GetLParam( g_cDevFiles.GetHwnd_LV(), pnSelItems[i] );

        if( NULL != pStorage && NULL != pStorage->m_pStorage)
        {
            // Set the name of the object and show the progress dialog
            //
            cProgress.SetDetails( pStorage->m_szName );
            ppIStorage[idxStg] = pStorage->m_pStorage;
            ppIStorage[idxStg]->AddRef();
            idxStg++;
        }
            
        cProgress.IncCount();
        cProgress.IncPos( 1 );
        cProgress.Show( TRUE );
    }

    if( idxStg )
    {
        HTREEITEM hItem = NULL;
        CItemData* pItemData = NULL;
        IWMDMStorage* pInStorage  = NULL;
        IWMDMStorage* pNewStorage = NULL;
        
        // Get the selected device/storage
        //
        hItem = g_cDevices.GetSelectedItem( NULL );
        ExitOnNull( hItem );

        // Get the itemdata class associated with the hItem and 
        // retrieve the IWMDMStorage for it
        //
        pItemData = (CItemData *) TreeView_GetLParam( g_cDevices.GetHwnd_TV(), hItem );
        ExitOnNull( pItemData );

        pInStorage = ( pItemData->m_fIsDevice ? pItemData->m_pRootStorage : pItemData->m_pStorage );
        ExitOnNull( pInStorage );

        // Acquire the storage control interface
        //
        hr = pInStorage->QueryInterface(
            IID_IWMDMStorageControl3,
            reinterpret_cast<void**>(&pStorageControl)
        );
        ExitOnFail(hr);

        // Add an new album using abstract album type in the metadata for the new storage object.
        hr = pInStorage->QueryInterface(IID_IWMDMStorage3, (void **)&pStg3);
        ExitOnFail(hr);
        
        hr = pStg3->CreateEmptyMetadataObject(&pIWMDMMetaData);
        ExitOnFail(hr);

        // 
        // Though not implemented here, the officially recommended way to set the format code for a
        //  playlist object is the following:
        //  
        //  1. Acquire a IWMDMDevice3 pointer to the target device.
        //
        //  2. Call IWMDMDevice3::GetProperty(g_wszWMDMFormatsSupported, ...) to obtain the format codes
        //      supported by the device. 
        //
        //  3. If no playlist formats are supported, disallow sending playlists to the device.
        // 
        //  4. Choose the device-supported format code that matches most closely the intended album object 
        //      type (e.g. WMDM_FORMATCODE_WPLPLAYLIST for a playlist originating from a .WPL file). Fall
        //      back to the generic WMDM_FORMATCODE_ABSTRACTAUDIOVIDEOPLAYLIST only if necessary.
        // 
        //  5. Use that format code instead of the following hard-coded value.
        // 

        DWORD dw = WMDM_FORMATCODE_ABSTRACTAUDIOVIDEOPLAYLIST;
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_DWORD, g_wszWMDMFormatCode, (BYTE *)&dw, sizeof(dw));
        ExitOnFail(hr);

        hr = pStorageControl->Insert3(WMDM_MODE_BLOCK | WMDM_CONTENT_FILE,
                                                0,
                                                NULL,
                                                wszName,
                                                NULL,
                                                NULL,
                                                pIWMDMMetaData,
                                                NULL,
                                                &pNewStorage);
        ExitOnFail(hr);
        SafeRelease(pIWMDMMetaData);

        hr = pNewStorage->QueryInterface( IID_IWMDMStorage4, reinterpret_cast<void**>(&pPlaylist));
        pNewStorage->Release();
        ExitOnFail(hr);
               
        hr = pPlaylist->SetReferences(idxStg, ppIStorage);
        ExitOnFail(hr);
                    
        CItemData *pItem = new CItemData;
        if( pItem )
        {
            hr = pItem->Init( pPlaylist );
            if( SUCCEEDED(hr) )
            {
                g_cDevFiles.AddItem( pItem );
            }
            else
            {
                delete pItem;
            }
        }
    }

    g_cDevices.UpdateSelection( NULL, FALSE );

lExit:
    if (fProgressVisible)
    {
        cProgress.Show( FALSE );
        cProgress.Destroy();
    }

    SafeRelease(pPlaylist);
    SafeRelease(pStg3);
    SafeRelease(pIWMDMMetaData);
    SafeRelease(pStorageControl);
   
    if( pnSelItems )
    {
        delete[] pnSelItems;
    }

    for (i = 0; i < idxStg; i++)
    {
        if (ppIStorage[i] != NULL)
        {
            ppIStorage[i]->Release();
            ppIStorage[i] = NULL;
        }
    }

    if (ppIStorage)
    {
        delete[] ppIStorage;
    }
   
    if (FAILED(hr))
    {
        MsgBoxDisplayError(hr);
    }
}

HRESULT WMDMFormatCodeToGdiplusEncoder(CLSID& clsidGdiplusEncoder, const WMDM_FORMATCODE fmt)
{
    for (int idx = 0; idx < sizeof(FormatCodeConversionTable) / sizeof(FormatCodeConversionTable[0]); idx++)
    {
        if (FormatCodeConversionTable[idx].FormatCode == fmt)
        {
            clsidGdiplusEncoder = FormatCodeConversionTable[idx].clsidGdiplusCodec;
            return S_OK;
        }
    }
    
    return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
}

HRESULT SetAlbumMetadataFromFile(IWMDMMetaData* pIWMDMMetaData, const WCHAR* wszImageFilePath, const WMDM_FORMATCODE fmtForDevice)
{
    HRESULT hr = S_OK;
    Status st = Ok;
    GUID guidGdiplusFormat =  { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
    CLSID clsidGdiplusEncoder =  { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
    CComPtr<IStream> spIStream;
    Image* pImage = NULL;
    HGLOBAL hMem = NULL;
    BYTE* pb = NULL;

    pImage = Image::FromFile(wszImageFilePath);
    FailOnNull(pImage, E_OUTOFMEMORY);
       
    st = pImage->GetLastStatus();
    FailOnFalse(Ok == st, HRESULT_FROM_WIN32(ERROR_INVALID_DATA));

    // keep this around for debug purposes
    st = pImage->GetRawFormat(&guidGdiplusFormat);
    FailOnFalse(Ok == st, E_UNEXPECTED);

    //
    // Set Image data as converted to the desired fmtForDevice
    //
    
    hr = WMDMFormatCodeToGdiplusEncoder(clsidGdiplusEncoder, fmtForDevice);
    ExitOnFail(hr);
    
    hMem = GlobalAlloc(GHND, 8192); // guessing that most images will be less than 8K
    FailOnNull(hMem, E_OUTOFMEMORY);
    hr = CreateStreamOnHGlobal(hMem, TRUE, &spIStream);
    ExitOnFail(hr);

    st = pImage->Save(spIStream, &clsidGdiplusEncoder, NULL);
    FailOnFalse(Ok == st, E_UNEXPECTED);

    pb = (BYTE*)GlobalLock(hMem);
    FailOnNull(pb, E_OUTOFMEMORY);

    size_t cb = GlobalSize(hMem);
    hr = pIWMDMMetaData->AddItem(WMDM_TYPE_BINARY, g_wszWMDMAlbumCoverData, pb, cb);
    ExitOnFail(hr);

    DWORD dw = fmtForDevice;
    hr = pIWMDMMetaData->AddItem(WMDM_TYPE_DWORD, g_wszWMDMAlbumCoverFormat, (BYTE *)&dw, sizeof(dw));
    ExitOnFail(hr);

    dw = pImage->GetWidth();        
    hr = pIWMDMMetaData->AddItem(WMDM_TYPE_DWORD, g_wszWMDMAlbumCoverWidth, (BYTE *)&dw, sizeof(dw));
    ExitOnFail(hr);

    dw = pImage->GetHeight();
    hr = pIWMDMMetaData->AddItem(WMDM_TYPE_DWORD, g_wszWMDMAlbumCoverHeight, (BYTE *)&dw, sizeof(dw));
    ExitOnFail(hr);

#if 0  // old file I/O method 
    //BYTE bData[] = "123456789ABCDEF";
    if (hFileImage != INVALID_HANDLE_VALUE)
    {
        DWORD cbRead = 0;
        DWORD cbBuffer = 0;

        ULONGLONG qwFileSizeSource = (ULONGLONG)GetFileSize(hFileImage, NULL);
        if (qwFileSizeSource & (0xffffffff00000000))
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            goto lExit;
        }
        cbBuffer = static_cast<size_t>(qwFileSizeSource & (0x00000000ffffffff));
        pData = new BYTE[cbBuffer];
        BOOL fResult = ReadFile(hFileImage, pData, cbBuffer, &cbRead, NULL);
        if (!fResult || cbRead != cbBuffer)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto lExit;
        }
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_BINARY, g_wszWMDMAlbumCoverData, pData, cbRead);
        ExitOnFail(hr);

    }
#endif    

lExit:
    if (hMem != NULL)
    {
        if (pb != NULL)
        {
            GlobalUnlock(pb);
        }
        GlobalFree(hMem);
    }
    return hr;
}

HRESULT BrowseImageFile(HWND hWnd, WCHAR* szPath, size_t cchPath)
{
    HRESULT hr = S_OK;
  
    HANDLE hFile = INVALID_HANDLE_VALUE;
    static WCHAR file[MAX_PATH] = L"";
    static WCHAR szFilepath[MAX_PATH] = L"";
    static WCHAR fileTitle[MAX_PATH] = L"";
    static WCHAR filter[] = 
        TEXT("JPEG (*.JPG;*.JPEG;*.JPE;*.JFIF)\0*.JPG;*.JPEG;*.JPE;*.JFIF\0")
        TEXT("Bitmap files (*.BMP)\0*.BMP\0")
        TEXT("GIF (*.GIF)\0*.GIF\0")
        TEXT("TIFF (*.TIFF;*.TIF)\0*.TIFF;*.TIF\0")
        TEXT("PNG (*.PNG)\0*.PNG\0")
        TEXT("All Files (*.*)\0*.*\0")
        TEXT("\0\0");
    OPENFILENAME ofn = {0};

    FailOnNull(szPath, E_INVALIDARG);
        
    ofn.lStructSize       = sizeof(ofn);
    ofn.hwndOwner         = hWnd;
    ofn.hInstance         = g_hInst;
    ofn.lpstrFilter       = filter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 1;  // JPEG by default 
    ofn.lpstrFile         = szPath;
    ofn.nMaxFile          = cchPath;
    ofn.lpstrFileTitle    = fileTitle;
    ofn.nMaxFileTitle     = sizeof(fileTitle);
    ofn.lpstrInitialDir   = NULL;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt       = TEXT("*.jpg");
    ofn.lCustData         = 0;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (!GetOpenFileName(&ofn))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

lExit:    
    return hr;
}


INT_PTR CALLBACK EnterAlbumInfo_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   
    static AlbumInfo* pInf = NULL;
    
    switch( uMsg )
    {
        case WM_INITDIALOG:
            pInf = (AlbumInfo*)(lParam);
            if (pInf)
            {
                SetDlgItemTextW(hWnd, IDC_ALBUM_NAME, pInf->m_szName);
                SetDlgItemTextW(hWnd, IDC_ALBUMIMAGE_FILEPATH, pInf->m_szImageFilePath);
                HWND hwndFormats = GetDlgItem(hWnd, IDC_ALBUM_FORMATCODE);
                if (NULL != hwndFormats)
                {
                    for (UINT idx = 0; idx < sizeof(FormatCodeConversionTable) / sizeof(FormatCodeConversionTable[0]); idx++)
                    {
                        DWORD dwItem = ComboBox_AddString(hwndFormats, FormatCodeConversionTable[idx].wzDescription);
                        ComboBox_SetItemData(hwndFormats, dwItem, FormatCodeConversionTable[idx].FormatCode);
                        if (pInf->m_FormatCode == FormatCodeConversionTable[idx].FormatCode)
                        {
                            ComboBox_SetCurSel(hwndFormats, dwItem);
                        }
                    }
                }
            }
        break;

        case WM_COMMAND:
            if (pInf)
            {
                if( GET_WM_COMMAND_ID(wParam, lParam) == IDOK ||
                    GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL) 
                {
                    GetDlgItemText(hWnd, IDC_ALBUM_NAME, pInf->m_szName, sizeof(pInf->m_szName) / sizeof(pInf->m_szName[0]));
                    GetDlgItemText(hWnd, IDC_ALBUMIMAGE_FILEPATH, pInf->m_szImageFilePath, sizeof(pInf->m_szImageFilePath) / sizeof(pInf->m_szImageFilePath[0]));
                    HWND hwndFormats = GetDlgItem(hWnd, IDC_ALBUM_FORMATCODE);
                    if (NULL != hwndFormats)
                    {
                        int nItem = ComboBox_GetCurSel(hwndFormats);
                        if (CB_ERR != nItem)
                        {
                            pInf->m_FormatCode = (WMDM_FORMATCODE)ComboBox_GetItemData(hwndFormats, nItem);
                        }
                    }
                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    return TRUE;
                }
                else if (GET_WM_COMMAND_ID(wParam, lParam) == IDC_BROWSE)
                {
                    WCHAR szPath[MAX_PATH] = L"";
                    if (SUCCEEDED(StringCchCopy(szPath, sizeof(szPath) / sizeof(szPath[0]), pInf->m_szImageFilePath)))
                    {
                        if (SUCCEEDED(BrowseImageFile(hWnd, szPath, sizeof(szPath) / sizeof(szPath[0]))))
                        {
                            // Ignore return. If it fails, nothing to be done anyway.
                            StringCchCopy(pInf->m_szImageFilePath, sizeof(pInf->m_szImageFilePath) / sizeof(pInf->m_szImageFilePath[0]), szPath);
                            SetDlgItemText(hWnd, IDC_ALBUMIMAGE_FILEPATH, pInf->m_szImageFilePath);
                        }
                    }
                }
            }
        break;

        default:
            break;
    }
    
    return 0;
}    


INT_PTR DlgGetAlbumInfo(AlbumInfo& album)
{
    return DialogBoxParam(g_hInst,
                MAKEINTRESOURCE(IDD_ENTER_ALBUMINFO),
                g_hwndMain,
                EnterAlbumInfo_DlgProc,
                (LPARAM)&album);
}

VOID _OnCreateAlbum( WPARAM wParam, LPARAM lParam )
{
    CProgress cProgress;
    HRESULT   hr = S_OK;
    INT       i;
    INT      *pnSelItems = NULL;
    INT       nNumSel = 0;
    INT       idxStg = 0;
    AlbumInfo album(L"DefaultAlbumName", L"", WMDM_FORMATCODE_IMAGE_JFIF);
    HANDLE  hFileImage = INVALID_HANDLE_VALUE;
    BOOL fProgressVisible = FALSE;
    BYTE* pData = NULL;
    IWMDMMetaData* pIWMDMMetaData = NULL;
    IWMDMStorage3* pStg3 = NULL;
    IWMDMStorageControl3* pStorageControl = NULL;
    IWMDMStorage4* pPlaylist = NULL;
    IWMDMStorage** ppIStorage = NULL;
    Image* pImage = NULL;

    // Get the number of selected items.
    // Explain usage and exit if there are no items selected.
    //
    g_cDevFiles.GetSelectedItems( pnSelItems, &nNumSel );
    if( 0 == nNumSel )
    {
        MsgBoxContainerUsage(L"Album");
        goto lExit;
    }

    INT_PTR nRet = IDOK;
    while (IDOK == nRet)
    {
        // Get album properties from the user
        nRet = DlgGetAlbumInfo(album);
        if (nRet == IDCANCEL)
        {
            goto lExit;
        }

        hFileImage = CreateFileW(album.m_szImageFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFileImage == INVALID_HANDLE_VALUE)
        {
            MsgBoxImageFileError(album.m_szImageFilePath);
            continue;
        }
        CloseHandle(hFileImage);

        break;
    }
    
    // Allocate space to hold them the selected items
    //
    pnSelItems = new INT[ nNumSel ];
    ExitOnNull( pnSelItems );

    // Get the selected file(s) to delete
    //
    ExitOnTrue( -1 == g_cDevFiles.GetSelectedItems(pnSelItems, &nNumSel) );

    // Create a progress dialog
    //
    ExitOnFalse( cProgress.Create(g_hwndMain) );
    fProgressVisible = TRUE;

    // Set operation progress values
    //
    cProgress.SetOperation( L"Creating album..." );
    cProgress.SetRange( 0, nNumSel );
    cProgress.SetCount( 0, nNumSel );
    cProgress.SetBytes( -1, -1 );

    ppIStorage = new IWMDMStorage*[nNumSel];
    ExitOnNull( ppIStorage );
        
    for( i=nNumSel-1; i >= 0; i-- )
    {
        CItemData *pStorage;

        // Get the storage object for the item being added to the playlist
        //
        pStorage = (CItemData *)ListView_GetLParam( g_cDevFiles.GetHwnd_LV(), pnSelItems[i] );

        if( NULL != pStorage && NULL != pStorage->m_pStorage)
        {
            // Set the name of the object and show the progress dialog
            //
            cProgress.SetDetails( pStorage->m_szName );
            ppIStorage[idxStg] = pStorage->m_pStorage;
            ppIStorage[idxStg]->AddRef();
            idxStg++;
        }
            
        cProgress.IncCount();
        cProgress.IncPos( 1 );
        cProgress.Show( TRUE );
    }

    if( idxStg )
    {
        HTREEITEM hItem = NULL;
        CItemData* pItemData = NULL;
        IWMDMStorage* pInStorage  = NULL;
        IWMDMStorage* pNewStorage = NULL;
        
        // Get the selected device/storage
        //
        hItem = g_cDevices.GetSelectedItem( NULL );
        ExitOnNull( hItem );

        // Get the itemdata class associated with the hItem and 
        // retrieve the IWMDMStorage for it
        //
        pItemData = (CItemData *) TreeView_GetLParam( g_cDevices.GetHwnd_TV(), hItem );
        ExitOnNull( pItemData );

        pInStorage = ( pItemData->m_fIsDevice ? pItemData->m_pRootStorage : pItemData->m_pStorage );
        ExitOnNull( pInStorage );

        // Acquire the storage control interface
        //
        hr = pInStorage->QueryInterface(
            IID_IWMDMStorageControl3,
            reinterpret_cast<void**>(&pStorageControl)
        );

        // Add an new album using abstract album type in the metadata for the new storage object.
        hr = pInStorage->QueryInterface(IID_IWMDMStorage3, (void **)&pStg3);
        ExitOnFail(hr);
        
        hr = pStg3->CreateEmptyMetadataObject(&pIWMDMMetaData);
        ExitOnFail(hr);

        // 
        // Though not implemented here, the officially recommended way to set the format code for an 
        //  album object is the following:
        //  
        //  1. Acquire a IWMDMDevice3 pointer to the target device.
        //
        //  2. Call IWMDMDevice3::GetProperty(g_wszWMDMFormatsSupported, ...) to obtain the format codes
        //      supported by the device. 
        //
        //  3. If no album formats are supported, disallow sending albums to the device.
        // 
        //  4. Choose the device-supported format code that matches most closely the intended album object 
        //      type (e.g. WMDM_FORMATCODE_ABSTRACTIMAGEALBUM for an image album).
        // 
        //  5. Use that format code instead of the following hard-coded value. 
        //
        
        DWORD dw = WMDM_FORMATCODE_ABSTRACTAUDIOALBUM;
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_DWORD, g_wszWMDMFormatCode, (BYTE *)&dw, sizeof(dw));
        ExitOnFail(hr);

        hr = pStorageControl->Insert3(WMDM_MODE_BLOCK | WMDM_CONTENT_FILE,
                                                0,
                                                NULL,
                                                album.m_szName,
                                                NULL,
                                                NULL,
                                                pIWMDMMetaData,
                                                NULL,
                                                &pNewStorage);
        ExitOnFail(hr);
        SafeRelease(pIWMDMMetaData);

        hr = pNewStorage->QueryInterface( IID_IWMDMStorage4, reinterpret_cast<void**>(&pPlaylist));
        pNewStorage->Release();
        ExitOnFail(hr);
               
        hr = pPlaylist->SetReferences(idxStg, ppIStorage);
        ExitOnFail(hr);
        
        hr = pStg3->CreateEmptyMetadataObject(&pIWMDMMetaData);
        ExitOnFail(hr);

        hr = SetAlbumMetadataFromFile(pIWMDMMetaData, album.m_szImageFilePath, album.m_FormatCode);
        ExitOnFail(hr);

        hr = pPlaylist->SetMetadata(pIWMDMMetaData);
        ExitOnFail(hr);
       
        CItemData *pItem = new CItemData;
        if( pItem )
        {
            hr = pItem->Init( pPlaylist );
            if( SUCCEEDED(hr) )
            {
                g_cDevFiles.AddItem( pItem );
            }
            else
            {
                delete pItem;
            }
        }
    }

    g_cDevices.UpdateSelection( NULL, FALSE );

lExit:
    if (fProgressVisible)
    {
        cProgress.Show( FALSE );
        cProgress.Destroy();
    }

    SafeRelease(pPlaylist);
    SafeRelease(pStg3);
    SafeRelease(pIWMDMMetaData);
    SafeRelease(pStorageControl);
   
    if( pnSelItems )
    {
        delete[] pnSelItems;
    }

    if (pData)
    {
        delete[] pData;
    }

    for (i = 0; i < idxStg; i++)
    {
        if (ppIStorage[i] != NULL)
        {
            ppIStorage[i]->Release();
            ppIStorage[i] = NULL;
        }
    }

    if (ppIStorage)
    {
        delete[] ppIStorage;
    }

    if (pImage)
    {
        delete pImage;
    }
   
    if (FAILED(hr))
    {
        MsgBoxDisplayError(hr);
    }
}

VOID _OnDeviceClose( WPARAM wParam, LPARAM lParam )
{
    PostMessage( g_hwndMain, WM_CLOSE, (WPARAM)0, (LPARAM)0 );
}

// 
VOID _OnOptionsUseOperationInterface( WPARAM wParam, LPARAM lParam )
{
    HMENU   hMainMenu;
    HMENU   hOptionsMenu;

    // Remember new state
    g_bUseOperationInterface = !g_bUseOperationInterface;

    // Check uncheck menu
    hMainMenu = GetMenu(g_hwndMain);
    hOptionsMenu = GetSubMenu( hMainMenu, 2 );

    CheckMenuItem( hOptionsMenu, IDM_OPTIONS_USE_OPERATION_INTERFACE, 
                        MF_BYCOMMAND |
                        (g_bUseOperationInterface ? MF_CHECKED : MF_UNCHECKED));
}

VOID _OnOptionsUseEnumDevices2( WPARAM wParam, LPARAM lParam )
{
    HMENU   hMainMenu;
    HMENU   hOptionsMenu;

    // Remember new state
    g_bUseEnumDevices2 = !g_bUseEnumDevices2;

    // Check uncheck menu
    hMainMenu = GetMenu(g_hwndMain);
    hOptionsMenu = GetSubMenu( hMainMenu, 2 );

    CheckMenuItem( hOptionsMenu, ID_OPTIONS_USEENUMDEVICES2, 
                        MF_BYCOMMAND |
                        (g_bUseEnumDevices2 ? MF_CHECKED : MF_UNCHECKED));
}



VOID _OnOptionsUseInsert3( WPARAM wParam, LPARAM lParam )
{
    HMENU   hMainMenu;
    HMENU   hOptionsMenu;

    // Remember new state
    g_bUseInsert3 = !g_bUseInsert3;

    // Check uncheck menu
    hMainMenu = GetMenu(g_hwndMain);
    hOptionsMenu = GetSubMenu( hMainMenu, 2 );

    CheckMenuItem( hOptionsMenu, ID_OPTIONS_USEINSERT3, 
                        MF_BYCOMMAND |
                        (g_bUseInsert3 ? MF_CHECKED : MF_UNCHECKED));
}

BOOL _InitWindow( void )
{
    BOOL fRet = FALSE;
    WCHAR szApp[MAX_PATH];

    LoadString( g_hInst, IDS_APP_TITLE, szApp, sizeof(szApp) / sizeof(szApp[0]) );

    g_hwndMain = CreateWindowEx(
        0L,
        _szWNDCLASS_MAIN,
        szApp,
        WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | DS_3DLOOK | WS_CLIPCHILDREN,
        0, 0, 0, 0,
        NULL, NULL, g_hInst, NULL
    );
    ExitOnNull( g_hwndMain );

    ExitOnFalse( g_cDevices.Create(g_hwndMain) );

    ExitOnFalse( g_cDevFiles.Create(g_hwndMain) );

    ExitOnFalse( g_cStatus.Create(g_hwndMain) );

    _InitSize();

    // Show the window
    //
    ShowWindow( g_hwndMain, SW_SHOW );

    fRet = TRUE;

lExit:

    return fRet;
}


BOOL _RegisterWindowClass (void)
{
    WNDCLASS  wc;

    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = WndProc_Main;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = DLGWINDOWEXTRA;
    wc.hInstance      = g_hInst;
    wc.hIcon          = LoadIcon( g_hInst, MAKEINTRESOURCE(IDI_ICON) );
    wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName   = MAKEINTRESOURCE( IDR_MENU );
    wc.lpszClassName  = _szWNDCLASS_MAIN;

    return RegisterClass( &wc );
}


VOID _CleanUp( void )
{
    if( _hMutexDrmXfer )
    {
        ReleaseMutex( _hMutexDrmXfer );
        CloseHandle( _hMutexDrmXfer );
    }

    g_cDevices.Destroy();

    g_cDevFiles.Destroy();
}


BOOL _UsePrevInstance( void )
{
    HWND  hwnd;
    DWORD dwErr;

    // Look for the mutex created by another instance of this app
    //
    _hMutexDrmXfer = CreateMutex( NULL, TRUE, _szMUTEX_APP );

    dwErr = GetLastError();

    if( !_hMutexDrmXfer )
    {
        // The function failed... don't use this instance
        //
        return TRUE;
    }

    // If mutex didn't exist, don't use a previous instance
    //
    if( dwErr != ERROR_ALREADY_EXISTS )
    {
        return FALSE;
    }

    hwnd = FindWindow( _szWNDCLASS_MAIN, NULL );

    if( !hwnd )
    {
        // Mutex exists, but the window doesn't?
        //
        ReleaseMutex( _hMutexDrmXfer );
        CloseHandle( _hMutexDrmXfer );

        return TRUE;
    }

    // Show main window that already exists
    //
    BringWndToTop( hwnd );

    return TRUE;
}


INT _GetRegSize( UINT uStrID_RegPath, UINT uStrID_DefVal )
{
    DWORD dwRet;

    dwRet = GetRegDword_StrTbl(
        IDS_REG_PATH_BASE,
        uStrID_RegPath,
        (DWORD)-1,
        FALSE
    );

    if( (DWORD)-1 == dwRet && -1 != uStrID_DefVal )
    {
        WCHAR szDef[32];

        LoadString( g_hInst, uStrID_DefVal, szDef, sizeof(szDef) / sizeof(szDef[0]) );
        dwRet = (DWORD)_wtoi( szDef );
    }

    return (INT) dwRet;
}


VOID _InitSize( void )
{
    INT nX, nY, nW, nH;

    //
    // Get the window position values from the registry
    //
    nX = _GetRegSize( IDS_REG_KEY_XPOS,   (UINT)-1 );
    nY = _GetRegSize( IDS_REG_KEY_YPOS,   (UINT)-1 );
    nW = _GetRegSize( IDS_REG_KEY_WIDTH,  IDS_DEF_WIDTH  );
    nH = _GetRegSize( IDS_REG_KEY_HEIGHT, IDS_DEF_HEIGHT );

    // if the position didn't exist in the registry or
    // the position is off the screen ( +/- nSHOWBUFFER )
    // then center the window, otherwise use the position
    if( nX == -1 || nY == -1
        || nX + nW < SHOWBUFFER
        || nX + SHOWBUFFER > GetSystemMetrics(SM_CXSCREEN)
        || nY + nH < SHOWBUFFER
        || nY + SHOWBUFFER > GetSystemMetrics(SM_CYSCREEN)
    )
    {
        SetWindowPos( g_hwndMain, NULL, 0, 0, nW, nH, SWP_NOMOVE | SWP_NOZORDER );
        CenterWindow( g_hwndMain, NULL );
    }
    else
    {
        SetWindowPos( g_hwndMain, NULL, nX, nY, nW, nH, SWP_NOZORDER );
    }
}

VOID _OnSize( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    WINDOWPLACEMENT wndpl;

    wndpl.length = sizeof( WINDOWPLACEMENT );

    if( GetWindowPlacement(hwnd, &wndpl) )
    {
        DWORD dwW = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
        DWORD dwH = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
        RECT  rcMain;

        WriteRegDword_StrTbl( IDS_REG_PATH_BASE, IDS_REG_KEY_WIDTH,  dwW );
        WriteRegDword_StrTbl( IDS_REG_PATH_BASE, IDS_REG_KEY_HEIGHT, dwH );

        GetClientRect( hwnd, &rcMain );

        // set the position and size of the device window
        //
        g_cDevices.OnSize( &rcMain );

        // set the position and size of the device files window
        //
        g_cDevFiles.OnSize( &rcMain );

        // set the position of the status bar
        //
        g_cStatus.OnSize( &rcMain );

    }
}

VOID _OnMove( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    WINDOWPLACEMENT wndpl;

    if( hwnd != g_hwndMain )
    {
        return;
    }

    wndpl.length = sizeof(WINDOWPLACEMENT);

    if( GetWindowPlacement(hwnd, &wndpl) )
    {
        WriteRegDword_StrTbl(
            IDS_REG_PATH_BASE,
            IDS_REG_KEY_XPOS,
            wndpl.rcNormalPosition.left
        );
        WriteRegDword_StrTbl(
            IDS_REG_PATH_BASE,
            IDS_REG_KEY_YPOS,
            wndpl.rcNormalPosition.top
        );
    }
}

VOID UpdateMenu(IWMDMStorageGlobals* pStorageGlobals)
{
    HMENU   hMainMenu = GetMenu( g_hwndMain );
    HMENU   hFileMenu = GetSubMenu( hMainMenu, 0 );

    if (hFileMenu)
    {
        DWORD dwCaps = 0;
        pStorageGlobals->GetCapabilities(&dwCaps); // ignore return value, failure indicates default value is kept
        // Enable/disable 'Initialize' - command
        EnableMenuItem( hFileMenu, IDM_DEVICE_RESET, MF_BYCOMMAND | ((dwCaps & WMDM_STORAGECAP_NOT_INITIALIZABLE) ? MF_GRAYED : MF_ENABLED) );
    }
}

struct MediaCastInfo 
{
    MediaCastInfo(WCHAR* szName, WCHAR* szOwner, WCHAR* szEditor, WCHAR* szWebmaster, WCHAR* szSourceURL, WCHAR* szDestinationURL, 
		WCHAR* szCategory, WCHAR* szSummary, WCHAR* szObjectBookmark, WCHAR* szLocalPath, WMDM_FORMATCODE FormatCode = WMDM_FORMATCODE_MEDIA_CAST) :
            m_FormatCode(FormatCode)
        { 
            m_szName[0] = L'\0';
	     m_szOwner[0]=L'\0';
	     m_szEditor[0]=L'\0';
	     m_szWebmaster[0]=L'\0';
            m_szSourceURL[0] = L'\0';
            m_szDestinationURL[0] = L'\0';
            m_szCategory[0] = L'\0';
            m_szSummary[0] = L'\0';
	     m_szObjectBookmark[0] = L'\0';
	     m_szLocalPath[0] = L'\0';
		 
            if (szName)
            {
                StringCbCopy(m_szName, sizeof(m_szName), szName); // ignore return code
            }
            if (szOwner)
            {
                StringCbCopy(m_szOwner, sizeof(m_szOwner), szOwner); // ignore return code
            }
            if (szEditor)
            {
                StringCbCopy(m_szEditor, sizeof(m_szEditor), szEditor); // ignore return code
            }
            if (szWebmaster)
            {
                StringCbCopy(m_szWebmaster, sizeof(m_szWebmaster), szWebmaster); // ignore return code
            }
            if (szSourceURL)
            {
                StringCbCopy(m_szSourceURL, sizeof(m_szSourceURL), szSourceURL); // ignore return code
            }
            if (szDestinationURL)
            {
                StringCbCopy(m_szDestinationURL, sizeof(m_szDestinationURL), szDestinationURL); // ignore return code
            }
            if (szCategory)
            {
                StringCbCopy(m_szCategory, sizeof(m_szCategory), szCategory); // ignore return code
            }
            if (szSummary)
            {
                StringCbCopy(m_szSummary, sizeof(m_szSummary), szSummary); // ignore return code
            }
            if (szObjectBookmark)
            {
                StringCbCopy(m_szObjectBookmark, sizeof(m_szObjectBookmark), szObjectBookmark); // ignore return code
            }
            if (szLocalPath)
            {
                StringCbCopy(m_szLocalPath, sizeof(m_szLocalPath), szLocalPath); // ignore return code
            }
        }
            
    WMDM_FORMATCODE m_FormatCode;
    WCHAR m_szName[MAX_PATH];
    WCHAR m_szOwner[MAX_PATH];
    WCHAR m_szEditor[MAX_PATH];
    WCHAR m_szWebmaster[MAX_PATH];
    WCHAR m_szSourceURL[MAX_PATH];
    WCHAR m_szDestinationURL[MAX_PATH];
    WCHAR m_szCategory[MAX_PATH];
    WCHAR m_szSummary[MAX_PATH];
    WCHAR m_szObjectBookmark[MAX_PATH];
    WCHAR m_szLocalPath[MAX_PATH];
};

INT_PTR CALLBACK EnterMediaCast_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   
    static MediaCastInfo* pInf = NULL;
    
    switch( uMsg )
    {
        case WM_INITDIALOG:
            pInf = (MediaCastInfo*)(lParam);
            if (pInf)
            {
		  SetDlgItemTextW(hWnd, IDC_MEDIACAST_NAME, pInf->m_szName);
                SetDlgItemTextW(hWnd, IDC_OWNER, pInf->m_szOwner);
                SetDlgItemTextW(hWnd, IDC_EDITOR, pInf->m_szEditor);
                SetDlgItemTextW(hWnd, IDC_WEBMASTER, pInf->m_szWebmaster);
                SetDlgItemTextW(hWnd, IDC_SOURCE_URL, pInf->m_szSourceURL);
                SetDlgItemTextW(hWnd, IDC_DESTINATION_URL, pInf->m_szDestinationURL);
                SetDlgItemTextW(hWnd, IDC_CATEGORY, pInf->m_szCategory);
                SetDlgItemTextW(hWnd, IDC_SUMMARY, pInf->m_szSummary);
                SetDlgItemTextW(hWnd, IDC_OBJECT_BOOKMARK, pInf->m_szObjectBookmark);
                SetDlgItemTextW(hWnd, IDC_PODCAST_PATH, pInf->m_szLocalPath);
            }
        break;

        case WM_COMMAND:
            if (pInf)
            {
                if( GET_WM_COMMAND_ID(wParam, lParam) == IDOK ||
                    GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL) 
                {
                    GetDlgItemText(hWnd, IDC_MEDIACAST_NAME, pInf->m_szName, sizeof(pInf->m_szName) / sizeof(pInf->m_szName[0]));
                    GetDlgItemText(hWnd, IDC_OWNER, pInf->m_szOwner, sizeof(pInf->m_szOwner) / sizeof(pInf->m_szOwner[0]));
                    GetDlgItemText(hWnd, IDC_EDITOR, pInf->m_szEditor, sizeof(pInf->m_szEditor) / sizeof(pInf->m_szEditor[0]));
                    GetDlgItemText(hWnd, IDC_WEBMASTER, pInf->m_szWebmaster, sizeof(pInf->m_szWebmaster) / sizeof(pInf->m_szWebmaster[0]));
                    GetDlgItemText(hWnd, IDC_SOURCE_URL, pInf->m_szSourceURL, sizeof(pInf->m_szSourceURL) / sizeof(pInf->m_szSourceURL[0]));
                    GetDlgItemText(hWnd, IDC_DESTINATION_URL, pInf->m_szDestinationURL, sizeof(pInf->m_szDestinationURL) / sizeof(pInf->m_szDestinationURL[0]));
                    GetDlgItemText(hWnd, IDC_CATEGORY, pInf->m_szCategory, sizeof(pInf->m_szCategory) / sizeof(pInf->m_szCategory[0]));
                    GetDlgItemText(hWnd, IDC_SUMMARY, pInf->m_szSummary, sizeof(pInf->m_szSummary) / sizeof(pInf->m_szSummary[0]));
                    GetDlgItemText(hWnd, IDC_OBJECT_BOOKMARK, pInf->m_szObjectBookmark, sizeof(pInf->m_szObjectBookmark) / sizeof(pInf->m_szObjectBookmark[0]));

                    GetDlgItemText(hWnd, IDC_PODCAST_PATH, pInf->m_szLocalPath, sizeof(pInf->m_szLocalPath) / sizeof(pInf->m_szLocalPath[0]));

                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    return TRUE;
                }
            }
        break;

        default:
            break;
    }
    
    return 0;
}    

INT_PTR DlgGetMediaCastInfo(MediaCastInfo& mediacast)
{
    return DialogBoxParam(g_hInst,
                MAKEINTRESOURCE(IDD_ENTER_MEDIACASTINFO),
                g_hwndMain,
                EnterMediaCast_DlgProc,
                (LPARAM)&mediacast);
}

VOID _OnCreateMediaCast( WPARAM wParam, LPARAM lParam )
{
    CProgress cProgress;
    HRESULT   hr = S_OK;
    INT       i;
    INT      *pnSelItems = NULL;
    INT       nNumSel = 0;
    INT       idxStg = 0;
    MediaCastInfo MediaCast(L"DefaultMediaCastName", L"DefaultOwner", L"DefaultEditor", L"DefaultWebmaster", L"DefaultSourceURL", L"DefaultDestinationURL", 
							L"DefaultCategory", L"DefaultSummary", L"DefaultObjectBookmark", NULL, WMDM_FORMATCODE_MEDIA_CAST);
    BOOL fProgressVisible = FALSE;
    IWMDMMetaData* pIWMDMMetaData = NULL;
    IWMDMStorage3* pStg3 = NULL;
    IWMDMStorageControl3* pStorageControl = NULL;
    IWMDMStorage4* pPlaylist = NULL;
    IWMDMStorage** ppIStorage = NULL;

    HANDLE                           hFind;
    WIN32_FIND_DATA		FindFileData;
    WCHAR				wszFolderPath[MAX_PATH];
    WCHAR			       wszFilePath[MAX_PATH];

    // Get, from the user, a name for the playlist
    INT_PTR nRet = DlgGetMediaCastInfo(MediaCast);
    ExitOnTrue(nRet == IDCANCEL);

    // Count number of items in folder
    //
    // define wildcard in order to list every file in the directory
    StringCbCopyW(wszFolderPath, sizeof(wszFolderPath), MediaCast.m_szLocalPath);
    StringCbCatW(wszFolderPath,sizeof(wszFolderPath), L"\\*");
	
    hFind = FindFirstFileW(wszFolderPath, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) 
    {
    	printf ("Invalid file handle. Error is %u\n", GetLastError());
	ExitOnFail(E_FAIL);
    } 
    
    // getting rid of . and .. as part of the directory info
    FindNextFile(hFind, &FindFileData);
    if(FindNextFile(hFind, &FindFileData) == 0)
   {			   
   	FindClose(hFind);
	ExitOnFail(E_FAIL);
   }

   while (FindNextFile(hFind, &FindFileData) != 0) 
   {
       StringCbCopyW(wszFilePath, sizeof(wszFilePath), MediaCast.m_szLocalPath);
       StringCbCatW(wszFilePath, sizeof(wszFilePath), L"\\");
       StringCbCatW(wszFilePath, sizeof(wszFilePath), FindFileData.cFileName);

	ExitOnFalse(g_cDevFiles.SendFilesToDevice(wszFilePath, 1));
	ExitOnFalse(g_cDevFiles.SetFocusOnItem());
   }

    // Get the number of selected items.
    // Explain usage and exit if there are no items selected.
    //
    // Call with NULL instead of pnSelItems for prefix
    g_cDevFiles.GetSelectedItems( NULL, &nNumSel );
    if( 0 == nNumSel )
    {
        goto lExit;
    }

    // Allocate space to hold them the selected items
    //
    pnSelItems = new INT[ nNumSel ];
    ExitOnNull( pnSelItems );

    // Get the selected file(s) to delete
    //
    ExitOnTrue( -1 == g_cDevFiles.GetSelectedItems(pnSelItems, &nNumSel) );

    // Create a progress dialog
    //
    ExitOnFalse( cProgress.Create(g_hwndMain) );
    fProgressVisible = TRUE;

    // Set operation progress values
    //
    cProgress.SetOperation( L"Creating MediaCast..." );
    cProgress.SetRange( 0, nNumSel );
    cProgress.SetCount( 0, nNumSel );
    cProgress.SetBytes( -1, -1 );

    ppIStorage = new IWMDMStorage*[nNumSel];
    ExitOnNull( ppIStorage );
        
    for( i=nNumSel-1; i >= 0; i-- )
    {
        CItemData *pStorage;

        // Get the storage object for the item being added to the playlist
        //
        pStorage = (CItemData *)ListView_GetLParam( g_cDevFiles.GetHwnd_LV(), pnSelItems[i] );

        if( NULL != pStorage && NULL != pStorage->m_pStorage)
        {
            // Set the name of the object and show the progress dialog
            //
            cProgress.SetDetails( pStorage->m_szName );
            ppIStorage[idxStg] = pStorage->m_pStorage;
            ppIStorage[idxStg]->AddRef();
            idxStg++;
        }
            
        cProgress.IncCount();
        cProgress.IncPos( 1 );
        cProgress.Show( TRUE );
    }

    if( idxStg )
    {
        HTREEITEM hItem = NULL;
        CItemData* pItemData = NULL;
        IWMDMStorage* pInStorage  = NULL;
        IWMDMStorage* pNewStorage = NULL;
        
        // Get the selected device/storage
        //
        hItem = g_cDevices.GetSelectedItem( NULL );
        ExitOnNull( hItem );

        // Get the itemdata class associated with the hItem and 
        // retrieve the IWMDMStorage for it
        //
        pItemData = (CItemData *) TreeView_GetLParam( g_cDevices.GetHwnd_TV(), hItem );
        ExitOnNull( pItemData );

        pInStorage = ( pItemData->m_fIsDevice ? pItemData->m_pRootStorage : pItemData->m_pStorage );
        ExitOnNull( pInStorage );

        // Acquire the storage control interface
        //
        hr = pInStorage->QueryInterface(
            IID_IWMDMStorageControl3,
            reinterpret_cast<void**>(&pStorageControl)
        );
        ExitOnFail(hr);

        // Add an new MediaCast object.
        hr = pInStorage->QueryInterface(IID_IWMDMStorage3, (void **)&pStg3);
        ExitOnFail(hr);
        
        hr = pStg3->CreateEmptyMetadataObject(&pIWMDMMetaData);
        ExitOnFail(hr);

        // 
        // Though not implemented here, the officially recommended way to set the format code for a
        //  playlist object is the following:
        //  
        //  1. Acquire a IWMDMDevice3 pointer to the target device.
        //
        //  2. Call IWMDMDevice3::GetProperty(g_wszWMDMFormatsSupported, ...) to obtain the format codes
        //      supported by the device. 
        //
        //  3. If no playlist formats are supported, disallow sending playlists to the device.
        // 
        //  4. Choose the device-supported format code that matches most closely the intended album object 
        //      type (e.g. WMDM_FORMATCODE_WPLPLAYLIST for a playlist originating from a .WPL file). Fall
        //      back to the generic WMDM_FORMATCODE_ABSTRACTAUDIOVIDEOPLAYLIST only if necessary.
        // 
        //  5. Use that format code instead of the following hard-coded value.
        // 

 	 // Setting object format
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_DWORD, g_wszWMDMFormatCode, (BYTE *)&MediaCast.m_FormatCode, sizeof(MediaCast.m_FormatCode));
        ExitOnFail(hr);

 	 // Setting primary ID
        WCHAR wszPrimaryID[MAX_PATH] = L"MEDIACLASS_PRIMARYID_AUDIO";
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_STRING, g_wszWMDMediaClassPrimaryID, (BYTE *)&wszPrimaryID, (sizeof(wszPrimaryID))/(sizeof(wszPrimaryID[0])));
        ExitOnFail(hr);

 	 // Setting secondary ID
        WCHAR wszSecondaryID[MAX_PATH] = L"MEDIACLASS_SECONDARYID_AUDIO_MEDIA_PODCAST";
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_STRING, g_wszWMDMMediaClassSecondaryID, (BYTE *)&wszSecondaryID, (sizeof(wszSecondaryID))/(sizeof(wszSecondaryID[0])));
        ExitOnFail(hr);

	 // Setting owner
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_STRING, g_wszWMDMOwner, (BYTE *)&MediaCast.m_szOwner, (sizeof(MediaCast.m_szOwner))/(sizeof(MediaCast.m_szOwner[0])));
        ExitOnFail(hr);

	 // Setting editor
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_STRING, g_wszWMDMEditor, (BYTE *)&MediaCast.m_szEditor, (sizeof(MediaCast.m_szEditor))/(sizeof(MediaCast.m_szEditor[0])));
        ExitOnFail(hr);

	 // Setting webmaster
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_STRING, g_wszWMDMWebmaster, (BYTE *)&MediaCast.m_szWebmaster, (sizeof(MediaCast.m_szWebmaster))/(sizeof(MediaCast.m_szWebmaster[0])));
        ExitOnFail(hr);

	 // Setting source url
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_STRING, g_wszWMDMSourceURL, (BYTE *)&MediaCast.m_szSourceURL, (sizeof(MediaCast.m_szSourceURL))/(sizeof(MediaCast.m_szSourceURL[0])));
        ExitOnFail(hr);

	 // Setting destination url
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_STRING, g_wszWMDMDestinationURL, (BYTE *)&MediaCast.m_szDestinationURL, (sizeof(MediaCast.m_szDestinationURL))/(sizeof(MediaCast.m_szDestinationURL[0])));
        ExitOnFail(hr);

	 // Setting category
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_STRING, g_wszWMDMCategory, (BYTE *)&MediaCast.m_szCategory, (sizeof(MediaCast.m_szCategory))/(sizeof(MediaCast.m_szCategory[0])));
        ExitOnFail(hr);

	 // Setting description
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_STRING, g_wszWMDMDescription, (BYTE *)&MediaCast.m_szSummary, (sizeof(MediaCast.m_szSummary))/(sizeof(MediaCast.m_szSummary[0])));
        ExitOnFail(hr);

 	 // Setting object bookmark
        hr = pIWMDMMetaData->AddItem(WMDM_TYPE_STRING, g_wszWMDMObjectBookmark, (BYTE *)&MediaCast.m_szObjectBookmark, (sizeof(MediaCast.m_szObjectBookmark))/(sizeof(MediaCast.m_szObjectBookmark[0])));
        ExitOnFail(hr);

        hr = pStorageControl->Insert3(WMDM_MODE_BLOCK | WMDM_CONTENT_FILE,
                                                0,
                                                NULL,
                                                MediaCast.m_szName,
                                                NULL,
                                                NULL,
                                                pIWMDMMetaData,
                                                NULL,
                                                &pNewStorage);
        ExitOnFail(hr);
        SafeRelease(pIWMDMMetaData);

        hr = pNewStorage->QueryInterface( IID_IWMDMStorage4, reinterpret_cast<void**>(&pPlaylist));
        pNewStorage->Release();
        ExitOnFail(hr);
               
        hr = pPlaylist->SetReferences(idxStg, ppIStorage);
        ExitOnFail(hr);
                    
        CItemData *pItem = new CItemData;
        if( pItem )
        {
            hr = pItem->Init( pPlaylist );
            if( SUCCEEDED(hr) )
            {
                g_cDevFiles.AddItem( pItem );
            }
            else
            {
                delete pItem;
            }
        }
    }

    g_cDevices.UpdateSelection( NULL, FALSE );

lExit:
    if (fProgressVisible)
    {
        cProgress.Show( FALSE );
        cProgress.Destroy();
    }

    SafeRelease(pPlaylist);
    SafeRelease(pStg3);
    SafeRelease(pIWMDMMetaData);
    SafeRelease(pStorageControl);
   
    if( pnSelItems )
    {
        delete[] pnSelItems;
    }

    for (i = 0; i < idxStg; i++)
    {
        if (ppIStorage[i] != NULL)
        {
            ppIStorage[i]->Release();
            ppIStorage[i] = NULL;
        }
    }

    if (ppIStorage)
    {
        delete[] ppIStorage;
    }
   
    if (FAILED(hr))
    {
        MsgBoxDisplayError(hr);
    }
}



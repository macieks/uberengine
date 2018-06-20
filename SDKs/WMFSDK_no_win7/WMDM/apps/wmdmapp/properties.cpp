//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//

//
// This workspace contains two projects -
// 1. ProgHelp which implements the Progress Interface 
// 2. The Sample application WmdmApp. 
//
//  ProgHelp.dll needs to be registered first for the SampleApp to run.


// Includes
//
#include "appPCH.h"
#include <atlbase.h>
#include <atlconv.h>

using namespace Gdiplus;

// Update serial id in device/storage dialog
void UpdateSerialId( HWND hDlg, CItemData* pDevice )
{
    WCHAR*  pszSerial;
    WCHAR*  pszSerialStep;
    int    iMaxStringLength;
    BOOL   bSerialIsString = TRUE;     // Should we also show the serial id as a string?

    if( pDevice == NULL || hDlg == 0 ) return;

    // No serial ?
    if( pDevice->m_SerialNumber.SerialNumberLength == 0 )
    {
        SetDlgItemText( hDlg, IDC_SERIALID, L"<none>" );
        return;
    }

    // Get serial # of device as a string
    iMaxStringLength = pDevice->m_SerialNumber.SerialNumberLength * sizeof(L"FF ") +1;
    pszSerial = new WCHAR[iMaxStringLength];
    pszSerialStep = pszSerial;
    for( UINT uIndex = 0; uIndex < pDevice->m_SerialNumber.SerialNumberLength; uIndex ++ )
    {
        // Add one byte at the time to the serial id string
        pszSerialStep += wsprintf( pszSerialStep, L"%X ", pDevice->m_SerialNumber.pID[uIndex] );

        if( !isprint( pDevice->m_SerialNumber.pID[uIndex] ) )
        {
            bSerialIsString = FALSE;
        }
    }
    SetDlgItemText( hDlg, IDC_SERIALID, pszSerial );

    // If the serial id is a string show it as a string
    if( bSerialIsString && (pDevice->m_SerialNumber.SerialNumberLength > 0) )
    {
        SetDlgItemText( hDlg, IDC_SERIALID_STRING, (WCHAR*)pDevice->m_SerialNumber.pID );
    }

    delete [] pszSerial;
}

// Update manufacturer value in device dialog
void UpdateManufacturer( HWND hDlg, CItemData* pDevice )
{
    SetDlgItemText( hDlg, IDC_MANUFACTURER, pDevice->m_szMfr );
}

// Update model name value in device dialog
void UpdateModelName( HWND hDlg, CItemData* pDevice )
{
    SetDlgItemText( hDlg, IDC_MODELNAME, pDevice->m_szModelName );
}

// Update firmware version value in device dialog
void UpdateFirmwareVer( HWND hDlg, CItemData* pDevice )
{
    SetDlgItemText( hDlg, IDC_FIRMWAREVER, pDevice->m_szFirmwareVer );
}

// Update device type value in device dialog
void UpdateDeviceType( HWND hDlg, CItemData* pDevice  )
{
    WCHAR    pszType[MAX_PATH];

    static SType_String sDeviceTypeStringArray[] = {
        { WMDM_DEVICE_TYPE_PLAYBACK, L"Playback" },
        { WMDM_DEVICE_TYPE_RECORD,   L"Record" },
        { WMDM_DEVICE_TYPE_DECODE,   L"Decode" },
        { WMDM_DEVICE_TYPE_ENCODE,   L"Encode" },
        { WMDM_DEVICE_TYPE_STORAGE,  L"Storage" },
        { WMDM_DEVICE_TYPE_VIRTUAL,  L"Virtual" },
        { WMDM_DEVICE_TYPE_SDMI,     L"Sdmi" },
        { WMDM_DEVICE_TYPE_NONSDMI,  L"non-sdmi" },
        { 0, NULL },
    };
    
    // Add all the types reported by the device to the string.
    pszType[0] = L'\0';
    for( int iIndex = 0; sDeviceTypeStringArray[iIndex].dwType != 0; iIndex++ )
    {
        // Is this bit set, if it is then add the type as a string
        if( sDeviceTypeStringArray[iIndex].dwType & pDevice->m_dwType )
        {
            if( wcslen(pszType) )
            {
                wcscat_s( pszType, ARRAYSIZE(pszType), L", " );
            }
            wcscat_s( pszType, ARRAYSIZE(pszType), sDeviceTypeStringArray[iIndex].pszString );
        }
    }

    SetDlgItemText( hDlg, IDC_DEVICE_TYPE, ((wcslen(pszType)) ? pszType : L"<none>") );
}

// Update icon in device dialog
void UpdateDeviceIcon( HWND hDlg, CItemData* pDevice )
{
    HICON      hIcon;

    hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDI_DEVICE ) );
    ::SendMessage(hDlg, WM_SETICON, FALSE, (LPARAM)hIcon );
}


// Update status property in device/storage dialog box
void UpdateStatus( HWND hDlg, CItemData* pDevice )
{
    WCHAR    pszStatus[MAX_PATH] = L"";

    static SType_String sDeviceTypeStringArray[] = {
        { WMDM_STATUS_READY                   , L"Ready" },
        { WMDM_STATUS_BUSY                    , L"Busy" },
        { WMDM_STATUS_DEVICE_NOTPRESENT       , L"Device not present" },
        { WMDM_STATUS_STORAGE_NOTPRESENT      , L"Storage not present" },
        { WMDM_STATUS_STORAGE_INITIALIZING    , L"Storage initializing" },
        { WMDM_STATUS_STORAGE_BROKEN          , L"Storage broken" },
        { WMDM_STATUS_STORAGE_NOTSUPPORTED    , L"Storage not supported" },
        { WMDM_STATUS_STORAGE_UNFORMATTED     , L"Storage unformatted" },
        { WMDM_STATUS_STORAGECONTROL_INSERTING, L"Storagecontrol inserting" },
        { WMDM_STATUS_STORAGECONTROL_DELETING , L"Storagecontrol deleting" },
        { WMDM_STATUS_STORAGECONTROL_MOVING   , L"Storagecontrol moving" },
        { WMDM_STATUS_STORAGECONTROL_READING  , L"Storagecontrol reading" },
        { 0, L"" },
    };
    
    // Add all the types reported by the device to the string.
    for( int iIndex = 0; sDeviceTypeStringArray[iIndex].dwType != 0; iIndex++ )
    {
        // Is this bit set, if it is then add the type as a string
        if( sDeviceTypeStringArray[iIndex].dwType & pDevice->m_dwType )
        {
            if( wcslen(pszStatus) )
            {
                wcscat_s( pszStatus, ARRAYSIZE(pszStatus), L", " );
            }
            wcscat_s( pszStatus, ARRAYSIZE(pszStatus), sDeviceTypeStringArray[iIndex].pszString );
        }
    }

    SetDlgItemText( hDlg, IDC_DEVICE_STATUS, ((wcslen(pszStatus)) ? pszStatus : L"<none>") );
}

// Update device status property in device dialog box
void UpdateDeviceVersion( HWND hDlg, CItemData* pDevice )
{   
    if( pDevice->m_dwVersion == (DWORD)-1 )
    {
        SetDlgItemText( hDlg, IDC_VERSION, L"<not supported>");
    }
    else
    {
        SetDlgItemInt( hDlg, IDC_VERSION, pDevice->m_dwVersion, FALSE );
    }
}

// Update device status property in device dialog box
void UpdatePowerSource( HWND hDlg, CItemData* pDevice )
{
    WCHAR    pszPowerSource[MAX_PATH];
    WCHAR    pszPowerIs[MAX_PATH];

    // Update capabileties
    if( (pDevice->m_dwPowerSource & WMDM_POWER_CAP_BATTERY) &&
        (pDevice->m_dwPowerSource & WMDM_POWER_CAP_EXTERNAL) )
    {
        SetDlgItemText( hDlg, IDC_POWER_CAP, L"Batteries and external");
    }
    else if(pDevice->m_dwPowerSource & WMDM_POWER_CAP_BATTERY)
    {
        SetDlgItemText( hDlg, IDC_POWER_CAP, L"Batteries");
    }
    else if(pDevice->m_dwPowerSource & WMDM_POWER_CAP_EXTERNAL)
    {
        SetDlgItemText( hDlg, IDC_POWER_CAP, L"External");
    }
    else
    {
        SetDlgItemText( hDlg, IDC_POWER_CAP, L"<non reported>");
    }

    // Update current power source string
    if( (pDevice->m_dwPowerSource & WMDM_POWER_CAP_BATTERY) &&
        (pDevice->m_dwPowerSource & WMDM_POWER_CAP_EXTERNAL) )
    {
        wcscpy_s( pszPowerSource, ARRAYSIZE(pszPowerSource), L"Batteries and external");
    }
    else if(pDevice->m_dwPowerSource & WMDM_POWER_CAP_BATTERY)
    {
        wcscpy_s( pszPowerSource, ARRAYSIZE(pszPowerSource),  L"Batteries");
    }
    else if(pDevice->m_dwPowerSource & WMDM_POWER_CAP_EXTERNAL)
    {
        wcscpy_s( pszPowerSource, ARRAYSIZE(pszPowerSource),  L"External");
    }
    else
    {
        wcscpy_s( pszPowerSource, ARRAYSIZE(pszPowerSource),  L"<non reported>");
    }
    
    if (SUCCEEDED(StringCbPrintf( pszPowerIs, sizeof(pszPowerIs), L"%s (%d%% remaning)", pszPowerSource, pDevice->m_dwPercentRemaining )))
    {
        SetDlgItemText( hDlg, IDC_POWER_IS, pszPowerIs );
    }
}


// Update dialog caption
void UpdateCaption( HWND hDlg, CItemData* pDevice )
{
    WCHAR    pszWndCaption[2*MAX_PATH];
    WCHAR    pszFormat[MAX_PATH];

    // Set window caption
    LoadString( g_hInst, IDS_PROPERTIES_CAPTION, pszFormat, sizeof(pszFormat) / sizeof(pszFormat[0]) );
    if (SUCCEEDED(StringCbPrintf(pszWndCaption, sizeof(pszWndCaption), pszFormat, pDevice->m_szName)))
    {
        SetWindowText( hDlg, pszWndCaption );
    }
}

void UpdateVendorExtDesc( HWND hDlg, CItemData* pDevice )
{
    if (pDevice->m_sbstrVendorExtDesc != NULL)
    {
        SetDlgItemText( hDlg, IDC_VENDOR_EXT_DESC, CW2T(pDevice->m_sbstrVendorExtDesc));
    }
}

void UpdateDeviceProtocol( HWND hDlg, CItemData* pDevice )
{
    if (IsEqualCLSID(pDevice->m_deviceProtocol, WMDM_DEVICE_PROTOCOL_MTP))
    {
        SetDlgItemText( hDlg, IDC_DEVICE_PROTOCOL, _T("MTP"));        
    }
    else if (IsEqualCLSID(pDevice->m_deviceProtocol, WMDM_DEVICE_PROTOCOL_MSC))
    {
        SetDlgItemText( hDlg, IDC_DEVICE_PROTOCOL, _T("MSC"));                
    }
    else if (IsEqualCLSID(pDevice->m_deviceProtocol, WMDM_DEVICE_PROTOCOL_RAPI))
    {
        SetDlgItemText( hDlg, IDC_DEVICE_PROTOCOL, _T("RAPI"));                
    }
    else
    {
        SetDlgItemText( hDlg, IDC_DEVICE_PROTOCOL, _T("<Unknown>"));                
    }
}

void UpdateSPVendor( HWND hDlg, CItemData* pDevice )
{
    if (IsEqualCLSID(pDevice->m_deviceSPVendor, WMDM_SERVICE_PROVIDER_VENDOR_MICROSOFT))
    {
        SetDlgItemText( hDlg, IDC_SP_VENDOR, _T("Microsoft"));        
    }
    else
    {
        SetDlgItemText( hDlg, IDC_SP_VENDOR, _T("<Unknown>"));        
    }
}

// Procedure for device property dialog box
INT_PTR CALLBACK DeviceProp_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) 
    {
        case WM_INITDIALOG:
        {
            CItemData*  pItem = (CItemData*)lParam;
            if( pItem == NULL ) return FALSE;

            UpdateSerialId( hDlg, pItem );
            UpdateManufacturer( hDlg, pItem );
            UpdateModelName( hDlg, pItem );
            UpdateFirmwareVer( hDlg, pItem );
            UpdateDeviceType( hDlg, pItem );
            UpdateDeviceIcon( hDlg, pItem );
            UpdateDeviceVersion( hDlg, pItem );
            UpdatePowerSource( hDlg, pItem  );
            UpdateStatus( hDlg, pItem  );
            UpdateCaption( hDlg, pItem );
            UpdateVendorExtDesc( hDlg, pItem );
            UpdateDeviceProtocol( hDlg, pItem );
            UpdateSPVendor( hDlg, pItem );
            
            CenterWindow(hDlg, g_hwndMain );
            return TRUE;
        }
        case WM_COMMAND:
            if( GET_WM_COMMAND_ID(wParam, lParam) == IDOK ||
                GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL) 
            {
                EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
                return TRUE;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;
}


// Storage properties
//

// Update storage attributes values in storage properties dialog
void UpdateAttributes( HWND hDlg, CItemData* pDevice )
{
    WCHAR    pszAttr[MAX_PATH];

    static SType_String sTypeStringArray[] = {
        { WMDM_STORAGE_ATTR_FILESYSTEM   , L"Filesystem" },
        { WMDM_STORAGE_ATTR_REMOVABLE    , L"Removable" },
        { WMDM_STORAGE_ATTR_NONREMOVABLE , L"Nonremovable" },
        { WMDM_STORAGE_ATTR_FOLDERS      , L"Folders" },
        { WMDM_STORAGE_ATTR_HAS_FOLDERS  , L"Has folders" },
        { WMDM_STORAGE_ATTR_HAS_FILES    , L"Has files" },
        { WMDM_FILE_ATTR_FOLDER          , L"Folder" },
        { WMDM_FILE_ATTR_LINK            , L"Link" },
        { WMDM_FILE_ATTR_FILE            , L"File" },
        { WMDM_FILE_ATTR_AUDIO           , L"Audio" },
        { WMDM_FILE_ATTR_DATA            , L"Data" },
        { WMDM_FILE_ATTR_MUSIC           , L"Music" },
        { WMDM_FILE_ATTR_AUDIOBOOK       , L"Audiobook" },
        { WMDM_FILE_ATTR_HIDDEN          , L"Hidden" },
        { WMDM_FILE_ATTR_SYSTEM          , L"System" },
        { WMDM_FILE_ATTR_READONLY        , L"Readonly" },
        { 0, L""},
    };

    // Add all the attributes reported by the device as to the string.
    pszAttr[0] = L'\0';
    for( int iIndex = 0; sTypeStringArray[iIndex].dwType != 0; iIndex++ )
    {
        // Is this bit set, if it is then add the attribute as a string
        if( sTypeStringArray[iIndex].dwType & pDevice->m_dwAttributes )
        {
            if( wcslen(pszAttr) )
            {
                wcscat_s( pszAttr, ARRAYSIZE(pszAttr), L", " );
            }
            wcscat_s( pszAttr, ARRAYSIZE(pszAttr), sTypeStringArray[iIndex].pszString );
        }
    }

    SetDlgItemText( hDlg, IDC_ATTRIBUTES, (wcslen(pszAttr) ? pszAttr : L"<none>") );

}

void GetFormatCodeString( WCHAR*& szFormatCode, DWORD dwFormatCode )
{
    static SType_String sTypeStringArray[] = {
        { WMDM_FORMATCODE_NOTUSED,      L"NotUsed"},
        { WMDM_FORMATCODE_ALLIMAGES,    L"AllImages"},
        { WMDM_FORMATCODE_UNDEFINED,    L"Undefined"},
        { WMDM_FORMATCODE_ASSOCIATION,  L"Association"},
        { WMDM_FORMATCODE_SCRIPT,       L"Script"},
        { WMDM_FORMATCODE_EXECUTABLE,   L"Executable"},
        { WMDM_FORMATCODE_TEXT,         L"Text"},
        { WMDM_FORMATCODE_HTML,         L"HTML"},
        { WMDM_FORMATCODE_DPOF,         L"DPOF"},
        { WMDM_FORMATCODE_AIFF,         L"AIFF"},
        { WMDM_FORMATCODE_WAVE,         L"Wave"},
        { WMDM_FORMATCODE_MP3,          L"MP3"},
        { WMDM_FORMATCODE_AVI,          L"AVI"},
        { WMDM_FORMATCODE_MPEG,         L"MPEG"},
        { WMDM_FORMATCODE_ASF,          L"ASF"},
            
// Skip this range            
//    WMDM_FORMATCODE_RESERVED_FIRST               = 0x300D,
//    WMDM_FORMATCODE_RESERVED_LAST                = 0x37FF,

        { WMDM_FORMATCODE_IMAGE_UNDEFINED,L"Image Undefined"},
        { WMDM_FORMATCODE_IMAGE_EXIF,   L"Image Exif"},
        { WMDM_FORMATCODE_IMAGE_TIFFEP, L"Image TiffEP"},
        { WMDM_FORMATCODE_IMAGE_FLASHPIX, L"Image FlashPix"},
        { WMDM_FORMATCODE_IMAGE_BMP,    L"Image BMP"},
        { WMDM_FORMATCODE_IMAGE_CIFF,   L"Image CIFF"},
        { WMDM_FORMATCODE_IMAGE_GIF ,   L"Image GIF"},
        { WMDM_FORMATCODE_IMAGE_JFIF,   L"Image JFIF"},
        { WMDM_FORMATCODE_IMAGE_PCD ,   L"Image PCD"},
        { WMDM_FORMATCODE_IMAGE_PICT,   L"Image PICT"},
        { WMDM_FORMATCODE_IMAGE_PNG ,   L"Image PNG"},
        { WMDM_FORMATCODE_IMAGE_TIFF,   L"Image TIFF"},
        { WMDM_FORMATCODE_IMAGE_TIFFIT, L"Image TIFFIT"},
        { WMDM_FORMATCODE_IMAGE_JP2,    L"Image JP2"},
        { WMDM_FORMATCODE_IMAGE_JPX,    L"Image JPX"},

// Skip this range        
//    WMDM_FORMATCODE_IMAGE_RESERVED_FIRST         = 0x3811,
//    WMDM_FORMATCODE_IMAGE_RESERVED_LAST          = 0x3FFF,
//
        { WMDM_FORMATCODE_UNDEFINEDFIRMWARE,    L"Undefined Firmware"},
        { WMDM_FORMATCODE_WINDOWSIMAGEFORMAT,   L"Windows Image Format"},
        { WMDM_FORMATCODE_UNDEFINEDAUDIO,       L"Undefined Audio"},
        { WMDM_FORMATCODE_WMA,                  L"WMA"},
        { WMDM_FORMATCODE_UNDEFINEDVIDEO,       L"Undefined Video"},
        { WMDM_FORMATCODE_WMV,                  L"WMV"},
        { WMDM_FORMATCODE_UNDEFINEDCOLLECTION,  L"Undefined Collection"},
        { WMDM_FORMATCODE_ABSTRACTMULTIMEDIAALBUM,      L"Abstract Multimedia Album"},
        { WMDM_FORMATCODE_ABSTRACTIMAGEALBUM,   L"Abstract Image Album"},
        { WMDM_FORMATCODE_ABSTRACTAUDIOALBUM,   L"Abstract Audio Album"},
        { WMDM_FORMATCODE_ABSTRACTVIDEOALBUM,   L"Abstract Video Album"},
        { WMDM_FORMATCODE_ABSTRACTAUDIOVIDEOPLAYLIST,   L"Abstract AudioVideo Playlist"},
        { WMDM_FORMATCODE_ABSTRACTCONTACTGROUP, L"Abstract Contact Group"},
        { WMDM_FORMATCODE_ABSTRACTMESSAGEFOLDER,L"Abstract Message Folder"},
        { WMDM_FORMATCODE_ABSTRACTCHAPTEREDPRODUCTION,  L"Abstract Chaptered Production"},
        { WMDM_FORMATCODE_WPLPLAYLIST,          L"WPL Playlist"},
        { WMDM_FORMATCODE_M3UPLAYLIST,          L"M3U Playlist"},
        { WMDM_FORMATCODE_MPLPLAYLIST,          L"MPL Playlist"},
        { WMDM_FORMATCODE_ASXPLAYLIST,          L"ASX Playlist"},
        { WMDM_FORMATCODE_PLSPLAYLIST,          L"PLS Playlist"},
        { WMDM_FORMATCODE_UNDEFINEDDOCUMENT,    L"Undefined Document"},
        { WMDM_FORMATCODE_ABSTRACTDOCUMENT,     L"Abstract Document"},
        { WMDM_FORMATCODE_UNDEFINEDMESSAGE,     L"Undefined Message"},
        { WMDM_FORMATCODE_ABSTRACTMESSAGE,      L"Abstract Message"},
        { WMDM_FORMATCODE_UNDEFINEDCONTACT,     L"Undefined Contact"},
        { WMDM_FORMATCODE_ABSTRACTCONTACT,      L"Abstract Contact"},
        { WMDM_FORMATCODE_VCARD2,               L"Vcard2"},
        { WMDM_FORMATCODE_VCARD3,               L"Vcard3"},
        { WMDM_FORMATCODE_UNDEFINEDCALENDARITEM,L"Undefined Calendaritem"},
        { WMDM_FORMATCODE_ABSTRACTCALENDARITEM, L"Abstract Calendaritem"},
        { WMDM_FORMATCODE_VCALENDAR1,           L"Vcalendar1"},
        { WMDM_FORMATCODE_UNDEFINEDWINDOWSEXECUTABLE,   L"Undefined Windows Executable"},
        { WMDM_FORMATCODE_MEDIA_CAST,   L"Media Cast"},
    };

    // Look for the format code
    szFormatCode = L"<invalid value>";
    for( int iIndex = 0; iIndex < sizeof(sTypeStringArray) / sizeof(sTypeStringArray[0]); iIndex++ )
    {
        // Is this bit set, if it is then add the attribute as a string
        if( sTypeStringArray[iIndex].dwType == dwFormatCode )
        {
            szFormatCode = sTypeStringArray[iIndex].pszString;
            break;
        }
    }
}

void UpdateFormatCode( HWND hDlg, CItemData* pItem )
{
    // Look for the format code
    WCHAR* pszFormatCode = L"<Failed retrieval of format code>";
    GetFormatCodeString( pszFormatCode, pItem->m_dwFormatCode ); // always returns non-null
    SetDlgItemText( hDlg, IDC_FORMAT, pszFormatCode );
}

void UpdateActualImageStats(HWND hDlg, UINT width, UINT height)
{
    WCHAR szDWORD[20] = L"";
    if (SUCCEEDED(StringCbPrintf(szDWORD, sizeof(szDWORD), L"%d", width )))
    {
        SetDlgItemText( hDlg, IDC_ALBUMACTUALWIDTH, szDWORD );
    }
    
    if (SUCCEEDED(StringCbPrintf(szDWORD, sizeof(szDWORD), L"%d", height )))
    {
        SetDlgItemText( hDlg, IDC_ALBUMACTUALHEIGHT, szDWORD );
    }
}

void UpdateAlbumCoverProperties( HWND hDlg, CItemData* pItem)
{
    if( pItem->m_pMetaData )
    {
        WMDM_TAG_DATATYPE tagType;
        BYTE* pb = NULL;
        UINT cb = 0;
        WCHAR* szFormatCode = NULL;
        WCHAR szDWORD[20] = L"";
        WCHAR* szNotFound = L"Not available";
        
        HRESULT hr = pItem->m_pMetaData->QueryByName(g_wszWMDMAlbumCoverFormat, &tagType, &pb, &cb);
        if( SUCCEEDED(hr) && sizeof(DWORD) == cb )
        {
            GetFormatCodeString( szFormatCode, *(reinterpret_cast<DWORD*>(pb)) ); // always returns non-null
            SetDlgItemText( hDlg, IDC_ALBUMFORMAT, szFormatCode );
        }

        hr = pItem->m_pMetaData->QueryByName(g_wszWMDMAlbumCoverSize, &tagType, &pb, &cb);
        if( SUCCEEDED(hr) && sizeof(DWORD) == cb )
        {
            hr = StringCbPrintf(szDWORD, sizeof(szDWORD), L"%d", *(reinterpret_cast<DWORD*>(pb)) );
        }
        SetDlgItemText( hDlg, IDC_ALBUMSIZE, SUCCEEDED(hr) ? szDWORD : szNotFound );
        

        hr = pItem->m_pMetaData->QueryByName(g_wszWMDMAlbumCoverDuration, &tagType, &pb, &cb);
        if( SUCCEEDED(hr) && sizeof(DWORD) == cb )
        {
            hr = StringCbPrintf(szDWORD, sizeof(szDWORD), L"%d", *(reinterpret_cast<DWORD*>(pb)) );
        }
        SetDlgItemText( hDlg, IDC_ALBUMDURATION, SUCCEEDED(hr) ? szDWORD : szNotFound );

        hr = pItem->m_pMetaData->QueryByName(g_wszWMDMAlbumCoverWidth, &tagType, &pb, &cb);
        if( SUCCEEDED(hr) && sizeof(DWORD) == cb )
        {
            hr = StringCbPrintf(szDWORD, sizeof(szDWORD), L"%d", *(reinterpret_cast<DWORD*>(pb)) );
        }
        SetDlgItemText( hDlg, IDC_ALBUMWIDTH, SUCCEEDED(hr) ? szDWORD : szNotFound );

        hr = pItem->m_pMetaData->QueryByName(g_wszWMDMAlbumCoverHeight, &tagType, &pb, &cb);
        if( SUCCEEDED(hr) && sizeof(DWORD) == cb )
        {
            hr = StringCbPrintf(szDWORD, sizeof(szDWORD), L"%d", *(reinterpret_cast<DWORD*>(pb)) );
        }
        SetDlgItemText( hDlg, IDC_ALBUMHEIGHT, SUCCEEDED(hr) ? szDWORD : szNotFound );
    }
}

void CenterImageAndPreserveAspect(Rect& rectNew, Rect& rectDst, Rect& rectSrc)
{
    //
    // Don't expand the image. Maintain source pixel stepsizes >= 1.
    //
    if (rectDst.Contains(rectSrc))
    {
        // Source image size is smaller than display size, center the image and map pixels 1:1.
        rectNew = rectSrc;
        int ofsX = (rectDst.Width - rectSrc.Width) / 2;
        int ofsY = (rectDst.Height - rectSrc.Height) / 2;
        rectNew.Offset((ofsX > 0) ? ofsX : 0, (ofsY > 0) ? ofsY : 0);
    }
    else
    {
        // Source image size is larger than the display size, maintain the original image aspect ratio.
        Rect rectX;
        rectNew = rectDst;
        if (Rect::Intersect(rectX, rectDst, rectSrc))
        {
            // aspect ratio
            double dAspect = (double)rectSrc.Width / (double)rectSrc.Height;

            // Determine shrink factor along each axis. The axis shrinking most (smaller ratio) is selected as the primary.
            double dX = (double)rectX.Width / (double)rectSrc.Width;
            double dY = (double)rectX.Height / (double)rectSrc.Height;
            if (dX < dY)
            {
                // Shrink along X-axis is greater, fix the Width and adjust destination Height using maintained aspect ratio.
                rectNew.Width = rectX.Width;

                _ASSERT(dAspect > 0); // negative is not expected here 
                
                // protect from div-by-0
                if ((dAspect - 0) > 1e-12) 
                {
                    rectNew.Height = static_cast<INT>(rectDst.Width / dAspect);
                }

                // ... centering along Y if necessary.
                int ofsY = (rectDst.Height - rectNew.Height) / 2;
                rectNew.Offset(0, (ofsY > 0 && ofsY < rectDst.Height) ? ofsY : 0);
            }
            else
            {
                // Shrink along Y-axis is greater, fix the Height and adjust destination Width using maintained aspect ratio.
                rectNew.Height = rectX.Height;
                rectNew.Width = static_cast<INT>(rectDst.Height * dAspect);

                // ... centering along X if necessary.
                int ofsX = (rectDst.Width - rectNew.Width) / 2;
                rectNew.Offset((ofsX > 0 && ofsX < rectDst.Width) ? ofsX : 0, 0);
            }
        }
    }
}

void UpdateImageDisplay( HWND hDlg, CItemData* pItem )
{
    CComPtr<IStream> spIStream;
    Image* pImage = NULL;
    HGLOBAL hMem = NULL;
    LPVOID pvMem = NULL;
    
    if( pItem->m_pMetaData )
    {
        WMDM_TAG_DATATYPE tagType;
        BYTE* pb = NULL;
        UINT cb = 0;
        HRESULT hr = pItem->m_pMetaData->QueryByName(g_wszWMDMAlbumCoverData, &tagType, &pb, &cb);
        ExitOnFail(hr);
        
        hMem = GlobalAlloc(GHND, cb);
        FailOnNull(hMem, E_OUTOFMEMORY);

        pvMem = GlobalLock(hMem);
        FailOnNull(pvMem, E_OUTOFMEMORY);
        
        memcpy(pvMem, pb, cb);
        
        hr = CreateStreamOnHGlobal(hMem, TRUE, &spIStream);
        ExitOnFail(hr);
       
        pImage = Image::FromStream(spIStream);
        FailOnNull(pImage, HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
       
        Status stat = pImage->GetLastStatus();
        FailOnFalse(stat == Ok, HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
        {
            UpdateActualImageStats(hDlg, pImage->GetWidth(), pImage->GetHeight());
                
            PAINTSTRUCT ps;
            HWND hwndAlbumArt = GetDlgItem(hDlg, IDC_ALBUMIMAGE);
            FailOnNull(hwndAlbumArt, E_UNEXPECTED);

            RECT rcWindow;
            if (GetWindowRect(hwndAlbumArt, &rcWindow))
            {
                HDC hdc = BeginPaint(hwndAlbumArt, &ps);
                Graphics graphics(hdc);
                Rect rectDst(0, 0, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top);
                Rect rectImage(0, 0, pImage->GetWidth(), pImage->GetHeight());
                Rect rectNew(rectDst);
                CenterImageAndPreserveAspect(rectNew, rectDst, rectImage);
                graphics.DrawImage(pImage, rectNew);                    
                EndPaint(hwndAlbumArt, &ps);
            }
        }
    }

lExit:
    if (pImage)
    {
        delete pImage;
    }
    if (hMem != NULL)
    {
        if (NULL != pvMem)
        {
            GlobalUnlock(hMem);
        }
        GlobalFree(hMem);
    }
}

void UpdateReferenceList( HWND hDlg, CItemData* pItem )
{
    HRESULT hr = S_OK;
    IWMDMStorage* pStorage = NULL;
    IWMDMStorage4* pPlaylist = NULL;
    IWMDMStorage** ppReferences = NULL;
    DWORD dwRefs = 0;
    DWORD i;
    
    ExitOnNull(pItem);
    
    ListBox_AddString(GetDlgItem(hDlg, IDC_REFERENCES), L"This item contains no references.");
   
    if (pItem->m_fIsDevice || (NULL == pItem->m_pStorage))
    {
        return;
    }

    hr = pItem->m_pStorage->QueryInterface(IID_IWMDMStorage4, reinterpret_cast<void**>(&pPlaylist));
    ExitOnFail(hr);
    
    hr = pPlaylist->GetReferences(&dwRefs, &ppReferences);
    ExitOnFail(hr);

    ListBox_ResetContent(GetDlgItem(hDlg, IDC_REFERENCES));

    for (i = 0; i < dwRefs; i++)
    {
        WCHAR wszName[MAX_PATH] = L"";
        if (NULL != ppReferences[i])
        {
            hr = ppReferences[i]->GetName(wszName, MAX_PATH);
            if (SUCCEEDED(hr))
            {
                ListBox_AddString(GetDlgItem(hDlg, IDC_REFERENCES), wszName);
            }
        }
    }
    
lExit:
    SafeRelease(pPlaylist);

    if (ppReferences)
    {
        for (i = 0; i < dwRefs; i++)
        {
            SafeRelease(ppReferences[i]);
        }
        ::CoTaskMemFree(ppReferences);
    }
}


// Update storage capabilities values in storage properties dialog
void UpdateCapabilities( HWND hDlg, CItemData* pDevice )
{
    WCHAR    pszCap[MAX_PATH];

    static SType_String sTypeStringArray[] = {
        { WMDM_FILE_ATTR_CANPLAY         , L"Play" },
        { WMDM_FILE_ATTR_CANDELETE       , L"Delete" },
        { WMDM_FILE_ATTR_CANMOVE         , L"Move" },
        { WMDM_FILE_ATTR_CANRENAME       , L"Rename" },
        { WMDM_FILE_ATTR_CANREAD         , L"Read" },
        { 0, NULL },
    };

    // Add all the attributes reported by the device as to the string.
    pszCap[0] = '\0';
    for( int iIndex = 0; sTypeStringArray[iIndex].dwType != 0; iIndex++ )
    {
        // Is this bit set, if it is then add the capability as a string
        if( sTypeStringArray[iIndex].dwType & pDevice->m_dwAttributes )
        {
            if( wcslen(pszCap) )
            {
                wcscat_s( pszCap, ARRAYSIZE(pszCap), L", " );
            }
            wcscat_s( pszCap, ARRAYSIZE(pszCap), sTypeStringArray[iIndex].pszString );
        }
    }

    SetDlgItemText( hDlg, IDC_CAPABILITIES, (wcslen(pszCap) ? pszCap : L"<none>") );
}

// Add the storage icon to the storage properties dialog
void UpdateStorageIcon( HWND hDlg, CItemData* pDevice )
{
    HICON   hIcon;
    BOOL    bIsDirectory;

    bIsDirectory = pDevice->m_dwAttributes & WMDM_FILE_ATTR_FOLDER;
    hIcon = GetShellIcon( pDevice->m_szName, bIsDirectory );
    ::SendMessage(hDlg, WM_SETICON, FALSE, (LPARAM)hIcon );
}

// Procedure for simple storage property dialog box
INT_PTR CALLBACK StoragePropSimple_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static CItemData*  pItem = NULL;
    
    switch (msg) 
    {
        case WM_INITDIALOG:
        {
            pItem = (CItemData*)lParam;
            if( pItem == NULL ) return FALSE;

            UpdateSerialId( hDlg, pItem );
            UpdateAttributes( hDlg, pItem );
            UpdateCapabilities( hDlg, pItem );
            UpdateFormatCode( hDlg, pItem );
            UpdateStorageIcon( hDlg, pItem );
            UpdateStatus( hDlg, pItem );
            UpdateCaption( hDlg, pItem );
            UpdateReferenceList( hDlg, pItem);
            CenterWindow(hDlg, g_hwndMain );

            return TRUE;
        }
        break;
        
        case WM_COMMAND:
            if( GET_WM_COMMAND_ID(wParam, lParam) == IDOK ||
                GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL) 
            {
                EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
                return TRUE;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;

}

// Procedure for album storage property dialog box
INT_PTR CALLBACK StoragePropAlbum_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static CItemData*  pItem = NULL;
    
    switch (msg) 
    {
        case WM_INITDIALOG:
        {
            pItem = (CItemData*)lParam;
            if( pItem == NULL ) return FALSE;

            UpdateSerialId( hDlg, pItem );
            UpdateAttributes( hDlg, pItem );
            UpdateCapabilities( hDlg, pItem );
            UpdateFormatCode( hDlg, pItem );
            UpdateAlbumCoverProperties( hDlg, pItem );
            UpdateStorageIcon( hDlg, pItem );
            UpdateStatus( hDlg, pItem );
            UpdateCaption( hDlg, pItem );
            UpdateReferenceList( hDlg, pItem);
            CenterWindow(hDlg, g_hwndMain );

            return TRUE;
        }
        break;
        
        case WM_PAINT:
            UpdateImageDisplay( hDlg, pItem );
        break;
        
        case WM_COMMAND:
            if( GET_WM_COMMAND_ID(wParam, lParam) == IDOK ||
                GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL) 
            {
                EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
                return TRUE;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;
}


#if 0
void UpdateAdvancedStorageProperties(HWND hDlg, CItemData* pItem)
{
    if(pItem->m_pMetaData)
    {
        UINT cItems = 0;
        hr = pItem->m_pMetaData->GetItemCount(&cItems);
        ExitOnFail(hr);
        for (UINT i = 0; i < cItems; i++)
        {
            WCHAR szBuf[MAX_PATH] = {L""};
            WCHAR* pszName = NULL;
            BYTE* pb = NULL;
            WMDM_TAG_DATATYPE type = WMDM_TYPE_BINARY;
            UINT cb = 0;
            
            hr = pItem->m_pMetaData->QueryByIndex(i, &pszName, &type, &pb, &cb);
            if (FAILED(hr))
            {
                continue;
            }

            switch(type)
            {
                case WMDM_TYPE_DWORD:
                    wsprintf(szBuf, L"0x%08x", *((DWORD*)pb));
                break;

                case WMDM_TYPE_STRING:
                    wsprintf(szBuf, L"%s", (LPWSTR)pb);
                break;

                case WMDM_TYPE_BINARY:
                {
                    WORD* pw = (WORD*)pb;
                    wsprintf(szBuf, L"len=%d: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x", 
                                pw[0], pw[1], pw[2], pw[3], pw[4], pw[5], pw[6], pw[7]);
                }
                break;

                case WMDM_TYPE_BOOL:
                break;

                case WMDM_TYPE_QWORD:
                    wsprintf(szBuf, L"%ul64x", *((QWORD*)pb));
                break;

                case WMDM_TYPE_WORD:
                    wsprintf(szBuf, L"0x%04x", *((WORD*)pb));
                break;

                case WMDM_TYPE_GUID:
                break;

                case WMDM_TYPE_DATE:
                break;
            }
        }
    }
}

void UpdateAdvancedDeviceProperties(HWND hDlg, IWMDMDevice *pDevice)
{
    CComPtr<IWMDMDevice3> spIDevice3;
    PROPVARIANT propvar;
    PropVariantInit (&propvar);
    if (SUCCEEDED(pDevice->QueryInterface (IID_IWMDMDevice3, reinterpret_cast<void**>(&spIDevice3))))
    {
        if (SUCCEEDED(spIDevice3->GetProperty(g_wszWMDMDeviceFriendlyName, &propvar)))
        {
            PropVariantClear (&propvar);
        }

        if (SUCCEEDED(spIDevice3->GetProperty(g_wszWMDMSupportedDeviceProperties, &propvar)))
        {
            UINT cProps = propvar.parray->rgsabound[0].cElements;
            for (UINT i = 0; i < cProps; i++)
            {
                BSTR str = ((BSTR*)propvar.parray->pvData)[i];
            }
        }

        if (SUCCEEDED(spIDevice3->GetProperty(g_wszWMDMFormatsSupported, &propvar)))
        {  
            UINT cFmts = propvar.parray->rgsabound[0].cElements;
            for (UINT i = 0; i < cFmts; i++)
            {
                DWORD dw = ((DWORD*)propvar.parray->pvData)[i];
            }
        }

        WMDM_FORMAT_CAPABILITY fmtCaps = {0};
        if (SUCCEEDED(spIDevice3->GetFormatCapability(WMDM_FORMATCODE_UNDEFINEDWINDOWSEXECUTABLE, &fmtCaps)))
        {
            
            // Still TODO: Display, then free this structure
            
            UINT n = fmtCaps.nPropConfig;
        }
    }
    PropVariantClear (&propvar);
}
    
#endif


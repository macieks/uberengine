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
// ItemData.cpp: implementation of the CItemData class
//

// Includes
//
#include "appPCH.h"
#include "SCClient.h"
#include <atlbase.h>

#define WMDM_ICON_FILE_NAME L"Device.ico"
#define WMDM_ICON_INDEX 0

// Opaque Command to get extended certification information
//
// GUID = {C39BF696-B776-459c-A13A-4B7116AB9F09}
//
static const GUID guidCertInfoEx = 
{ 0xc39bf696, 0xb776, 0x459c, { 0xa1, 0x3a, 0x4b, 0x71, 0x16, 0xab, 0x9f, 0x9 } };

typedef struct
{
    HRESULT hr;
    DWORD   cbCert;
    BYTE    pbCert[1];

} CERTINFOEX;

static const BYTE bCertInfoEx_App[] =
{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

static const BYTE bCertInfoEx_SP[] =
{ 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
  0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

//
// Construction/Destruction
//

CItemData::CItemData()
{
    m_fIsDevice          = TRUE;

    // Shared device/storage members
    //
    m_pStorageGlobals    = NULL;
    m_pEnumStorage       = NULL;

    m_szName[0]          = 0;

    // Device-only members
    //
    m_pDevice            = NULL;
    m_pRootStorage       = NULL;
    m_dwType             = 0;
    FillMemory( (void *)&m_SerialNumber, sizeof(m_SerialNumber), 0 );
    m_szMfr[0]           = 0;
    m_szModelName[0]     = 0;
    m_szFirmwareVer[0]   = 0;
    m_dwVersion          = 0;
    m_dwPowerSource      = 0;
    m_dwPercentRemaining = 0;
    m_hIcon              = NULL;
    m_dwMemSizeKB        = 0;
    m_dwMemBadKB         = 0;
    m_dwMemFreeKB        = 0;
    m_fExtraCertified    = FALSE;

    // Storage-only members
    //
    m_pStorage           = NULL;
    m_dwAttributes       = 0;
    FillMemory( (void *)&m_Format, sizeof(m_Format), 0 );
    FillMemory( (void *)&m_DateTime, sizeof(m_DateTime), 0 );
    m_dwSizeLow          = 0;
    m_dwSizeHigh         = 0;
    m_pMetaData          = NULL;

    ZeroMemory(&m_deviceProtocol, sizeof(m_deviceProtocol));
    ZeroMemory(&m_deviceSPVendor, sizeof(m_deviceSPVendor));
}

CItemData::~CItemData()
{
    if( m_hIcon )
    {
        DestroyIcon( m_hIcon );
        m_hIcon = NULL;
    }

    SafeRelease( m_pMetaData);
    SafeRelease( m_pStorageGlobals );
    SafeRelease( m_pEnumStorage );
    SafeRelease( m_pRootStorage );
    SafeRelease( m_pStorage );
    SafeRelease( m_pDevice );
}

//////////////////////////////////////////////////////////////////////
//
// Class methods
//
//////////////////////////////////////////////////////////////////////
HRESULT UtilGetIconStorage (IWMDMDevice * pIDevice, IWMDMStorage ** ppIIconStg)
{
    HRESULT hr = S_OK;
    CComPtr<IWMDMEnumStorage> spIRootEnumerator;
    CComPtr<IWMDMStorage> spIRootStg;
    CComPtr<IWMDMStorage2> spIRootStg2;
    ULONG celt = 0;
    
    CARg (pIDevice && ppIIconStg);

    *ppIIconStg = NULL;

    //
    //Get the enumerator for root storages
    //
    hr = pIDevice->EnumStorage(&spIRootEnumerator);
    CORg(hr);

    //
    //Get the first root storage, this will contain the icon file
    //
    hr = spIRootEnumerator->Next(1, &spIRootStg, &celt);
    CORg(hr);

    if ((S_OK != hr) || (spIRootStg == NULL) || (celt == 0))
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND); //no root storage found
        goto Error;
    }

    //
    //Get the storage object for the icon file
    //
    hr = spIRootStg->QueryInterface(__uuidof(IWMDMStorage2), reinterpret_cast<void**>(&spIRootStg2));
    CORg(hr);
    
    hr = spIRootStg2->GetStorage(WMDM_ICON_FILE_NAME, ppIIconStg);
    CORg(hr);

    if ((S_OK != hr) || ((*ppIIconStg) == NULL))
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND); //no icon file found
        goto Error;
    }
Error:
    if (FAILED(hr) && ppIIconStg)
    {
        *ppIIconStg = NULL;
    }
    return hr;
    
}

HRESULT UtilGetIconFromStorage (IWMDMStorage * pIIconStg, HICON * phIcon)
{
    HRESULT hr = S_OK;
    DWORD dwRet = 0;
    BOOL bRet = 0;
    WCHAR wszTempPath[MAX_PATH] = L"";
    WCHAR wszTempFileName[MAX_PATH] = L"";
    CComPtr<IWMDMStorageControl> spIStgCtrl;

    CARg (pIIconStg && phIcon);

    *phIcon = NULL;
    //
    //Create temp file into which we will copy icon file located on the device
    //
    dwRet = GetTempPath(ARRAYSIZE(wszTempPath), wszTempPath);
    CWRg(dwRet);

    dwRet = GetTempFileName(wszTempPath, L"DeviceIcon", 0, wszTempFileName);
    CWRg(dwRet);

    //
    //Read icon storage object into the temp file we just created
    //
    hr = pIIconStg->QueryInterface (__uuidof(IWMDMStorageControl), reinterpret_cast<void**>(&spIStgCtrl));
    CORg(hr);

    hr = spIStgCtrl->Read(WMDM_MODE_BLOCK | WMDM_CONTENT_FILE, wszTempFileName, NULL, NULL);
    CORg(hr);

    //
    //Extract the icon from temp file
    //
    HICON hIcon = ExtractIcon(g_hInst, wszTempFileName, WMDM_ICON_INDEX);
    CWRg (NULL != hIcon);

    bRet = DeleteFile(wszTempFileName);
    _Assert (bRet);

    *phIcon = hIcon;
Error:
    if (wszTempPath[0]) //GetTempFileName creates the file when it returns the name (with the flags we pass)
    {
        bRet = DeleteFile(wszTempFileName);        
        _Assert (bRet);
    }
    
    if (FAILED(hr) && phIcon)
    {
        *phIcon = NULL;
    }
    return hr;    
}


HRESULT UtilGetDeviceIcon(IWMDMDevice * pIDevice, HICON *phIcon)
{
    HRESULT hr = S_OK;
    CComPtr<IWMDMStorage> spIIconStg;

    CARg(pIDevice && phIcon);

    *phIcon = NULL;

    //
    //Get the storage corresponding to the icon file
    //
    hr = UtilGetIconStorage(pIDevice, &spIIconStg);
    CORg(hr);

    //
    //Get the icon from icon storage
    //
    hr = UtilGetIconFromStorage(spIIconStg, phIcon);
Error:
    if (FAILED(hr) && phIcon)
    {
        *phIcon = NULL;
    }
    return hr;    
}

HRESULT CItemData::Init( IWMDMDevice *pDevice )
{
    HRESULT hr;
    ULONG   ulFetched;
    CComPtr<IWMDMDevice3> spIDevice3;
    PROPVARIANT propvar;

    // This is a device object
    //
    m_fIsDevice = TRUE;

    //
    // Shared device/storage members
    //

    // Get the RootStorage, SotrageGlobals, and EnumStorage interfaces
    //
    m_pRootStorage    = NULL;
    m_pEnumStorage    = NULL;
    m_pStorageGlobals = NULL;

    {
        IWMDMEnumStorage *pEnumRootStorage;

        hr = pDevice->EnumStorage( &pEnumRootStorage );
        ExitOnFalse( SUCCEEDED( hr ) && pEnumRootStorage );

        hr = pEnumRootStorage->Next( 1, &m_pRootStorage, &ulFetched );
        ExitOnFalse( SUCCEEDED( hr ) && m_pRootStorage );

        hr = m_pRootStorage->GetStorageGlobals( &m_pStorageGlobals );
        ExitOnFalse( SUCCEEDED( hr ) && m_pStorageGlobals );

        hr = m_pRootStorage->EnumStorage( &m_pEnumStorage );
        ExitOnFalse( SUCCEEDED( hr ) && m_pEnumStorage );

        pEnumRootStorage->Release();
    }

    // Get device name
    //
    hr = pDevice->GetName( m_szName, sizeof(m_szName)/sizeof(m_szName[0]) - 1 );
    if( FAILED(hr) )
    {
        lstrcpy( m_szName, L"" );
    }

    //
    // Device-only members
    //

    // Set the device pointer and addref it
    //
    m_pDevice = pDevice;
    m_pDevice->AddRef();

    // Get device type
    //
    hr = pDevice->GetType( &m_dwType );
    if( FAILED(hr) )
    {
        m_dwType = 0L;
    }

    /// Get device serial number
    //
    BYTE abMAC[SAC_MAC_LEN];
    BYTE abMACVerify[SAC_MAC_LEN];
    HMAC hMACVerify;

    hr = pDevice->GetSerialNumber( &m_SerialNumber, (BYTE*)abMAC );
    if( SUCCEEDED(hr) )
    {
        g_cWmdm.m_pSAC->MACInit(&hMACVerify);
        g_cWmdm.m_pSAC->MACUpdate(hMACVerify, (BYTE*)(&m_SerialNumber), sizeof(m_SerialNumber));
        g_cWmdm.m_pSAC->MACFinal(hMACVerify, (BYTE*)abMACVerify);
        if( memcmp(abMACVerify, abMAC, sizeof(abMAC)) != 0 )
        {
            hr = E_FAIL;
        }
    }
    if( FAILED(hr) )
    {
        FillMemory( (void *)&m_SerialNumber, sizeof(m_SerialNumber), 0 );
    }

    // Get device manufacturer
    //
    hr = pDevice->GetManufacturer( m_szMfr, sizeof(m_szMfr)/sizeof(m_szMfr[0]) - 1 );
    if( FAILED(hr) )
    {
        lstrcpy( m_szMfr, L"" );
    }

    // Get device version
    //
    hr = pDevice->GetVersion( &m_dwVersion );
    if( FAILED(hr) )
    {
        m_dwVersion = (DWORD)-1;
    }

    // Get power source and power remaining
    //
    hr = pDevice->GetPowerSource( &m_dwPowerSource, &m_dwPercentRemaining );
    if( FAILED(hr) ) 
    {
        m_dwPowerSource      = 0;
        m_dwPercentRemaining = 0;
    }

    // Get device icon
    //
    hr = pDevice->GetDeviceIcon( (ULONG *)&m_hIcon );
    if( FAILED(hr) )
    {
        hr = UtilGetDeviceIcon(pDevice, &m_hIcon);
        if (FAILED(hr))
        {
            m_hIcon = NULL;
        }
    }

    //
    //Get device properties through GetProperty method
    //
    PropVariantInit (&propvar);
    hr = pDevice->QueryInterface (__uuidof(IWMDMDevice3), reinterpret_cast<void**>(&spIDevice3));
    if (SUCCEEDED (hr))
    {
        //
        //Get g_wszWMDMDeviceVendorExtension to set m_bstrVendorExtDesc
        //
        hr = spIDevice3->GetProperty (g_wszWMDMDeviceVendorExtension, &propvar);
        if (SUCCEEDED (hr) && (VT_BSTR == propvar.vt))
        {
            m_sbstrVendorExtDesc.Attach(propvar.bstrVal);
        }
        propvar.vt = VT_EMPTY; //since we have transferred BSTR ownsership to m_bstrVendorExtDesc;

        //
        //Get g_wszWMDMDeviceProtocol to set m_deviceProtocol
        //
        hr = spIDevice3->GetProperty(g_wszWMDMDeviceProtocol, &propvar);
        if (SUCCEEDED(hr) && (VT_CLSID == propvar.vt))
        {
            m_deviceProtocol = *propvar.puuid;
        }
        PropVariantClear(&propvar); //clear the propvariant obtained
        
        //
        //Get g_wszWMDMDeviceServiceProviderVendor to set m_deviceSPVendor
        //
        hr = spIDevice3->GetProperty(g_wszWMDMDeviceServiceProviderVendor, &propvar);
        if (SUCCEEDED(hr) && (VT_CLSID == propvar.vt))
        {
            m_deviceSPVendor= *propvar.puuid;
        }
        PropVariantClear(&propvar); //clear the propvariant obtained

        //
        //Get g_wszWMDMDeviceModelName to set m_szModelName
        //
        hr = spIDevice3->GetProperty(g_wszWMDMDeviceModelName, &propvar);
        if (SUCCEEDED(hr) && (VT_BSTR == propvar.vt))
        {
            StringCchCopy(m_szModelName, ARRAYSIZE(m_szModelName), propvar.bstrVal);
        }
        PropVariantClear(&propvar); //clear the propvariant obtained

        //
        //Get g_wszWMDMDeviceFirmwareVersion to set m_szFirmwareVer
        //
        hr = spIDevice3->GetProperty(g_wszWMDMDeviceFirmwareVersion, &propvar);
        if (SUCCEEDED(hr) && (VT_BSTR == propvar.vt))
        {
            StringCchCopy(m_szFirmwareVer, ARRAYSIZE(m_szFirmwareVer), propvar.bstrVal);
        }
        PropVariantClear(&propvar); //clear the propvariant obtained
    }

    // Get the total, free, and bad space on the storage
    //
    {
        DWORD dwLow;
        DWORD dwHigh;

        m_dwMemSizeKB = 0;
        hr = m_pStorageGlobals->GetTotalSize( &dwLow, &dwHigh );
        if( SUCCEEDED(hr) )
        {
            INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

            m_dwMemSizeKB = (DWORD)nSize;
        }

        m_dwMemBadKB = 0;
        hr = m_pStorageGlobals->GetTotalBad( &dwLow, &dwHigh );
        if( SUCCEEDED(hr) )
        {
            INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

            m_dwMemBadKB = (DWORD)nSize;
        }

        m_dwMemFreeKB = 0;
        hr = m_pStorageGlobals->GetTotalFree( &dwLow, &dwHigh );
        if( SUCCEEDED(hr) )
        {
            INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

            m_dwMemFreeKB = (DWORD)nSize;
        }
    }

    // Call opaque command to exchange extended uethentication info
    //
    {
        HMAC           hMAC;
        OPAQUECOMMAND  Command;
        CERTINFOEX    *pCertInfoEx;
        DWORD          cbData_App   = sizeof( bCertInfoEx_App )/sizeof( bCertInfoEx_App[0] );
        DWORD          cbData_SP    = sizeof( bCertInfoEx_SP )/sizeof( bCertInfoEx_SP[0] );
        DWORD          cbData_Send  = sizeof( CERTINFOEX ) + cbData_App;

        // Fill out opaque command structure
        //
        memcpy( &(Command.guidCommand), &guidCertInfoEx, sizeof(GUID) );

        Command.pData = (BYTE *)CoTaskMemAlloc( cbData_Send );
        if( !Command.pData )
        {
            ExitOnFail( hr = E_OUTOFMEMORY );
        }
        Command.dwDataLen = cbData_Send;

        // Map the data in the opaque command to a CERTINFOEX structure, and
        // fill in the cert info to send
        //
        pCertInfoEx = (CERTINFOEX *)Command.pData;

        pCertInfoEx->hr     = S_OK;
        pCertInfoEx->cbCert = cbData_App;
        memcpy( pCertInfoEx->pbCert, bCertInfoEx_App, cbData_App );

        // Compute MAC
        //
        g_cWmdm.m_pSAC->MACInit( &hMAC );
        g_cWmdm.m_pSAC->MACUpdate( hMAC, (BYTE*)(&(Command.guidCommand)), sizeof(GUID) );
        g_cWmdm.m_pSAC->MACUpdate( hMAC, (BYTE*)(&(Command.dwDataLen)), sizeof(Command.dwDataLen) );
        if( Command.pData )
        {
            g_cWmdm.m_pSAC->MACUpdate( hMAC, Command.pData, Command.dwDataLen );
        }
        g_cWmdm.m_pSAC->MACFinal( hMAC, Command.abMAC );

        // Send the command
        //
        hr = pDevice->SendOpaqueCommand( &Command );
        if( SUCCEEDED(hr) )
        {
            BYTE abMACVerify2[ WMDM_MAC_LENGTH ];

            // Compute MAC
            //
            g_cWmdm.m_pSAC->MACInit( &hMAC );
            g_cWmdm.m_pSAC->MACUpdate( hMAC, (BYTE*)(&(Command.guidCommand)), sizeof(GUID) );
            g_cWmdm.m_pSAC->MACUpdate( hMAC, (BYTE*)(&(Command.dwDataLen)), sizeof(Command.dwDataLen) );
            if( Command.pData )
            {
                g_cWmdm.m_pSAC->MACUpdate( hMAC, Command.pData, Command.dwDataLen );
            }
            g_cWmdm.m_pSAC->MACFinal( hMAC, abMACVerify2 );

            // Verify MAC matches
            //
            if( memcmp(abMACVerify2, Command.abMAC, WMDM_MAC_LENGTH) == 0 )
            {
                // Map the data in the opaque command to a CERTINFOEX structure
                //
                pCertInfoEx = (CERTINFOEX *)Command.pData;

                // In this simple extended uethentication scheme, the callee must
                // provide the exact cert info
                //
                if( (pCertInfoEx->cbCert != cbData_SP) ||
                    (memcmp(pCertInfoEx->pbCert, bCertInfoEx_SP, cbData_SP) == 0) )
                {
                    m_fExtraCertified = TRUE;
                }
            }
        }

        if( Command.pData )
        {
            CoTaskMemFree( Command.pData );
        }
    }

    //
    // Storage-only members (pointers/handles only)
    //

    m_pStorage = NULL;

    // 
    // Successful init
    //

    hr = S_OK;

lExit:

    return hr;
}

HRESULT CItemData::Init( IWMDMStorage *pStorage )
{
    HRESULT hr;
    IWMDMStorage3* pStg3 = NULL;

    // This is a storage object
    //
    m_fIsDevice = FALSE;

    //
    // Shared device/storage members
    //

    // Get a pointer to the StorageGlobals interface
    //
    hr = pStorage->GetStorageGlobals( &m_pStorageGlobals );
    ExitOnFail( hr );

    // Get the storage attributes
    //
    hr = pStorage->GetAttributes( &m_dwAttributes, &m_Format );
    if( FAILED(hr) )
    {
        m_dwAttributes = 0;
    }

    // Get a pointer to the EnumStorage interface
    //
    if( m_dwAttributes & WMDM_FILE_ATTR_FOLDER )
    {
        hr = pStorage->EnumStorage( &m_pEnumStorage );
        ExitOnFail( hr );
    }
    else
    {
        m_pEnumStorage = NULL;
    }

    // Get the storage name
    //
    hr = pStorage->GetName( m_szName, sizeof(m_szName)/sizeof(m_szName[0]) - 1 );
    if( FAILED(hr) )
    {
        lstrcpy( m_szName, L"" );
    }

    /// Get storage serial number
    //
    BYTE abMAC[SAC_MAC_LEN];
    BYTE abMACVerify[SAC_MAC_LEN];
    HMAC hMAC;

    hr = m_pStorageGlobals->GetSerialNumber( &m_SerialNumber, (BYTE*)abMAC );
    if( SUCCEEDED(hr) )
    {
        g_cWmdm.m_pSAC->MACInit(&hMAC);
        g_cWmdm.m_pSAC->MACUpdate(hMAC, (BYTE*)(&m_SerialNumber), sizeof(m_SerialNumber));
        g_cWmdm.m_pSAC->MACFinal(hMAC, (BYTE*)abMACVerify);
        if( memcmp(abMACVerify, abMAC, sizeof(abMAC)) != 0 )
        {
            hr = E_FAIL;
        }
    }
    if( FAILED(hr) )
    {
        FillMemory( (void *)&m_SerialNumber, sizeof(m_SerialNumber), 0 );
    }



    //
    // Device-only members (pointers/handles only)
    //

    m_pDevice         = NULL;
    m_pRootStorage    = NULL;
    m_hIcon           = NULL;
    m_fExtraCertified = FALSE;

    //
    // Storage-only members
    //

    // Save the WMDM storage pointer
    //
    m_pStorage = pStorage;
    m_pStorage->AddRef();

    // Get the storage date
    //
    hr = pStorage->GetDate( &m_DateTime );
    if( FAILED(hr) )
    {
        FillMemory( (void *)&m_DateTime, sizeof(m_DateTime), 0 );
    }

    // If the stoarge is a file, get its size
    // If the storage is a folder, set the size to zero
    //
    m_dwSizeLow  = 0;
    m_dwSizeHigh = 0;
    m_pMetaData = NULL;
    m_dwFormatCode = WMDM_FORMATCODE_NOTUSED;
    if( !(m_dwAttributes & WMDM_FILE_ATTR_FOLDER) )
    {
        hr = pStorage->GetSize( &m_dwSizeLow, &m_dwSizeHigh );
        
        HRESULT hr1 = pStorage->QueryInterface( IID_IWMDMStorage3, (void**)&pStg3 );
        if(SUCCEEDED(hr1))
        {
            if(SUCCEEDED(pStg3->GetMetadata(&m_pMetaData) ))
            {
                WMDM_TAG_DATATYPE dt;
                BYTE* pb;
                UINT cb;
                if (SUCCEEDED(m_pMetaData->QueryByName(g_wszWMDMFormatCode, &dt, &pb, &cb)))
                {
                    if( sizeof(DWORD) == cb )
                    {
                        m_dwFormatCode = *(reinterpret_cast<DWORD*>(pb));
                    }
                }
            }
        }
        else
        {
            hr = hr1;
        }
    }

    // 
    // Successful init
    //

    hr = S_OK;

lExit:
    SafeRelease(pStg3);
    return hr;
}

BOOL CItemData::IsAlbum()
{
    return (m_dwFormatCode == WMDM_FORMATCODE_UNDEFINEDCOLLECTION ||
            m_dwFormatCode == WMDM_FORMATCODE_ABSTRACTMULTIMEDIAALBUM || 
            m_dwFormatCode == WMDM_FORMATCODE_ABSTRACTIMAGEALBUM ||
            m_dwFormatCode == WMDM_FORMATCODE_ABSTRACTAUDIOALBUM ||
            m_dwFormatCode == WMDM_FORMATCODE_ABSTRACTVIDEOALBUM ||
            m_dwFormatCode == WMDM_FORMATCODE_ABSTRACTCONTACTGROUP ||
            m_dwFormatCode == WMDM_FORMATCODE_ABSTRACTMESSAGEFOLDER ||
            m_dwFormatCode == WMDM_FORMATCODE_ABSTRACTCHAPTEREDPRODUCTION);
}

HRESULT CItemData::Refresh( void )
{
    HRESULT hr;

    // Only valid for a device
    //
    if( !m_fIsDevice )
    {
        ExitOnFail( hr = E_UNEXPECTED );
    }

    // Get power source and power remaining
    //
    hr = m_pDevice->GetPowerSource( &m_dwPowerSource, &m_dwPercentRemaining );
    if( FAILED(hr) ) 
    {
        m_dwPowerSource      = 0;
        m_dwPercentRemaining = 0;
    }

    // Get the total, free, and bad space on the storage
    //
    {
        DWORD dwLow;
        DWORD dwHigh;

        m_dwMemSizeKB = 0;
        hr = m_pStorageGlobals->GetTotalSize( &dwLow, &dwHigh );
        if( SUCCEEDED(hr) )
        {
            INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

            m_dwMemSizeKB = (DWORD)nSize;
        }

        m_dwMemBadKB = 0;
        hr = m_pStorageGlobals->GetTotalBad( &dwLow, &dwHigh );
        if( SUCCEEDED(hr) )
        {
            INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

            m_dwMemBadKB = (DWORD)nSize;
        }

        m_dwMemFreeKB = 0;
        hr = m_pStorageGlobals->GetTotalFree( &dwLow, &dwHigh );
        if( SUCCEEDED(hr) )
        {
            INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

            m_dwMemFreeKB = (DWORD)nSize;
        }
    }

    hr = S_OK;

lExit:

    return hr;
}

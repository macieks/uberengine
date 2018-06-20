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


//
// WMDM.cpp: implementation of the CWMDM class.
//

// Includes
//
#include "appPCH.h"
#include "mswmdm_i.c"
#include "sac.h"
#include "SCClient.h"

#include "key.c"

#include "NotificationHandler.h"

#include <atlbase.h>
//////////////////////////////////////////////////////////////////////
//
// Construction/Destruction
//
//////////////////////////////////////////////////////////////////////

CWMDM::CWMDM()
{
    HRESULT hr;
    CComPtr<IComponentAuthenticate> pAuth = NULL;
    CNotificationHandler * pCallBackObject = NULL;
    // Initialize member variables
    //
    m_pSAC        = NULL;
    m_pWMDevMgr   = NULL;

    // Acquire the uethentication interface of WMDM
    //
    hr = CoCreateInstance(
        CLSID_MediaDevMgr,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IComponentAuthenticate,
        (void**)&pAuth
    );
    ExitOnFail( hr );

    // Create the client uethentication object
    //
    m_pSAC = new CSecureChannelClient;
    ExitOnNull( m_pSAC );

    // Select the cert and the associated private key into the SAC
    //
    hr = m_pSAC->SetCertificate(
        SAC_CERT_V1,
        (BYTE *)abCert, sizeof(abCert),
        (BYTE *)abPVK,  sizeof(abPVK)
    );
    ExitOnFail( hr );
            
    // Select the uethentication interface into the SAC
    //
    m_pSAC->SetInterface( pAuth );

    // Authenticate with the V1 protocol
    //
    hr = m_pSAC->Authenticate( SAC_PROTOCOL_V1 );
    ExitOnFail( hr );

    // Authenticated succeeded, so we can use the WMDM functionality.
    // Acquire an interface to the top-level WMDM interface.
    //
    hr = pAuth->QueryInterface( IID_IWMDeviceManager, (void**)&m_pWMDevMgr );
    ExitOnFail( hr );

    //IMPORTANT: The order is important, first register for notifications and then call EnumDevices
    
    pCallBackObject = new CNotificationHandler;

    if (NULL == pCallBackObject)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = pCallBackObject->QueryInterface(IID_IWMDMNotification, (void**) &m_pICallbackObject);
    }
    ExitOnFail( hr );

    m_dwNotificationCookie = -1;

    _RegisterForNotifications(); //ignoring return value, since it will not be supported on XP version of WMDM

    hr = S_OK;

lExit:

    m_hrInit = hr;
}

void CWMDM::Uninit()
{
    _UnregisterForNotifications();

    // Release the top-level WMDM interface
    //
    if( m_pWMDevMgr )
    {
        m_pWMDevMgr->Release();
        m_pWMDevMgr = NULL;
    }

    // Release the SAC
    //
    if( m_pSAC )
    {
        delete m_pSAC;
        m_pSAC = NULL;
    }

    if (m_pICallbackObject)
    {
        m_pICallbackObject->Release();
        m_pICallbackObject = NULL;
    }
}

CWMDM::~CWMDM()
{
#if 0
    _UnregisterForNotifications();
    // Release the device enumeration interface
    //
    if( m_pEnumDevice )
    {
        m_pEnumDevice->Release();
    }

    // Release the top-level WMDM interface
    //
    if( m_pWMDevMgr )
    {
        m_pWMDevMgr->Release();
    }

    // Release the SAC
    //
    if( m_pSAC )
    {
        delete m_pSAC;
    }
#endif
}

//////////////////////////////////////////////////////////////////////
//
// Class methods
//
//////////////////////////////////////////////////////////////////////

HRESULT CWMDM::Init( void )
{
    return m_hrInit;
}

HRESULT CWMDM::_RegisterForNotifications()
{
    HRESULT hr = S_OK;

    CComPtr<IConnectionPointContainer> spICPC;
    CComPtr<IConnectionPoint> spICP;

    if (SUCCEEDED (hr = m_pWMDevMgr->QueryInterface(IID_IConnectionPointContainer, (void**) & spICPC)))
    {
        if (SUCCEEDED (hr = spICPC->FindConnectionPoint(IID_IWMDMNotification, &spICP)))
        {
            DWORD dwCookie;
            if (SUCCEEDED (hr = spICP->Advise(m_pICallbackObject, &dwCookie)))
            {
                m_dwNotificationCookie = dwCookie;
            }
        }
    }

    return hr;
}

HRESULT CWMDM::_UnregisterForNotifications()
{
    HRESULT hr = S_FALSE;

    if (-1 != m_dwNotificationCookie)
    {
        CComPtr<IConnectionPointContainer> spICPC;
        CComPtr<IConnectionPoint> spICP;

        if (SUCCEEDED (hr = m_pWMDevMgr->QueryInterface(IID_IConnectionPointContainer, (void**) & spICPC)))
        {
            if (SUCCEEDED (hr = spICPC->FindConnectionPoint(IID_IWMDMNotification, &spICP)))
            {
                if (SUCCEEDED (hr = spICP->Unadvise(m_dwNotificationCookie)))
                {
                    m_dwNotificationCookie = -1;
                    hr = S_OK;
                }
            }
        }
    }

    return hr;
}


//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//

#include "appPCH.h"
#include "NotificationHandler.h"
#include "ItemData.h"
#include <atlbase.h>

HRESULT CNotificationHandler::WMDMMessage (/*[in]*/ DWORD dwMessageType, /*[in]*/ LPCWSTR pwszCanonicalName)
{
    HRESULT hr = S_OK;
    
    switch (dwMessageType)
    {
    case WMDM_MSG_DEVICE_ARRIVAL:
        hr = _HandleDeviceArrival(pwszCanonicalName);
        break;
    case WMDM_MSG_DEVICE_REMOVAL:
        hr = _HandleDeviceRemoval(pwszCanonicalName);
        break;
    case WMDM_MSG_MEDIA_ARRIVAL:
        hr = _HandleMediaArrival(pwszCanonicalName);
        break;
    case WMDM_MSG_MEDIA_REMOVAL:
        hr = _HandleMediaRemoval(pwszCanonicalName);
        break;
    }

    return hr;
}

HRESULT CNotificationHandler::_HandleDeviceArrival(LPCWSTR pwszCanonicalName)
{
    HRESULT hr = S_OK;

    CComQIPtr<IWMDeviceManager2, &IID_IWMDeviceManager2> spIWmdm2 = g_cWmdm.m_pWMDevMgr;

    IWMDMDevice * pDevice;

    if (spIWmdm2 && SUCCEEDED (spIWmdm2->GetDeviceFromCanonicalName(pwszCanonicalName, &pDevice)))
    {
        CItemData * pItemDevice = new CItemData;
        if( pItemDevice )
        {
            hr = pItemDevice->Init( pDevice );
            if( SUCCEEDED(hr) )
            {
                g_cDevices.AddItem( pItemDevice );
            }
            else
            {
                delete pItemDevice;
            }
        }
        pDevice->Release();
    }

    return S_OK;
}

HRESULT CNotificationHandler::_HandleDeviceRemoval(LPCWSTR pwszCanonicalName)
{
    g_cDevices.RemoveDevices(pwszCanonicalName);

    return S_OK;
}

HRESULT CNotificationHandler::_HandleMediaArrival(LPCWSTR pwszCanonicalName)
{
    g_cDevices.SetMediaState (pwszCanonicalName, true);
    return S_OK;
}

HRESULT CNotificationHandler::_HandleMediaRemoval(LPCWSTR pwszCanonicalName)
{
    g_cDevices.SetMediaState (pwszCanonicalName, false);
    return S_OK;
}


//IUnknown impl

STDMETHODIMP CNotificationHandler::QueryInterface(REFIID riid, void ** ppvObject)
{
    HRESULT hr = E_INVALIDARG;
    if (NULL != ppvObject)
    {
        *ppvObject = NULL;

        if (IsEqualIID(riid, IID_IWMDMNotification) || IsEqualIID(riid, IID_IUnknown))
        {
            *ppvObject = this;
            AddRef();
            hr = S_OK;
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }
    return hr;
}

ULONG STDMETHODCALLTYPE CNotificationHandler::AddRef()
{
    return ++m_dwRefCount;
}

ULONG STDMETHODCALLTYPE CNotificationHandler::Release()
{
    DWORD dwRetVal = --m_dwRefCount;
    if (0 == dwRetVal)
    {
        delete this;
    }
    return dwRetVal;
}

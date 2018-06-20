//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//

#ifndef __NOTIFICATIONHANDLER_H_
#define __NOTIFICATIONHANDLER_H_

class CNotificationHandler : public IWMDMNotification
{
public:
    CNotificationHandler() {m_dwRefCount = 0;}
    //IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    //IWMDMNotification
    STDMETHOD (WMDMMessage) (/*[in]*/ DWORD dwMessageType, /*[in]*/ LPCWSTR pwszCanonicalName);
private:
    HRESULT _HandleDeviceArrival(LPCWSTR pwszCanonicalName);
    HRESULT _HandleDeviceRemoval(LPCWSTR pwszCanonicalName);
    HRESULT _HandleMediaArrival(LPCWSTR pwszCanonicalName);
    HRESULT _HandleMediaRemoval(LPCWSTR pwszCanonicalName);

private:
    DWORD m_dwRefCount;
};

#endif //__NOTIFICATIONHANDLER_H_
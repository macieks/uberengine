//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

#ifndef DECODE_H
#define DECODE_H

#include <streams.h>
#include <vfw.h>
#include <tchar.h>
#include <dmo.h>
#include "uediodec.h"
#include "videodec.h"

//////////////////////////////////////////////////////////////////////////////
//
// Media buffer helper
//
//////////////////////////////////////////////////////////////////////////////
class CHandlingMediaBuffer : public IMediaBuffer {
    BYTE *m_pData;
    ULONG m_ulSize;
    ULONG m_ulData;
    ULONG m_cRef;
    BYTE bInterlaceProperty;
public:
    CHandlingMediaBuffer(): bInterlaceProperty( 0 ) {
        m_cRef = 1;
    }
    
    void set_Buffer(BYTE *pData, DWORD ulData, DWORD ulSize) {
        m_pData = pData;
        m_ulSize = ulSize;
        m_ulData = ulData;
    }
    
    //
    // IUnknown
    //
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {
        if( NULL == ppv ){
            return ( E_POINTER );
        }

        *ppv = NULL;

        if (riid == IID_IUnknown) {
            AddRef();
            *ppv = /*reinterpret_cast<IUnknown*>*(IUnknown*)( this );
            return ( NOERROR );
        } else if (riid == IID_IMediaBuffer) {
            AddRef();
            *ppv = /*reinterpret_cast<IMediaBuffer*>*/(IMediaBuffer*)( this );
            return ( NOERROR );
        } else {
            return ( E_NOINTERFACE );
        }
    }
    
    STDMETHODIMP_(ULONG) AddRef() {
        return InterlockedIncrement((long*)&m_cRef);
    }
    STDMETHODIMP_(ULONG) Release() {
        long l = InterlockedDecrement((long*)&m_cRef);
        if (l == 0)
            delete this;
        return l;
    }
    
    //
    // IMediaBuffer
    //
    STDMETHODIMP GetLength(DWORD *pulLength ){
        if( NULL == pulLength ){
            return ( E_POINTER );
        }
        *pulLength = m_ulData;
        return ( NOERROR );
    }

    STDMETHODIMP SetLength(DWORD ulLength ){
        m_ulData = ulLength;
        return ( NOERROR );
    }
    
    STDMETHODIMP GetMaxLength(DWORD *pcbMaxLength){
        *pcbMaxLength = m_ulSize;
        return ( NOERROR );
    }
    STDMETHODIMP GetBufferAndLength(BYTE **ppBuffer, DWORD *pcbLength) {
        if( NULL != ppBuffer ){
            *ppBuffer = m_pData;
        }
        if( NULL != pcbLength ){
            *pcbLength = m_ulData;
        }
        return ( NOERROR );
    }
};

//////////////////////////////////////////////////////////////////////////////
struct HandlingContext {
    CHandlingMediaBuffer *pInputBuffer;
    CHandlingMediaBuffer *pOutputBuffer;

    BYTE                 *pbBuffer;
    LONG                 cbBuffer;

    AM_MEDIA_TYPE        mtIn;
    AM_MEDIA_TYPE        mtOut;
 
    PAVISTREAM           pAviInStream;
    PAVISTREAM           pAviOutStream;
    PGETFRAME            pGetFrame;
    DWORD                dwScale;
    DWORD                dwRate;

    IMediaObject         *pDMO;

    REFERENCE_TIME       rtFrameDuration;
    REFERENCE_TIME       rtTimeStamp;
};

//////////////////////////////////////////////////////////////////////////////
//
// General Params Structure
//
//////////////////////////////////////////////////////////////////////////////
struct GeneralParams {
    TCHAR *pszInput;
    TCHAR *pszOutput;
    BOOL  fAudioOnly;
    BOOL  fVideoOnly;

    GeneralParams(){
        memset(this, 0, sizeof( GeneralParams) );
    }
    ~GeneralParams(){
        SAFEFREE(pszInput);
        SAFEFREE(pszOutput);
    }
};

//////////////////////////////////////////////////////////////////////////////
HRESULT        DecodeFile( GeneralParams  *pIOParams,    \
                           AudioDecParams *pAudioParams, \
                           VideoDecParams *pVideoParams  );
#endif /*DECODE_H*/

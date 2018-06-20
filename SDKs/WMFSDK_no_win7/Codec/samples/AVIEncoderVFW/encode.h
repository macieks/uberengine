//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

#ifndef ENCODE_H
#define ENCODE_H

#include <streams.h>
#include <vfw.h>
#include <tchar.h>
#include <dmo.h>
#include <nserror.h>
#include "wmsbuffer.h"
#include "uedioenc.h"
#include "videoenc.h"

//////////////////////////////////////////////////////////////////////////////
//
// Constants used in the dropped frames detection and frame decimation
//
//////////////////////////////////////////////////////////////////////////////

#define   MAX_JITTER_PERCENT       20
#define   JITTER_OFFSET            50000

//////////////////////////////////////////////////////////////////////////////
//
// Media buffer helper
//
//////////////////////////////////////////////////////////////////////////////
EXTERN_GUID( WM_SampleExtension_OutputCleanPoint,
    0xf72a3c6f, 0x6eb4, 0x4ebc, 0xb1, 0x92, 0x9, 0xad, 0x97, 0x59, 0xe8, 0x28 );

EXTERN_GUID( WM_SampleExtension_ContentType,
    0xd590dc20, 0x07bc, 0x436c, 0x9c, 0xf7, 0xf3, 0xbb, 0xfb, 0xf1, 0xa4, 0xdc );

#define WM_SampleExtension_ContentType_Size      1
#define WM_CT_INTERLACED  128

class CHandlingMediaBuffer : public IMediaBuffer
                           , public INSSBuffer3
 {
    BYTE *m_pData;
    ULONG m_ulSize;
    ULONG m_ulData;
    ULONG m_cRef;
    BYTE  m_bInterlaceProperty;
    BOOL  m_fOutputCleanPoint;

public:
    CHandlingMediaBuffer(): m_bInterlaceProperty( 0 ), m_fOutputCleanPoint( FALSE ) {
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
            *ppv = reinterpret_cast<IUnknown*>( this );
            return ( NOERROR );
        } else if (riid == IID_IMediaBuffer) {
            AddRef();
            *ppv = (IMediaBuffer*)( this );
            return ( NOERROR );
        } else if ( riid == IID_INSSBuffer3 ){
            AddRef();
            *ppv = (INSSBuffer3*)( this );
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

    //
    // INSSBufferx members
    //
    HRESULT STDMETHODCALLTYPE GetBuffer( BYTE**  ppdwBuffer ){
        if( NULL == ppdwBuffer ){
            return ( E_POINTER );
        }
        *ppdwBuffer = m_pData;
        return ( S_OK );
    }
    
    HRESULT STDMETHODCALLTYPE GetSampleProperties( DWORD cbProperties,
        BYTE *pbProperties ){
        return ( E_NOTIMPL );
    }
    
    HRESULT STDMETHODCALLTYPE SetSampleProperties( DWORD cbProperties,
        BYTE *pbProperties ){
        return ( E_NOTIMPL );
    }

    HRESULT STDMETHODCALLTYPE GetProperty(  GUID   guidBufferProperty,
        void*  pvBufferProperty,
        DWORD* pdwBufferPropertySize){
        if( NULL == pdwBufferPropertySize ){
            return ( E_POINTER );
        }
#ifdef SUPPORT_INTERLACE            
        if( WM_SampleExtension_ContentType == guidBufferProperty ){
            if( NULL == pvBufferProperty ){
                *pdwBufferPropertySize = WM_SampleExtension_ContentType_Size;
                return ( S_OK );
            } else {
                if( *pdwBufferPropertySize < WM_SampleExtension_ContentType_Size ){
                    return ( E_INVALIDARG );
                } else {
                    *(BYTE*)pvBufferProperty = m_bInterlaceProperty;
                    return ( S_OK );
                }
            }
        }
#endif //SUPPORT_INTERLACE
        if( WM_SampleExtension_OutputCleanPoint == guidBufferProperty ){
            if( NULL == pvBufferProperty ){
                *pdwBufferPropertySize = sizeof( BOOL );
                return ( S_OK );
            } else {
                if( *pdwBufferPropertySize < sizeof( BOOL ) ){
                    return ( E_INVALIDARG );
                } else {
                    *(BOOL*)pvBufferProperty = m_fOutputCleanPoint;
                    return ( S_OK );
                }
            }
        }
        return ( NS_E_UNSUPPORTED_PROPERTY );
    }
    
    HRESULT STDMETHODCALLTYPE SetProperty(  GUID   guidBufferProperty,
        void*  pvBufferProperty,
        DWORD  dwBufferPropertySize){
        if( NULL == pvBufferProperty ){
            return ( E_POINTER );
        }
#ifdef SUPPORT_INTERLACE    
        if( WM_SampleExtension_ContentType == guidBufferProperty ){
            if( dwBufferPropertySize != WM_SampleExtension_ContentType_Size ){
                return ( E_INVALIDARG );
            } else {
                m_bInterlaceProperty = *(BYTE*)pvBufferProperty;
                return ( S_OK );
            }
        }
#endif //SUPPORT_INTERLACE
        if( WM_SampleExtension_OutputCleanPoint == guidBufferProperty ){
            if( dwBufferPropertySize != sizeof( BOOL ) ){
                return ( E_INVALIDARG );
            } else {
                m_fOutputCleanPoint = *(BOOL*)pvBufferProperty;
                return ( S_OK );
            }
        }
        return ( NS_E_UNSUPPORTED_PROPERTY );
    }
};

//////////////////////////////////////////////////////////////////////////////
struct HandlingContext {
    int                  nPasses;
    int                  nCurrentPass;
    BOOL                 fBlockFirstPass;

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
	void                 *pParams;

    REFERENCE_TIME       rtDecimatorTimeStamp;
    REFERENCE_TIME       rtDecimatorDuration;

    REFERENCE_TIME       rtMaxJitter;
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
HRESULT        EncodeFile( GeneralParams  *pIOParams,    \
                           AudioEncParams *pAudioParams, \
                           VideoEncParams *pVideoParams  );
#endif /*ENCODE_H*/


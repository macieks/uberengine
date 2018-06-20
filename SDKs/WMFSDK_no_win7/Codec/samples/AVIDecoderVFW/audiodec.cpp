//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

#include <streams.h>
#include "wmcodecconst.h"
#include "wmcodeciface.h"
#include "decappErr.h"
#include "uediodec.h"


//////////////////////////////////////////////////////////////////////////////
HRESULT MatchAudioParams(IMediaObject* pDMO,      \
                         WAVEFORMATEX   *pwfx,    \
                         AudioDecParams *pParams, \
                         AM_MEDIA_TYPE *pmt       ) {
    WAVEFORMATEX   *wfmt = NULL;
    
    //
    // We'll accept the first mediatype offered
    //
    HRESULT hr = pDMO->GetOutputType(0, 0, pmt);
    if ( FAILED( hr ) ){
         return ( hr );
    }
    if( pmt->formattype != FORMAT_WaveFormatEx   ||
        pmt->pbFormat   == NULL                  ||
        pmt->cbFormat   < sizeof( WAVEFORMATEX ) ){
        return ( E_AUDIO_INVALID_FORMAT );
    }

    wfmt = (WAVEFORMATEX*)pmt->pbFormat;
    
    return ( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT InitializeAudioDecoder(  AM_MEDIA_TYPE *pmtInput, \
                                 AudioDecParams *pParams, \
                                 IMediaObject   **ppDMO,  \
                                 AM_MEDIA_TYPE  *pmtOutput){
    HRESULT       hr;
    DWORD         cbInputBuffer  = 0;
    DWORD         cbOutputBuffer = 0;
    DWORD         dwDummy;
    
    if( NULL == pmtInput || NULL == pParams || NULL == ppDMO ){
        return ( E_INVALIDARG );
    }
    
    *ppDMO = NULL;

    //
    // Create the DMO Decoder    
    //
    do {        
        hr = CoCreateInstance( CLSID_CWMSPDecMediaObject,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IMediaObject,
            (void**)ppDMO);
        if( FAILED( hr ) ){
            break;
        }

        hr = (*ppDMO)->SetInputType(0, pmtInput, 0);
        if( S_OK != hr ){
            return ( hr );
        }

        hr = MatchAudioParams( *ppDMO,                              \
            ((WAVEFORMATEX*)pmtInput->pbFormat), \
            pParams,                             \
            pmtOutput );
        if( FAILED( hr ) ){
            break;
        }
        
        hr = (*ppDMO)->SetOutputType(0, pmtOutput, 0);
        if( S_OK != hr ){
            return ( hr );
        }

        hr = (*ppDMO)->GetInputSizeInfo( 0, &cbInputBuffer, &dwDummy, &dwDummy);
        if( FAILED( hr ) ){
            return ( hr );
        }
    
        hr = (*ppDMO)->GetOutputSizeInfo( 0, &cbOutputBuffer, &dwDummy);
        if( FAILED( hr ) ){
            return ( hr );
        }

    } while( FALSE );
    
    if( FAILED( hr ) ){
        SAFERELEASE( *ppDMO );
    }
    
    return ( hr );     
}

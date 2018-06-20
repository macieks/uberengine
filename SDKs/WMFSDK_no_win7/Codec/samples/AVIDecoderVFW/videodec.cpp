//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

#include <streams.h>
#include <dvdmedia.h>
#include "wmcodecconst.h"
#include "wmcodeciface.h"
#include "decappErr.h"
#include "videodec.h"
#include "decode.h"

//////////////////////////////////////////////////////////////////////////////
HRESULT MatchVideoParams( IMediaObject   *pDMO,
                          VideoDecParams *pParams,
                          AM_MEDIA_TYPE  *pmt       ) {
    VIDEOINFOHEADER *pvih = NULL;
    int             c     = 0;
    BOOL            fDone = FALSE;
    
    do {
        HRESULT hr = pDMO->GetOutputType(0, c, pmt);
        if ( FAILED( hr ) ){
            break;
        }
        if( pmt->formattype != FORMAT_VideoInfo      ||
            pmt->pbFormat   == NULL                  ||
            pmt->cbFormat   < sizeof( VIDEOINFOHEADER ) )
            return ( E_VIDEO_INVALID_FORMAT );
        
        pvih = (VIDEOINFOHEADER*)pmt->pbFormat;
        
        if( pvih->bmiHeader.biCompression == pParams->dwTag          &&
            pvih->bmiHeader.biBitCount    == pParams->dwBitsPerPixel ){            
            fDone = TRUE;
        }
        
        if( fDone == FALSE ){
            CoTaskMemFree( pmt->pbFormat );
            memset(pmt, 0, sizeof( *pmt ) );
        }
        c++;
    } while( fDone == FALSE );
    
    return ( fDone == TRUE ? S_OK : ( E_VIDEO_FORMAT_DOESNT_MATCH ) );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT SetVideoTypes( IMediaObject   *pDMO,    \
                      AM_MEDIA_TYPE  *pmtIn,   \
                      VideoDecParams *pParams, \
                      AM_MEDIA_TYPE  *pmtOut,  \
                      DWORD *pcbIn,            \
                      DWORD *pcbOut ){
    HRESULT hr;
    DWORD   dwDummy;
    
    hr = pDMO->SetInputType(0, pmtIn, 0);
    if( S_OK != hr ){
        return ( hr );
    }
    
    hr = MatchVideoParams( pDMO, pParams, pmtOut );
    if( FAILED( hr ) ){
        return ( hr );
    }
    
    hr = pDMO->SetOutputType(0, pmtOut, 0);
    if( S_OK != hr ){
        return ( hr );
    }
    
    hr = pDMO->GetInputSizeInfo( 0, pcbIn, &dwDummy, &dwDummy);
    if( FAILED(hr) ){
        return ( hr );
    }
    
    hr = pDMO->GetOutputSizeInfo( 0, pcbOut, &dwDummy);
    if( FAILED( hr ) ){
        return ( hr );
    }
    
    return ( hr == S_FALSE ? E_VIDEO_TYPE_NOT_SET: hr );
} 

//////////////////////////////////////////////////////////////////////////////
HRESULT InitializeVideoDecoder(  AM_MEDIA_TYPE      *pmtInput,
                                VideoDecParams       *pParams,
                                IMediaObject         **ppDMO,
                                AM_MEDIA_TYPE        *pmtOutput,
                                CHandlingMediaBuffer *pMediaBuffer ){
    HRESULT       hr             = S_OK;
    DWORD         cbInputBuffer  = 0;
    DWORD         cbOutputBuffer = 0;
    
    if( NULL == pmtInput || NULL == pParams || NULL == ppDMO ){
        return ( E_INVALIDARG );
    }

    *ppDMO = NULL;
    
    do {
        //
        // Create the DMO Encoder
        //
        hr = CoCreateInstance( CLSID_CWMVDecMediaObject,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IMediaObject,
                (void**)ppDMO);
        if( FAILED( hr ) ){
            break;
        }

        hr = SetVideoTypes( *ppDMO, pmtInput, pParams, pmtOutput, &cbInputBuffer, &cbOutputBuffer );
        if( FAILED( hr ) ){
            break;
        }

    } while ( FALSE );

    if( S_OK != hr ){
        SAFERELEASE( *ppDMO );
    }
    
    return ( hr );     
}

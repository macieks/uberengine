//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

#include <streams.h>
#include <stdio.h>
#include <mediaobj.h>
#include <uuids.h>
#include <ks.h>
#include <ksmedia.h>
#include <dvdmedia.h>
#include <tchar.h>
#include <vfw.h>
#include "decappErr.h"
#include "resource.h"
#include "wmcodecconst.h"
#include "wmcodeciface.h"
#include "decode.h"

//////////////////////////////////////////////////////////////////////////////
void FreeContext( HandlingContext  *pHContext ){
    if( NULL != pHContext->pDMO ){
        SAFERELEASE( pHContext->pDMO );
    }
    if( NULL != pHContext->mtOut.pbFormat ){
        CoTaskMemFree( pHContext->mtOut.pbFormat );
        memset( &pHContext->mtOut, 0, sizeof( AM_MEDIA_TYPE ) );
    }
    if( NULL != pHContext->mtIn.pbFormat ){
        CoTaskMemFree( pHContext->mtIn.pbFormat );
        memset( &pHContext->mtIn, 0, sizeof( AM_MEDIA_TYPE ) );
    }
    if( NULL != pHContext->pAviInStream ){
        AVIStreamRelease( pHContext->pAviInStream );
    }
    if( NULL != pHContext->pAviOutStream ){
        AVIStreamRelease( pHContext->pAviOutStream );
    }
    
    if( NULL != pHContext->pGetFrame ){
        AVIStreamGetFrameClose( pHContext->pGetFrame );
        pHContext->pGetFrame = NULL;
    }
    
    SAFEDELETE ( pHContext->pbBuffer );
    SAFEDELETES( pHContext->pInputBuffer );        
    SAFEDELETES( pHContext->pOutputBuffer );        
    SAFEDELETES( pHContext );        
}

//////////////////////////////////////////////////////////////////////////////
HRESULT InitializeDecoder( void                 *pParams,
                           IMediaObject         **ppDMO,
                           AM_MEDIA_TYPE        *pmtIn,
                           AM_MEDIA_TYPE        *pmtOut,
                           CHandlingMediaBuffer *pMediaBuffer){
    HRESULT         hr = S_OK;
    
    if( NULL == pParams ||
        NULL == ppDMO   ||
        NULL == pmtIn   ||
        NULL == pmtOut  ){
        return ( E_INVALIDARG );
    }
    
    if( FORMAT_WaveFormatEx == pmtIn->formattype ) {
        hr = InitializeAudioDecoder( pmtIn, (AudioDecParams*)pParams, ppDMO, pmtOut );
    } else if ( FORMAT_VideoInfo  == pmtIn->formattype  ||
        FORMAT_VideoInfo2 == pmtIn->formattype ){
        hr = InitializeVideoDecoder( pmtIn, 
                                    (VideoDecParams*)pParams,
                                     ppDMO,
                                     pmtOut,
                                     pMediaBuffer );
    } else {
        CoTaskMemFree( pmtIn->pbFormat );
        memset( pmtIn, 0, sizeof( AM_MEDIA_TYPE ) );
        hr = ( E_FORMAT_NOT_SUPPORTED );
    }
    return ( hr );    
}

//////////////////////////////////////////////////////////////////////////////
HRESULT SetUpDecoder( void           *pParams,
                      HandlingContext *pContext ){
    HRESULT  hr      = S_OK;
    DWORD    dwFlags;

    if( NULL    == pContext ||
        NULL    == pParams  ){
        return ( E_INVALIDARG );
    }
    
    pContext->pDMO               = NULL;
    
    do{
        pContext->pInputBuffer    = new CHandlingMediaBuffer;
        if( pContext->pInputBuffer == NULL ){
            hr = E_OUTOFMEMORY;
            break;
        }
        
        pContext->pOutputBuffer   = new CHandlingMediaBuffer;
        if( pContext->pOutputBuffer == NULL ){
            hr = E_OUTOFMEMORY;
            break;
        }
        
        //
        //instantiate the DMO encoder
        //
        hr = InitializeDecoder( pParams,
                                &pContext->pDMO,
                                &pContext->mtIn,
                                &pContext->mtOut,
                                pContext->pInputBuffer );
        if( FAILED( hr ) ){
            break;
        }
        //
        // The codecs used in this sample don't perform lookahead on the incoming data.
        // If this changes the scheme used here may not work.
        // FAIL is the encoder uses lookahead.
        //
        hr = pContext->pDMO->GetInputStreamInfo( 0, &dwFlags );
        if( FAILED( hr ) ){
            break;
        }
        if( dwFlags & DMO_INPUT_STREAMF_HOLDS_BUFFERS ){
            hr = E_NEEDS_LOOKAHEAD;
            break;
        }        
    } while( FALSE );
    
    return ( hr );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT GetFormat( PAVISTREAM paviStream, BYTE **ppbFormat, int *pcbFormat ){
    HRESULT hr       = S_OK;
    BYTE   *pbFormat = NULL;
    LONG   cbFormat  = 0;
    
    if( NULL == paviStream ||
        NULL == ppbFormat  ||
        NULL == pcbFormat  ){
        return ( E_INVALIDARG );
    }
    
    *ppbFormat = NULL;
    *pcbFormat = NULL;
    do {
        hr = AVIStreamFormatSize( paviStream, 0, &cbFormat );
        if( S_OK != hr ){
            break;
        }
        if( 0 >= cbFormat ){
            hr = E_INVALID_FORMAT;
            break;
        }
        
        pbFormat = (BYTE*)CoTaskMemAlloc( cbFormat );
        if( NULL == pbFormat ){
            hr = E_OUTOFMEMORY;
            break;
        }
        hr = AVIStreamReadFormat( paviStream, 0, pbFormat, &cbFormat );
        if( S_OK != hr ){
            break;
        }
    }while( FALSE );
    
    if( S_OK != hr ){
        if( NULL != pbFormat ){
            CoTaskMemFree( pbFormat );
        }
    } else {
        *ppbFormat = pbFormat;
        *pcbFormat = cbFormat;
    }
    return ( hr );   
}

HRESULT GetVideoMediaType( AVISTREAMINFO   *streamInfo,
                           BYTE            *pbFormat,
                           int             cbFormat,
                           HandlingContext *pContext ){
    HRESULT          hr           = S_OK;
    DWORD            *pdwMask     = NULL;
    VIDEOINFOHEADER  *pvih        = NULL;
    BITMAPINFOHEADER *pbmi        = NULL;
    BITMAPINFOHEADER *pbmiIn      = NULL;
    PGETFRAME        pGetFrame    = NULL;
    BOOL             fUnsupported = FALSE;
    LONG             lFormat      = 0;
    BYTE             *pbVideoBits = NULL;

    if( NULL                       == streamInfo ||
        NULL                       == pbFormat   ||
        sizeof( BITMAPINFOHEADER ) >  cbFormat   ||
        NULL                       == pContext   ){
        return ( E_INVALIDARG );
    }
    
    
    do {
        //
        //check the fourcc
        //
        pbmi = (BITMAPINFOHEADER*)pbFormat;
        pbmiIn = pbmi;
        switch( pbmi->biCompression ){
        case mmioFOURCC( 'W', 'M', 'V', '3' ):
            pContext->mtIn.subtype = WMCMEDIASUBTYPE_WMV3;
            break;
        default:
            fUnsupported = TRUE;
        }
        
        if( fUnsupported ){
            hr = E_CANT_UNCOMPRESS;
            break;
        }

        lFormat = sizeof( VIDEOINFOHEADER ) + pbmiIn->biSize - sizeof( BITMAPINFOHEADER ) + pbmiIn->biClrUsed * 4;
        pvih = (VIDEOINFOHEADER*)CoTaskMemAlloc( lFormat );
        if( NULL == pvih ){
            hr = E_OUTOFMEMORY;
            break;
        }
        
        memset( pvih, 0, lFormat );
        //
        //Fill the format fields
        //
        pvih->rcSource.left   = 0;
        pvih->rcSource.top    = 0;
        pvih->rcSource.right  = pbmiIn->biWidth;
        pvih->rcSource.bottom = pbmiIn->biHeight;
        pvih->rcTarget        = pvih->rcSource;
        pvih->dwBitErrorRate  = 0;
        if( 0 != streamInfo->dwRate ){
            pContext->rtFrameDuration = (REFERENCE_TIME)streamInfo->dwScale * 10000000 / streamInfo->dwRate;
            pvih->AvgTimePerFrame     = pContext->rtFrameDuration;
        }
        if( 0 != streamInfo->dwScale ){
            pvih->dwBitRate       = (DWORD)((__int64)pbmiIn->biSizeImage *  streamInfo->dwRate * 8 / streamInfo->dwScale );
        }
        memcpy( &pvih->bmiHeader, pbmiIn, pbmiIn->biSize + pbmiIn->biClrUsed * 4 );
        //
        //Fill the media type fields
        //
        pContext->mtIn.majortype            = MEDIATYPE_Video;
        pContext->mtIn.formattype           = FORMAT_VideoInfo;
        pContext->mtIn.bFixedSizeSamples    = FALSE;
        pContext->mtIn.bTemporalCompression = FALSE;
        pContext->mtIn.lSampleSize          = 0;
        pContext->mtIn.pbFormat             = (BYTE*)pvih;
        pContext->mtIn.cbFormat             = lFormat;
    } while ( FALSE );
    
    return ( hr );
}

HRESULT GetAudioMediaType( AVISTREAMINFO   *streamInfo,
                          BYTE            *pbFormat,
                          int             cbFormat,
                          HandlingContext *pContext ){
    WAVEFORMATEX         *pwfx  = NULL;
    WAVEFORMATEXTENSIBLE *pwfxt = NULL;
    
    if( NULL                   == streamInfo ||
        NULL                   == pbFormat   ||
        NULL                   == pContext   ){
        return ( E_INVALIDARG );
    }
    
    //
    //allocate a WAVEFORMATEX chunk for the format if necessary
    //
    if( sizeof( WAVEFORMATEX )  >   cbFormat &&
        sizeof( PCMWAVEFORMAT ) ==  cbFormat ){
        pwfx = (WAVEFORMATEX*)pbFormat;
        pbFormat = (BYTE*)CoTaskMemAlloc( sizeof(WAVEFORMATEX ) );
        if( NULL == pbFormat ){
            return ( E_OUTOFMEMORY );
        }
        memcpy( pbFormat, pwfx, cbFormat );
        CoTaskMemFree( pwfx );
        cbFormat = sizeof( WAVEFORMATEX );
        ((WAVEFORMATEX*)pbFormat)->cbSize = 0;
    }
    
    pwfx = (WAVEFORMATEX*)pbFormat;
    //
    //Check the format
    //
    if( WMC_WAVE_WMAVOICE != pwfx->wFormatTag ){
        return ( E_CANT_UNCOMPRESS );
    }
    
    //
    //Fill the media type fields
    //
    pContext->mtIn.majortype            = MEDIATYPE_Audio;
    pContext->mtIn.subtype              = WMCMEDIASUBTYPE_WMA9Voice;
    pContext->mtIn.formattype           = FORMAT_WaveFormatEx;
    pContext->mtIn.bFixedSizeSamples    = FALSE;
    pContext->mtIn.bTemporalCompression = TRUE;
    pContext->mtIn.lSampleSize          = pwfx->nBlockAlign;
    pContext->mtIn.pbFormat             = pbFormat;
    pContext->mtIn.cbFormat             = cbFormat;
    
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
HRESULT GetMediaType( PAVISTREAM paviStream, DWORD dwType, HandlingContext *pContext ){
    HRESULT          hr         = S_OK;
    BYTE             *pbFormat  = NULL;
    int              cbFormat   = 0;
    DWORD            *pdwMask   = NULL;
    WAVEFORMATEX     *pwfx      = NULL;
    VIDEOINFOHEADER  *pvih      = NULL;
    BITMAPINFOHEADER *pbmi      = NULL;
    BITMAPINFOHEADER *pbmiIn    = NULL;
    AVISTREAMINFO    streamInfo;
    PGETFRAME        pGetFrame  = NULL;
    
    if( NULL == paviStream || NULL == pContext ){
        return ( E_INVALIDARG );
    }

    if( streamtypeVIDEO != dwType && streamtypeAUDIO != dwType ){
        return ( E_INVALIDARG );
    }
    
    memset( &pContext->mtIn, 0, sizeof( AM_MEDIA_TYPE ) );
    memset( &streamInfo,     0, sizeof( AVISTREAMINFO ) );
    
    do {
        hr = AVIStreamInfo( paviStream, &streamInfo, sizeof( AVISTREAMINFO ) );
        if( S_OK != hr ){
            break;
        }
        pContext->dwRate  = streamInfo.dwRate;
        pContext->dwScale = streamInfo.dwScale;
        
        hr = GetFormat( paviStream, &pbFormat, &cbFormat );
        if( S_OK != hr ){
            break;
        }
        if( streamtypeVIDEO == dwType ){
            hr = GetVideoMediaType( &streamInfo, pbFormat, cbFormat, pContext );
            if( S_OK != hr ){
                break;
            }
        } else {
            hr = GetAudioMediaType( &streamInfo, pbFormat, cbFormat, pContext );
            if( S_OK != hr ){
                break;
            }
            pbFormat = NULL;
        }
    } while( FALSE );
    
    if( NULL != pbFormat ){
        CoTaskMemFree( pbFormat );
    }
    return ( hr );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT FeedData( IMediaObject         *pDMO,
                  BYTE                 *pbData,
                  LONG                 cbData,
                  REFERENCE_TIME       rtStart,
                  REFERENCE_TIME       rtLength,
                  CHandlingMediaBuffer *pMediaBuffer ){
    HRESULT hr  = S_OK;
    DWORD   dwFlags = DMO_INPUT_DATA_BUFFERF_SYNCPOINT  |
        DMO_INPUT_DATA_BUFFERF_TIME       |
        DMO_INPUT_DATA_BUFFERF_TIMELENGTH;
    DWORD   dwStatus;
    
    if( NULL == pDMO         ||
        NULL == pbData       ||
        NULL == pMediaBuffer ||
        0    >= cbData   ){
        return ( E_INVALIDARG );
    }
    
    hr = pDMO->GetInputStatus(0, &dwStatus);
    if (FAILED(hr)) {
        return ( hr );
    }
    
    if (!(dwStatus & DMO_INPUT_STATUSF_ACCEPT_DATA)) {
        return ( E_DMO_NOTACCEPTING );
    }
    //
    //set the params for the input buffer
    //
    pMediaBuffer->set_Buffer( pbData,
        cbData,
        cbData );
    
    return pDMO->ProcessInput(0, pMediaBuffer, dwFlags, rtStart, rtLength );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT PullData( IMediaObject         *pDMO,
                 BYTE                 *pbData,
                 LONG                 cbData,
                 DWORD                *pdwStatus,
                 REFERENCE_TIME       *prtStart,
                 REFERENCE_TIME       *prtLength,
                 CHandlingMediaBuffer *pMediaBuffer ){
    DMO_OUTPUT_DATA_BUFFER OutputBufferStructs[1];
    HRESULT                hr             = S_OK;
    DWORD                  dwStatus       = 0;
    DWORD                  cbProduced     = NULL;
    
    if( NULL == pDMO         ||
        NULL == pMediaBuffer ||
        NULL == pdwStatus    ||
        NULL == pbData       ||
        0    >= cbData       ){
        return ( E_INVALIDARG );
    }
    
    *pdwStatus = 0;
    
    pMediaBuffer->set_Buffer( pbData, 0, cbData );
    
    OutputBufferStructs[0].pBuffer = pMediaBuffer;
    OutputBufferStructs[0].dwStatus = 0;
    
    hr = pDMO->ProcessOutput(0, 1, OutputBufferStructs, &dwStatus);
    if( S_OK != hr ) {
        return ( hr );
    }
    
    *pdwStatus = OutputBufferStructs[0].dwStatus;
    
    if( NULL != prtStart ){
        *prtStart = OutputBufferStructs[0].rtTimestamp;
    }
    if( NULL != prtLength ){
        *prtLength = OutputBufferStructs[0].rtTimelength;
    }
    
    return ( hr );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT FeedFrame( HandlingContext *pVideoContext, LONG nFrame ){
    
    REFERENCE_TIME  rtStartIn;
    REFERENCE_TIME  rtLenIn;

    BYTE    *pbVideoIn;
    LONG    lSize       = 0;
    LONG    nVideoSamples;
    DWORD   dwStatus    = 0;
    HRESULT hr          = S_OK;
    LONG    nBytesRead  = 0;
    //
    //Compute the time stamps
    //
    rtLenIn = (REFERENCE_TIME)10000000 * pVideoContext->dwScale / pVideoContext->dwRate;
    rtStartIn = (REFERENCE_TIME)nFrame * 10000000 * pVideoContext->dwScale / pVideoContext->dwRate;

    //
    //Get the frame
    //
    lSize = ((VIDEOINFOHEADER*)pVideoContext->mtIn.pbFormat)->bmiHeader.biSizeImage;
    if( NULL == pVideoContext->pbBuffer ){
        pVideoContext->pbBuffer = new BYTE[ lSize ];
        if( NULL == pVideoContext->pbBuffer ){
            return ( E_OUTOFMEMORY );
        }
        pVideoContext->cbBuffer = lSize;
    }

    hr = AVIStreamRead( pVideoContext->pAviInStream,
                        nFrame,
                        1,
                        pVideoContext->pbBuffer,
                        lSize,
                        &nBytesRead,
                        &nVideoSamples );
    if( FAILED( hr ) || nVideoSamples <= 0 ){
        return (  E_READ_FRAME );
    }
    pbVideoIn   = pVideoContext->pbBuffer;

    if( nBytesRead <= 0 ){
        return S_FALSE;
    }
    
    hr = FeedData( pVideoContext->pDMO,
        pbVideoIn,
        nBytesRead,
        rtStartIn,
        rtLenIn,
        pVideoContext->pInputBuffer );

    return ( hr );    
}

//////////////////////////////////////////////////////////////////////////////
HRESULT PullAllData( IMediaObject         *pDMO,
                    CHandlingMediaBuffer *pOutputBuffer,
                    BYTE                 *pbAudioOut,
                    DWORD                *pcbAudioOut){
    HRESULT hr = S_OK;
    DWORD   lOutBytes;
    DWORD   dwStatus    = 0;
    LONG    cbOutBuffer = *pcbAudioOut;
    
    *pcbAudioOut = 0;
    do{
        lOutBytes = cbOutBuffer;
        
        hr = PullData( pDMO,
            pbAudioOut,
            cbOutBuffer,
            &dwStatus,
            NULL,
            NULL,
            pOutputBuffer );
        if( S_OK != hr ){
            if( *pcbAudioOut <= 0 ){
                break;
            }
        }
        hr = pOutputBuffer->GetBufferAndLength( NULL, &lOutBytes );
        if( FAILED( hr ) ){
            break;
        }
        
        *pcbAudioOut += lOutBytes;
        pbAudioOut   += lOutBytes;
        cbOutBuffer  -= lOutBytes;
    } while( S_OK == hr      &&
        cbOutBuffer > 0 &&
        lOutBytes   > 0 );
    
    if( FAILED( hr ) ){
        *pcbAudioOut = 0;
        return ( hr );
    }
    
    return ( 0 == *pcbAudioOut ? S_FALSE : S_OK );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT DecodeSamples( HandlingContext *pContext, 
                       LONG            nInSample,
                       BYTE            *pbAudioIn,
                       LONG             cbAudioIn,
                       BYTE            *pbAudioOut,
                       DWORD           *pcbAudioOut ){
    REFERENCE_TIME  rtStartIn;
    REFERENCE_TIME  rtLenIn;
    HRESULT         hr          = S_OK;
    
    rtStartIn = (REFERENCE_TIME)nInSample * 10000000 / ((WAVEFORMATEX*)pContext->mtIn.pbFormat)->nSamplesPerSec;
    rtLenIn   = (REFERENCE_TIME)cbAudioIn * 10000000 / ((WAVEFORMATEX*)pContext->mtIn.pbFormat)->nSamplesPerSec /
        ((WAVEFORMATEX*)pContext->mtIn.pbFormat)->nBlockAlign;
    
    hr = FeedData( pContext->pDMO,
        pbAudioIn,
        cbAudioIn,
        rtStartIn,
        rtLenIn,
        pContext->pInputBuffer );
    if( S_OK != hr ){
        *pcbAudioOut = 0;
        return ( hr );
    }
    return PullAllData( pContext->pDMO,
        pContext->pOutputBuffer,
        pbAudioOut,
        pcbAudioOut );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT ConfigVideoStreams( PAVIFILE        paviInput,
                           PAVIFILE        paviOutput,
                           GeneralParams   *pIOParams,
                           VideoDecParams  *pVideoParams,
                           HandlingContext **ppVideoContext ){
    PAVISTREAM      paviStreamVideoIn  = NULL;
    PAVISTREAM      paviStreamVideoOut = NULL;
    HandlingContext *pVideoContext     = NULL;
    HRESULT         hr                 = S_OK;
    AVISTREAMINFO   streamInfo;  
    
    if( NULL == paviInput    ||
        NULL == paviOutput   ||
        NULL == pVideoParams ||
        NULL == pIOParams    ){
        return ( E_INVALIDARG );
    }
    
    memset( &streamInfo, 0, sizeof( streamInfo ) );
    
    do {
        hr = AVIFileGetStream( paviInput, &paviStreamVideoIn, streamtypeVIDEO, 0 );
        if( S_OK != hr ){
            break;
        }
        
        pVideoContext = new HandlingContext;
        if( NULL == pVideoContext ){
            hr = E_OUTOFMEMORY;
            break;
        }
        memset( pVideoContext, 0, sizeof( HandlingContext ) );
        pVideoContext->pAviInStream = paviStreamVideoIn;
        AVIStreamAddRef( pVideoContext->pAviInStream );
        hr = GetMediaType( paviStreamVideoIn, streamtypeVIDEO, pVideoContext );
        if( S_OK != hr ){
            break;
        }
        
        hr = SetUpDecoder( pVideoParams, pVideoContext );
        if( FAILED( hr ) ){
            break;
        }
        //
        //Add the video out stream
        //
        hr = AVIStreamInfo( paviStreamVideoIn, &streamInfo, sizeof( AVISTREAMINFO ) );
        if( FAILED( hr ) ){
            break;
        }
        streamInfo.fccHandler            = pVideoParams->dwTag;
        streamInfo.dwSuggestedBufferSize = 0;
        streamInfo.dwStart               = 0;
        
        hr = AVIFileCreateStream( paviOutput, &paviStreamVideoOut, &streamInfo );
        if( FAILED( hr ) ){
            break;
        }
        
        hr = AVIStreamSetFormat( paviStreamVideoOut,
            0,
            &((VIDEOINFOHEADER*)pVideoContext->mtOut.pbFormat)->bmiHeader,
            ((VIDEOINFOHEADER*)pVideoContext->mtOut.pbFormat)->bmiHeader.biSize );
        if( FAILED( hr ) ){
            break;
        }
        pVideoContext->pAviOutStream = paviStreamVideoOut;
        AVIStreamAddRef( pVideoContext->pAviOutStream );
        
    } while( FALSE );
    
    if( S_OK == hr ){
        *ppVideoContext = pVideoContext;
    } else {
        SAFEDELETES( pVideoContext );
    }
    
    if( NULL != paviStreamVideoIn ){
        AVIStreamRelease( paviStreamVideoIn);
    }
    
    if( NULL != paviStreamVideoOut ){
        AVIStreamRelease( paviStreamVideoOut);
    }
    
    return ( hr );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT ConfigAudioStreams( PAVIFILE        paviInput,
                           PAVIFILE        paviOutput,
                           GeneralParams   *pIOParams,
                           AudioDecParams  *pAudioParams,
                           HandlingContext **ppAudioContext ){
    PAVISTREAM      paviStreamAudioIn  = NULL;
    PAVISTREAM      paviStreamAudioOut = NULL;
    HandlingContext *pAudioContext     = NULL;
    HRESULT         hr                 = S_OK;
    AVISTREAMINFO   streamInfo;  
    
    if( NULL == paviInput    ||
        NULL == paviOutput   ||
        NULL == pAudioParams ||
        NULL == pIOParams    ){
        return ( E_INVALIDARG );
    }
    
    memset( &streamInfo, 0, sizeof( streamInfo ) );
    
    do {
        hr = AVIFileGetStream( paviInput, &paviStreamAudioIn, streamtypeAUDIO, 0 );
        if( S_OK != hr ){
            break;
        }
        
        pAudioContext = new HandlingContext;
        if( NULL == pAudioContext ){
            hr = E_OUTOFMEMORY;
            break;
        }
        memset( pAudioContext, 0, sizeof( HandlingContext ) );
        pAudioContext->pAviInStream = paviStreamAudioIn;
        AVIStreamAddRef( pAudioContext->pAviInStream );
        hr = GetMediaType( paviStreamAudioIn, streamtypeAUDIO, pAudioContext );
        if( S_OK != hr ){
            break;
        }
        
        hr = SetUpDecoder( pAudioParams, pAudioContext );
        if( FAILED( hr ) ){
            break;
        }
        
        
        hr = AVIStreamInfo( paviStreamAudioIn, &streamInfo, sizeof( AVISTREAMINFO ) );
        if( FAILED( hr ) ){
            break;
        }
        hr = AVIFileCreateStream( paviOutput, &paviStreamAudioOut, &streamInfo );
        if( FAILED( hr ) ){
            break;
        }
        
        hr = AVIStreamSetFormat( paviStreamAudioOut, 0, pAudioContext->mtOut.pbFormat, pAudioContext->mtOut.cbFormat );
        if( FAILED( hr ) ){
            break;
        }
        pAudioContext->pAviOutStream = paviStreamAudioOut;
        AVIStreamAddRef( pAudioContext->pAviOutStream );
        
    } while( FALSE );
    
    if( S_OK == hr ){
        *ppAudioContext = pAudioContext;
    } else {
        SAFEDELETES( pAudioContext );
    }
    
    if( NULL != paviStreamAudioIn ){
        AVIStreamRelease( paviStreamAudioIn);
    }
    
    if( NULL != paviStreamAudioOut ){
        AVIStreamRelease( paviStreamAudioOut);
    }
    
    return ( hr );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT DecodeVideo( HandlingContext *pVideoContext, BOOL fFirstPass, LONG *pnOutFrames ){
    HRESULT hr = S_OK;
    BYTE            *pbVideoOut        = NULL;
    DWORD           cbVideoOut         = 0;
    DWORD           cbAlignOut         = 0;
    LONG            nVideoLength       = 0;
    LONG            nVideoBytes        = 0;
    LONG            nVideoSamples      = 0;
    REFERENCE_TIME  rtStartOut;
    REFERENCE_TIME  rtLenOut;
    DWORD           dwVideoStatus;
    LONG            nInFrame;
    DWORD           ulLength;
    
    if( NULL == pVideoContext ||
        NULL == pnOutFrames   ){
        return ( E_INVALIDARG );
    }
    
    if( NULL == pVideoContext->pDMO ){
        return ( E_INVALIDARG );
    }
    
    *pnOutFrames = 0;
    
    do {
        //
        //Get the length for the video stream
        //
        nVideoLength = AVIStreamLength( pVideoContext->pAviInStream );
        if( nVideoLength < 0 ){
            hr = E_BAD_STREAM_LENGTH;
            break;
        }
        //
        // Allocate memory
        //
        cbVideoOut = max(128 * 128 * 2,
                    (((((VIDEOINFOHEADER*)pVideoContext->mtIn.pbFormat)->bmiHeader.biWidth  + 15) & ~15) *
                    ((((VIDEOINFOHEADER*)pVideoContext->mtIn.pbFormat)->bmiHeader.biHeight + 15) & ~15) * 3));

        pbVideoOut = new BYTE[ cbVideoOut ];
        if( NULL == pbVideoOut ){
            hr = E_OUTOFMEMORY;
            break;
        }
        //
        // Decode video stream
        //
        
        *pnOutFrames = 0;
        for( nInFrame = AVIStreamStart( pVideoContext->pAviInStream ); nInFrame < nVideoLength; nInFrame++ ){
            ulLength = cbVideoOut;
            hr = FeedFrame( pVideoContext, nInFrame );
            if( S_FALSE == hr ){
                //
                // Write a zero length frame
                //
                hr = AVIStreamWrite( pVideoContext->pAviOutStream,
                        (*pnOutFrames)++,
                        1,
                        NULL,
                        0,
                        0,
                        NULL,
                        NULL );
                if( FAILED( hr ) ){
                    break;
                }
                continue;
            }
            if( FAILED( hr ) ){
                break;
            }
            do {
                hr = PullData( pVideoContext->pDMO,
                    pbVideoOut,
                    cbVideoOut,
                    &dwVideoStatus,
                    &rtStartOut,
                    &rtLenOut,
                    pVideoContext->pOutputBuffer );
                if( S_OK != hr ){
                    break;
                }
                
                hr = pVideoContext->pOutputBuffer->GetBufferAndLength( NULL, &ulLength );
                if( S_OK != hr || 0 == ulLength ){
                    break;
                }
                                
                //
                // Write the data to the output stream
                //
                hr = AVIStreamWrite( pVideoContext->pAviOutStream,
                    (*pnOutFrames)++,
                    1,
                    pbVideoOut,
                    ulLength,
                    0,
                    &nVideoSamples,
                    &nVideoBytes );
                if( FAILED( hr ) ){
                    break;
                }
                
                pVideoContext->rtTimeStamp = rtStartOut;

            } while( TRUE );
            if( FAILED( hr ) ){
                break;
            }
        }
        
        if( FAILED(hr ) ){
            break;
        }
        
        //
        //Flush the decoder
        //
        hr = pVideoContext->pDMO->Discontinuity( 0 );
        if( FAILED(hr ) ){
            break;
        }
        do {
            ulLength = cbVideoOut;
            hr = PullData( pVideoContext->pDMO,
                pbVideoOut,
                cbVideoOut,
                &dwVideoStatus,
                NULL,
                NULL,
                pVideoContext->pOutputBuffer );
            if( S_OK != hr ){
                if( S_FALSE == hr ){
                    hr = S_OK;
                }
                break;
            }
            hr = pVideoContext->pOutputBuffer->GetBufferAndLength( NULL, &ulLength );
            if( FAILED( hr ) ){
                break;
            }
            hr = AVIStreamWrite( pVideoContext->pAviOutStream,
                    (*pnOutFrames)++,
                    1,
                    pbVideoOut,
                    ulLength,
                    0,
                    &nVideoSamples,
                    &nVideoBytes );
            if( FAILED( hr ) ){
                break;
            }
        } while( S_OK == hr && ulLength > 0 );

    }while( FALSE );
    
    SAFEDELETE( pbVideoOut );
    
    return (hr );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT DecodeAudio( HandlingContext *pAudioContext, LONG *pnOutSamples ){
    HRESULT         hr             = S_OK;
    BYTE            *pbAudioIn     = NULL;
    LONG            cbAudioIn      = 0;
    BYTE            *pbAudioOut    = NULL;
    LONG            cbAudioOut     = 0;
    LONG            nAudioBytes    = 0;
    LONG            nAudioSamples  = 0;
    DWORD           ulLength;
    LONG            nInSamples;
    LONG            nSamplesToRead;
    LONG            nInSample;
    LONG            nAudioLength;
    LONG            cbBlockAlign;
    
    if( NULL == pAudioContext ||
        NULL == pnOutSamples  ){
        return ( E_INVALIDARG );
    }
    
    if( NULL == pAudioContext->pDMO ){
        return ( E_INVALIDARG );
    }
    
    *pnOutSamples = 0;
    
    do {
        //Get the length for the uedio stream
        //
        nAudioLength = AVIStreamLength( pAudioContext->pAviInStream );
        if( nAudioLength < 0 ){
            hr = E_BAD_STREAM_LENGTH;
            break;
        }
        
        //
        //allocate the input/output buffers 
        //
        cbAudioIn = ((WAVEFORMATEX*)pAudioContext->mtIn.pbFormat)->nAvgBytesPerSec / 2;
        pbAudioIn = new BYTE[ cbAudioIn ];
        if( NULL == pbAudioIn ){
            hr = E_OUTOFMEMORY;
            break;
        }
        cbAudioOut = ((((WAVEFORMATEX*)pAudioContext->mtOut.pbFormat)->nAvgBytesPerSec * 5  /
                        ((WAVEFORMATEX*)pAudioContext->mtOut.pbFormat)->nBlockAlign ) + 1 ) *
                        ((WAVEFORMATEX*)pAudioContext->mtOut.pbFormat)->nBlockAlign;
        pbAudioOut = new BYTE[ cbAudioOut ];
        if( NULL == pbAudioOut ){
            hr = E_OUTOFMEMORY;
            break;
        }

        cbBlockAlign = ((WAVEFORMATEX*)pAudioContext->mtOut.pbFormat)->nBlockAlign;

        *pnOutSamples = 0;
        nSamplesToRead = cbAudioIn / ((WAVEFORMATEX*)pAudioContext->mtIn.pbFormat)->nBlockAlign;
        for( nInSample = AVIStreamStart( pAudioContext->pAviInStream ); nInSample < nAudioLength; nInSample += nInSamples ){
            ulLength   = cbAudioOut;
            //
            //Get the samples
            //
            hr = AVIStreamRead( pAudioContext->pAviInStream,
                nInSample,
                nSamplesToRead,
                pbAudioIn,
                cbAudioIn,
                &nAudioBytes,
                &nInSamples   );
            
            if( S_OK != hr || nAudioBytes <= 0 ){
                break;
            }
            hr = DecodeSamples( pAudioContext, 
                nInSample,
                pbAudioIn,
                nAudioBytes,
                pbAudioOut,
                &ulLength );
            
            if( S_FALSE == hr ){
                continue;
            }
            if( FAILED( hr ) ){
                break;
            }
            
            //
            // Write the data to the output stream
            //
            hr = AVIStreamWrite( pAudioContext->pAviOutStream,
                *pnOutSamples,
                ulLength / cbBlockAlign,
                pbAudioOut,
                ulLength,
                0,
                &nAudioSamples,
                &nAudioBytes );
            if( FAILED( hr ) ){
                break;
            }
            *pnOutSamples += nAudioSamples;
        }
        //
        // Send EOS
        //
        hr = pAudioContext->pDMO->Discontinuity(0);
        if( S_OK != hr ){
            break;
        }
        //
        // Flush the decoder
        //
        ulLength = cbAudioOut;
        hr = PullAllData( pAudioContext->pDMO,
            pAudioContext->pOutputBuffer,
            pbAudioOut,
            &ulLength );
        if( S_OK != hr ){
            if( S_FALSE == hr ){
                hr = S_OK;
            }
            break;
        }

        hr = AVIStreamWrite( pAudioContext->pAviOutStream,
            *pnOutSamples,
            ulLength / cbBlockAlign,
            pbAudioOut,
            ulLength,
            0,
            &nAudioSamples,
            &nAudioBytes );
        if( FAILED( hr ) ){
            break;
        }
        *pnOutSamples += nAudioSamples;
        
    } while( FALSE );
    
    SAFEDELETE( pbAudioIn );
    SAFEDELETE( pbAudioOut );
    return ( hr );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT DecodeFile( GeneralParams  *pIOParams,    \
                    AudioDecParams *pAudioParams, \
                    VideoDecParams *pVideoParams  ){
    
    PAVIFILE        paviInput          = NULL;
    PAVIFILE        paviOutput         = NULL;
    HandlingContext *pAudioContext     = NULL;
    HandlingContext *pVideoContext     = NULL;
    AVIFILEINFO     aviInfo;
    HRESULT         hr                 = S_OK;
    
    LONG            nOutFrames         = 0;
    LONG            nOutSamples        = 0;
    
    if( NULL == pIOParams    ||
        NULL == pAudioParams ||
        NULL == pVideoParams ){
        return ( E_INVALIDARG );
    }
    
    memset( &aviInfo, 0, sizeof( AVIFILEINFO ) );
    
    AVIFileInit();
    
    do {
        hr = AVIFileOpen( &paviInput, pIOParams->pszInput, OF_SHARE_DENY_WRITE | OF_READ, NULL );
        if( FAILED( hr ) ){
            hr = E_INVALID_INPUT;
            break;
        }
        
        //
        //Open up the output
        //
        hr = AVIFileOpen( &paviOutput, pIOParams->pszOutput, OF_WRITE | OF_CREATE, NULL );
        if( FAILED( hr ) ){
            hr = E_INVALID_OUTPUT;
            break;
        }
        
        //
        //Get the AVI info
        //        
        hr = AVIFileInfo( paviInput, &aviInfo, sizeof( AVIFILEINFO ) );
        if( FAILED( hr ) ){
            break;
        }
        
        
        if( !pIOParams->fAudioOnly ){
            hr = ConfigVideoStreams( paviInput, paviOutput, pIOParams, pVideoParams, &pVideoContext );
            if( S_OK != hr ){
                if( NULL != pVideoContext ){
                    break;
                }
            }
        }
        
        if( !pIOParams->fVideoOnly ){
            hr = ConfigAudioStreams( paviInput, paviOutput, pIOParams, pAudioParams, &pAudioContext );
            if( S_OK != hr ){
                if( NULL != pAudioContext ){
                    break;
                }
            }
        }
        
        if( NULL != pAudioContext &&  NULL != pVideoContext ){
            FreeContext(pAudioContext);
            pAudioContext = NULL;
        }

        if( NULL == pAudioContext &&  NULL == pVideoContext ){
            hr = FAILED( hr ) ? hr : E_NO_STREAM;
            break;
        }
        //
        // Generate Audio Data
        //
        if( NULL != pAudioContext ){
            //
            // Send discontinuity
            //
            if( NULL != pAudioContext->pDMO ){
                hr = pAudioContext->pDMO->Discontinuity(0);
            }

            hr = DecodeAudio( pAudioContext, &nOutSamples );
            if( FAILED(hr ) ){
                break;
            }
        }        
        
        //
        // Generate Video Data
        //
        if( NULL != pVideoContext ){
            //
            // Send discontinuity
            //
            hr = pVideoContext->pDMO->Discontinuity(0);
            if( S_OK != hr ){
                break;
            }

            hr = DecodeVideo( pVideoContext, FALSE, &nOutFrames );
            if( FAILED(hr ) ){
                break;
            }
        }        
    } while( FALSE );
    
    if( NULL != pAudioContext ){
        FreeContext( pAudioContext );
    }
    
    if( NULL != pVideoContext ){
        FreeContext( pVideoContext );
    }
    
    if( NULL != paviInput ){
        AVIFileRelease( paviInput);
    }
    if( NULL != paviOutput ){
        AVIFileRelease( paviOutput);
    }
    
    AVIFileExit();
    if( SUCCEEDED( hr )  &&
        0 == nOutFrames  &&
        0 == nOutSamples ){
        hr = E_NO_FRAMES;
    }
    
    return ( hr );
}

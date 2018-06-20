//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

#include <streams.h>
#include "wmcodecconst.h"
#include "wmcodeciface.h"
#include "encappErr.h"
#include "uedioenc.h"


//////////////////////////////////////////////////////////////////////////////
//
// Enumerate all of the DMO's output types and return info from the one that
// matches the specified sampling rate / channels / bitrate combination.
//
HRESULT MatchAudioParams(IMediaObject* pDMO,      \
                         WAVEFORMATEX   *pwfx,    \
                         AudioEncParams *pParams, \
                         AM_MEDIA_TYPE *pmt       ) {
    WAVEFORMATEX   *wfmt = NULL;
    int            c     = 0;
    BOOL           fDone = FALSE;
    
    do {
        HRESULT hr = pDMO->GetOutputType(0, c, pmt);
        if ( FAILED( hr ) ){
            break;
        }
        if( pmt->formattype != FORMAT_WaveFormatEx   || \
            pmt->pbFormat   == NULL                  || \
            pmt->cbFormat   < sizeof( WAVEFORMATEX ) )
            return ( E_AUDIO_INVALID_FORMAT );
        
        wfmt = (WAVEFORMATEX*)pmt->pbFormat;
        
        if ( (wfmt->nSamplesPerSec == pwfx->nSamplesPerSec) && \
             (wfmt->nChannels      == pwfx->nChannels)      && \
             (wfmt->wBitsPerSample == pwfx->wBitsPerSample) ){
            
            if( TRUE == pParams->fIsVBR && 1 == pParams->nPasses ){
                if( 0x7fffff00 == ( wfmt->nAvgBytesPerSec & 0x7fffff00 ) ){
                    if( ( wfmt->nAvgBytesPerSec & 0xff ) ==  (DWORD)pParams->nVBRQuality ){
                        fDone = TRUE;
                    }
                }
            } else {
                if( (abs(wfmt->nAvgBytesPerSec * 8 - pParams->nBitrate) < 500 ) ){
                    fDone = TRUE;
                }
                //
                // the AV sync modes have more or equal than 3 packets / sec for bitrates under 32kbps or
                // 5 packets / sec otherwise
                //
                if( TRUE == pParams->fAVMode ){
                    int nThreshold = wfmt->nAvgBytesPerSec > ( 32000 / 8 ) ? 5 : 3;

                    //
                    // if nBlockAlign is 0 we won't be able to test for this
                    // skip in the hope that other mt's will allow the test
                    //
                    if( 0 != wfmt->nBlockAlign ){
                        if( ( (int)wfmt->nAvgBytesPerSec / wfmt->nBlockAlign )  < nThreshold ){
                            fDone = FALSE;
                        }
                    }
                }
            }
        }
        
        if( fDone == FALSE ){
            CoTaskMemFree( pmt->pbFormat );
            memset(pmt, 0, sizeof( *pmt ) );
        }
        c++;
    } while( fDone == FALSE );
    
    return ( fDone == TRUE ? S_OK : ( E_AUDIO_FORMAT_DOESNT_MATCH ) );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT AdjustOutputBitrate( IMediaObject   *pDMO, int *pnBitrate ){
    HRESULT      hr;
    VARIANT      varg;
    IPropertyBag *pPropertyBag = NULL;
    
    if( NULL == pDMO || NULL == pnBitrate ){
        return ( E_INVALIDARG );
    }
    do {
        //
        // Get the IPropertyBag IF and set the appropriate params
        //
        hr = pDMO->QueryInterface(IID_IPropertyBag, (void**)&pPropertyBag);
        if( FAILED( hr ) ){
            break;
        }

        ::VariantInit(&varg);
            
        hr = pPropertyBag->Read( g_wszWMACAvgBytesPerSec, &varg, NULL );
        if( FAILED( hr ) ){
            hr = E_AUDIO_VBR_NOT_SUPPORTED;
            break;
        }
        if( VT_I4 != varg.vt  ){
            hr = E_FAIL;
            break;
        }

        *pnBitrate = varg.lVal;

    } while( FALSE );
    SAFERELEASE( pPropertyBag );
    return ( hr );    
}

//////////////////////////////////////////////////////////////////////////////
HRESULT SetAudioTypes( IMediaObject   *pDMO,
                       AudioEncParams *pParams,
                       AM_MEDIA_TYPE  *pmtIn,
                       AM_MEDIA_TYPE  *pmtOut,
                       DWORD          *pcbIn,
                       DWORD          *pcbOut ){
    HRESULT hr;
    DWORD   dwDummy;
    
    hr = pDMO->SetInputType(0, pmtIn, 0);
    if( S_OK != hr ){
        return ( hr );
    }
    
    hr = pDMO->SetOutputType(0, pmtOut, 0);
    if( S_OK != hr ){
        return ( hr );
    }
    
    //
    //if 1 pass vbr we'll need to modify the bitrate for the output type
    //
    if( pParams->fIsVBR && 1 == pParams->nPasses ){
        int nBitrate = 0;
        hr = AdjustOutputBitrate( pDMO, &nBitrate );
        if( FAILED( hr ) ){
            return ( hr );
        }
        ( (WAVEFORMATEX*)pmtOut->pbFormat )->nAvgBytesPerSec = nBitrate;
    }        

    hr = pDMO->GetInputSizeInfo( 0, pcbIn, &dwDummy, &dwDummy);
    if( FAILED( hr ) ){
        return ( hr );
    }
    
    hr = pDMO->GetOutputSizeInfo( 0, pcbOut, &dwDummy);
    if( FAILED( hr ) ){
        return ( hr );
    }
    
    return ( hr == S_FALSE ? E_AUDIO_TYPE_NOT_SET : S_OK );
} 

//////////////////////////////////////////////////////////////////////////////
HRESULT SetAudioParams( IMediaObject *pDMO, AudioEncParams *pParams ){
    HRESULT      hr;
    VARIANT      varg;
    IPropertyBag *pPropertyBag = NULL;
    
    if( NULL == pDMO || NULL == pParams ){
        return ( E_INVALIDARG );
    }
    
    if( pParams->fIsVBR         == FALSE && \
        pParams->nPasses        == 1     && \
        pParams->fIsConstrained == FALSE && \
        pParams->fIsSpeechMode  == TRUE     ) // nothing to do
        return ( S_OK );
    
    do {
        //
        // Get the IPropertyBag IF and set the appropriate params
        //
        hr = pDMO->QueryInterface(IID_IPropertyBag, (void**)&pPropertyBag);
        if( FAILED( hr ) ){
            break;
        }
        
        if( TRUE == pParams->fIsVBR ){
            ::VariantInit(&varg);
            varg.vt      = VT_BOOL;
            varg.boolVal = TRUE;
            
            hr = pPropertyBag->Write( g_wszWMVCVBREnabled, &varg );
            if( FAILED( hr ) ){
                hr = E_AUDIO_VBR_NOT_SUPPORTED;
                break;
            }
        }
        
        if( pParams->nPasses > 1 ){
            ::VariantInit(&varg);
            varg.vt   = VT_I4;
            varg.lVal = pParams->nPasses;
            
            hr = pPropertyBag->Write( g_wszWMVCPassesUsed, &varg );
            if( FAILED( hr ) ){
                hr = E_AUDIO_NPASS_NOT_SUPPORTED;
                break;
            }
        }
        
        if( TRUE == pParams->fIsConstrained ){
            ::VariantInit(&varg);
            varg.vt   = VT_I4;
            varg.lVal = pParams->nPeakBitrate;
            
            hr = pPropertyBag->Write( g_wszWMVCMaxBitrate, &varg );
            if( FAILED( hr ) ){
                hr = E_AUDIO_PEAK_BITRATE_REJECTED;
                break;
            }
            
            ::VariantInit(&varg);
            varg.vt   = VT_I4;
            varg.lVal = pParams->nPeakBuffer;
            
            hr = pPropertyBag->Write( g_wszWMVCBMax, &varg );
            if( FAILED( hr ) ){
                hr = E_AUDIO_PEAK_BUFFER_REJECTED;
                break;
            }
        }
        
        if( pParams->fIsSpeechMode == FALSE ){
            ::VariantInit(&varg);
            varg.vt   = VT_I4;
            varg.lVal = 2;
            
            hr = pPropertyBag->Write( g_wszWMACMusicSpeechClassMode, &varg );
            if( FAILED( hr ) ){
                hr = E_SPEECH_MODE_NOT_SUPPORTED;
                break;
            }
        }
        
    } while( FALSE );
    SAFERELEASE( pPropertyBag );
    return ( hr );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT InitializeAudioEncoder(  AM_MEDIA_TYPE *pmtInput, \
                               AudioEncParams *pParams, \
                               IMediaObject   **ppDMO,  \
                               AM_MEDIA_TYPE  *pmtOutput){
    HRESULT       hr;
    DWORD         cbInputBuffer  = 0;
    DWORD         cbOutputBuffer = 0;
    
    if( NULL == pmtInput || NULL == pParams || NULL == ppDMO ){
        return ( E_INVALIDARG );
    }
    
    *ppDMO = NULL;
    
    // Create the DMO Encoder
    switch( pParams->dwTag ){
    case WAVE_FORMAT_WMSPEECH1:
        hr = CoCreateInstance( CLSID_CWMSPEncMediaObject, \
            NULL,                      \
            CLSCTX_INPROC_SERVER,      \
            IID_IMediaObject,          \
            (void**)ppDMO);
        break;
    default:
        return ( E_AUDIO_COMPRESSION_NOT_SUPPORTED );
    }
    
    if( FAILED( hr ) ){
        return ( hr );
    }
    
    do {
        hr = SetAudioParams( *ppDMO, pParams );
        if( FAILED( hr ) ){
            break;
        }
        
        hr = MatchAudioParams( *ppDMO,                              \
            ((WAVEFORMATEX*)pmtInput->pbFormat), \
            pParams,                             \
            pmtOutput );
        if( FAILED( hr ) ){
            break;
        }
        
        hr = SetAudioTypes( *ppDMO, pParams, pmtInput, pmtOutput, &cbInputBuffer, &cbOutputBuffer );
        
        if( FAILED( hr ) ){
            break;
        }
    } while( FALSE );
    
    if( FAILED( hr ) ){
        SAFERELEASE( *ppDMO );
    }
    
    return ( hr );     
}

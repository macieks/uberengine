//-----------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#include <streams.h>
#include <dvdmedia.h>
#include "wmcodecconst.h"
#include "wmcodeciface.h"
#include "encappErr.h"

#include "videoenc.h"
#include "macros.h"

//////////////////////////////////////////////////////////////////////////////
//
// MakeVideoOutputType
//
//////////////////////////////////////////////////////////////////////////////
HRESULT MakeVideoOutputType(IMediaObject   *pDMO,    \
                            AM_MEDIA_TYPE  *pmtIn,   \
                            VideoEncParams *pParams, \
                            AM_MEDIA_TYPE  *pmt      ) {
    
    HRESULT             hr                   = S_OK;
    VIDEOINFOHEADER2    *pvih2               = NULL;
    VIDEOINFOHEADER     vih;
    IWMCodecPrivateData *pWMCodecPrivateData = NULL;
    DWORD               cbPrivateData        = 0;
    BYTE                *pbPrivateData       = NULL;
    BYTE                *pNewFormat          = NULL;
    
    if( pDMO == NULL || pmtIn == NULL || pParams == NULL || pmt == NULL )
        return E_INVALIDARG;
    
    if( pmtIn->pbFormat == NULL || pmtIn->cbFormat <= 0 )
        return E_INVALIDARG;
    
    // make up a partial media type
    pmt->majortype            = MEDIATYPE_Video;
    pmt->formattype           = FORMAT_VideoInfo;
    pmt->bFixedSizeSamples    = FALSE;
    pmt->bTemporalCompression = TRUE;
    
    if( pmtIn->formattype == FORMAT_VideoInfo ){
        vih = *(VIDEOINFOHEADER*)pmtIn->pbFormat; 
    } else if( pmtIn->formattype == FORMAT_VideoInfo2 ){
        pvih2 = (VIDEOINFOHEADER2*)pmtIn->pbFormat;
        vih.rcSource        = pvih2->rcSource;
        vih.rcTarget        = pvih2->rcTarget;
        vih.AvgTimePerFrame = pvih2->AvgTimePerFrame;
        vih.bmiHeader       = pvih2->bmiHeader;
    } else
        return E_VIDEO_INVALID_INPUT_TYPE;
    
    vih.dwBitRate            = (DWORD)pParams->nBitrate;
    vih.dwBitErrorRate       = 0;
    vih.bmiHeader.biPlanes   = 1;
    vih.bmiHeader.biBitCount = 24;
    
    switch( pParams->dwTag ){
    case WMCFOURCC_WMV3:
        pmt->subtype = WMCMEDIASUBTYPE_WMV3;
        vih.bmiHeader.biCompression = WMCFOURCC_WMV3;
        break;
    default:
        return E_VIDEO_COMPRESSION_NOT_SUPPORTED;
    }
    
    //use the fake format above to get the private data
    pmt->pbFormat = (BYTE*)&vih;
    pmt->cbFormat = sizeof( vih );
    pmt->pUnk = NULL;
    
    hr = pDMO->QueryInterface(IID_IWMCodecPrivateData, (void**)&pWMCodecPrivateData);    
    if( FAILED( hr ) )
        return E_NO_PRIVATE_DATA;
    
    hr = pWMCodecPrivateData->SetPartialOutputType( pmt );
    if( FAILED( hr ) ){
        SAFERELEASE( pWMCodecPrivateData );
        return E_PARTIAL_TYPE_REJECTED;;
    }
    
    hr = pWMCodecPrivateData->GetPrivateData( NULL, &cbPrivateData );
    if( FAILED( hr ) ){
        SAFERELEASE( pWMCodecPrivateData );        
        return E_NO_PRIVATE_DATA_COUNT;
    }
    
    if( cbPrivateData != 0 ){
        pbPrivateData = new BYTE[ cbPrivateData ];
        if( pbPrivateData == NULL ){
            SAFERELEASE( pWMCodecPrivateData );
            return E_OUTOFMEMORY;
        }
        
        // get the private data
        hr = pWMCodecPrivateData->GetPrivateData( pbPrivateData, &cbPrivateData );
        SAFERELEASE( pWMCodecPrivateData );
        if( FAILED( hr ) ) 
            return E_PRIVATE_DATA_FAILED;
    }
    
    //modify the media type accordingly
    pNewFormat = (BYTE*)CoTaskMemAlloc( sizeof( VIDEOINFOHEADER) + cbPrivateData );
    if( pNewFormat == NULL ){
        SAFEDELETE( pbPrivateData );
        return E_OUTOFMEMORY;
    }
    
    memcpy( pNewFormat, pmt->pbFormat, sizeof( VIDEOINFOHEADER));
    if( pbPrivateData != NULL )
        memcpy( pNewFormat + sizeof( VIDEOINFOHEADER), pbPrivateData, cbPrivateData);
    SAFEDELETE( pbPrivateData );
    
    pmt->pbFormat = pNewFormat;
    pmt->cbFormat = sizeof( VIDEOINFOHEADER) + cbPrivateData;
    ((VIDEOINFOHEADER*)pmt->pbFormat)->bmiHeader.biSize += cbPrivateData;
    
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
//
// SetVideoTypes
//
//////////////////////////////////////////////////////////////////////////////
HRESULT SetVideoTypes( IMediaObject   *pDMO,    \
                      AM_MEDIA_TYPE  *pmtIn,   \
                      VideoEncParams *pParams, \
                      AM_MEDIA_TYPE  *pmtOut,  \
                      DWORD *pcbIn,            \
                      DWORD *pcbOut ){
    HRESULT hr;
    DWORD   dwDummy;
    
    //Now we can make the output type...
    hr = MakeVideoOutputType( pDMO, pmtIn, pParams, pmtOut );
    if( FAILED( hr ) )
    {
        ON_FAIL("Failed MakeVideoOutputType\r\n", hr)
            return hr;
    }
    
    //... and set it on the DMO just to check whether it is accepted or not
    hr = pDMO->SetOutputType(0, pmtOut, 0);
    if( FAILED( hr ) )
    {
        ON_FAIL("Failed SetOutputType\r\n", hr)
            return hr;
    }
    
    //     
    hr = pDMO->GetInputSizeInfo( 0, pcbIn, &dwDummy, &dwDummy);
    if( FAILED(hr) )
    {
        ON_FAIL("Failed GetInputSizeInfo\r\n", hr)
            return hr;
    }
    
    hr = pDMO->GetOutputSizeInfo( 0, pcbOut, &dwDummy);
    if( FAILED( hr ) )
    {
        ON_FAIL("Failed GetOutputSizeInfo\r\n", hr)
            return hr;
    }
    
    return hr;
} 

//////////////////////////////////////////////////////////////////////////////
//
// SetVideoParams
//
//////////////////////////////////////////////////////////////////////////////
HRESULT SetVideoParams( IMediaObject *pDMO, VideoEncParams *pParams ){
    HRESULT      hr = S_OK;
    VARIANT      varg;
    BSTR         bstrIn;
    IPropertyBag *pPropertyBag = NULL;
    
    if( pDMO == NULL || pParams == NULL )
        return E_INVALIDARG;
    
    do {
        // Get the IPropertyBag IF and set the appropriate params
        hr = pDMO->QueryInterface(IID_IPropertyBag, (void**)&pPropertyBag);
        if( FAILED( hr ) ){
            break;
        }
        
        //set the encoder in VBR mode if required
        if( pParams->fIsVBR == TRUE ){
            ::VariantInit(&varg);
            varg.vt      = VT_BOOL;
            varg.boolVal = TRUE;
            
            hr = pPropertyBag->Write( g_wszWMVCVBREnabled, &varg );
            if( FAILED( hr ) ){
                hr = E_VIDEO_VBR_NOT_SUPPORTED;
                break;
            }
            
            ::VariantInit(&varg);
            varg.vt = VT_I4;
            varg.lVal = pParams->nVBRQuality;
            hr = pPropertyBag->Write( g_wszWMVCVBRQuality, &varg );
            if( FAILED( hr ) ){
                hr = E_VBR_QUALITY_REJECTED;
                break;
            }
        }
        
        // set the bitrate if not VBR
        if( pParams->fIsVBR == FALSE ){
            ::VariantInit(&varg);
            varg.vt = VT_I4;
            varg.lVal = pParams->nBitrate;
            hr = pPropertyBag->Write( g_wszWMVCAvgBitrate, &varg );
            if( FAILED( hr ) ){
                hr = E_VIDEO_BITRATE_REJECTED;
                break;
            }
        }
        
        // set the buffer window
        ::VariantInit(&varg);
        varg.vt = VT_I4;
        varg.lVal = pParams->nBufferDelay;
        hr = pPropertyBag->Write( g_wszWMVCVideoWindow, &varg );
        if( FAILED( hr ) ){
            hr = E_VIDEO_BUFFER_REJECTED;
            break;
        }
        
        // set the profile for WMV# only
        if( pParams->dwTag == WMCFOURCC_WMV3 ){
            switch( pParams->nProfile ){
            case P_MAIN:
                bstrIn = ::SysAllocString(L"MP");
                break;
            case P_SIMPLE:
                bstrIn = ::SysAllocString(L"SP");
                break;
            case P_COMPLEX:
                bstrIn = ::SysAllocString(L"CP");
                break;
            default:
                hr = E_VIDEO_INVALID_PROFILE;
                break;
            }
            ::VariantInit(&varg);
            varg.vt      = VT_BSTR;
            varg.bstrVal = bstrIn;
            hr = pPropertyBag->Write( g_wszWMVCDecoderComplexityRequested, &varg );
            ::SysFreeString( bstrIn );
            if( FAILED( hr ) ){
                hr = E_VIDEO_PROFILE_REJECTED;
                break;
            }
        }
        
        // set the ecoder complexity
        ::VariantInit(&varg);
        varg.vt = VT_I4;
        varg.lVal = pParams->nComplexity;
        hr = pPropertyBag->Write(g_wszWMVCComplexityEx, &varg);
        if( FAILED( hr ) ){
            hr = E_VIDEO_COMPLEXITY_REJECTED;
            break;
        }
        
        // set the max distance between the key frames
        ::VariantInit(&varg);
        varg.vt = VT_I4;
        varg.lVal = pParams->nKeyDist * 1000;
        hr = pPropertyBag->Write( g_wszWMVCKeyframeDistance, &varg );
        if( FAILED( hr ) ){
            hr = E_VIDEO_KEYDIST_REJECTED;
            break;
        }
        
        // set the crispness params for WMV# only
        if( pParams->dwTag == WMCFOURCC_WMV3 ){
            ::VariantInit(&varg);
            varg.vt = VT_I4;
            varg.lVal = pParams->nQuality;
            hr = pPropertyBag->Write( g_wszWMVCCrisp, &varg );
            if( FAILED( hr ) ){
                hr = E_VIDEO_CRISPNESS_REJECTED;
                break;
            }
        }
        
    } while( FALSE );

    SAFERELEASE( pPropertyBag );
    return hr;
}

//////////////////////////////////////////////////////////////////////////////
//
// DefaultVideoBitrate
//
//////////////////////////////////////////////////////////////////////////////
// a decent guess would be 0.4 bits per pixel

HRESULT DefaultVideoBitrate( AM_MEDIA_TYPE *pmt, double dFramesPerSec, int *pBitrate ){
    if( pmt == NULL || pBitrate == NULL )
        return E_INVALIDARG;
    
    if( pmt->pbFormat == NULL || pmt->cbFormat <= 0 )
        return E_INVALIDARG;
    
    if( pmt->formattype == FORMAT_VideoInfo ){
        VIDEOINFOHEADER *pvih = (VIDEOINFOHEADER*)pmt->pbFormat;
        
        // 0.4 bits per pixel will generate good quality content at a reasonable bitrate
        *pBitrate = ( pvih->bmiHeader.biWidth * pvih->bmiHeader.biHeight * 2 ) / 5;
        if( dFramesPerSec == 0 ) { // use the input rate
            if( pvih->AvgTimePerFrame > 0 )
                dFramesPerSec = 10000000.0 / pvih->AvgTimePerFrame;
            else
                dFramesPerSec = 29.97;   // 30 frames per sec
        }
    } else if( pmt->formattype == FORMAT_VideoInfo2 ){
        VIDEOINFOHEADER2 *pvih = (VIDEOINFOHEADER2*)pmt->pbFormat;
        
        // 0.4 bits per pixel will generate good quality content at a reasonable bitrate
        *pBitrate = ( pvih->bmiHeader.biWidth * pvih->bmiHeader.biHeight * 2 ) / 5;
        
        if( dFramesPerSec == 0 ) { // use the input rate
            if( pvih->AvgTimePerFrame > 0 )
                dFramesPerSec = 10000000.0 / pvih->AvgTimePerFrame;
            else
                dFramesPerSec = 29.97;   // 30 frames per sec
        }
    } else
        return E_VIDEO_INVALID_INPUT_TYPE;
    *pBitrate = (int)( *pBitrate * dFramesPerSec );
    
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
//
// InitializeVideoEncoder
//
//////////////////////////////////////////////////////////////////////////////
HRESULT InitializeVideoEncoder(  AM_MEDIA_TYPE *pmtInput, \
                               VideoEncParams *pParams, \
                               IMediaObject   *pDMO,  \
                               AM_MEDIA_TYPE  *pmtOutput){
    HRESULT       hr;
    DWORD         cbInputBuffer  = 0;
    DWORD         cbOutputBuffer = 0;
    
    if( pmtInput == NULL || pParams == NULL || pDMO == NULL )
        return E_INVALIDARG;
    
    
    
    //check to see if the video bitrate has been set; if not calculate the default
    if( pParams->nBitrate <= 0 )
    {
        hr = DefaultVideoBitrate( pmtInput, pParams->fFrameRate, &pParams->nBitrate );
        if( FAILED( hr ) )
            return hr;
    }
    
    
    
    hr = SetVideoParams( pDMO, pParams );
    ON_FAIL("Failed SetVideoParams", hr)
        
        
        // cbInputBuffer and cbOutputBuffer are actually not used here.
        hr = SetVideoTypes( pDMO, pmtInput, pParams, pmtOutput, &cbInputBuffer, &cbOutputBuffer );
    if( FAILED( hr ) )
    {
        ON_FAIL("Failed SetVideoTypes", hr)
            return hr;
    }
    return hr;     
}

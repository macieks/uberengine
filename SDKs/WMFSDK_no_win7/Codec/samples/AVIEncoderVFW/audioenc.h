//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

#ifndef AUDIOENC_H
#define AUDIOENC_H

#include <dmo.h>
#include <tchar.h>
#include "cmn_macr.h"

//////////////////////////////////////////////////////////////////////////////
//
// Audio Tags
//
//////////////////////////////////////////////////////////////////////////////

#define WAVE_FORMAT_WMSPEECH1 0x0a


//////////////////////////////////////////////////////////////////////////////
//
// Media SubTypes
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_GUID(MEDIASUBTYPE_WMSPEECH1,
            WAVE_FORMAT_WMSPEECH1,0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

//////////////////////////////////////////////////////////////////////////////
//
// Audio params struct
//
//////////////////////////////////////////////////////////////////////////////
struct AudioEncParams {
    TCHAR *pszCodecString;
    TCHAR *pszMode;
    int   nBitrate;
    int   nSamplingRate;
    int   nChannels;
    int   nBitDepth;
    int   nVBRQuality;
    int   nPeakBuffer;
    int   nPeakBitrate;
    TCHAR *pszSpeechMode;
    BOOL  fAVMode;

    BOOL  fIsSpeechMode;

    BOOL  fIsVBR;
    BOOL  fIsConstrained;
    int   nPasses;
    DWORD dwTag;

    AudioEncParams(){
        memset(  this, 0, sizeof( AudioEncParams ) );
    }
    ~AudioEncParams(){
        SAFEFREE(pszCodecString);
        SAFEFREE(pszMode);
        SAFEFREE(pszSpeechMode);
    }
};

// some prototypes
    HRESULT InitializeAudioEncoder( AM_MEDIA_TYPE *pmtInput, \
                                    AudioEncParams *pParams, \
                                    IMediaObject   **ppDMO,  \
                                    AM_MEDIA_TYPE  *pmtOutPut);
#endif /*AUDIOENC_H*/

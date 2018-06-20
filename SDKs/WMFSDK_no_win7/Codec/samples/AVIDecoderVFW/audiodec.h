//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

#ifndef AUDIODEC_H
#define AUDIODEC_H

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
struct AudioDecParams {
    int   nBitrate;
    int   nSamplingRate;
    int   nChannels;
    int   nBitDepth;
    DWORD dwTag;

    AudioDecParams(){
        memset(  this, 0, sizeof( AudioDecParams ) );
    }
};

// some prototypes
    HRESULT InitializeAudioDecoder( AM_MEDIA_TYPE *pmtInput, \
                                    AudioDecParams *pParams, \
                                    IMediaObject   **ppDMO,  \
                                    AM_MEDIA_TYPE  *pmtOutPut);
#endif /*AUDIODEC_H*/

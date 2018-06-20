//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

#ifndef VIDEODEC_H
#define VIDEODEC_H

#include <dmo.h>
#include <tchar.h>
#include "cmn_macr.h"

class CHandlingMediaBuffer;

//////////////////////////////////////////////////////////////////////////////
//
// Video params struct
//
//////////////////////////////////////////////////////////////////////////////
struct VideoDecParams {
    int    nWidth;
    int    nHeight;
    double dFrameRate;

    DWORD  dwTag;
    DWORD  dwBitsPerPixel;
    
    VideoDecParams(){
        memset(  this, 0, sizeof( VideoDecParams ) );
        dwBitsPerPixel = 24;
    }
};

// some prototypes
    HRESULT InitializeVideoDecoder( AM_MEDIA_TYPE *pmtInput, \
                                    VideoDecParams *pParams, \
                                    IMediaObject   **ppDMO,  \
                                    AM_MEDIA_TYPE  *pmtOutPut,
                                    CHandlingMediaBuffer *pMBIn);

#endif /*VIDEODEC_H*/
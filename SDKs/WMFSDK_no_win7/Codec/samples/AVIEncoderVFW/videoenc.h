//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

#ifndef VIDEOENC_H
#define VIDEOENC_H

#include <dmo.h>
#include <tchar.h>
#include "cmn_macr.h"

class CHandlingMediaBuffer;

//////////////////////////////////////////////////////////////////////////////
//
// Video FOURCCs
//
//////////////////////////////////////////////////////////////////////////////

enum { P_MAIN = 0, P_SIMPLE, P_COMPLEX };

//////////////////////////////////////////////////////////////////////////////
//
// Video params struct
//
//////////////////////////////////////////////////////////////////////////////
struct VideoEncParams {
    TCHAR  *pszCodecString;
    TCHAR  *pszMode;
    int    nBitrate;
    int    nWidth;
    int    nHeight;
    double dFrameRate;
    int    nKeyDist;
    int    nBufferDelay;
    int    nQuality;
    int    nVBRQuality;
    int    nPeakBuffer;
    int    nPeakBitrate;
    TCHAR  *pszProfileString;
    TCHAR  *pszVideoType;
    int    nComplexity;

    int    nProfile;
#ifdef SUPPORT_INTERLACE
    BOOL   fIsInterlaced;
#endif //SUPPORT_INTERLACE
    BOOL   fIsVBR;
    BOOL   fIsConstrained;
    int    nPasses;
    DWORD  dwTag;

    int    nForcedKeyFrame;
    
    VideoEncParams(){
        memset(  this, 0, sizeof( VideoEncParams ) );
    }
    ~VideoEncParams(){
        SAFEFREE(pszCodecString);
        SAFEFREE(pszMode);
        SAFEFREE(pszProfileString);
        SAFEFREE(pszVideoType);
    }
};

// some prototypes
    HRESULT InitializeVideoEncoder( AM_MEDIA_TYPE *pmtInput, \
                                    VideoEncParams *pParams, \
                                    IMediaObject   **ppDMO,  \
                                    AM_MEDIA_TYPE  *pmtOutPut,
                                    CHandlingMediaBuffer *pMBIn);

#endif /*VIDEOENC_H*/
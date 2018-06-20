
//-----------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef VIDEOENC_H
#define VIDEOENC_H

#include <dmo.h>
#include <tchar.h>

enum { P_MAIN = 0, P_SIMPLE, P_COMPLEX };

//////////////////////////////////////////////////////////////////////////////
//
// Video params struct
//
//////////////////////////////////////////////////////////////////////////////
struct VideoEncParams {
    int    nBitrate;
    float  fFrameRate;
    int    nKeyDist;
    int    nBufferDelay;
    int    nQuality;
    int    nVBRQuality;
    int    nComplexity;
    int    nPeakBuffer;
    int    nPeakBitrate;

    int    nProfile;

    BOOL   fIsVBR;
    DWORD  dwTag;
    
    VideoEncParams(){
        memset(  this, 0, sizeof( VideoEncParams ) );
    }
    ~VideoEncParams(){}
};

// some prototypes
    HRESULT InitializeVideoEncoder( AM_MEDIA_TYPE *pmtInput, \
                                    VideoEncParams *pParams, \
                                    IMediaObject   *pDMO,  \
                                    AM_MEDIA_TYPE  *pmtOutPut);

#define   SAFERELEASE(x)            if((x)){ (x)->Release(); (x) = NULL;}
#define   SAFEDELETE(x)             if((x)){ delete [] (x);  (x) = NULL;}
#define   SAFEDELETES(x)            if((x)){ delete    (x);  (x) = NULL;}
#define   SAFEFREE(x)               if((x)){ free(x);        (x) = NULL;}

#endif /*VIDEOENC_H*/
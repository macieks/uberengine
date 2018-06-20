//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

//
//  Interlace encoding mode is disabled by default.
//  Interlace WMV 9 (4cc WMV3) bit streams should not
//  currently be created in AVI, because AVI does not
//  provide the ability to specify top-field-first or
//  bottom-field-first at frame level. However, the code
//  is provided in case the user may want to wrap interlace
//  WMV 9 streams in other file formats.
//

#include    <wtypes.h>
#include    <stdio.h>
#include    <math.h>
#include    <tchar.h>

#include    "cmn_macr.h"
#include    "resource.h"
#include    "encappErr.h"
#include    "encode.h"
#include    "uedioenc.h"
#include    "videoenc.h"

#include    "wmcodecconst.h"

#define       TIMEOUT_INTERVAL       400
#define       MAX_STRING_LEN         4095

//////////////////////////////////////////////////////////////////////////////
//
// The Name of the Application
//
//////////////////////////////////////////////////////////////////////////////
TCHAR         AppName[ FILENAME_MAX + 1 ];

//////////////////////////////////////////////////////////////////////////////
//
// A few defines and structures to parse the command line args
//
//////////////////////////////////////////////////////////////////////////////

#define     GET_OFFSET(x,y)         ((BYTE*)(&(((x*)(NULL))->y)) - (BYTE*)NULL)

enum { D_GENERAL = 0, D_AUDIO, D_VIDEO };
enum { T_BOOL, T_INTEGER, T_DOUBLE, T_STRING };

//////////////////////////////////////////////////////////////////////////////
//
// Tables to parse the command line args
//
//////////////////////////////////////////////////////////////////////////////

//  The range for each parameter

    // the modes are the same for the uedio and video
    TCHAR *ModeRange[] = {
        NULL, NULL, NULL, NULL, NULL, NULL 
    };

    //Audio params ranges
    TCHAR *AudioCodecRange[] = {
         NULL, NULL, NULL, NULL, NULL, NULL
    };

    LONG AudioBitrateRange[] = {
        4000, 20000
    };

    LONG QualityRange[] = {
        0, 100
    };

    TCHAR *AudioTypeRange[] = {
        NULL, NULL, NULL
    };

    //Video params ranges    
    TCHAR *VideoCodecRange[] = {
         NULL, NULL
    };

    LONG VideoBitrateRange[] = {
        0,24000000
    };

    double FrameRateRange[] = {
        0, 60.0
    };

    LONG KeyDistanceRange[] = {
        0, 20000
    };

    LONG BufferDelayRange[] = {
        0, 128000
    };

    LONG CrispRange[] = {
        0, 100
    };

    LONG VideoPeakBufferRange[] = {
        1000, 128000
    };

    LONG VideoPeakBitrateRange[] = {
        0, 24000000
    };

    TCHAR *VideoProfileRange[] = {
        NULL, NULL, NULL, NULL
    };

#ifdef SUPPORT_INTERLACE
    TCHAR *VideoTypeRange[] = {
        NULL, NULL, NULL
    };
#endif //SUPPORT_INTERLACE

    LONG VideoComplexityRange[] = {
        0, 4
    };

    LONG VideoForcedKeyFrameRange[] = {
        -1, 24000000
    };


    //
    //  The big table
    //
    struct CommandEntry {
        TCHAR  *pszName;
        DWORD  dwNameID;
        LONG    nDomain;
        LONG    nType;
        LONG    nOffset;
        void   *pRange;
        double dDefault;    //if nType == T_STRING this is the index in the pRange table
    } CommandEntryTable[] = {
        //
        //  General
        //
        { NULL, IDS_CL_INPUT,        D_GENERAL, T_STRING,  GET_OFFSET(GeneralParams, pszInput),       0,                     0 },
        { NULL, IDS_CL_OUTPUT,       D_GENERAL, T_STRING,  GET_OFFSET(GeneralParams, pszOutput),      0,                     0 },
        { NULL, IDS_CL_AUDIO_ONLY,   D_GENERAL, T_BOOL,    GET_OFFSET(GeneralParams, fAudioOnly ),    0,                     0 },
        { NULL, IDS_CL_VIDEO_ONLY,   D_GENERAL, T_BOOL,    GET_OFFSET(GeneralParams, fVideoOnly ),    0,                     0 },
        //
        //  Audio
        //
        { NULL, IDS_CL_A_CODEC,      D_AUDIO,   T_STRING,  GET_OFFSET(AudioEncParams,pszCodecString ), AudioCodecRange,      0 },
        { NULL, IDS_CL_A_BITRATE,    D_AUDIO,   T_INTEGER, GET_OFFSET(AudioEncParams,nBitrate ),       AudioBitrateRange,    20000 },
        { NULL, IDS_CL_A_TYPE,       D_AUDIO,   T_STRING,  GET_OFFSET(AudioEncParams,pszSpeechMode ),  AudioTypeRange,       0 },
        //
        // Video
        //
        { NULL, IDS_CL_V_CODEC,      D_VIDEO,   T_STRING,  GET_OFFSET(VideoEncParams,pszCodecString ), VideoCodecRange,      0 },
        { NULL, IDS_CL_V_MODE,       D_VIDEO,   T_STRING,  GET_OFFSET(VideoEncParams,pszMode ),        ModeRange,            0 },
        { NULL, IDS_CL_V_BITRATE,    D_VIDEO,   T_INTEGER, GET_OFFSET(VideoEncParams,nBitrate ),       VideoBitrateRange,    0 },
        { NULL, IDS_CL_V_FRAMERATE,  D_VIDEO,   T_DOUBLE,  GET_OFFSET(VideoEncParams,dFrameRate ),     FrameRateRange,       0 },
        { NULL, IDS_CL_V_KEYDIST,    D_VIDEO,   T_INTEGER, GET_OFFSET(VideoEncParams,nKeyDist ),       KeyDistanceRange,     10000 },
        { NULL, IDS_CL_V_BUFFER,     D_VIDEO,   T_INTEGER, GET_OFFSET(VideoEncParams,nBufferDelay ),   BufferDelayRange,     5000 },
        { NULL, IDS_CL_V_QUALITY,    D_VIDEO,   T_INTEGER, GET_OFFSET(VideoEncParams,nQuality ),       CrispRange,           75 },
        { NULL, IDS_CL_V_VBRQUALITY, D_VIDEO,   T_INTEGER, GET_OFFSET(VideoEncParams,nVBRQuality ),    QualityRange,         98 },
        { NULL, IDS_CL_V_PEAKBUFFER, D_VIDEO,   T_INTEGER, GET_OFFSET(VideoEncParams,nPeakBuffer ),    VideoPeakBufferRange, 5000 },
        { NULL, IDS_CL_V_PEAKBITRATE,D_VIDEO,   T_INTEGER, GET_OFFSET(VideoEncParams,nPeakBitrate ),   VideoPeakBitrateRange,0 },
        { NULL, IDS_CL_V_PROFILE,    D_VIDEO,   T_STRING,  GET_OFFSET(VideoEncParams,pszProfileString),VideoProfileRange,    0 },
#ifdef SUPPORT_INTERLACE
        { NULL, IDS_CL_V_TYPE,       D_VIDEO,   T_STRING,  GET_OFFSET(VideoEncParams,pszVideoType ),   VideoTypeRange,       0 },
#endif //SUPPORT_INTERLACE
        { NULL, IDS_CL_V_FORCEDKEY,  D_VIDEO,   T_INTEGER, GET_OFFSET(VideoEncParams,nForcedKeyFrame), VideoForcedKeyFrameRange,  -1},
        { NULL, IDS_CL_V_COMPLEXITY, D_VIDEO,   T_INTEGER, GET_OFFSET(VideoEncParams,nComplexity),     VideoComplexityRange, 2 }
    };

/////////////////////////////////////////////////////////////////////////
TCHAR *FormatOutString( TCHAR *FormatString, va_list *Marker){
    TCHAR *OutString = NULL;
    if (0 == FormatMessage(
                FORMAT_MESSAGE_FROM_STRING | \
                FORMAT_MESSAGE_ALLOCATE_BUFFER,
                FormatString,
                0,
                0,
                (LPTSTR) &OutString,
                0,
                Marker)){
            return ( NULL );
    }
return OutString;
}

//////////////////////////////////////////////////////////////////////////////
TCHAR *FormatOutStringE( int ID, va_list *Marker){
    TCHAR *OutString = NULL;
    if (0 == FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE   | \
                FORMAT_MESSAGE_FROM_SYSTEM    | \
                FORMAT_MESSAGE_ALLOCATE_BUFFER,
                GetModuleHandle( NULL ),
                ID,
                0,
                (LPTSTR) &OutString,
                0,
                Marker)){
            return ( NULL );
    }
return OutString;
}

//////////////////////////////////////////////////////////////////////////////
int PrintOutlet( DWORD dwSource, int ID, ... ){
    TCHAR   Format[MAX_STRING_LEN], *pOutString = NULL;
    HMODULE hThisLib;
    va_list marker;
    va_start(marker, ID);
    
    switch( dwSource ){
    case S_RESOURCE:
        hThisLib = GetModuleHandle( NULL );
        if( 0 == LoadString( hThisLib, ID, Format, sizeof( Format )/sizeof(TCHAR) ) )
            return E_FAIL;
        pOutString = FormatOutString( Format, &marker );
        break;
    case S_MC:
        pOutString = FormatOutStringE( ID, &marker );
        break;
    default:
        return ( E_INVALIDARG );            
    }
    
    if( NULL == pOutString ){
        _tprintf( _T("0x%x"), (DWORD)ID );    
        return S_FALSE;
    }
    
    _tprintf( _T("%s"), pOutString );
    if( S_MC == dwSource ){
        _tprintf( _T("\n") );
    }
    LocalFree( pOutString );
    return ( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT LoadStringHelper( DWORD dwID, TCHAR **ppsz ){
    int     nLen;
    TCHAR   Buffer[ MAX_STRING_LEN ];

   if( NULL == ppsz ){
       return ( E_INVALIDARG );
   }

    nLen = LoadString( GetModuleHandle(NULL), \
                       dwID,                  \
                       Buffer,                \
                       sizeof( Buffer ) / sizeof( Buffer[0] ) );
    if( 0 >= nLen ){
        return GetLastError();
    }

    *ppsz = _tcsdup( Buffer );
    if( NULL == *ppsz ){
        return ( E_OUTOFMEMORY );
    }
return ( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT LoadCLStrings(){
    int     i  = sizeof( CommandEntryTable ) / sizeof( CommandEntryTable[0] );
    HRESULT hr = S_OK;

    //
    // Load the command line options strings
    //
    while( --i >= 0 ){
        hr = LoadStringHelper( CommandEntryTable[i].dwNameID, &CommandEntryTable[i].pszName );
        if( FAILED( hr ) ){
            return hr;
        }
    }

    //
    // load the uedio params strings
    //
    hr = LoadStringHelper( IDS_CL_ARG_A_WMSP, &AudioCodecRange[0] );
    if( FAILED( hr ) ){
        return hr;
    }
    hr = LoadStringHelper( IDS_CL_ARG_A_SPEECH, &AudioTypeRange[0] );
    if( FAILED( hr ) ){
        return hr;
    }
    hr = LoadStringHelper( IDS_CL_ARG_A_AUTO,   &AudioTypeRange[1] );
    if( FAILED( hr ) ){
        return hr;
    }
    //
    // load the Video params strings
    //
    hr = LoadStringHelper( IDS_CL_ARG_V_WMV3, &VideoCodecRange[0] );
    if( FAILED( hr ) ){
        return hr;
    }

    hr = LoadStringHelper( IDS_CL_ARG_V_MAIN,    &VideoProfileRange[0] );
    if( FAILED( hr ) ){
        return hr;
    }
    hr = LoadStringHelper( IDS_CL_ARG_V_SIMPLE,  &VideoProfileRange[1] );
    if( FAILED( hr ) ){
        return hr;
    }
    hr = LoadStringHelper( IDS_CL_ARG_V_COMPLEX, &VideoProfileRange[2] );
    if( FAILED( hr ) ){
        return hr;
    }
#ifdef SUPPORT_INTERLACE
    hr = LoadStringHelper( IDS_CL_ARG_V_PROGRESSIVE, &VideoTypeRange[0] );
    if( FAILED( hr ) ){
        return hr;
    }
    hr = LoadStringHelper( IDS_CL_ARG_V_INTERLACED,  &VideoTypeRange[1] );
    if( FAILED( hr ) ){
        return hr;
    }
#endif //SUPPORT_INTERLACE
   //
   // mode options
   //
    hr = LoadStringHelper( IDS_CL_ARG_1_PASS_CBR, &ModeRange[0] );
    if( FAILED( hr ) ){
        return hr;
    }
    hr = LoadStringHelper( IDS_CL_ARG_2_PASS_CBR, &ModeRange[1] );
    if( FAILED( hr ) ){
        return hr;
    }
    hr = LoadStringHelper( IDS_CL_ARG_1_PASS_VBR, &ModeRange[2] );
    if( FAILED( hr ) ){
        return hr;
    }
    hr = LoadStringHelper( IDS_CL_ARG_2_PASS_VBR, &ModeRange[3] );
    if( FAILED( hr ) ){
        return hr;
    }
    hr = LoadStringHelper( IDS_CL_ARG_2_PASS_CONSTRAINED_VBR, &ModeRange[4] );
    if( FAILED( hr ) ){
        return hr;
    }

return ( hr );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT FreeCLStrings(){
    int     i  = sizeof( CommandEntryTable ) / sizeof( CommandEntryTable[0] );

    //
    // Free the options strings
    //
    while( --i >= 0 ){
        SAFEFREE( CommandEntryTable[i].pszName );
    }

    //
    // Free the uedio params strings
    //
    SAFEFREE( AudioCodecRange[0] );
    SAFEFREE( AudioCodecRange[1] );
    SAFEFREE( AudioCodecRange[2] );
    SAFEFREE( AudioCodecRange[3] );
    SAFEFREE( AudioCodecRange[4] );

    SAFEFREE( AudioTypeRange[0] );
    SAFEFREE( AudioTypeRange[1] );

    //
    // Free the Video params strings
    //
    SAFEFREE( VideoCodecRange[0] );
    SAFEFREE( VideoCodecRange[1] );
    SAFEFREE( VideoCodecRange[2] );
    SAFEFREE( VideoCodecRange[3] );

    SAFEFREE( VideoProfileRange[0] );
    SAFEFREE( VideoProfileRange[1] );
    SAFEFREE( VideoProfileRange[2] );
#ifdef SUPPORT_INTERLACE
    SAFEFREE( VideoTypeRange[0] );
    SAFEFREE( VideoTypeRange[1] );
#endif //SUPPORT_INTERLACE
    //
    // Free mode options
    //
    SAFEFREE( ModeRange[0] );
    SAFEFREE( ModeRange[1] );
    SAFEFREE( ModeRange[2] );
    SAFEFREE( ModeRange[3] );
    SAFEFREE( ModeRange[4] );
return ( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
int ParseArgs( int argc, 
              TCHAR **argv, 
              GeneralParams  *pIOParams, 
              AudioEncParams *pAudioParams,
              VideoEncParams *pVideoParams ){
    BYTE  *pBase;
    TCHAR *pEndPtr;
    BOOL  fFound;
    int   i;
    
    if( 0 >= argc || NULL == argv ){
        return ( E_INVALIDARG );
    }
    
    if( NULL == *argv ){
        return ( E_INVALIDARG );
    }
    
    if( NULL == pIOParams || NULL == pAudioParams || NULL == pVideoParams ){
        return ( E_INVALIDARG );
    }
    
    while( argc-- ){
        if( (*argv)[0] == '/' || (*argv)[0] == '-' ){
            for( i = 0, fFound = FALSE ; i < sizeof( CommandEntryTable )/sizeof( CommandEntry );i++ ){
                if( 0 == _tcsicmp( (*argv) + 1, CommandEntryTable[i].pszName ) ){
                    switch( CommandEntryTable[i].nDomain ){
                    case D_GENERAL:
                        pBase = (BYTE*)pIOParams;
                        break;
                    case D_AUDIO:
                        pBase = (BYTE*)pAudioParams;
                        break;
                    case D_VIDEO:
                        pBase = (BYTE*)pVideoParams;
                        break;
                    default:
                        return ( E_PARSE_INTERNAL );
                    }
                    
                    if( NULL == pBase ){
                        return ( E_PARSE_INTERNAL );
                    }
                    
                    switch( CommandEntryTable[i].nType ){
                    case T_BOOL :
                        *((BOOL*)(pBase + CommandEntryTable[i].nOffset )) = TRUE;
                        break;
                    case T_INTEGER:
                        if( NULL != argv[1] ){
                            *((int*)(pBase + CommandEntryTable[i].nOffset )) = _ttoi(*++argv);
                            argc--;
                        } else {
                            return ( E_PARSE_NO_PARAM );
                        }
                        break;
                    case T_DOUBLE:
                        if( NULL != argv[1] ){
                            argv++;
                            *(double*)(pBase + CommandEntryTable[i].nOffset ) = _tcstod( *argv, &pEndPtr );
                            argc--;
                        } else {
                            return ( E_PARSE_NO_PARAM );
                        }
                        break;
                    case T_STRING:
                        if( NULL != argv[1] ){
                            SAFEFREE( *((TCHAR**)(pBase + CommandEntryTable[i].nOffset )) );
                            *((TCHAR**)(pBase + CommandEntryTable[i].nOffset )) = _tcsdup( *++argv );
                            if( NULL == (TCHAR*)(pBase + CommandEntryTable[i].nOffset ) ){
                                return ( E_OUTOFMEMORY );
                            }
                            argc--;
                        } else {
                            return ( E_PARSE_NO_PARAM );
                        }
                        break; 
                    default:
                        return ( E_PARSE_INTERNAL );
                        
                    }
                    fFound = TRUE;
                    break;
                }
            }
            if( !fFound ){
                return ( E_INVALID_ARG );
            }
            
        } else {
            return ( E_INVALID_ARG );
        }
        argv++;
    }
    
    return ( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
int CheckArgs( GeneralParams  *pIOParams, 
              AudioEncParams *pAudioParams,
              VideoEncParams *pVideoParams ){    
    TCHAR  *pszValue;
    BYTE   *pBase;
    int    nValue, i, j;
    double dValue;
    BOOL   fFound;
    
    if( NULL == pIOParams || NULL == pAudioParams || NULL == pVideoParams ){
        return ( E_INVALIDARG );
    }
    
    for( i = 0, fFound = FALSE;i < sizeof( CommandEntryTable ) / sizeof( CommandEntryTable[0] ); i++ ){
        if( NULL == CommandEntryTable[i].pRange  ){ //no range to test against
            continue;
        }
        
        switch( CommandEntryTable[i].nDomain ){
        case D_GENERAL:
            pBase = (BYTE*)pIOParams;
            break;
        case D_AUDIO:
            pBase = (BYTE*)pAudioParams;
            break;
        case D_VIDEO:
            pBase = (BYTE*)pVideoParams;
            break;
        default:
            PrintOutlet( S_MC, E_PARSE_INTERNAL );
            return ( E_PARSE_INTERNAL );
        }
        
        switch( CommandEntryTable[i].nType ){
        case T_BOOL :
            break;
        case T_INTEGER:
            nValue = *((int*)(pBase + CommandEntryTable[i].nOffset ));
            if( nValue < ((int*)CommandEntryTable[i].pRange)[0] || \
                nValue > ((int*)CommandEntryTable[i].pRange)[1] ){
                PrintOutlet( S_MC, E_PARSE_OUT_OF_RANGE, CommandEntryTable[i].pszName );
                return ( E_PARSE_OUT_OF_RANGE );
            }
            break;
        case T_DOUBLE:
            dValue = *((double*)(pBase + CommandEntryTable[i].nOffset ));
            if( dValue < ((double*)CommandEntryTable[i].pRange)[0] || \
                dValue > ((double*)CommandEntryTable[i].pRange)[1] ){
                PrintOutlet( S_MC, E_PARSE_OUT_OF_RANGE, CommandEntryTable[i].pszName );
                return ( E_PARSE_OUT_OF_RANGE );
            }
            break;
        case T_STRING:
            pszValue = *((TCHAR**)(pBase + CommandEntryTable[i].nOffset ));
            for( j=0, fFound = FALSE; NULL != ((TCHAR**)CommandEntryTable[i].pRange)[j]; j++ ){
                if( 0 == _tcsicmp( ((TCHAR**)CommandEntryTable[i].pRange)[j], pszValue ) ){
                    fFound = TRUE;
                    break;
                }
            }
            if( !fFound ){
                PrintOutlet( S_MC, E_PARSE_OUT_OF_RANGE, CommandEntryTable[i].pszName );
                return ( E_PARSE_OUT_OF_RANGE );
            }
            break; 
        default:
            PrintOutlet( S_MC, E_PARSE_INTERNAL );
            return ( E_PARSE_INTERNAL );
        }
    }
    //
    // additional checks for the input and output files
    //
    if( NULL == pIOParams->pszInput ){
        PrintOutlet( S_MC, E_PARSE_NO_INPUT );
        return ( E_PARSE_NO_INPUT );
    }
    if( NULL == pIOParams->pszOutput ){
        PrintOutlet( S_MC, E_PARSE_NO_OUTPUT );
        return ( E_PARSE_NO_OUTPUT );
    }
    
    if( pIOParams->fAudioOnly && pIOParams->fVideoOnly ){
        pIOParams->fVideoOnly = FALSE;
    }
    return ( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
int DefaultArgs( GeneralParams  *pIOParams, 
                AudioEncParams *pAudioParams,
                VideoEncParams *pVideoParams ){    
    TCHAR  **ppszTable;
    BYTE   *pBase;
    int    i;
    
    if( NULL == pIOParams || NULL == pAudioParams || NULL == pVideoParams ){
        return ( E_INVALIDARG );
    }
    
    for( i = 0; i < sizeof( CommandEntryTable ) / sizeof( CommandEntryTable[0] ); i++ ){
        switch( CommandEntryTable[i].nDomain ){
        case D_GENERAL:
            pBase = (BYTE*)pIOParams;
            break;
        case D_AUDIO:
            pBase = (BYTE*)pAudioParams;
            break;
        case D_VIDEO:
            pBase = (BYTE*)pVideoParams;
            break;
        default:
            return ( E_PARSE_INTERNAL );
        }
        
        switch( CommandEntryTable[i].nType ){
        case T_BOOL :
            *((BOOL*)(pBase + CommandEntryTable[i].nOffset )) = (BOOL)CommandEntryTable[i].dDefault;
            break;
        case T_INTEGER:
            *((int*)(pBase + CommandEntryTable[i].nOffset )) = (int)CommandEntryTable[i].dDefault;
            break;
        case T_DOUBLE:
            *((double*)(pBase + CommandEntryTable[i].nOffset )) = CommandEntryTable[i].dDefault;
            break;
        case T_STRING:
            SAFEFREE( *((TCHAR**)(pBase + CommandEntryTable[i].nOffset )) );
            ppszTable = (TCHAR**)CommandEntryTable[i].pRange;
            if( NULL == ppszTable ){
                *((TCHAR**)(pBase + CommandEntryTable[i].nOffset )) = NULL;
                break;
            }
            *((TCHAR**)(pBase + CommandEntryTable[i].nOffset )) = \
                _tcsdup( ppszTable[ (int)CommandEntryTable[i].dDefault] );
            if( NULL == (TCHAR*)(pBase + CommandEntryTable[i].nOffset ) ){
                return ( E_OUTOFMEMORY );
            }
            break; 
        default:
            return ( E_PARSE_INTERNAL );
        }
    }
    return ( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
int RemapAudioParams( AudioEncParams *pParams ){
    if( NULL == pParams ){
       return ( E_INVALIDARG );
    }

    if( NULL == pParams->pszCodecString ||
        NULL == pParams->pszSpeechMode  ){
       return ( E_INVALIDARG );
    }

    //
    // set the params for 1 pass CBR
    //
    pParams->fIsConstrained = FALSE;
    pParams->nPasses        = 1;
    pParams->fIsVBR         = FALSE;

    pParams->fIsSpeechMode = TRUE;
    if( 0 == _tcsicmp( pParams->pszSpeechMode, AudioTypeRange[1] ) ){ // ueto
        pParams->fIsSpeechMode = FALSE;
    }

    pParams->dwTag = WAVE_FORMAT_WMSPEECH1;
    if( pParams->nBitrate < 5500 &&
        pParams->nBitrate > 3500 &&
        !pParams->fIsSpeechMode ){
        return ( E_AUTO_MODE_NOT_SUPPORTED );
    }
return ( S_OK );
} 

//////////////////////////////////////////////////////////////////////////////
int RemapVideoParams( VideoEncParams *pParams ){
    if( NULL == pParams ){
       return ( E_INVALIDARG );
    }

    if( NULL == pParams->pszMode          ||
        NULL == pParams->pszCodecString   ||
#ifdef SUPPORT_INTERLACE
        NULL == pParams->pszVideoType     ||
#endif //SUPPORT_INTERLACE
        NULL == pParams->pszProfileString ){
       return ( E_INVALIDARG );
    }

    //
    //the modes are indexed as 
    //    0 - "1 pass cbr"
    //    1 - "2 pass cbr"
    //    2 - "1 pass vbr"
    //    3 - "2 pass vbr"
    //    4 - "2 pass constrained vbr"
    //

    //
    // set the params for 2 pass VBR
    //
    pParams->fIsConstrained = FALSE;
    pParams->nPasses        = 2;
    pParams->fIsVBR         = TRUE;

    if( 0 == _tcsicmp( pParams->pszMode, ModeRange[0] ) ){ // 1 pass CBR
        pParams->nPasses  = 1;
        pParams->fIsVBR   = FALSE;
    } else if( 0 == _tcsicmp( pParams->pszMode, ModeRange[1] ) ){ // 2 pass CBR
        pParams->fIsVBR   = FALSE;
    } else if( 0 == _tcsicmp( pParams->pszMode, ModeRange[2] ) ){ // 1 pass VBR
        pParams->nPasses  = 1;
    } else if( 0 == _tcsicmp( pParams->pszMode, ModeRange[4] ) ){ // 2 pass constrained VBR
        pParams->fIsConstrained = TRUE;
    }
#ifdef SUPPORT_INTERLACE
    pParams->fIsInterlaced = FALSE;
    if( 0 == _tcsicmp( pParams->pszVideoType, VideoTypeRange[1] ) ){ // interlaced
        pParams->fIsInterlaced = TRUE;
    }
#endif //SUPPORT_INTERLACE

    pParams->dwTag = WMCFOURCC_WMV3;

    pParams->nProfile = P_MAIN;
    if( 0 == _tcsicmp( pParams->pszProfileString, VideoProfileRange[1] ) ){ //simple
        pParams->nProfile = P_SIMPLE;
    } else if( 0 == _tcsicmp( pParams->pszProfileString, VideoProfileRange[2] ) ){ //complex
        pParams->nProfile = P_COMPLEX;
    }
    if( 0 != pParams->nBitrate && 0 == pParams->nPeakBitrate ){
        pParams->nPeakBitrate = pParams->nBitrate * 3;
    }
return ( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
void ShowProgress( TCHAR *pText, void *pContext ){
    if( NULL == pText )
        _tprintf( _T("*") );
    else 
        _tprintf( _T("%s\n"), pText );
}

//////////////////////////////////////////////////////////////////////////////
void usage(){
    int i, j;
    PrintOutlet( S_RESOURCE, IDS_USAGE, AppName );
    for( i = 0;i < sizeof( CommandEntryTable ) / sizeof( CommandEntryTable[0] ); i++ ){
        _tprintf( _T("-%s\t"), CommandEntryTable[i].pszName );

        if( NULL == CommandEntryTable[i].pRange ){
            _tprintf( _T("\n"));
            continue;
        }
                
        switch( CommandEntryTable[i].nType ){
        case T_BOOL :   
            break;
        case T_INTEGER: PrintOutlet( S_RESOURCE, IDS_RANGE );
            _tprintf(_T("%d, %d"),
                ((int*)CommandEntryTable[i].pRange)[0],
                ((int*)CommandEntryTable[i].pRange)[1] );
            PrintOutlet( S_RESOURCE, IDS_DEFAULT );
            _tprintf(_T("%g"), CommandEntryTable[i].dDefault );
            
            break;
        case T_DOUBLE:PrintOutlet( S_RESOURCE, IDS_RANGE );
            _tprintf(_T("%g, %g"),
                ((double*)CommandEntryTable[i].pRange)[0],
                ((double*)CommandEntryTable[i].pRange)[1] );
            PrintOutlet( S_RESOURCE, IDS_DEFAULT );
            _tprintf(_T("%f"), CommandEntryTable[i].dDefault );
            break;
        case T_STRING: PrintOutlet( S_RESOURCE, IDS_RANGE );
            for( j = 0; NULL != ((BYTE**)CommandEntryTable[i].pRange)[j]; j++ ){
                _tprintf(_T("%s %s"),
                    ( 0 == j ? _T("") : _T("|") ),
                    ((BYTE**)CommandEntryTable[i].pRange)[j] );
            }
            PrintOutlet( S_RESOURCE, IDS_DEFAULT );
            _tprintf(_T("%s"), ((BYTE**)CommandEntryTable[i].pRange)[(int)(CommandEntryTable[i].dDefault)] );
            break;
            break; 
        default:       PrintOutlet( S_MC, E_PARSE_INTERNAL );
            return;
        }
        _tprintf( _T("\n"));
    }
}

//////////////////////////////////////////////////////////////////////////////
int __cdecl main( int argc, TCHAR **argv ){
    IGraphBuilder  *pGraph      = NULL;
    GeneralParams  IOParams;
    AudioEncParams AudioParams;
    VideoEncParams VideoParams;
    HRESULT        hr           = S_OK;

    memset( &IOParams,    0, sizeof( IOParams ) );

    // set the app name - it's going to be used by some of the error messages 
    memset( AppName, 0, sizeof( AppName ) );
    {
    TCHAR Drive[_MAX_DRIVE + 1];
    TCHAR Dir[_MAX_DIR + 1];
    TCHAR Name[_MAX_FNAME + 1];
    TCHAR Ext[_MAX_EXT + 1];

    _tsplitpath( *argv, Drive, Dir, Name, Ext );
    _tcsncpy( &AppName[0], Name, sizeof( Name )/sizeof( TCHAR ) - 1 );    
    }

    argv++; argc--;

    do{
        hr = LoadCLStrings();
        if( hr != S_OK ){
            PrintOutlet( S_MC, hr );
            break;
        }

        hr = DefaultArgs( &IOParams, &AudioParams, &VideoParams );
        if( hr != S_OK ){
            PrintOutlet( S_MC, hr );   
            usage();
            break;
        }

        hr = ParseArgs( argc, argv, &IOParams, &AudioParams, &VideoParams );
        if( hr != S_OK ){
            PrintOutlet( S_MC, hr );
            usage();
            break;
        }

        hr = CheckArgs( &IOParams, &AudioParams, &VideoParams );
        if( hr != S_OK ){
            break;
        }

        hr = RemapAudioParams( &AudioParams );
        if( hr != S_OK ){
            PrintOutlet( S_MC, hr );
            break;
        }

        hr = RemapVideoParams( &VideoParams );
        if( hr != S_OK ){
            PrintOutlet( S_MC, hr );
            break;
        }

        hr = CoInitialize(NULL);
        if( FAILED( hr ) ){
            PrintOutlet( S_MC, E_NO_COM, AppName );
            break;
        }

        hr = EncodeFile( &IOParams, &AudioParams, &VideoParams );
        if( FAILED( hr ) ){
            PrintOutlet( S_MC, hr );
            break;
        }

    } while( FALSE );

    FreeCLStrings();
    CoUninitialize();
    return ( hr );
}

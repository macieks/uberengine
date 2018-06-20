//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

#include    <wtypes.h>
#include    <stdio.h>
#include    <math.h>
#include    <tchar.h>

#include    "cmn_macr.h"
#include    "resource.h"
#include    "decappErr.h"
#include    "decode.h"
#include    "uediodec.h"
#include    "videodec.h"

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
        { NULL, IDS_CL_INPUT,        D_GENERAL, T_STRING,  GET_OFFSET(GeneralParams, pszInput),       0,    0 },
        { NULL, IDS_CL_OUTPUT,       D_GENERAL, T_STRING,  GET_OFFSET(GeneralParams, pszOutput),      0,    0 },
        { NULL, IDS_CL_AUDIO_ONLY,   D_GENERAL, T_BOOL,    GET_OFFSET(GeneralParams, fAudioOnly ),    0,    0 },
        { NULL, IDS_CL_VIDEO_ONLY,   D_GENERAL, T_BOOL,    GET_OFFSET(GeneralParams, fVideoOnly ),    0,    0 },
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

return ( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
int ParseArgs( int argc, 
              TCHAR **argv, 
              GeneralParams  *pIOParams, 
              AudioDecParams *pAudioParams,
              VideoDecParams *pVideoParams ){
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
              AudioDecParams *pAudioParams,
              VideoDecParams *pVideoParams ){    
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
                AudioDecParams *pAudioParams,
                VideoDecParams *pVideoParams ){    
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
        case T_INTEGER:
           PrintOutlet( S_RESOURCE, IDS_RANGE );
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
    AudioDecParams AudioParams;
    VideoDecParams VideoParams;
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

        hr = CoInitialize(NULL);
        if( FAILED( hr ) ){
            PrintOutlet( S_MC, E_NO_COM, AppName );
            break;
        }

        hr = DecodeFile( &IOParams, &AudioParams, &VideoParams );
        if( FAILED( hr ) ){
            PrintOutlet( S_MC, hr );
            break;
        }

    } while( FALSE );

    FreeCLStrings();
    CoUninitialize();
    return ( hr );
}

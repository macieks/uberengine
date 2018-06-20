//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Macros.h
//
// Abstract:            Contains macros used to perform operations safely on
//                      pointers or handles
//
//*****************************************************************************

extern void OutputMsg(_TCHAR* msg);

#ifndef _MACROS_H_
#define _MACROS_H_

#define SAFE_RELEASE( x )   \
    if ( x )                \
    {                       \
        x->Release();       \
        x = NULL;           \
    }
    
#define SAFE_ADDREF( x )    \
    if ( x )                \
    {                       \
        x->AddRef();        \
    }

#define SAFE_DELETE( x )    \
    if ( x )                \
    {                       \
        delete x;           \
        x = NULL;           \
    }

#define SAFE_ARRAYDELETE( x )   \
    if ( x )                    \
    {                           \
        delete[] x;             \
        x = NULL;               \
    }

#define SAFE_SYSFREESTRING( x ) \
    if ( x )                    \
    {                           \
        SysFreeString( x );     \
        x = NULL;               \
    }

#define SAFE_CLOSEHANDLE( x )               \
    if ( x && INVALID_HANDLE_VALUE != x )   \
    {                                       \
        CloseHandle( x );                   \
        x = NULL;                           \
    }
#ifndef NUMELMS
   #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

#define ON_FAIL(string, x)			                            \
	if ( FAILED(x))							                    \
	{	                                                        \
        DbgLog((LOG_TRACE, 3, _T("%s  hr=0x%x\n"), string, x));	\
        _TCHAR buffer[1024];\
        wsprintf(buffer, "%s  hr=0x%x\r\n", string, x);\
        OutputMsg(buffer);\
        return x;							                    \
    }										                    \

#define ON_QI_FAIL(string, p)			\
	if (!p)							\
	{										\
		DbgLog((LOG_TRACE, 3, _T("%s\n"), string));	\
        return E_FAIL;							\
    }										\



#endif // _MACROS_H_

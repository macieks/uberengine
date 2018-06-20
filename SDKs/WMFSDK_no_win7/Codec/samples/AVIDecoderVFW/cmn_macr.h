//*@@@+++@@@@******************************************************************
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************

#ifndef CMN_MACR_H
#define CMN_MACR_H

//////////////////////////////////////////////////////////////////////////////
//
// General purpose defines
//
//////////////////////////////////////////////////////////////////////////////

enum        { S_RESOURCE, S_MC };

#define   SAFERELEASE(x)            if((x)){ (x)->Release(); (x) = NULL;}
#define   SAFEDELETE(x)             if((x)){ delete [] (x);  (x) = NULL;}
#define   SAFEDELETES(x)            if((x)){ delete    (x);  (x) = NULL;}
#define   SAFEFREE(x)               if((x)){ free(x);        (x) = NULL;}

#endif /*CMN_MACR_H*/

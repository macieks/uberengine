/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this 
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License 
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
 * implied. See the License for the specific language governing permissions and limitations under the 
 * License. 
 */

#ifndef __DAE_DOM_TYPES__
#define __DAE_DOM_TYPES__

#include <dae/daeElement.h>
#include <dae/daeMetaElement.h>
#include <dae/daeArray.h>
#include <dae/daeURI.h>
#include <dae/daeIDRef.h>

//This line is used as a workaround  because the array types enum is invalid when uetogenerated
//typedef daeString			domArrayTypes;				// ENUM
typedef daeElement			domElement;

typedef daeURI				xsAnyURI;
typedef daeString			xsDateTime;		

typedef daeString			xsID;
typedef daeIDRef			xsIDREF;
typedef daeTArray<daeIDRef> xsIDREFS;
typedef daeString			xsNCName;
typedef daeString			xsNMTOKEN;
typedef daeString			xsName;
typedef daeString			xsToken;
typedef daeString			xsString;
typedef daeBool				xsBoolean;
typedef daeShort			xsShort;
typedef daeInt				xsInt;
typedef daeLong				xsInteger;
typedef daeUInt				xsNonNegativeInteger;
typedef daeLong				xsLong;
typedef daeFloat			xsFloat;
typedef daeDouble			xsDouble;
typedef daeDouble			xsDecimal;
typedef daeCharArray		xsHexBinaryArray;
typedef daeBoolArray		xsBooleanArray;
typedef daeFloatArray		xsFloatArray;
typedef daeDoubleArray		xsDoubleArray;
typedef daeShortArray		xsShortArray;
typedef daeIntArray			xsIntegerArray;
typedef daeLongArray		xsLongArray;
typedef daeStringRefArray	xsNameArray;
typedef daeStringRefArray	xsNCNameArray;
typedef daeStringRefArray	xsTokenArray;

typedef daeChar				xsByte;
typedef daeUChar			xsUnsignedByte;
typedef	daeUInt				xsUnsignedInt;
typedef daeUInt				xsPositiveInteger;
typedef daeULong			xsUnsignedLong;


#define  daeTSmartRef			daeSmartRef

#endif	//__DAE_DOM_TYPES__


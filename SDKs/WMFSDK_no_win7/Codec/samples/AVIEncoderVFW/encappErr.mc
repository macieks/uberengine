;//*@@@+++@@@@******************************************************************
;//
;// Copyright (C) Microsoft Corporation. All rights reserved.
;//
;//*@@@---@@@@******************************************************************
;//###########################################################################
;//## AVIEncoderVFW                                                         ##
;//##-----------------------------------------------------------------------##
;//## 0x6000 - 0x6FFF Errors for AVIEncoderVFW                              ##
;//## This file contains the message definitions for the AVIEncoderVFW      ##
;//##                                                                       ##
;//###########################################################################
;
;
;/*
;-----------------------------------------------------------------------------
; HEADER SECTION
;
; The header section defines names and language identifiers for use
; by the message definitions later in this file. The MessageIdTypedef,
; SeverityNames, FacilityNames, and LanguageNames keywords are
; optional and not required.
;

MessageIdTypedef=DWORD

;
; The MessageIdTypedef keyword gives a typedef name that is used in a
; type cast for each message code in the generated include file. Each
; message code appears in the include file with the format: #define
; name ((type) 0xnnnnnnnn) The default value for type is empty, and no
; type cast is generated. It is the programmer's responsibility to
; specify a typedef statement in the application source code to define
; the type. The type used in the typedef must be large enough to
; accommodate the entire 32-bit message code.
;

SeverityNames = (
                    Success=0x0:STATUS_SEVERITY_SUCCESS
                    Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
                    Warning=0x2:STATUS_SEVERITY_WARNING
                    Error=0x3:STATUS_SEVERITY_ERROR
                )

;
; The SeverityNames keyword defines the set of names that are allowed
; as the value of the Severity keyword in the message definition. The
; set is delimited by left and right parentheses. Associated with each
; severity name is a number that, when shifted left by 30, gives the
; bit pattern to logical-OR with the Facility value and MessageId
; value to form the full 32-bit message code. The default value of
; this keyword is:
;
; SeverityNames=(
;   Success=0x0
;   Informational=0x1
;   Warning=0x2
;   Error=0x3
;   )
;
; Severity values occupy the high two bits of a 32-bit message code.
; Any severity value that does not fit in two bits is an error. The
; severity codes can be given symbolic names by following each value
; with :name
;

FacilityNames=  (
                    System=0x0:FACILITY_SYSTEM
                    Runtime=0x2:FACILITY_RUNTIME
                    Stubs=0x3:FACILITY_STUBS
                    Itf=0x4
                )

;
; The FacilityNames keyword defines the set of names that are allowed
; as the value of the Facility keyword in the message definition. The
; set is delimited by left and right parentheses. Associated with each
; facility name is a number that, when shifted left by 16 bits, gives
; the bit pattern to logical-OR with the Severity value and MessageId
; value to form the full 32-bit message code. The default value of
; this keyword is:
;
; FacilityNames=(
;   System=0x0FF
;   Application=0xFFF
; )
;
; Facility codes occupy the low order 12 bits of the high order
; 16-bits of a 32-bit message code. Any facility code that does not
; fit in 12 bits is an error. This allows for 4,096 facility codes.
; The first 256 codes are reserved for use by the system software. The
; facility codes can be given symbolic names by following each value
; with :name
;

LanguageNames=(English=1033:WM00001)

;
; The 1033 comes from the result of the MAKELANGID() macro
; (SUBLANG_ENGLISH_US << 10) | (LANG_ENGLISH)

;
; The LanguageNames keyword defines the set of names that are allowed
; as the value of the Language keyword in the message definition. The
; set is delimited by left and right parentheses. Associated with each
; language name is a number and a file name that are used to name the
; generated resource file that contains the messages for that
; language. The number corresponds to the language identifier to use
; in the resource table. The number is separated from the file name
; with a colon. The initial value of LanguageNames is:
;
; LanguageNames=(English=1:MSG00001)
;
; Any new names in the source file that don't override the built-in
; names are added to the list of valid languages. This allows an
; application to support private languages with descriptive names.
;
;
;-------------------------------------------------------------------------
; MESSAGE DEFINITION SECTION
;
; Following the header section is the body of the Message Compiler
; source file. The body consists of zero or more message definitions.
; Each message definition begins with one or more of the following
; statements:
;
; MessageId = [number|+number]
; Severity = severity_name
; Facility = facility_name
; SymbolicName = name
;
; The MessageId statement marks the beginning of the message
; definition. A MessageID statement is required for each message,
; although the value is optional. If no value is specified, the value
; used is the previous value for the facility plus one. If the value
; is specified as +number, then the value used is the previous value
; for the facility plus the number after the plus sign. Otherwise, if
; a numeric value is given, that value is used. Any MessageId value
; that does not fit in 16 bits is an error.
;
; The Severity and Facility statements are optional. These statements
; specify additional bits to OR into the final 32-bit message code. If
; not specified, they default to the value last specified for a message
; definition. The initial values prior to processing the first message
; definition are:
;
; Severity=Success
; Facility=Application
;
; The value associated with Severity and Facility must match one of
; the names given in the FacilityNames and SeverityNames statements in
; the header section. The SymbolicName statement allows you to
; associate a C/C++ symbolic constant with the final 32-bit message
; code.
;
;
;-----------------------------------------------------------------------------
;  IMPORTANT - PLEASE READ BEFORE EDITING FILE
;  This file is divided into four sections. They are:
;   1. Success Codes
;   2. Information Codes
;   3. Warning Codes
;   4. Error Codes
;
;  Please enter your codes in the appropriate section.
;  All codes must be in sorted order. Please use codes
;  in the middle that are free before using codes at the end.
;*/


MessageId=0x6000
Severity=Error
Facility=Itf
SymbolicName=E_INVALID_ARG
Language=English
One of the arguments is not supported.%0
.

MessageId=0x6001
Severity=Error
Facility=Itf
SymbolicName=E_PARSE_INTERNAL
Language=English
Internal parsing error occured. Please report this error.%0
.

MessageId=0x6002
Severity=Error
Facility=Itf
SymbolicName=E_PARSE_NO_PARAM
Language=English
Argument "%1" needs a parameter.%0
.

MessageId=0x6004
Severity=Error
Facility=Itf
SymbolicName=E_PARSE_OUT_OF_RANGE
Language=English
Parameter "%1" is out of range.%0
.

MessageId=0x6005
Severity=Error
Facility=Itf
SymbolicName=E_PARSE_NO_INPUT
Language=English
Input file was not specified.%0
.

MessageId=0x6006
Severity=Error
Facility=Itf
SymbolicName=E_PARSE_NO_OUTPUT
Language=English
Output file was not specified.%0
.

MessageId=0x6007
Severity=Error
Facility=Itf
SymbolicName=E_CONFLICTING_ONLYS
Language=English
"audio_only" and "video_only" arguments are conflicting.%0
.

MessageId=0x6008
Severity=Error
Facility=Itf
SymbolicName=E_REMAP_AUDIO
Language=English
Failed to remap audio arguments.%0
.

MessageId=0x6009
Severity=Error
Facility=Itf
SymbolicName=E_REMAP_VIDEO
Language=English
Failed to remap video arguments.%0
.

MessageId=0x6010
Severity=Error
Facility=Itf
SymbolicName=E_NO_COM
Language=English
Application "%1" failed to initialize COM.%0
.

MessageId=0x6019
Severity=Error
Facility=Itf
SymbolicName=E_INVALID_INPUT
Language=English
Can't open input file.%0
.

MessageId=0x601a
Severity=Error
Facility=Itf
SymbolicName=E_INVALID_OUTPUT
Language=English
Can't open output file.%0
.

MessageId=0x601c
Severity=Error
Facility=Itf
SymbolicName=E_CANT_UNCOMPRESS
Language=English
Can't uncompress the input stream.%0
.

MessageId=0x6020
Severity=Error
Facility=Itf
SymbolicName=E_NO_STREAM
Language=English
There is no stream available to encode.%0
.

MessageId=0x6033
Severity=Error
Facility=Itf
SymbolicName=E_NO_INPUT_TYPE
Language=English
Can't obtain the input type for the audio stream.%0
.

MessageId=0x603a
Severity=Error
Facility=Itf
SymbolicName=E_FORMAT_NOT_SUPPORTED
Language=English
One stream has a format wich is not WAVFORMATEX, VIDEOINFO or VIDEOINFO2.%0
.

MessageId=0x603b
Severity=Error
Facility=Itf
SymbolicName=E_MEDIA_TYPE_REJECTED
Language=English
Pump Filter can't accept this format.%0
.

MessageId=0x603c
Severity=Error
Facility=Itf
SymbolicName=E_NO_CONTEXT
Language=English
Tap Filter can't provide the contextpad.%0
.

MessageId=0x6050
Severity=Error
Facility=Itf
SymbolicName=E_DMO_NO_STATUS
Language=English
DMO encoder status cannot be obtained.%0
.

MessageId=0x6051
Severity=Error
Facility=Itf
SymbolicName=E_DMO_NOTACCEPTING
Language=English
DMO encoder doesn't accept input data.%0
.

MessageId=0x6060
Severity=Error
Facility=Itf
SymbolicName=E_AUDIO_INVALID_FORMAT
Language=English
Audio DMO encoder enumerated an invalid media type.%0
.

MessageId=0x6061
Severity=Error
Facility=Itf
SymbolicName=E_AUDIO_FORMAT_DOESNT_MATCH
Language=English
Audio parameters don't match with the supported encoder modes.%0
.

MessageId=0x6062
Severity=Error
Facility=Itf
SymbolicName=E_AUDIO_COMPRESSION_NOT_SUPPORTED
Language=English
Requested audio compression algorithm is not supported.%0
.

MessageId=0x6063
Severity=Error
Facility=Itf
SymbolicName=E_AUDIO_VBR_NOT_SUPPORTED
Language=English
VBR is not supported for the current settings.%0
.

MessageId=0x6064
Severity=Error
Facility=Itf
SymbolicName=E_AUDIO_NPASS_NOT_SUPPORTED
Language=English
Multiple passes not supported for the current settings.%0
.

MessageId=0x6065
Severity=Error
Facility=Itf
SymbolicName=E_AUDIO_PEAK_BITRATE_REJECTED
Language=English
Audio peak bitrate was rejected.%0
.

MessageId=0x6066
Severity=Error
Facility=Itf
SymbolicName=E_AUDIO_PEAK_BUFFER_REJECTED
Language=English
Audio peak buffer was rejected.%0
.

MessageId=0x6067
Severity=Error
Facility=Itf
SymbolicName=E_SPEECH_MODE_NOT_SUPPORTED
Language=English
Audio type ( speech/music ) not supported for the current settings.%0
.

MessageId=0x6068
Severity=Error
Facility=Itf
SymbolicName=E_AUDIO_TYPE_NOT_SET
Language=English
DMO audio encoder rejected the type.%0
.

MessageId=0x6070
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_COMPRESSION_NOT_SUPPORTED
Language=English
Requested video compression algorithm is not supported.%0
.

MessageId=0x6071
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_INVALID_INPUT_TYPE
Language=English
Video input type is invalid.%0
.

MessageId=0x6072
Severity=Error
Facility=Itf
SymbolicName=E_NO_PRIVATE_DATA
Language=English
Video encoder doesn't support IWMPrivateData interface.%0
.

MessageId=0x6073
Severity=Error
Facility=Itf
SymbolicName=E_PARTIAL_TYPE_REJECTED
Language=English
Video partial output type was rejected.%0
.

MessageId=0x6074
Severity=Error
Facility=Itf
SymbolicName=E_NO_PRIVATE_DATA_COUNT
Language=English
Video encoder failed to provide the size of private data.%0
.

MessageId=0x6075
Severity=Error
Facility=Itf
SymbolicName=E_PRIVATE_DATA_FAILED
Language=English
Video encoder failed to provide the private data.%0
.

MessageId=0x6076
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_VBR_NOT_SUPPORTED
Language=English
Video encoder doesn't support VBR mode.%0
.

MessageId=0x6077
Severity=Error
Facility=Itf
SymbolicName=E_VBR_QUALITY_REJECTED
Language=English
Video encoder rejected the quality for 1 pass VBR mode.%0
.

MessageId=0x6078
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_NPASS_NOT_SUPPORTED
Language=English
Video encoder doesn't support multiple pass encoding.%0
.

MessageId=0x6079
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_BITRATE_REJECTED
Language=English
Video encoder rejected the average bitrate provided.%0
.

MessageId=0x607a
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_BUFFER_REJECTED
Language=English
Video encoder rejected the buffer size provided.%0
.

MessageId=0x607b
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_PEAK_BITRATE_REJECTED
Language=English
Video encoder rejected the maximum bitrate provided.%0
.

MessageId=0x607c
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_PEAK_BUFFER_REJECTED
Language=English
Video encoder rejected the maximum buffer size provided.%0
.

MessageId=0x607d
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_PROFILE_REJECTED
Language=English
Video encoder rejected the profile provided.%0
.

MessageId=0x607e
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_KEYDIST_REJECTED
Language=English
Video encoder rejected the key frame distance provided.%0
.

MessageId=0x607f
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_CRISPNESS_REJECTED
Language=English
Video encoder rejected the quality parameter provided.%0
.

MessageId=0x6080
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_INTERLACE_REJECTED
Language=English
Tnterlace mode mode rejected.%0
.

MessageId=0x6081
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_INVALID_PROFILE
Language=English
Video profile index out of range.%0
.

MessageId=0x6082
Severity=Error
Facility=Itf
SymbolicName=E_INVALID_FORMAT
Language=English
Input stream format is invalid.%0
.

MessageId=0x6083
Severity=Error
Facility=Itf
SymbolicName=E_VIDEO_TYPE_NOT_SET
Language=English
DMO video encoder rejected the type.%0
.

MessageId=0x6090
Severity=Error
Facility=Itf
SymbolicName=E_BAD_STREAM_LENGTH
Language=English
Input stream has incorrect length.%0
.

MessageId=0x6091
Severity=Error
Facility=Itf
SymbolicName=E_READ_FRAME
Language=English
Failed to read one input frame.%0
.

MessageId=0x6092
Severity=Error
Facility=Itf
SymbolicName=E_NO_FRAMES
Language=English
No frames written to the output file.%0
.

MessageId=0x6093
Severity=Error
Facility=Itf
SymbolicName=E_NEEDS_LOOKAHEAD
Language=English
The encoder needs lookahead.%0
.

MessageId=0x6094
Severity=Error
Facility=Itf
SymbolicName=E_AUTO_MODE_NOT_SUPPORTED
Language=English
The speech codec doesn't support auto mode for this bitrate.%0
.

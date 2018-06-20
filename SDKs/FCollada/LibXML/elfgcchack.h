/*
 * elfgcchack.h: hack by Arjan van de Ven <arjanv@redhat.com> to speed
 *               up the code when using gcc for call within the library.
 *
 * Based on the analysis http://people.redhat.com/drepper/dsohowto.pdf
 * from Ulrich drepper. Rewritten to be generated from the XML description
 * file for libxml2 API
 * uetogenerated with xsltproc doc/elfgcchack.xsl doc/libxml2-api.xml
 */

#ifdef IN_LIBXML
#ifdef __GNUC__
#ifdef PIC
#ifdef linux
#if (__GNUC__ == 3 && __GNUC_MINOR__ >= 3) || (__GNUC__ > 3)

#include "libxml/c14n.h"
#include "libxml/catalog.h"
#include "libxml/chvalid.h"
#include "libxml/debugXML.h"
#include "libxml/dict.h"
#include "libxml/DOCBparser.h"
#include "libxml/encoding.h"
#include "libxml/entities.h"
#include "libxml/globals.h"
#include "libxml/hash.h"
#include "libxml/HTMLparser.h"
#include "libxml/HTMLtree.h"
#include "libxml/list.h"
#include "libxml/nanoftp.h"
#include "libxml/nanohttp.h"
#include "libxml/parser.h"
#include "libxml/parserInternals.h"
#include "libxml/pattern.h"
#include "libxml/relaxng.h"
#include "libxml/SAX2.h"
#include "libxml/SAX.h"
#include "libxml/schemasInternals.h"
#include "libxml/threads.h"
#include "libxml/tree.h"
#include "libxml/uri.h"
#include "libxml/valid.h"
#include "libxml/xinclude.h"
#include "libxml/xlink.h"
#include "libxml/xmlautomata.h"
#include "libxml/xmlerror.h"
#include "libxml/xmlexports.h"
#include "libxml/xmlIO.h"
#include "libxml/xmlmemory.h"
#include "libxml/xmlreader.h"
#include "libxml/xmlregexp.h"
#include "libxml/xmlsave.h"
#include "libxml/xmlschemas.h"
#include "libxml/xmlschemastypes.h"
#include "libxml/xmlstring.h"
#include "libxml/xmlunicode.h"
#include "libxml/xmlversion.h"
#include "libxml/xmlwriter.h"
#include "libxml/xpath.h"
#include "libxml/xpathInternals.h"
#include "libxml/xpointer.h"
#include "libxml/xmlmodule.h"

/* special hot spot not exported ones */

#ifdef bottom_globals
#undef __xmlGenericError
extern __typeof (__xmlGenericError) __xmlGenericError __attribute((alias("__xmlGenericError__internal_alias")));
#else
#ifndef __xmlGenericError
extern __typeof (__xmlGenericError) __xmlGenericError__internal_alias __attribute((visibility("hidden")));
#define __xmlGenericError __xmlGenericError__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef __xmlGenericErrorContext
extern __typeof (__xmlGenericErrorContext) __xmlGenericErrorContext __attribute((alias("__xmlGenericErrorContext__internal_alias")));
#else
#ifndef __xmlGenericErrorContext
extern __typeof (__xmlGenericErrorContext) __xmlGenericErrorContext__internal_alias __attribute((visibility("hidden")));
#define __xmlGenericErrorContext __xmlGenericErrorContext__internal_alias
#endif
#endif

/* list generated from libxml2-api.xml */
#if defined(LIBXML_DOCB_ENABLED)
#ifdef bottom_DOCBparser
#undef docbCreatePushParserCtxt
extern __typeof (docbCreatePushParserCtxt) docbCreatePushParserCtxt __attribute((alias("docbCreatePushParserCtxt__internal_alias")));
#else
#ifndef docbCreatePushParserCtxt
extern __typeof (docbCreatePushParserCtxt) docbCreatePushParserCtxt__internal_alias __attribute((visibility("hidden")));
#define docbCreatePushParserCtxt docbCreatePushParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlAttrAllowed
extern __typeof (htmlAttrAllowed) htmlAttrAllowed __attribute((alias("htmlAttrAllowed__internal_alias")));
#else
#ifndef htmlAttrAllowed
extern __typeof (htmlAttrAllowed) htmlAttrAllowed__internal_alias __attribute((visibility("hidden")));
#define htmlAttrAllowed htmlAttrAllowed__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlAutoCloseTag
extern __typeof (htmlAutoCloseTag) htmlAutoCloseTag __attribute((alias("htmlAutoCloseTag__internal_alias")));
#else
#ifndef htmlAutoCloseTag
extern __typeof (htmlAutoCloseTag) htmlAutoCloseTag__internal_alias __attribute((visibility("hidden")));
#define htmlAutoCloseTag htmlAutoCloseTag__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlCreateFileParserCtxt
extern __typeof (htmlCreateFileParserCtxt) htmlCreateFileParserCtxt __attribute((alias("htmlCreateFileParserCtxt__internal_alias")));
#else
#ifndef htmlCreateFileParserCtxt
extern __typeof (htmlCreateFileParserCtxt) htmlCreateFileParserCtxt__internal_alias __attribute((visibility("hidden")));
#define htmlCreateFileParserCtxt htmlCreateFileParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlCreateMemoryParserCtxt
extern __typeof (htmlCreateMemoryParserCtxt) htmlCreateMemoryParserCtxt __attribute((alias("htmlCreateMemoryParserCtxt__internal_alias")));
#else
#ifndef htmlCreateMemoryParserCtxt
extern __typeof (htmlCreateMemoryParserCtxt) htmlCreateMemoryParserCtxt__internal_alias __attribute((visibility("hidden")));
#define htmlCreateMemoryParserCtxt htmlCreateMemoryParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_PUSH_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlCreatePushParserCtxt
extern __typeof (htmlCreatePushParserCtxt) htmlCreatePushParserCtxt __attribute((alias("htmlCreatePushParserCtxt__internal_alias")));
#else
#ifndef htmlCreatePushParserCtxt
extern __typeof (htmlCreatePushParserCtxt) htmlCreatePushParserCtxt__internal_alias __attribute((visibility("hidden")));
#define htmlCreatePushParserCtxt htmlCreatePushParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlCtxtReadDoc
extern __typeof (htmlCtxtReadDoc) htmlCtxtReadDoc __attribute((alias("htmlCtxtReadDoc__internal_alias")));
#else
#ifndef htmlCtxtReadDoc
extern __typeof (htmlCtxtReadDoc) htmlCtxtReadDoc__internal_alias __attribute((visibility("hidden")));
#define htmlCtxtReadDoc htmlCtxtReadDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlCtxtReadFd
extern __typeof (htmlCtxtReadFd) htmlCtxtReadFd __attribute((alias("htmlCtxtReadFd__internal_alias")));
#else
#ifndef htmlCtxtReadFd
extern __typeof (htmlCtxtReadFd) htmlCtxtReadFd__internal_alias __attribute((visibility("hidden")));
#define htmlCtxtReadFd htmlCtxtReadFd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlCtxtReadFile
extern __typeof (htmlCtxtReadFile) htmlCtxtReadFile __attribute((alias("htmlCtxtReadFile__internal_alias")));
#else
#ifndef htmlCtxtReadFile
extern __typeof (htmlCtxtReadFile) htmlCtxtReadFile__internal_alias __attribute((visibility("hidden")));
#define htmlCtxtReadFile htmlCtxtReadFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlCtxtReadIO
extern __typeof (htmlCtxtReadIO) htmlCtxtReadIO __attribute((alias("htmlCtxtReadIO__internal_alias")));
#else
#ifndef htmlCtxtReadIO
extern __typeof (htmlCtxtReadIO) htmlCtxtReadIO__internal_alias __attribute((visibility("hidden")));
#define htmlCtxtReadIO htmlCtxtReadIO__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlCtxtReadMemory
extern __typeof (htmlCtxtReadMemory) htmlCtxtReadMemory __attribute((alias("htmlCtxtReadMemory__internal_alias")));
#else
#ifndef htmlCtxtReadMemory
extern __typeof (htmlCtxtReadMemory) htmlCtxtReadMemory__internal_alias __attribute((visibility("hidden")));
#define htmlCtxtReadMemory htmlCtxtReadMemory__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlCtxtReset
extern __typeof (htmlCtxtReset) htmlCtxtReset __attribute((alias("htmlCtxtReset__internal_alias")));
#else
#ifndef htmlCtxtReset
extern __typeof (htmlCtxtReset) htmlCtxtReset__internal_alias __attribute((visibility("hidden")));
#define htmlCtxtReset htmlCtxtReset__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlCtxtUseOptions
extern __typeof (htmlCtxtUseOptions) htmlCtxtUseOptions __attribute((alias("htmlCtxtUseOptions__internal_alias")));
#else
#ifndef htmlCtxtUseOptions
extern __typeof (htmlCtxtUseOptions) htmlCtxtUseOptions__internal_alias __attribute((visibility("hidden")));
#define htmlCtxtUseOptions htmlCtxtUseOptions__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_SAX2
#undef htmlDefaultSAXHandlerInit
extern __typeof (htmlDefaultSAXHandlerInit) htmlDefaultSAXHandlerInit __attribute((alias("htmlDefaultSAXHandlerInit__internal_alias")));
#else
#ifndef htmlDefaultSAXHandlerInit
extern __typeof (htmlDefaultSAXHandlerInit) htmlDefaultSAXHandlerInit__internal_alias __attribute((visibility("hidden")));
#define htmlDefaultSAXHandlerInit htmlDefaultSAXHandlerInit__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlDocContentDumpFormatOutput
extern __typeof (htmlDocContentDumpFormatOutput) htmlDocContentDumpFormatOutput __attribute((alias("htmlDocContentDumpFormatOutput__internal_alias")));
#else
#ifndef htmlDocContentDumpFormatOutput
extern __typeof (htmlDocContentDumpFormatOutput) htmlDocContentDumpFormatOutput__internal_alias __attribute((visibility("hidden")));
#define htmlDocContentDumpFormatOutput htmlDocContentDumpFormatOutput__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlDocContentDumpOutput
extern __typeof (htmlDocContentDumpOutput) htmlDocContentDumpOutput __attribute((alias("htmlDocContentDumpOutput__internal_alias")));
#else
#ifndef htmlDocContentDumpOutput
extern __typeof (htmlDocContentDumpOutput) htmlDocContentDumpOutput__internal_alias __attribute((visibility("hidden")));
#define htmlDocContentDumpOutput htmlDocContentDumpOutput__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlDocDump
extern __typeof (htmlDocDump) htmlDocDump __attribute((alias("htmlDocDump__internal_alias")));
#else
#ifndef htmlDocDump
extern __typeof (htmlDocDump) htmlDocDump__internal_alias __attribute((visibility("hidden")));
#define htmlDocDump htmlDocDump__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlDocDumpMemory
extern __typeof (htmlDocDumpMemory) htmlDocDumpMemory __attribute((alias("htmlDocDumpMemory__internal_alias")));
#else
#ifndef htmlDocDumpMemory
extern __typeof (htmlDocDumpMemory) htmlDocDumpMemory__internal_alias __attribute((visibility("hidden")));
#define htmlDocDumpMemory htmlDocDumpMemory__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlElementAllowedHere
extern __typeof (htmlElementAllowedHere) htmlElementAllowedHere __attribute((alias("htmlElementAllowedHere__internal_alias")));
#else
#ifndef htmlElementAllowedHere
extern __typeof (htmlElementAllowedHere) htmlElementAllowedHere__internal_alias __attribute((visibility("hidden")));
#define htmlElementAllowedHere htmlElementAllowedHere__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlElementStatusHere
extern __typeof (htmlElementStatusHere) htmlElementStatusHere __attribute((alias("htmlElementStatusHere__internal_alias")));
#else
#ifndef htmlElementStatusHere
extern __typeof (htmlElementStatusHere) htmlElementStatusHere__internal_alias __attribute((visibility("hidden")));
#define htmlElementStatusHere htmlElementStatusHere__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlEncodeEntities
extern __typeof (htmlEncodeEntities) htmlEncodeEntities __attribute((alias("htmlEncodeEntities__internal_alias")));
#else
#ifndef htmlEncodeEntities
extern __typeof (htmlEncodeEntities) htmlEncodeEntities__internal_alias __attribute((visibility("hidden")));
#define htmlEncodeEntities htmlEncodeEntities__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlEntityLookup
extern __typeof (htmlEntityLookup) htmlEntityLookup __attribute((alias("htmlEntityLookup__internal_alias")));
#else
#ifndef htmlEntityLookup
extern __typeof (htmlEntityLookup) htmlEntityLookup__internal_alias __attribute((visibility("hidden")));
#define htmlEntityLookup htmlEntityLookup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlEntityValueLookup
extern __typeof (htmlEntityValueLookup) htmlEntityValueLookup __attribute((alias("htmlEntityValueLookup__internal_alias")));
#else
#ifndef htmlEntityValueLookup
extern __typeof (htmlEntityValueLookup) htmlEntityValueLookup__internal_alias __attribute((visibility("hidden")));
#define htmlEntityValueLookup htmlEntityValueLookup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlFreeParserCtxt
extern __typeof (htmlFreeParserCtxt) htmlFreeParserCtxt __attribute((alias("htmlFreeParserCtxt__internal_alias")));
#else
#ifndef htmlFreeParserCtxt
extern __typeof (htmlFreeParserCtxt) htmlFreeParserCtxt__internal_alias __attribute((visibility("hidden")));
#define htmlFreeParserCtxt htmlFreeParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlGetMetaEncoding
extern __typeof (htmlGetMetaEncoding) htmlGetMetaEncoding __attribute((alias("htmlGetMetaEncoding__internal_alias")));
#else
#ifndef htmlGetMetaEncoding
extern __typeof (htmlGetMetaEncoding) htmlGetMetaEncoding__internal_alias __attribute((visibility("hidden")));
#define htmlGetMetaEncoding htmlGetMetaEncoding__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlHandleOmittedElem
extern __typeof (htmlHandleOmittedElem) htmlHandleOmittedElem __attribute((alias("htmlHandleOmittedElem__internal_alias")));
#else
#ifndef htmlHandleOmittedElem
extern __typeof (htmlHandleOmittedElem) htmlHandleOmittedElem__internal_alias __attribute((visibility("hidden")));
#define htmlHandleOmittedElem htmlHandleOmittedElem__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlInitAutoClose
extern __typeof (htmlInitAutoClose) htmlInitAutoClose __attribute((alias("htmlInitAutoClose__internal_alias")));
#else
#ifndef htmlInitAutoClose
extern __typeof (htmlInitAutoClose) htmlInitAutoClose__internal_alias __attribute((visibility("hidden")));
#define htmlInitAutoClose htmlInitAutoClose__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlIsAutoClosed
extern __typeof (htmlIsAutoClosed) htmlIsAutoClosed __attribute((alias("htmlIsAutoClosed__internal_alias")));
#else
#ifndef htmlIsAutoClosed
extern __typeof (htmlIsAutoClosed) htmlIsAutoClosed__internal_alias __attribute((visibility("hidden")));
#define htmlIsAutoClosed htmlIsAutoClosed__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlIsBooleanAttr
extern __typeof (htmlIsBooleanAttr) htmlIsBooleanAttr __attribute((alias("htmlIsBooleanAttr__internal_alias")));
#else
#ifndef htmlIsBooleanAttr
extern __typeof (htmlIsBooleanAttr) htmlIsBooleanAttr__internal_alias __attribute((visibility("hidden")));
#define htmlIsBooleanAttr htmlIsBooleanAttr__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlIsScriptAttribute
extern __typeof (htmlIsScriptAttribute) htmlIsScriptAttribute __attribute((alias("htmlIsScriptAttribute__internal_alias")));
#else
#ifndef htmlIsScriptAttribute
extern __typeof (htmlIsScriptAttribute) htmlIsScriptAttribute__internal_alias __attribute((visibility("hidden")));
#define htmlIsScriptAttribute htmlIsScriptAttribute__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlNewDoc
extern __typeof (htmlNewDoc) htmlNewDoc __attribute((alias("htmlNewDoc__internal_alias")));
#else
#ifndef htmlNewDoc
extern __typeof (htmlNewDoc) htmlNewDoc__internal_alias __attribute((visibility("hidden")));
#define htmlNewDoc htmlNewDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlNewDocNoDtD
extern __typeof (htmlNewDocNoDtD) htmlNewDocNoDtD __attribute((alias("htmlNewDocNoDtD__internal_alias")));
#else
#ifndef htmlNewDocNoDtD
extern __typeof (htmlNewDocNoDtD) htmlNewDocNoDtD__internal_alias __attribute((visibility("hidden")));
#define htmlNewDocNoDtD htmlNewDocNoDtD__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlNodeDump
extern __typeof (htmlNodeDump) htmlNodeDump __attribute((alias("htmlNodeDump__internal_alias")));
#else
#ifndef htmlNodeDump
extern __typeof (htmlNodeDump) htmlNodeDump__internal_alias __attribute((visibility("hidden")));
#define htmlNodeDump htmlNodeDump__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlNodeDumpFile
extern __typeof (htmlNodeDumpFile) htmlNodeDumpFile __attribute((alias("htmlNodeDumpFile__internal_alias")));
#else
#ifndef htmlNodeDumpFile
extern __typeof (htmlNodeDumpFile) htmlNodeDumpFile__internal_alias __attribute((visibility("hidden")));
#define htmlNodeDumpFile htmlNodeDumpFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlNodeDumpFileFormat
extern __typeof (htmlNodeDumpFileFormat) htmlNodeDumpFileFormat __attribute((alias("htmlNodeDumpFileFormat__internal_alias")));
#else
#ifndef htmlNodeDumpFileFormat
extern __typeof (htmlNodeDumpFileFormat) htmlNodeDumpFileFormat__internal_alias __attribute((visibility("hidden")));
#define htmlNodeDumpFileFormat htmlNodeDumpFileFormat__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlNodeDumpFormatOutput
extern __typeof (htmlNodeDumpFormatOutput) htmlNodeDumpFormatOutput __attribute((alias("htmlNodeDumpFormatOutput__internal_alias")));
#else
#ifndef htmlNodeDumpFormatOutput
extern __typeof (htmlNodeDumpFormatOutput) htmlNodeDumpFormatOutput__internal_alias __attribute((visibility("hidden")));
#define htmlNodeDumpFormatOutput htmlNodeDumpFormatOutput__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlNodeDumpOutput
extern __typeof (htmlNodeDumpOutput) htmlNodeDumpOutput __attribute((alias("htmlNodeDumpOutput__internal_alias")));
#else
#ifndef htmlNodeDumpOutput
extern __typeof (htmlNodeDumpOutput) htmlNodeDumpOutput__internal_alias __attribute((visibility("hidden")));
#define htmlNodeDumpOutput htmlNodeDumpOutput__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlNodeStatus
extern __typeof (htmlNodeStatus) htmlNodeStatus __attribute((alias("htmlNodeStatus__internal_alias")));
#else
#ifndef htmlNodeStatus
extern __typeof (htmlNodeStatus) htmlNodeStatus__internal_alias __attribute((visibility("hidden")));
#define htmlNodeStatus htmlNodeStatus__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlParseCharRef
extern __typeof (htmlParseCharRef) htmlParseCharRef __attribute((alias("htmlParseCharRef__internal_alias")));
#else
#ifndef htmlParseCharRef
extern __typeof (htmlParseCharRef) htmlParseCharRef__internal_alias __attribute((visibility("hidden")));
#define htmlParseCharRef htmlParseCharRef__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_PUSH_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlParseChunk
extern __typeof (htmlParseChunk) htmlParseChunk __attribute((alias("htmlParseChunk__internal_alias")));
#else
#ifndef htmlParseChunk
extern __typeof (htmlParseChunk) htmlParseChunk__internal_alias __attribute((visibility("hidden")));
#define htmlParseChunk htmlParseChunk__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlParseDoc
extern __typeof (htmlParseDoc) htmlParseDoc __attribute((alias("htmlParseDoc__internal_alias")));
#else
#ifndef htmlParseDoc
extern __typeof (htmlParseDoc) htmlParseDoc__internal_alias __attribute((visibility("hidden")));
#define htmlParseDoc htmlParseDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlParseDocument
extern __typeof (htmlParseDocument) htmlParseDocument __attribute((alias("htmlParseDocument__internal_alias")));
#else
#ifndef htmlParseDocument
extern __typeof (htmlParseDocument) htmlParseDocument__internal_alias __attribute((visibility("hidden")));
#define htmlParseDocument htmlParseDocument__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlParseElement
extern __typeof (htmlParseElement) htmlParseElement __attribute((alias("htmlParseElement__internal_alias")));
#else
#ifndef htmlParseElement
extern __typeof (htmlParseElement) htmlParseElement__internal_alias __attribute((visibility("hidden")));
#define htmlParseElement htmlParseElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlParseEntityRef
extern __typeof (htmlParseEntityRef) htmlParseEntityRef __attribute((alias("htmlParseEntityRef__internal_alias")));
#else
#ifndef htmlParseEntityRef
extern __typeof (htmlParseEntityRef) htmlParseEntityRef__internal_alias __attribute((visibility("hidden")));
#define htmlParseEntityRef htmlParseEntityRef__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlParseFile
extern __typeof (htmlParseFile) htmlParseFile __attribute((alias("htmlParseFile__internal_alias")));
#else
#ifndef htmlParseFile
extern __typeof (htmlParseFile) htmlParseFile__internal_alias __attribute((visibility("hidden")));
#define htmlParseFile htmlParseFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlReadDoc
extern __typeof (htmlReadDoc) htmlReadDoc __attribute((alias("htmlReadDoc__internal_alias")));
#else
#ifndef htmlReadDoc
extern __typeof (htmlReadDoc) htmlReadDoc__internal_alias __attribute((visibility("hidden")));
#define htmlReadDoc htmlReadDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlReadFd
extern __typeof (htmlReadFd) htmlReadFd __attribute((alias("htmlReadFd__internal_alias")));
#else
#ifndef htmlReadFd
extern __typeof (htmlReadFd) htmlReadFd__internal_alias __attribute((visibility("hidden")));
#define htmlReadFd htmlReadFd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlReadFile
extern __typeof (htmlReadFile) htmlReadFile __attribute((alias("htmlReadFile__internal_alias")));
#else
#ifndef htmlReadFile
extern __typeof (htmlReadFile) htmlReadFile__internal_alias __attribute((visibility("hidden")));
#define htmlReadFile htmlReadFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlReadIO
extern __typeof (htmlReadIO) htmlReadIO __attribute((alias("htmlReadIO__internal_alias")));
#else
#ifndef htmlReadIO
extern __typeof (htmlReadIO) htmlReadIO__internal_alias __attribute((visibility("hidden")));
#define htmlReadIO htmlReadIO__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlReadMemory
extern __typeof (htmlReadMemory) htmlReadMemory __attribute((alias("htmlReadMemory__internal_alias")));
#else
#ifndef htmlReadMemory
extern __typeof (htmlReadMemory) htmlReadMemory__internal_alias __attribute((visibility("hidden")));
#define htmlReadMemory htmlReadMemory__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlSAXParseDoc
extern __typeof (htmlSAXParseDoc) htmlSAXParseDoc __attribute((alias("htmlSAXParseDoc__internal_alias")));
#else
#ifndef htmlSAXParseDoc
extern __typeof (htmlSAXParseDoc) htmlSAXParseDoc__internal_alias __attribute((visibility("hidden")));
#define htmlSAXParseDoc htmlSAXParseDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlSAXParseFile
extern __typeof (htmlSAXParseFile) htmlSAXParseFile __attribute((alias("htmlSAXParseFile__internal_alias")));
#else
#ifndef htmlSAXParseFile
extern __typeof (htmlSAXParseFile) htmlSAXParseFile__internal_alias __attribute((visibility("hidden")));
#define htmlSAXParseFile htmlSAXParseFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlSaveFile
extern __typeof (htmlSaveFile) htmlSaveFile __attribute((alias("htmlSaveFile__internal_alias")));
#else
#ifndef htmlSaveFile
extern __typeof (htmlSaveFile) htmlSaveFile__internal_alias __attribute((visibility("hidden")));
#define htmlSaveFile htmlSaveFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlSaveFileEnc
extern __typeof (htmlSaveFileEnc) htmlSaveFileEnc __attribute((alias("htmlSaveFileEnc__internal_alias")));
#else
#ifndef htmlSaveFileEnc
extern __typeof (htmlSaveFileEnc) htmlSaveFileEnc__internal_alias __attribute((visibility("hidden")));
#define htmlSaveFileEnc htmlSaveFileEnc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlSaveFileFormat
extern __typeof (htmlSaveFileFormat) htmlSaveFileFormat __attribute((alias("htmlSaveFileFormat__internal_alias")));
#else
#ifndef htmlSaveFileFormat
extern __typeof (htmlSaveFileFormat) htmlSaveFileFormat__internal_alias __attribute((visibility("hidden")));
#define htmlSaveFileFormat htmlSaveFileFormat__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLtree
#undef htmlSetMetaEncoding
extern __typeof (htmlSetMetaEncoding) htmlSetMetaEncoding __attribute((alias("htmlSetMetaEncoding__internal_alias")));
#else
#ifndef htmlSetMetaEncoding
extern __typeof (htmlSetMetaEncoding) htmlSetMetaEncoding__internal_alias __attribute((visibility("hidden")));
#define htmlSetMetaEncoding htmlSetMetaEncoding__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_HTMLparser
#undef htmlTagLookup
extern __typeof (htmlTagLookup) htmlTagLookup __attribute((alias("htmlTagLookup__internal_alias")));
#else
#ifndef htmlTagLookup
extern __typeof (htmlTagLookup) htmlTagLookup__internal_alias __attribute((visibility("hidden")));
#define htmlTagLookup htmlTagLookup__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef inputPop
extern __typeof (inputPop) inputPop __attribute((alias("inputPop__internal_alias")));
#else
#ifndef inputPop
extern __typeof (inputPop) inputPop__internal_alias __attribute((visibility("hidden")));
#define inputPop inputPop__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef inputPush
extern __typeof (inputPush) inputPush __attribute((alias("inputPush__internal_alias")));
#else
#ifndef inputPush
extern __typeof (inputPush) inputPush__internal_alias __attribute((visibility("hidden")));
#define inputPush inputPush__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef namePop
extern __typeof (namePop) namePop __attribute((alias("namePop__internal_alias")));
#else
#ifndef namePop
extern __typeof (namePop) namePop__internal_alias __attribute((visibility("hidden")));
#define namePop namePop__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef namePush
extern __typeof (namePush) namePush __attribute((alias("namePush__internal_alias")));
#else
#ifndef namePush
extern __typeof (namePush) namePush__internal_alias __attribute((visibility("hidden")));
#define namePush namePush__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef nodePop
extern __typeof (nodePop) nodePop __attribute((alias("nodePop__internal_alias")));
#else
#ifndef nodePop
extern __typeof (nodePop) nodePop__internal_alias __attribute((visibility("hidden")));
#define nodePop nodePop__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef nodePush
extern __typeof (nodePush) nodePush __attribute((alias("nodePush__internal_alias")));
#else
#ifndef nodePush
extern __typeof (nodePush) nodePush__internal_alias __attribute((visibility("hidden")));
#define nodePush nodePush__internal_alias
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef valuePop
extern __typeof (valuePop) valuePop __attribute((alias("valuePop__internal_alias")));
#else
#ifndef valuePop
extern __typeof (valuePop) valuePop__internal_alias __attribute((visibility("hidden")));
#define valuePop valuePop__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef valuePush
extern __typeof (valuePush) valuePush __attribute((alias("valuePush__internal_alias")));
#else
#ifndef valuePush
extern __typeof (valuePush) valuePush__internal_alias __attribute((visibility("hidden")));
#define valuePush valuePush__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlACatalogAdd
extern __typeof (xmlACatalogAdd) xmlACatalogAdd __attribute((alias("xmlACatalogAdd__internal_alias")));
#else
#ifndef xmlACatalogAdd
extern __typeof (xmlACatalogAdd) xmlACatalogAdd__internal_alias __attribute((visibility("hidden")));
#define xmlACatalogAdd xmlACatalogAdd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_catalog
#undef xmlACatalogDump
extern __typeof (xmlACatalogDump) xmlACatalogDump __attribute((alias("xmlACatalogDump__internal_alias")));
#else
#ifndef xmlACatalogDump
extern __typeof (xmlACatalogDump) xmlACatalogDump__internal_alias __attribute((visibility("hidden")));
#define xmlACatalogDump xmlACatalogDump__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlACatalogRemove
extern __typeof (xmlACatalogRemove) xmlACatalogRemove __attribute((alias("xmlACatalogRemove__internal_alias")));
#else
#ifndef xmlACatalogRemove
extern __typeof (xmlACatalogRemove) xmlACatalogRemove__internal_alias __attribute((visibility("hidden")));
#define xmlACatalogRemove xmlACatalogRemove__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlACatalogResolve
extern __typeof (xmlACatalogResolve) xmlACatalogResolve __attribute((alias("xmlACatalogResolve__internal_alias")));
#else
#ifndef xmlACatalogResolve
extern __typeof (xmlACatalogResolve) xmlACatalogResolve__internal_alias __attribute((visibility("hidden")));
#define xmlACatalogResolve xmlACatalogResolve__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlACatalogResolvePublic
extern __typeof (xmlACatalogResolvePublic) xmlACatalogResolvePublic __attribute((alias("xmlACatalogResolvePublic__internal_alias")));
#else
#ifndef xmlACatalogResolvePublic
extern __typeof (xmlACatalogResolvePublic) xmlACatalogResolvePublic__internal_alias __attribute((visibility("hidden")));
#define xmlACatalogResolvePublic xmlACatalogResolvePublic__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlACatalogResolveSystem
extern __typeof (xmlACatalogResolveSystem) xmlACatalogResolveSystem __attribute((alias("xmlACatalogResolveSystem__internal_alias")));
#else
#ifndef xmlACatalogResolveSystem
extern __typeof (xmlACatalogResolveSystem) xmlACatalogResolveSystem__internal_alias __attribute((visibility("hidden")));
#define xmlACatalogResolveSystem xmlACatalogResolveSystem__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlACatalogResolveURI
extern __typeof (xmlACatalogResolveURI) xmlACatalogResolveURI __attribute((alias("xmlACatalogResolveURI__internal_alias")));
#else
#ifndef xmlACatalogResolveURI
extern __typeof (xmlACatalogResolveURI) xmlACatalogResolveURI__internal_alias __attribute((visibility("hidden")));
#define xmlACatalogResolveURI xmlACatalogResolveURI__internal_alias
#endif
#endif
#endif

#ifdef bottom_valid
#undef xmlAddAttributeDecl
extern __typeof (xmlAddAttributeDecl) xmlAddAttributeDecl __attribute((alias("xmlAddAttributeDecl__internal_alias")));
#else
#ifndef xmlAddAttributeDecl
extern __typeof (xmlAddAttributeDecl) xmlAddAttributeDecl__internal_alias __attribute((visibility("hidden")));
#define xmlAddAttributeDecl xmlAddAttributeDecl__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlAddChild
extern __typeof (xmlAddChild) xmlAddChild __attribute((alias("xmlAddChild__internal_alias")));
#else
#ifndef xmlAddChild
extern __typeof (xmlAddChild) xmlAddChild__internal_alias __attribute((visibility("hidden")));
#define xmlAddChild xmlAddChild__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlAddChildList
extern __typeof (xmlAddChildList) xmlAddChildList __attribute((alias("xmlAddChildList__internal_alias")));
#else
#ifndef xmlAddChildList
extern __typeof (xmlAddChildList) xmlAddChildList__internal_alias __attribute((visibility("hidden")));
#define xmlAddChildList xmlAddChildList__internal_alias
#endif
#endif

#ifdef bottom_entities
#undef xmlAddDocEntity
extern __typeof (xmlAddDocEntity) xmlAddDocEntity __attribute((alias("xmlAddDocEntity__internal_alias")));
#else
#ifndef xmlAddDocEntity
extern __typeof (xmlAddDocEntity) xmlAddDocEntity__internal_alias __attribute((visibility("hidden")));
#define xmlAddDocEntity xmlAddDocEntity__internal_alias
#endif
#endif

#ifdef bottom_entities
#undef xmlAddDtdEntity
extern __typeof (xmlAddDtdEntity) xmlAddDtdEntity __attribute((alias("xmlAddDtdEntity__internal_alias")));
#else
#ifndef xmlAddDtdEntity
extern __typeof (xmlAddDtdEntity) xmlAddDtdEntity__internal_alias __attribute((visibility("hidden")));
#define xmlAddDtdEntity xmlAddDtdEntity__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlAddElementDecl
extern __typeof (xmlAddElementDecl) xmlAddElementDecl __attribute((alias("xmlAddElementDecl__internal_alias")));
#else
#ifndef xmlAddElementDecl
extern __typeof (xmlAddElementDecl) xmlAddElementDecl__internal_alias __attribute((visibility("hidden")));
#define xmlAddElementDecl xmlAddElementDecl__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlAddEncodingAlias
extern __typeof (xmlAddEncodingAlias) xmlAddEncodingAlias __attribute((alias("xmlAddEncodingAlias__internal_alias")));
#else
#ifndef xmlAddEncodingAlias
extern __typeof (xmlAddEncodingAlias) xmlAddEncodingAlias__internal_alias __attribute((visibility("hidden")));
#define xmlAddEncodingAlias xmlAddEncodingAlias__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlAddID
extern __typeof (xmlAddID) xmlAddID __attribute((alias("xmlAddID__internal_alias")));
#else
#ifndef xmlAddID
extern __typeof (xmlAddID) xmlAddID__internal_alias __attribute((visibility("hidden")));
#define xmlAddID xmlAddID__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlAddNextSibling
extern __typeof (xmlAddNextSibling) xmlAddNextSibling __attribute((alias("xmlAddNextSibling__internal_alias")));
#else
#ifndef xmlAddNextSibling
extern __typeof (xmlAddNextSibling) xmlAddNextSibling__internal_alias __attribute((visibility("hidden")));
#define xmlAddNextSibling xmlAddNextSibling__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlAddNotationDecl
extern __typeof (xmlAddNotationDecl) xmlAddNotationDecl __attribute((alias("xmlAddNotationDecl__internal_alias")));
#else
#ifndef xmlAddNotationDecl
extern __typeof (xmlAddNotationDecl) xmlAddNotationDecl__internal_alias __attribute((visibility("hidden")));
#define xmlAddNotationDecl xmlAddNotationDecl__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_HTML_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_tree
#undef xmlAddPrevSibling
extern __typeof (xmlAddPrevSibling) xmlAddPrevSibling __attribute((alias("xmlAddPrevSibling__internal_alias")));
#else
#ifndef xmlAddPrevSibling
extern __typeof (xmlAddPrevSibling) xmlAddPrevSibling__internal_alias __attribute((visibility("hidden")));
#define xmlAddPrevSibling xmlAddPrevSibling__internal_alias
#endif
#endif
#endif

#ifdef bottom_valid
#undef xmlAddRef
extern __typeof (xmlAddRef) xmlAddRef __attribute((alias("xmlAddRef__internal_alias")));
#else
#ifndef xmlAddRef
extern __typeof (xmlAddRef) xmlAddRef__internal_alias __attribute((visibility("hidden")));
#define xmlAddRef xmlAddRef__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlAddSibling
extern __typeof (xmlAddSibling) xmlAddSibling __attribute((alias("xmlAddSibling__internal_alias")));
#else
#ifndef xmlAddSibling
extern __typeof (xmlAddSibling) xmlAddSibling__internal_alias __attribute((visibility("hidden")));
#define xmlAddSibling xmlAddSibling__internal_alias
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlAllocOutputBuffer
extern __typeof (xmlAllocOutputBuffer) xmlAllocOutputBuffer __attribute((alias("xmlAllocOutputBuffer__internal_alias")));
#else
#ifndef xmlAllocOutputBuffer
extern __typeof (xmlAllocOutputBuffer) xmlAllocOutputBuffer__internal_alias __attribute((visibility("hidden")));
#define xmlAllocOutputBuffer xmlAllocOutputBuffer__internal_alias
#endif
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlAllocParserInputBuffer
extern __typeof (xmlAllocParserInputBuffer) xmlAllocParserInputBuffer __attribute((alias("xmlAllocParserInputBuffer__internal_alias")));
#else
#ifndef xmlAllocParserInputBuffer
extern __typeof (xmlAllocParserInputBuffer) xmlAllocParserInputBuffer__internal_alias __attribute((visibility("hidden")));
#define xmlAllocParserInputBuffer xmlAllocParserInputBuffer__internal_alias
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlAttrSerializeTxtContent
extern __typeof (xmlAttrSerializeTxtContent) xmlAttrSerializeTxtContent __attribute((alias("xmlAttrSerializeTxtContent__internal_alias")));
#else
#ifndef xmlAttrSerializeTxtContent
extern __typeof (xmlAttrSerializeTxtContent) xmlAttrSerializeTxtContent__internal_alias __attribute((visibility("hidden")));
#define xmlAttrSerializeTxtContent xmlAttrSerializeTxtContent__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataCompile
extern __typeof (xmlAutomataCompile) xmlAutomataCompile __attribute((alias("xmlAutomataCompile__internal_alias")));
#else
#ifndef xmlAutomataCompile
extern __typeof (xmlAutomataCompile) xmlAutomataCompile__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataCompile xmlAutomataCompile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataGetInitState
extern __typeof (xmlAutomataGetInitState) xmlAutomataGetInitState __attribute((alias("xmlAutomataGetInitState__internal_alias")));
#else
#ifndef xmlAutomataGetInitState
extern __typeof (xmlAutomataGetInitState) xmlAutomataGetInitState__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataGetInitState xmlAutomataGetInitState__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataIsDeterminist
extern __typeof (xmlAutomataIsDeterminist) xmlAutomataIsDeterminist __attribute((alias("xmlAutomataIsDeterminist__internal_alias")));
#else
#ifndef xmlAutomataIsDeterminist
extern __typeof (xmlAutomataIsDeterminist) xmlAutomataIsDeterminist__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataIsDeterminist xmlAutomataIsDeterminist__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataNewAllTrans
extern __typeof (xmlAutomataNewAllTrans) xmlAutomataNewAllTrans __attribute((alias("xmlAutomataNewAllTrans__internal_alias")));
#else
#ifndef xmlAutomataNewAllTrans
extern __typeof (xmlAutomataNewAllTrans) xmlAutomataNewAllTrans__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataNewAllTrans xmlAutomataNewAllTrans__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataNewCountTrans
extern __typeof (xmlAutomataNewCountTrans) xmlAutomataNewCountTrans __attribute((alias("xmlAutomataNewCountTrans__internal_alias")));
#else
#ifndef xmlAutomataNewCountTrans
extern __typeof (xmlAutomataNewCountTrans) xmlAutomataNewCountTrans__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataNewCountTrans xmlAutomataNewCountTrans__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataNewCountTrans2
extern __typeof (xmlAutomataNewCountTrans2) xmlAutomataNewCountTrans2 __attribute((alias("xmlAutomataNewCountTrans2__internal_alias")));
#else
#ifndef xmlAutomataNewCountTrans2
extern __typeof (xmlAutomataNewCountTrans2) xmlAutomataNewCountTrans2__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataNewCountTrans2 xmlAutomataNewCountTrans2__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataNewCountedTrans
extern __typeof (xmlAutomataNewCountedTrans) xmlAutomataNewCountedTrans __attribute((alias("xmlAutomataNewCountedTrans__internal_alias")));
#else
#ifndef xmlAutomataNewCountedTrans
extern __typeof (xmlAutomataNewCountedTrans) xmlAutomataNewCountedTrans__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataNewCountedTrans xmlAutomataNewCountedTrans__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataNewCounter
extern __typeof (xmlAutomataNewCounter) xmlAutomataNewCounter __attribute((alias("xmlAutomataNewCounter__internal_alias")));
#else
#ifndef xmlAutomataNewCounter
extern __typeof (xmlAutomataNewCounter) xmlAutomataNewCounter__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataNewCounter xmlAutomataNewCounter__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataNewCounterTrans
extern __typeof (xmlAutomataNewCounterTrans) xmlAutomataNewCounterTrans __attribute((alias("xmlAutomataNewCounterTrans__internal_alias")));
#else
#ifndef xmlAutomataNewCounterTrans
extern __typeof (xmlAutomataNewCounterTrans) xmlAutomataNewCounterTrans__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataNewCounterTrans xmlAutomataNewCounterTrans__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataNewEpsilon
extern __typeof (xmlAutomataNewEpsilon) xmlAutomataNewEpsilon __attribute((alias("xmlAutomataNewEpsilon__internal_alias")));
#else
#ifndef xmlAutomataNewEpsilon
extern __typeof (xmlAutomataNewEpsilon) xmlAutomataNewEpsilon__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataNewEpsilon xmlAutomataNewEpsilon__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataNewOnceTrans
extern __typeof (xmlAutomataNewOnceTrans) xmlAutomataNewOnceTrans __attribute((alias("xmlAutomataNewOnceTrans__internal_alias")));
#else
#ifndef xmlAutomataNewOnceTrans
extern __typeof (xmlAutomataNewOnceTrans) xmlAutomataNewOnceTrans__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataNewOnceTrans xmlAutomataNewOnceTrans__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataNewOnceTrans2
extern __typeof (xmlAutomataNewOnceTrans2) xmlAutomataNewOnceTrans2 __attribute((alias("xmlAutomataNewOnceTrans2__internal_alias")));
#else
#ifndef xmlAutomataNewOnceTrans2
extern __typeof (xmlAutomataNewOnceTrans2) xmlAutomataNewOnceTrans2__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataNewOnceTrans2 xmlAutomataNewOnceTrans2__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataNewState
extern __typeof (xmlAutomataNewState) xmlAutomataNewState __attribute((alias("xmlAutomataNewState__internal_alias")));
#else
#ifndef xmlAutomataNewState
extern __typeof (xmlAutomataNewState) xmlAutomataNewState__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataNewState xmlAutomataNewState__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataNewTransition
extern __typeof (xmlAutomataNewTransition) xmlAutomataNewTransition __attribute((alias("xmlAutomataNewTransition__internal_alias")));
#else
#ifndef xmlAutomataNewTransition
extern __typeof (xmlAutomataNewTransition) xmlAutomataNewTransition__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataNewTransition xmlAutomataNewTransition__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataNewTransition2
extern __typeof (xmlAutomataNewTransition2) xmlAutomataNewTransition2 __attribute((alias("xmlAutomataNewTransition2__internal_alias")));
#else
#ifndef xmlAutomataNewTransition2
extern __typeof (xmlAutomataNewTransition2) xmlAutomataNewTransition2__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataNewTransition2 xmlAutomataNewTransition2__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlAutomataSetFinalState
extern __typeof (xmlAutomataSetFinalState) xmlAutomataSetFinalState __attribute((alias("xmlAutomataSetFinalState__internal_alias")));
#else
#ifndef xmlAutomataSetFinalState
extern __typeof (xmlAutomataSetFinalState) xmlAutomataSetFinalState__internal_alias __attribute((visibility("hidden")));
#define xmlAutomataSetFinalState xmlAutomataSetFinalState__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlBoolToText
extern __typeof (xmlBoolToText) xmlBoolToText __attribute((alias("xmlBoolToText__internal_alias")));
#else
#ifndef xmlBoolToText
extern __typeof (xmlBoolToText) xmlBoolToText__internal_alias __attribute((visibility("hidden")));
#define xmlBoolToText xmlBoolToText__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferAdd
extern __typeof (xmlBufferAdd) xmlBufferAdd __attribute((alias("xmlBufferAdd__internal_alias")));
#else
#ifndef xmlBufferAdd
extern __typeof (xmlBufferAdd) xmlBufferAdd__internal_alias __attribute((visibility("hidden")));
#define xmlBufferAdd xmlBufferAdd__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferAddHead
extern __typeof (xmlBufferAddHead) xmlBufferAddHead __attribute((alias("xmlBufferAddHead__internal_alias")));
#else
#ifndef xmlBufferAddHead
extern __typeof (xmlBufferAddHead) xmlBufferAddHead__internal_alias __attribute((visibility("hidden")));
#define xmlBufferAddHead xmlBufferAddHead__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferCCat
extern __typeof (xmlBufferCCat) xmlBufferCCat __attribute((alias("xmlBufferCCat__internal_alias")));
#else
#ifndef xmlBufferCCat
extern __typeof (xmlBufferCCat) xmlBufferCCat__internal_alias __attribute((visibility("hidden")));
#define xmlBufferCCat xmlBufferCCat__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferCat
extern __typeof (xmlBufferCat) xmlBufferCat __attribute((alias("xmlBufferCat__internal_alias")));
#else
#ifndef xmlBufferCat
extern __typeof (xmlBufferCat) xmlBufferCat__internal_alias __attribute((visibility("hidden")));
#define xmlBufferCat xmlBufferCat__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferContent
extern __typeof (xmlBufferContent) xmlBufferContent __attribute((alias("xmlBufferContent__internal_alias")));
#else
#ifndef xmlBufferContent
extern __typeof (xmlBufferContent) xmlBufferContent__internal_alias __attribute((visibility("hidden")));
#define xmlBufferContent xmlBufferContent__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferCreate
extern __typeof (xmlBufferCreate) xmlBufferCreate __attribute((alias("xmlBufferCreate__internal_alias")));
#else
#ifndef xmlBufferCreate
extern __typeof (xmlBufferCreate) xmlBufferCreate__internal_alias __attribute((visibility("hidden")));
#define xmlBufferCreate xmlBufferCreate__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferCreateSize
extern __typeof (xmlBufferCreateSize) xmlBufferCreateSize __attribute((alias("xmlBufferCreateSize__internal_alias")));
#else
#ifndef xmlBufferCreateSize
extern __typeof (xmlBufferCreateSize) xmlBufferCreateSize__internal_alias __attribute((visibility("hidden")));
#define xmlBufferCreateSize xmlBufferCreateSize__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferCreateStatic
extern __typeof (xmlBufferCreateStatic) xmlBufferCreateStatic __attribute((alias("xmlBufferCreateStatic__internal_alias")));
#else
#ifndef xmlBufferCreateStatic
extern __typeof (xmlBufferCreateStatic) xmlBufferCreateStatic__internal_alias __attribute((visibility("hidden")));
#define xmlBufferCreateStatic xmlBufferCreateStatic__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferDump
extern __typeof (xmlBufferDump) xmlBufferDump __attribute((alias("xmlBufferDump__internal_alias")));
#else
#ifndef xmlBufferDump
extern __typeof (xmlBufferDump) xmlBufferDump__internal_alias __attribute((visibility("hidden")));
#define xmlBufferDump xmlBufferDump__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferEmpty
extern __typeof (xmlBufferEmpty) xmlBufferEmpty __attribute((alias("xmlBufferEmpty__internal_alias")));
#else
#ifndef xmlBufferEmpty
extern __typeof (xmlBufferEmpty) xmlBufferEmpty__internal_alias __attribute((visibility("hidden")));
#define xmlBufferEmpty xmlBufferEmpty__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferFree
extern __typeof (xmlBufferFree) xmlBufferFree __attribute((alias("xmlBufferFree__internal_alias")));
#else
#ifndef xmlBufferFree
extern __typeof (xmlBufferFree) xmlBufferFree__internal_alias __attribute((visibility("hidden")));
#define xmlBufferFree xmlBufferFree__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferGrow
extern __typeof (xmlBufferGrow) xmlBufferGrow __attribute((alias("xmlBufferGrow__internal_alias")));
#else
#ifndef xmlBufferGrow
extern __typeof (xmlBufferGrow) xmlBufferGrow__internal_alias __attribute((visibility("hidden")));
#define xmlBufferGrow xmlBufferGrow__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferLength
extern __typeof (xmlBufferLength) xmlBufferLength __attribute((alias("xmlBufferLength__internal_alias")));
#else
#ifndef xmlBufferLength
extern __typeof (xmlBufferLength) xmlBufferLength__internal_alias __attribute((visibility("hidden")));
#define xmlBufferLength xmlBufferLength__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferResize
extern __typeof (xmlBufferResize) xmlBufferResize __attribute((alias("xmlBufferResize__internal_alias")));
#else
#ifndef xmlBufferResize
extern __typeof (xmlBufferResize) xmlBufferResize__internal_alias __attribute((visibility("hidden")));
#define xmlBufferResize xmlBufferResize__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferSetAllocationScheme
extern __typeof (xmlBufferSetAllocationScheme) xmlBufferSetAllocationScheme __attribute((alias("xmlBufferSetAllocationScheme__internal_alias")));
#else
#ifndef xmlBufferSetAllocationScheme
extern __typeof (xmlBufferSetAllocationScheme) xmlBufferSetAllocationScheme__internal_alias __attribute((visibility("hidden")));
#define xmlBufferSetAllocationScheme xmlBufferSetAllocationScheme__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferShrink
extern __typeof (xmlBufferShrink) xmlBufferShrink __attribute((alias("xmlBufferShrink__internal_alias")));
#else
#ifndef xmlBufferShrink
extern __typeof (xmlBufferShrink) xmlBufferShrink__internal_alias __attribute((visibility("hidden")));
#define xmlBufferShrink xmlBufferShrink__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferWriteCHAR
extern __typeof (xmlBufferWriteCHAR) xmlBufferWriteCHAR __attribute((alias("xmlBufferWriteCHAR__internal_alias")));
#else
#ifndef xmlBufferWriteCHAR
extern __typeof (xmlBufferWriteCHAR) xmlBufferWriteCHAR__internal_alias __attribute((visibility("hidden")));
#define xmlBufferWriteCHAR xmlBufferWriteCHAR__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferWriteChar
extern __typeof (xmlBufferWriteChar) xmlBufferWriteChar __attribute((alias("xmlBufferWriteChar__internal_alias")));
#else
#ifndef xmlBufferWriteChar
extern __typeof (xmlBufferWriteChar) xmlBufferWriteChar__internal_alias __attribute((visibility("hidden")));
#define xmlBufferWriteChar xmlBufferWriteChar__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBufferWriteQuotedString
extern __typeof (xmlBufferWriteQuotedString) xmlBufferWriteQuotedString __attribute((alias("xmlBufferWriteQuotedString__internal_alias")));
#else
#ifndef xmlBufferWriteQuotedString
extern __typeof (xmlBufferWriteQuotedString) xmlBufferWriteQuotedString__internal_alias __attribute((visibility("hidden")));
#define xmlBufferWriteQuotedString xmlBufferWriteQuotedString__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlBuildQName
extern __typeof (xmlBuildQName) xmlBuildQName __attribute((alias("xmlBuildQName__internal_alias")));
#else
#ifndef xmlBuildQName
extern __typeof (xmlBuildQName) xmlBuildQName__internal_alias __attribute((visibility("hidden")));
#define xmlBuildQName xmlBuildQName__internal_alias
#endif
#endif

#ifdef bottom_uri
#undef xmlBuildRelativeURI
extern __typeof (xmlBuildRelativeURI) xmlBuildRelativeURI __attribute((alias("xmlBuildRelativeURI__internal_alias")));
#else
#ifndef xmlBuildRelativeURI
extern __typeof (xmlBuildRelativeURI) xmlBuildRelativeURI__internal_alias __attribute((visibility("hidden")));
#define xmlBuildRelativeURI xmlBuildRelativeURI__internal_alias
#endif
#endif

#ifdef bottom_uri
#undef xmlBuildURI
extern __typeof (xmlBuildURI) xmlBuildURI __attribute((alias("xmlBuildURI__internal_alias")));
#else
#ifndef xmlBuildURI
extern __typeof (xmlBuildURI) xmlBuildURI__internal_alias __attribute((visibility("hidden")));
#define xmlBuildURI xmlBuildURI__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlByteConsumed
extern __typeof (xmlByteConsumed) xmlByteConsumed __attribute((alias("xmlByteConsumed__internal_alias")));
#else
#ifndef xmlByteConsumed
extern __typeof (xmlByteConsumed) xmlByteConsumed__internal_alias __attribute((visibility("hidden")));
#define xmlByteConsumed xmlByteConsumed__internal_alias
#endif
#endif

#if defined(LIBXML_C14N_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_c14n
#undef xmlC14NDocDumpMemory
extern __typeof (xmlC14NDocDumpMemory) xmlC14NDocDumpMemory __attribute((alias("xmlC14NDocDumpMemory__internal_alias")));
#else
#ifndef xmlC14NDocDumpMemory
extern __typeof (xmlC14NDocDumpMemory) xmlC14NDocDumpMemory__internal_alias __attribute((visibility("hidden")));
#define xmlC14NDocDumpMemory xmlC14NDocDumpMemory__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_C14N_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_c14n
#undef xmlC14NDocSave
extern __typeof (xmlC14NDocSave) xmlC14NDocSave __attribute((alias("xmlC14NDocSave__internal_alias")));
#else
#ifndef xmlC14NDocSave
extern __typeof (xmlC14NDocSave) xmlC14NDocSave__internal_alias __attribute((visibility("hidden")));
#define xmlC14NDocSave xmlC14NDocSave__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_C14N_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_c14n
#undef xmlC14NDocSaveTo
extern __typeof (xmlC14NDocSaveTo) xmlC14NDocSaveTo __attribute((alias("xmlC14NDocSaveTo__internal_alias")));
#else
#ifndef xmlC14NDocSaveTo
extern __typeof (xmlC14NDocSaveTo) xmlC14NDocSaveTo__internal_alias __attribute((visibility("hidden")));
#define xmlC14NDocSaveTo xmlC14NDocSaveTo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_C14N_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_c14n
#undef xmlC14NExecute
extern __typeof (xmlC14NExecute) xmlC14NExecute __attribute((alias("xmlC14NExecute__internal_alias")));
#else
#ifndef xmlC14NExecute
extern __typeof (xmlC14NExecute) xmlC14NExecute__internal_alias __attribute((visibility("hidden")));
#define xmlC14NExecute xmlC14NExecute__internal_alias
#endif
#endif
#endif

#ifdef bottom_uri
#undef xmlCanonicPath
extern __typeof (xmlCanonicPath) xmlCanonicPath __attribute((alias("xmlCanonicPath__internal_alias")));
#else
#ifndef xmlCanonicPath
extern __typeof (xmlCanonicPath) xmlCanonicPath__internal_alias __attribute((visibility("hidden")));
#define xmlCanonicPath xmlCanonicPath__internal_alias
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogAdd
extern __typeof (xmlCatalogAdd) xmlCatalogAdd __attribute((alias("xmlCatalogAdd__internal_alias")));
#else
#ifndef xmlCatalogAdd
extern __typeof (xmlCatalogAdd) xmlCatalogAdd__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogAdd xmlCatalogAdd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogAddLocal
extern __typeof (xmlCatalogAddLocal) xmlCatalogAddLocal __attribute((alias("xmlCatalogAddLocal__internal_alias")));
#else
#ifndef xmlCatalogAddLocal
extern __typeof (xmlCatalogAddLocal) xmlCatalogAddLocal__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogAddLocal xmlCatalogAddLocal__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogCleanup
extern __typeof (xmlCatalogCleanup) xmlCatalogCleanup __attribute((alias("xmlCatalogCleanup__internal_alias")));
#else
#ifndef xmlCatalogCleanup
extern __typeof (xmlCatalogCleanup) xmlCatalogCleanup__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogCleanup xmlCatalogCleanup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogConvert
extern __typeof (xmlCatalogConvert) xmlCatalogConvert __attribute((alias("xmlCatalogConvert__internal_alias")));
#else
#ifndef xmlCatalogConvert
extern __typeof (xmlCatalogConvert) xmlCatalogConvert__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogConvert xmlCatalogConvert__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogDump
extern __typeof (xmlCatalogDump) xmlCatalogDump __attribute((alias("xmlCatalogDump__internal_alias")));
#else
#ifndef xmlCatalogDump
extern __typeof (xmlCatalogDump) xmlCatalogDump__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogDump xmlCatalogDump__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogFreeLocal
extern __typeof (xmlCatalogFreeLocal) xmlCatalogFreeLocal __attribute((alias("xmlCatalogFreeLocal__internal_alias")));
#else
#ifndef xmlCatalogFreeLocal
extern __typeof (xmlCatalogFreeLocal) xmlCatalogFreeLocal__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogFreeLocal xmlCatalogFreeLocal__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogGetDefaults
extern __typeof (xmlCatalogGetDefaults) xmlCatalogGetDefaults __attribute((alias("xmlCatalogGetDefaults__internal_alias")));
#else
#ifndef xmlCatalogGetDefaults
extern __typeof (xmlCatalogGetDefaults) xmlCatalogGetDefaults__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogGetDefaults xmlCatalogGetDefaults__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogGetPublic
extern __typeof (xmlCatalogGetPublic) xmlCatalogGetPublic __attribute((alias("xmlCatalogGetPublic__internal_alias")));
#else
#ifndef xmlCatalogGetPublic
extern __typeof (xmlCatalogGetPublic) xmlCatalogGetPublic__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogGetPublic xmlCatalogGetPublic__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogGetSystem
extern __typeof (xmlCatalogGetSystem) xmlCatalogGetSystem __attribute((alias("xmlCatalogGetSystem__internal_alias")));
#else
#ifndef xmlCatalogGetSystem
extern __typeof (xmlCatalogGetSystem) xmlCatalogGetSystem__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogGetSystem xmlCatalogGetSystem__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogIsEmpty
extern __typeof (xmlCatalogIsEmpty) xmlCatalogIsEmpty __attribute((alias("xmlCatalogIsEmpty__internal_alias")));
#else
#ifndef xmlCatalogIsEmpty
extern __typeof (xmlCatalogIsEmpty) xmlCatalogIsEmpty__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogIsEmpty xmlCatalogIsEmpty__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogLocalResolve
extern __typeof (xmlCatalogLocalResolve) xmlCatalogLocalResolve __attribute((alias("xmlCatalogLocalResolve__internal_alias")));
#else
#ifndef xmlCatalogLocalResolve
extern __typeof (xmlCatalogLocalResolve) xmlCatalogLocalResolve__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogLocalResolve xmlCatalogLocalResolve__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogLocalResolveURI
extern __typeof (xmlCatalogLocalResolveURI) xmlCatalogLocalResolveURI __attribute((alias("xmlCatalogLocalResolveURI__internal_alias")));
#else
#ifndef xmlCatalogLocalResolveURI
extern __typeof (xmlCatalogLocalResolveURI) xmlCatalogLocalResolveURI__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogLocalResolveURI xmlCatalogLocalResolveURI__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogRemove
extern __typeof (xmlCatalogRemove) xmlCatalogRemove __attribute((alias("xmlCatalogRemove__internal_alias")));
#else
#ifndef xmlCatalogRemove
extern __typeof (xmlCatalogRemove) xmlCatalogRemove__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogRemove xmlCatalogRemove__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogResolve
extern __typeof (xmlCatalogResolve) xmlCatalogResolve __attribute((alias("xmlCatalogResolve__internal_alias")));
#else
#ifndef xmlCatalogResolve
extern __typeof (xmlCatalogResolve) xmlCatalogResolve__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogResolve xmlCatalogResolve__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogResolvePublic
extern __typeof (xmlCatalogResolvePublic) xmlCatalogResolvePublic __attribute((alias("xmlCatalogResolvePublic__internal_alias")));
#else
#ifndef xmlCatalogResolvePublic
extern __typeof (xmlCatalogResolvePublic) xmlCatalogResolvePublic__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogResolvePublic xmlCatalogResolvePublic__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogResolveSystem
extern __typeof (xmlCatalogResolveSystem) xmlCatalogResolveSystem __attribute((alias("xmlCatalogResolveSystem__internal_alias")));
#else
#ifndef xmlCatalogResolveSystem
extern __typeof (xmlCatalogResolveSystem) xmlCatalogResolveSystem__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogResolveSystem xmlCatalogResolveSystem__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogResolveURI
extern __typeof (xmlCatalogResolveURI) xmlCatalogResolveURI __attribute((alias("xmlCatalogResolveURI__internal_alias")));
#else
#ifndef xmlCatalogResolveURI
extern __typeof (xmlCatalogResolveURI) xmlCatalogResolveURI__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogResolveURI xmlCatalogResolveURI__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogSetDebug
extern __typeof (xmlCatalogSetDebug) xmlCatalogSetDebug __attribute((alias("xmlCatalogSetDebug__internal_alias")));
#else
#ifndef xmlCatalogSetDebug
extern __typeof (xmlCatalogSetDebug) xmlCatalogSetDebug__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogSetDebug xmlCatalogSetDebug__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogSetDefaultPrefer
extern __typeof (xmlCatalogSetDefaultPrefer) xmlCatalogSetDefaultPrefer __attribute((alias("xmlCatalogSetDefaultPrefer__internal_alias")));
#else
#ifndef xmlCatalogSetDefaultPrefer
extern __typeof (xmlCatalogSetDefaultPrefer) xmlCatalogSetDefaultPrefer__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogSetDefaultPrefer xmlCatalogSetDefaultPrefer__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlCatalogSetDefaults
extern __typeof (xmlCatalogSetDefaults) xmlCatalogSetDefaults __attribute((alias("xmlCatalogSetDefaults__internal_alias")));
#else
#ifndef xmlCatalogSetDefaults
extern __typeof (xmlCatalogSetDefaults) xmlCatalogSetDefaults__internal_alias __attribute((visibility("hidden")));
#define xmlCatalogSetDefaults xmlCatalogSetDefaults__internal_alias
#endif
#endif
#endif

#ifdef bottom_encoding
#undef xmlCharEncCloseFunc
extern __typeof (xmlCharEncCloseFunc) xmlCharEncCloseFunc __attribute((alias("xmlCharEncCloseFunc__internal_alias")));
#else
#ifndef xmlCharEncCloseFunc
extern __typeof (xmlCharEncCloseFunc) xmlCharEncCloseFunc__internal_alias __attribute((visibility("hidden")));
#define xmlCharEncCloseFunc xmlCharEncCloseFunc__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlCharEncFirstLine
extern __typeof (xmlCharEncFirstLine) xmlCharEncFirstLine __attribute((alias("xmlCharEncFirstLine__internal_alias")));
#else
#ifndef xmlCharEncFirstLine
extern __typeof (xmlCharEncFirstLine) xmlCharEncFirstLine__internal_alias __attribute((visibility("hidden")));
#define xmlCharEncFirstLine xmlCharEncFirstLine__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlCharEncInFunc
extern __typeof (xmlCharEncInFunc) xmlCharEncInFunc __attribute((alias("xmlCharEncInFunc__internal_alias")));
#else
#ifndef xmlCharEncInFunc
extern __typeof (xmlCharEncInFunc) xmlCharEncInFunc__internal_alias __attribute((visibility("hidden")));
#define xmlCharEncInFunc xmlCharEncInFunc__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlCharEncOutFunc
extern __typeof (xmlCharEncOutFunc) xmlCharEncOutFunc __attribute((alias("xmlCharEncOutFunc__internal_alias")));
#else
#ifndef xmlCharEncOutFunc
extern __typeof (xmlCharEncOutFunc) xmlCharEncOutFunc__internal_alias __attribute((visibility("hidden")));
#define xmlCharEncOutFunc xmlCharEncOutFunc__internal_alias
#endif
#endif

#ifdef bottom_chvalid
#undef xmlCharInRange
extern __typeof (xmlCharInRange) xmlCharInRange __attribute((alias("xmlCharInRange__internal_alias")));
#else
#ifndef xmlCharInRange
extern __typeof (xmlCharInRange) xmlCharInRange__internal_alias __attribute((visibility("hidden")));
#define xmlCharInRange xmlCharInRange__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlCharStrdup
extern __typeof (xmlCharStrdup) xmlCharStrdup __attribute((alias("xmlCharStrdup__internal_alias")));
#else
#ifndef xmlCharStrdup
extern __typeof (xmlCharStrdup) xmlCharStrdup__internal_alias __attribute((visibility("hidden")));
#define xmlCharStrdup xmlCharStrdup__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlCharStrndup
extern __typeof (xmlCharStrndup) xmlCharStrndup __attribute((alias("xmlCharStrndup__internal_alias")));
#else
#ifndef xmlCharStrndup
extern __typeof (xmlCharStrndup) xmlCharStrndup__internal_alias __attribute((visibility("hidden")));
#define xmlCharStrndup xmlCharStrndup__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlCheckFilename
extern __typeof (xmlCheckFilename) xmlCheckFilename __attribute((alias("xmlCheckFilename__internal_alias")));
#else
#ifndef xmlCheckFilename
extern __typeof (xmlCheckFilename) xmlCheckFilename__internal_alias __attribute((visibility("hidden")));
#define xmlCheckFilename xmlCheckFilename__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlCheckHTTPInput
extern __typeof (xmlCheckHTTPInput) xmlCheckHTTPInput __attribute((alias("xmlCheckHTTPInput__internal_alias")));
#else
#ifndef xmlCheckHTTPInput
extern __typeof (xmlCheckHTTPInput) xmlCheckHTTPInput__internal_alias __attribute((visibility("hidden")));
#define xmlCheckHTTPInput xmlCheckHTTPInput__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCheckLanguageID
extern __typeof (xmlCheckLanguageID) xmlCheckLanguageID __attribute((alias("xmlCheckLanguageID__internal_alias")));
#else
#ifndef xmlCheckLanguageID
extern __typeof (xmlCheckLanguageID) xmlCheckLanguageID__internal_alias __attribute((visibility("hidden")));
#define xmlCheckLanguageID xmlCheckLanguageID__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlCheckUTF8
extern __typeof (xmlCheckUTF8) xmlCheckUTF8 __attribute((alias("xmlCheckUTF8__internal_alias")));
#else
#ifndef xmlCheckUTF8
extern __typeof (xmlCheckUTF8) xmlCheckUTF8__internal_alias __attribute((visibility("hidden")));
#define xmlCheckUTF8 xmlCheckUTF8__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlCheckVersion
extern __typeof (xmlCheckVersion) xmlCheckVersion __attribute((alias("xmlCheckVersion__internal_alias")));
#else
#ifndef xmlCheckVersion
extern __typeof (xmlCheckVersion) xmlCheckVersion__internal_alias __attribute((visibility("hidden")));
#define xmlCheckVersion xmlCheckVersion__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlCleanupCharEncodingHandlers
extern __typeof (xmlCleanupCharEncodingHandlers) xmlCleanupCharEncodingHandlers __attribute((alias("xmlCleanupCharEncodingHandlers__internal_alias")));
#else
#ifndef xmlCleanupCharEncodingHandlers
extern __typeof (xmlCleanupCharEncodingHandlers) xmlCleanupCharEncodingHandlers__internal_alias __attribute((visibility("hidden")));
#define xmlCleanupCharEncodingHandlers xmlCleanupCharEncodingHandlers__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlCleanupEncodingAliases
extern __typeof (xmlCleanupEncodingAliases) xmlCleanupEncodingAliases __attribute((alias("xmlCleanupEncodingAliases__internal_alias")));
#else
#ifndef xmlCleanupEncodingAliases
extern __typeof (xmlCleanupEncodingAliases) xmlCleanupEncodingAliases__internal_alias __attribute((visibility("hidden")));
#define xmlCleanupEncodingAliases xmlCleanupEncodingAliases__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlCleanupGlobals
extern __typeof (xmlCleanupGlobals) xmlCleanupGlobals __attribute((alias("xmlCleanupGlobals__internal_alias")));
#else
#ifndef xmlCleanupGlobals
extern __typeof (xmlCleanupGlobals) xmlCleanupGlobals__internal_alias __attribute((visibility("hidden")));
#define xmlCleanupGlobals xmlCleanupGlobals__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlCleanupInputCallbacks
extern __typeof (xmlCleanupInputCallbacks) xmlCleanupInputCallbacks __attribute((alias("xmlCleanupInputCallbacks__internal_alias")));
#else
#ifndef xmlCleanupInputCallbacks
extern __typeof (xmlCleanupInputCallbacks) xmlCleanupInputCallbacks__internal_alias __attribute((visibility("hidden")));
#define xmlCleanupInputCallbacks xmlCleanupInputCallbacks__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlCleanupMemory
extern __typeof (xmlCleanupMemory) xmlCleanupMemory __attribute((alias("xmlCleanupMemory__internal_alias")));
#else
#ifndef xmlCleanupMemory
extern __typeof (xmlCleanupMemory) xmlCleanupMemory__internal_alias __attribute((visibility("hidden")));
#define xmlCleanupMemory xmlCleanupMemory__internal_alias
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlCleanupOutputCallbacks
extern __typeof (xmlCleanupOutputCallbacks) xmlCleanupOutputCallbacks __attribute((alias("xmlCleanupOutputCallbacks__internal_alias")));
#else
#ifndef xmlCleanupOutputCallbacks
extern __typeof (xmlCleanupOutputCallbacks) xmlCleanupOutputCallbacks__internal_alias __attribute((visibility("hidden")));
#define xmlCleanupOutputCallbacks xmlCleanupOutputCallbacks__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlCleanupParser
extern __typeof (xmlCleanupParser) xmlCleanupParser __attribute((alias("xmlCleanupParser__internal_alias")));
#else
#ifndef xmlCleanupParser
extern __typeof (xmlCleanupParser) xmlCleanupParser__internal_alias __attribute((visibility("hidden")));
#define xmlCleanupParser xmlCleanupParser__internal_alias
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlCleanupPredefinedEntities
extern __typeof (xmlCleanupPredefinedEntities) xmlCleanupPredefinedEntities __attribute((alias("xmlCleanupPredefinedEntities__internal_alias")));
#else
#ifndef xmlCleanupPredefinedEntities
extern __typeof (xmlCleanupPredefinedEntities) xmlCleanupPredefinedEntities__internal_alias __attribute((visibility("hidden")));
#define xmlCleanupPredefinedEntities xmlCleanupPredefinedEntities__internal_alias
#endif
#endif
#endif

#ifdef bottom_threads
#undef xmlCleanupThreads
extern __typeof (xmlCleanupThreads) xmlCleanupThreads __attribute((alias("xmlCleanupThreads__internal_alias")));
#else
#ifndef xmlCleanupThreads
extern __typeof (xmlCleanupThreads) xmlCleanupThreads__internal_alias __attribute((visibility("hidden")));
#define xmlCleanupThreads xmlCleanupThreads__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlClearNodeInfoSeq
extern __typeof (xmlClearNodeInfoSeq) xmlClearNodeInfoSeq __attribute((alias("xmlClearNodeInfoSeq__internal_alias")));
#else
#ifndef xmlClearNodeInfoSeq
extern __typeof (xmlClearNodeInfoSeq) xmlClearNodeInfoSeq__internal_alias __attribute((visibility("hidden")));
#define xmlClearNodeInfoSeq xmlClearNodeInfoSeq__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlClearParserCtxt
extern __typeof (xmlClearParserCtxt) xmlClearParserCtxt __attribute((alias("xmlClearParserCtxt__internal_alias")));
#else
#ifndef xmlClearParserCtxt
extern __typeof (xmlClearParserCtxt) xmlClearParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlClearParserCtxt xmlClearParserCtxt__internal_alias
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlConvertSGMLCatalog
extern __typeof (xmlConvertSGMLCatalog) xmlConvertSGMLCatalog __attribute((alias("xmlConvertSGMLCatalog__internal_alias")));
#else
#ifndef xmlConvertSGMLCatalog
extern __typeof (xmlConvertSGMLCatalog) xmlConvertSGMLCatalog__internal_alias __attribute((visibility("hidden")));
#define xmlConvertSGMLCatalog xmlConvertSGMLCatalog__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_valid
#undef xmlCopyAttributeTable
extern __typeof (xmlCopyAttributeTable) xmlCopyAttributeTable __attribute((alias("xmlCopyAttributeTable__internal_alias")));
#else
#ifndef xmlCopyAttributeTable
extern __typeof (xmlCopyAttributeTable) xmlCopyAttributeTable__internal_alias __attribute((visibility("hidden")));
#define xmlCopyAttributeTable xmlCopyAttributeTable__internal_alias
#endif
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlCopyChar
extern __typeof (xmlCopyChar) xmlCopyChar __attribute((alias("xmlCopyChar__internal_alias")));
#else
#ifndef xmlCopyChar
extern __typeof (xmlCopyChar) xmlCopyChar__internal_alias __attribute((visibility("hidden")));
#define xmlCopyChar xmlCopyChar__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlCopyCharMultiByte
extern __typeof (xmlCopyCharMultiByte) xmlCopyCharMultiByte __attribute((alias("xmlCopyCharMultiByte__internal_alias")));
#else
#ifndef xmlCopyCharMultiByte
extern __typeof (xmlCopyCharMultiByte) xmlCopyCharMultiByte__internal_alias __attribute((visibility("hidden")));
#define xmlCopyCharMultiByte xmlCopyCharMultiByte__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_tree
#undef xmlCopyDoc
extern __typeof (xmlCopyDoc) xmlCopyDoc __attribute((alias("xmlCopyDoc__internal_alias")));
#else
#ifndef xmlCopyDoc
extern __typeof (xmlCopyDoc) xmlCopyDoc__internal_alias __attribute((visibility("hidden")));
#define xmlCopyDoc xmlCopyDoc__internal_alias
#endif
#endif
#endif

#ifdef bottom_valid
#undef xmlCopyDocElementContent
extern __typeof (xmlCopyDocElementContent) xmlCopyDocElementContent __attribute((alias("xmlCopyDocElementContent__internal_alias")));
#else
#ifndef xmlCopyDocElementContent
extern __typeof (xmlCopyDocElementContent) xmlCopyDocElementContent__internal_alias __attribute((visibility("hidden")));
#define xmlCopyDocElementContent xmlCopyDocElementContent__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_tree
#undef xmlCopyDtd
extern __typeof (xmlCopyDtd) xmlCopyDtd __attribute((alias("xmlCopyDtd__internal_alias")));
#else
#ifndef xmlCopyDtd
extern __typeof (xmlCopyDtd) xmlCopyDtd__internal_alias __attribute((visibility("hidden")));
#define xmlCopyDtd xmlCopyDtd__internal_alias
#endif
#endif
#endif

#ifdef bottom_valid
#undef xmlCopyElementContent
extern __typeof (xmlCopyElementContent) xmlCopyElementContent __attribute((alias("xmlCopyElementContent__internal_alias")));
#else
#ifndef xmlCopyElementContent
extern __typeof (xmlCopyElementContent) xmlCopyElementContent__internal_alias __attribute((visibility("hidden")));
#define xmlCopyElementContent xmlCopyElementContent__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_valid
#undef xmlCopyElementTable
extern __typeof (xmlCopyElementTable) xmlCopyElementTable __attribute((alias("xmlCopyElementTable__internal_alias")));
#else
#ifndef xmlCopyElementTable
extern __typeof (xmlCopyElementTable) xmlCopyElementTable__internal_alias __attribute((visibility("hidden")));
#define xmlCopyElementTable xmlCopyElementTable__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_entities
#undef xmlCopyEntitiesTable
extern __typeof (xmlCopyEntitiesTable) xmlCopyEntitiesTable __attribute((alias("xmlCopyEntitiesTable__internal_alias")));
#else
#ifndef xmlCopyEntitiesTable
extern __typeof (xmlCopyEntitiesTable) xmlCopyEntitiesTable__internal_alias __attribute((visibility("hidden")));
#define xmlCopyEntitiesTable xmlCopyEntitiesTable__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_valid
#undef xmlCopyEnumeration
extern __typeof (xmlCopyEnumeration) xmlCopyEnumeration __attribute((alias("xmlCopyEnumeration__internal_alias")));
#else
#ifndef xmlCopyEnumeration
extern __typeof (xmlCopyEnumeration) xmlCopyEnumeration__internal_alias __attribute((visibility("hidden")));
#define xmlCopyEnumeration xmlCopyEnumeration__internal_alias
#endif
#endif
#endif

#ifdef bottom_error
#undef xmlCopyError
extern __typeof (xmlCopyError) xmlCopyError __attribute((alias("xmlCopyError__internal_alias")));
#else
#ifndef xmlCopyError
extern __typeof (xmlCopyError) xmlCopyError__internal_alias __attribute((visibility("hidden")));
#define xmlCopyError xmlCopyError__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlCopyNamespace
extern __typeof (xmlCopyNamespace) xmlCopyNamespace __attribute((alias("xmlCopyNamespace__internal_alias")));
#else
#ifndef xmlCopyNamespace
extern __typeof (xmlCopyNamespace) xmlCopyNamespace__internal_alias __attribute((visibility("hidden")));
#define xmlCopyNamespace xmlCopyNamespace__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlCopyNamespaceList
extern __typeof (xmlCopyNamespaceList) xmlCopyNamespaceList __attribute((alias("xmlCopyNamespaceList__internal_alias")));
#else
#ifndef xmlCopyNamespaceList
extern __typeof (xmlCopyNamespaceList) xmlCopyNamespaceList__internal_alias __attribute((visibility("hidden")));
#define xmlCopyNamespaceList xmlCopyNamespaceList__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlCopyNode
extern __typeof (xmlCopyNode) xmlCopyNode __attribute((alias("xmlCopyNode__internal_alias")));
#else
#ifndef xmlCopyNode
extern __typeof (xmlCopyNode) xmlCopyNode__internal_alias __attribute((visibility("hidden")));
#define xmlCopyNode xmlCopyNode__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlCopyNodeList
extern __typeof (xmlCopyNodeList) xmlCopyNodeList __attribute((alias("xmlCopyNodeList__internal_alias")));
#else
#ifndef xmlCopyNodeList
extern __typeof (xmlCopyNodeList) xmlCopyNodeList__internal_alias __attribute((visibility("hidden")));
#define xmlCopyNodeList xmlCopyNodeList__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_valid
#undef xmlCopyNotationTable
extern __typeof (xmlCopyNotationTable) xmlCopyNotationTable __attribute((alias("xmlCopyNotationTable__internal_alias")));
#else
#ifndef xmlCopyNotationTable
extern __typeof (xmlCopyNotationTable) xmlCopyNotationTable__internal_alias __attribute((visibility("hidden")));
#define xmlCopyNotationTable xmlCopyNotationTable__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlCopyProp
extern __typeof (xmlCopyProp) xmlCopyProp __attribute((alias("xmlCopyProp__internal_alias")));
#else
#ifndef xmlCopyProp
extern __typeof (xmlCopyProp) xmlCopyProp__internal_alias __attribute((visibility("hidden")));
#define xmlCopyProp xmlCopyProp__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlCopyPropList
extern __typeof (xmlCopyPropList) xmlCopyPropList __attribute((alias("xmlCopyPropList__internal_alias")));
#else
#ifndef xmlCopyPropList
extern __typeof (xmlCopyPropList) xmlCopyPropList__internal_alias __attribute((visibility("hidden")));
#define xmlCopyPropList xmlCopyPropList__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCreateDocParserCtxt
extern __typeof (xmlCreateDocParserCtxt) xmlCreateDocParserCtxt __attribute((alias("xmlCreateDocParserCtxt__internal_alias")));
#else
#ifndef xmlCreateDocParserCtxt
extern __typeof (xmlCreateDocParserCtxt) xmlCreateDocParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlCreateDocParserCtxt xmlCreateDocParserCtxt__internal_alias
#endif
#endif

#ifdef bottom_entities
#undef xmlCreateEntitiesTable
extern __typeof (xmlCreateEntitiesTable) xmlCreateEntitiesTable __attribute((alias("xmlCreateEntitiesTable__internal_alias")));
#else
#ifndef xmlCreateEntitiesTable
extern __typeof (xmlCreateEntitiesTable) xmlCreateEntitiesTable__internal_alias __attribute((visibility("hidden")));
#define xmlCreateEntitiesTable xmlCreateEntitiesTable__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCreateEntityParserCtxt
extern __typeof (xmlCreateEntityParserCtxt) xmlCreateEntityParserCtxt __attribute((alias("xmlCreateEntityParserCtxt__internal_alias")));
#else
#ifndef xmlCreateEntityParserCtxt
extern __typeof (xmlCreateEntityParserCtxt) xmlCreateEntityParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlCreateEntityParserCtxt xmlCreateEntityParserCtxt__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlCreateEnumeration
extern __typeof (xmlCreateEnumeration) xmlCreateEnumeration __attribute((alias("xmlCreateEnumeration__internal_alias")));
#else
#ifndef xmlCreateEnumeration
extern __typeof (xmlCreateEnumeration) xmlCreateEnumeration__internal_alias __attribute((visibility("hidden")));
#define xmlCreateEnumeration xmlCreateEnumeration__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCreateFileParserCtxt
extern __typeof (xmlCreateFileParserCtxt) xmlCreateFileParserCtxt __attribute((alias("xmlCreateFileParserCtxt__internal_alias")));
#else
#ifndef xmlCreateFileParserCtxt
extern __typeof (xmlCreateFileParserCtxt) xmlCreateFileParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlCreateFileParserCtxt xmlCreateFileParserCtxt__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCreateIOParserCtxt
extern __typeof (xmlCreateIOParserCtxt) xmlCreateIOParserCtxt __attribute((alias("xmlCreateIOParserCtxt__internal_alias")));
#else
#ifndef xmlCreateIOParserCtxt
extern __typeof (xmlCreateIOParserCtxt) xmlCreateIOParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlCreateIOParserCtxt xmlCreateIOParserCtxt__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlCreateIntSubset
extern __typeof (xmlCreateIntSubset) xmlCreateIntSubset __attribute((alias("xmlCreateIntSubset__internal_alias")));
#else
#ifndef xmlCreateIntSubset
extern __typeof (xmlCreateIntSubset) xmlCreateIntSubset__internal_alias __attribute((visibility("hidden")));
#define xmlCreateIntSubset xmlCreateIntSubset__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCreateMemoryParserCtxt
extern __typeof (xmlCreateMemoryParserCtxt) xmlCreateMemoryParserCtxt __attribute((alias("xmlCreateMemoryParserCtxt__internal_alias")));
#else
#ifndef xmlCreateMemoryParserCtxt
extern __typeof (xmlCreateMemoryParserCtxt) xmlCreateMemoryParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlCreateMemoryParserCtxt xmlCreateMemoryParserCtxt__internal_alias
#endif
#endif

#if defined(LIBXML_PUSH_ENABLED)
#ifdef bottom_parser
#undef xmlCreatePushParserCtxt
extern __typeof (xmlCreatePushParserCtxt) xmlCreatePushParserCtxt __attribute((alias("xmlCreatePushParserCtxt__internal_alias")));
#else
#ifndef xmlCreatePushParserCtxt
extern __typeof (xmlCreatePushParserCtxt) xmlCreatePushParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlCreatePushParserCtxt xmlCreatePushParserCtxt__internal_alias
#endif
#endif
#endif

#ifdef bottom_uri
#undef xmlCreateURI
extern __typeof (xmlCreateURI) xmlCreateURI __attribute((alias("xmlCreateURI__internal_alias")));
#else
#ifndef xmlCreateURI
extern __typeof (xmlCreateURI) xmlCreateURI__internal_alias __attribute((visibility("hidden")));
#define xmlCreateURI xmlCreateURI__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCreateURLParserCtxt
extern __typeof (xmlCreateURLParserCtxt) xmlCreateURLParserCtxt __attribute((alias("xmlCreateURLParserCtxt__internal_alias")));
#else
#ifndef xmlCreateURLParserCtxt
extern __typeof (xmlCreateURLParserCtxt) xmlCreateURLParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlCreateURLParserCtxt xmlCreateURLParserCtxt__internal_alias
#endif
#endif

#ifdef bottom_error
#undef xmlCtxtGetLastError
extern __typeof (xmlCtxtGetLastError) xmlCtxtGetLastError __attribute((alias("xmlCtxtGetLastError__internal_alias")));
#else
#ifndef xmlCtxtGetLastError
extern __typeof (xmlCtxtGetLastError) xmlCtxtGetLastError__internal_alias __attribute((visibility("hidden")));
#define xmlCtxtGetLastError xmlCtxtGetLastError__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCtxtReadDoc
extern __typeof (xmlCtxtReadDoc) xmlCtxtReadDoc __attribute((alias("xmlCtxtReadDoc__internal_alias")));
#else
#ifndef xmlCtxtReadDoc
extern __typeof (xmlCtxtReadDoc) xmlCtxtReadDoc__internal_alias __attribute((visibility("hidden")));
#define xmlCtxtReadDoc xmlCtxtReadDoc__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCtxtReadFd
extern __typeof (xmlCtxtReadFd) xmlCtxtReadFd __attribute((alias("xmlCtxtReadFd__internal_alias")));
#else
#ifndef xmlCtxtReadFd
extern __typeof (xmlCtxtReadFd) xmlCtxtReadFd__internal_alias __attribute((visibility("hidden")));
#define xmlCtxtReadFd xmlCtxtReadFd__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCtxtReadFile
extern __typeof (xmlCtxtReadFile) xmlCtxtReadFile __attribute((alias("xmlCtxtReadFile__internal_alias")));
#else
#ifndef xmlCtxtReadFile
extern __typeof (xmlCtxtReadFile) xmlCtxtReadFile__internal_alias __attribute((visibility("hidden")));
#define xmlCtxtReadFile xmlCtxtReadFile__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCtxtReadIO
extern __typeof (xmlCtxtReadIO) xmlCtxtReadIO __attribute((alias("xmlCtxtReadIO__internal_alias")));
#else
#ifndef xmlCtxtReadIO
extern __typeof (xmlCtxtReadIO) xmlCtxtReadIO__internal_alias __attribute((visibility("hidden")));
#define xmlCtxtReadIO xmlCtxtReadIO__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCtxtReadMemory
extern __typeof (xmlCtxtReadMemory) xmlCtxtReadMemory __attribute((alias("xmlCtxtReadMemory__internal_alias")));
#else
#ifndef xmlCtxtReadMemory
extern __typeof (xmlCtxtReadMemory) xmlCtxtReadMemory__internal_alias __attribute((visibility("hidden")));
#define xmlCtxtReadMemory xmlCtxtReadMemory__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCtxtReset
extern __typeof (xmlCtxtReset) xmlCtxtReset __attribute((alias("xmlCtxtReset__internal_alias")));
#else
#ifndef xmlCtxtReset
extern __typeof (xmlCtxtReset) xmlCtxtReset__internal_alias __attribute((visibility("hidden")));
#define xmlCtxtReset xmlCtxtReset__internal_alias
#endif
#endif

#ifdef bottom_error
#undef xmlCtxtResetLastError
extern __typeof (xmlCtxtResetLastError) xmlCtxtResetLastError __attribute((alias("xmlCtxtResetLastError__internal_alias")));
#else
#ifndef xmlCtxtResetLastError
extern __typeof (xmlCtxtResetLastError) xmlCtxtResetLastError__internal_alias __attribute((visibility("hidden")));
#define xmlCtxtResetLastError xmlCtxtResetLastError__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCtxtResetPush
extern __typeof (xmlCtxtResetPush) xmlCtxtResetPush __attribute((alias("xmlCtxtResetPush__internal_alias")));
#else
#ifndef xmlCtxtResetPush
extern __typeof (xmlCtxtResetPush) xmlCtxtResetPush__internal_alias __attribute((visibility("hidden")));
#define xmlCtxtResetPush xmlCtxtResetPush__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlCtxtUseOptions
extern __typeof (xmlCtxtUseOptions) xmlCtxtUseOptions __attribute((alias("xmlCtxtUseOptions__internal_alias")));
#else
#ifndef xmlCtxtUseOptions
extern __typeof (xmlCtxtUseOptions) xmlCtxtUseOptions__internal_alias __attribute((visibility("hidden")));
#define xmlCtxtUseOptions xmlCtxtUseOptions__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlCurrentChar
extern __typeof (xmlCurrentChar) xmlCurrentChar __attribute((alias("xmlCurrentChar__internal_alias")));
#else
#ifndef xmlCurrentChar
extern __typeof (xmlCurrentChar) xmlCurrentChar__internal_alias __attribute((visibility("hidden")));
#define xmlCurrentChar xmlCurrentChar__internal_alias
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlDebugCheckDocument
extern __typeof (xmlDebugCheckDocument) xmlDebugCheckDocument __attribute((alias("xmlDebugCheckDocument__internal_alias")));
#else
#ifndef xmlDebugCheckDocument
extern __typeof (xmlDebugCheckDocument) xmlDebugCheckDocument__internal_alias __attribute((visibility("hidden")));
#define xmlDebugCheckDocument xmlDebugCheckDocument__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlDebugDumpAttr
extern __typeof (xmlDebugDumpAttr) xmlDebugDumpAttr __attribute((alias("xmlDebugDumpAttr__internal_alias")));
#else
#ifndef xmlDebugDumpAttr
extern __typeof (xmlDebugDumpAttr) xmlDebugDumpAttr__internal_alias __attribute((visibility("hidden")));
#define xmlDebugDumpAttr xmlDebugDumpAttr__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlDebugDumpAttrList
extern __typeof (xmlDebugDumpAttrList) xmlDebugDumpAttrList __attribute((alias("xmlDebugDumpAttrList__internal_alias")));
#else
#ifndef xmlDebugDumpAttrList
extern __typeof (xmlDebugDumpAttrList) xmlDebugDumpAttrList__internal_alias __attribute((visibility("hidden")));
#define xmlDebugDumpAttrList xmlDebugDumpAttrList__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlDebugDumpDTD
extern __typeof (xmlDebugDumpDTD) xmlDebugDumpDTD __attribute((alias("xmlDebugDumpDTD__internal_alias")));
#else
#ifndef xmlDebugDumpDTD
extern __typeof (xmlDebugDumpDTD) xmlDebugDumpDTD__internal_alias __attribute((visibility("hidden")));
#define xmlDebugDumpDTD xmlDebugDumpDTD__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlDebugDumpDocument
extern __typeof (xmlDebugDumpDocument) xmlDebugDumpDocument __attribute((alias("xmlDebugDumpDocument__internal_alias")));
#else
#ifndef xmlDebugDumpDocument
extern __typeof (xmlDebugDumpDocument) xmlDebugDumpDocument__internal_alias __attribute((visibility("hidden")));
#define xmlDebugDumpDocument xmlDebugDumpDocument__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlDebugDumpDocumentHead
extern __typeof (xmlDebugDumpDocumentHead) xmlDebugDumpDocumentHead __attribute((alias("xmlDebugDumpDocumentHead__internal_alias")));
#else
#ifndef xmlDebugDumpDocumentHead
extern __typeof (xmlDebugDumpDocumentHead) xmlDebugDumpDocumentHead__internal_alias __attribute((visibility("hidden")));
#define xmlDebugDumpDocumentHead xmlDebugDumpDocumentHead__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlDebugDumpEntities
extern __typeof (xmlDebugDumpEntities) xmlDebugDumpEntities __attribute((alias("xmlDebugDumpEntities__internal_alias")));
#else
#ifndef xmlDebugDumpEntities
extern __typeof (xmlDebugDumpEntities) xmlDebugDumpEntities__internal_alias __attribute((visibility("hidden")));
#define xmlDebugDumpEntities xmlDebugDumpEntities__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlDebugDumpNode
extern __typeof (xmlDebugDumpNode) xmlDebugDumpNode __attribute((alias("xmlDebugDumpNode__internal_alias")));
#else
#ifndef xmlDebugDumpNode
extern __typeof (xmlDebugDumpNode) xmlDebugDumpNode__internal_alias __attribute((visibility("hidden")));
#define xmlDebugDumpNode xmlDebugDumpNode__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlDebugDumpNodeList
extern __typeof (xmlDebugDumpNodeList) xmlDebugDumpNodeList __attribute((alias("xmlDebugDumpNodeList__internal_alias")));
#else
#ifndef xmlDebugDumpNodeList
extern __typeof (xmlDebugDumpNodeList) xmlDebugDumpNodeList__internal_alias __attribute((visibility("hidden")));
#define xmlDebugDumpNodeList xmlDebugDumpNodeList__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlDebugDumpOneNode
extern __typeof (xmlDebugDumpOneNode) xmlDebugDumpOneNode __attribute((alias("xmlDebugDumpOneNode__internal_alias")));
#else
#ifndef xmlDebugDumpOneNode
extern __typeof (xmlDebugDumpOneNode) xmlDebugDumpOneNode__internal_alias __attribute((visibility("hidden")));
#define xmlDebugDumpOneNode xmlDebugDumpOneNode__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlDebugDumpString
extern __typeof (xmlDebugDumpString) xmlDebugDumpString __attribute((alias("xmlDebugDumpString__internal_alias")));
#else
#ifndef xmlDebugDumpString
extern __typeof (xmlDebugDumpString) xmlDebugDumpString__internal_alias __attribute((visibility("hidden")));
#define xmlDebugDumpString xmlDebugDumpString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlDecodeEntities
extern __typeof (xmlDecodeEntities) xmlDecodeEntities __attribute((alias("xmlDecodeEntities__internal_alias")));
#else
#ifndef xmlDecodeEntities
extern __typeof (xmlDecodeEntities) xmlDecodeEntities__internal_alias __attribute((visibility("hidden")));
#define xmlDecodeEntities xmlDecodeEntities__internal_alias
#endif
#endif
#endif

#ifdef bottom_SAX2
#undef xmlDefaultSAXHandlerInit
extern __typeof (xmlDefaultSAXHandlerInit) xmlDefaultSAXHandlerInit __attribute((alias("xmlDefaultSAXHandlerInit__internal_alias")));
#else
#ifndef xmlDefaultSAXHandlerInit
extern __typeof (xmlDefaultSAXHandlerInit) xmlDefaultSAXHandlerInit__internal_alias __attribute((visibility("hidden")));
#define xmlDefaultSAXHandlerInit xmlDefaultSAXHandlerInit__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlDelEncodingAlias
extern __typeof (xmlDelEncodingAlias) xmlDelEncodingAlias __attribute((alias("xmlDelEncodingAlias__internal_alias")));
#else
#ifndef xmlDelEncodingAlias
extern __typeof (xmlDelEncodingAlias) xmlDelEncodingAlias__internal_alias __attribute((visibility("hidden")));
#define xmlDelEncodingAlias xmlDelEncodingAlias__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlDeregisterNodeDefault
extern __typeof (xmlDeregisterNodeDefault) xmlDeregisterNodeDefault __attribute((alias("xmlDeregisterNodeDefault__internal_alias")));
#else
#ifndef xmlDeregisterNodeDefault
extern __typeof (xmlDeregisterNodeDefault) xmlDeregisterNodeDefault__internal_alias __attribute((visibility("hidden")));
#define xmlDeregisterNodeDefault xmlDeregisterNodeDefault__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlDetectCharEncoding
extern __typeof (xmlDetectCharEncoding) xmlDetectCharEncoding __attribute((alias("xmlDetectCharEncoding__internal_alias")));
#else
#ifndef xmlDetectCharEncoding
extern __typeof (xmlDetectCharEncoding) xmlDetectCharEncoding__internal_alias __attribute((visibility("hidden")));
#define xmlDetectCharEncoding xmlDetectCharEncoding__internal_alias
#endif
#endif

#ifdef bottom_dict
#undef xmlDictCleanup
extern __typeof (xmlDictCleanup) xmlDictCleanup __attribute((alias("xmlDictCleanup__internal_alias")));
#else
#ifndef xmlDictCleanup
extern __typeof (xmlDictCleanup) xmlDictCleanup__internal_alias __attribute((visibility("hidden")));
#define xmlDictCleanup xmlDictCleanup__internal_alias
#endif
#endif

#ifdef bottom_dict
#undef xmlDictCreate
extern __typeof (xmlDictCreate) xmlDictCreate __attribute((alias("xmlDictCreate__internal_alias")));
#else
#ifndef xmlDictCreate
extern __typeof (xmlDictCreate) xmlDictCreate__internal_alias __attribute((visibility("hidden")));
#define xmlDictCreate xmlDictCreate__internal_alias
#endif
#endif

#ifdef bottom_dict
#undef xmlDictCreateSub
extern __typeof (xmlDictCreateSub) xmlDictCreateSub __attribute((alias("xmlDictCreateSub__internal_alias")));
#else
#ifndef xmlDictCreateSub
extern __typeof (xmlDictCreateSub) xmlDictCreateSub__internal_alias __attribute((visibility("hidden")));
#define xmlDictCreateSub xmlDictCreateSub__internal_alias
#endif
#endif

#ifdef bottom_dict
#undef xmlDictExists
extern __typeof (xmlDictExists) xmlDictExists __attribute((alias("xmlDictExists__internal_alias")));
#else
#ifndef xmlDictExists
extern __typeof (xmlDictExists) xmlDictExists__internal_alias __attribute((visibility("hidden")));
#define xmlDictExists xmlDictExists__internal_alias
#endif
#endif

#ifdef bottom_dict
#undef xmlDictFree
extern __typeof (xmlDictFree) xmlDictFree __attribute((alias("xmlDictFree__internal_alias")));
#else
#ifndef xmlDictFree
extern __typeof (xmlDictFree) xmlDictFree__internal_alias __attribute((visibility("hidden")));
#define xmlDictFree xmlDictFree__internal_alias
#endif
#endif

#ifdef bottom_dict
#undef xmlDictLookup
extern __typeof (xmlDictLookup) xmlDictLookup __attribute((alias("xmlDictLookup__internal_alias")));
#else
#ifndef xmlDictLookup
extern __typeof (xmlDictLookup) xmlDictLookup__internal_alias __attribute((visibility("hidden")));
#define xmlDictLookup xmlDictLookup__internal_alias
#endif
#endif

#ifdef bottom_dict
#undef xmlDictOwns
extern __typeof (xmlDictOwns) xmlDictOwns __attribute((alias("xmlDictOwns__internal_alias")));
#else
#ifndef xmlDictOwns
extern __typeof (xmlDictOwns) xmlDictOwns__internal_alias __attribute((visibility("hidden")));
#define xmlDictOwns xmlDictOwns__internal_alias
#endif
#endif

#ifdef bottom_dict
#undef xmlDictQLookup
extern __typeof (xmlDictQLookup) xmlDictQLookup __attribute((alias("xmlDictQLookup__internal_alias")));
#else
#ifndef xmlDictQLookup
extern __typeof (xmlDictQLookup) xmlDictQLookup__internal_alias __attribute((visibility("hidden")));
#define xmlDictQLookup xmlDictQLookup__internal_alias
#endif
#endif

#ifdef bottom_dict
#undef xmlDictReference
extern __typeof (xmlDictReference) xmlDictReference __attribute((alias("xmlDictReference__internal_alias")));
#else
#ifndef xmlDictReference
extern __typeof (xmlDictReference) xmlDictReference__internal_alias __attribute((visibility("hidden")));
#define xmlDictReference xmlDictReference__internal_alias
#endif
#endif

#ifdef bottom_dict
#undef xmlDictSize
extern __typeof (xmlDictSize) xmlDictSize __attribute((alias("xmlDictSize__internal_alias")));
#else
#ifndef xmlDictSize
extern __typeof (xmlDictSize) xmlDictSize__internal_alias __attribute((visibility("hidden")));
#define xmlDictSize xmlDictSize__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlDocCopyNode
extern __typeof (xmlDocCopyNode) xmlDocCopyNode __attribute((alias("xmlDocCopyNode__internal_alias")));
#else
#ifndef xmlDocCopyNode
extern __typeof (xmlDocCopyNode) xmlDocCopyNode__internal_alias __attribute((visibility("hidden")));
#define xmlDocCopyNode xmlDocCopyNode__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlDocCopyNodeList
extern __typeof (xmlDocCopyNodeList) xmlDocCopyNodeList __attribute((alias("xmlDocCopyNodeList__internal_alias")));
#else
#ifndef xmlDocCopyNodeList
extern __typeof (xmlDocCopyNodeList) xmlDocCopyNodeList__internal_alias __attribute((visibility("hidden")));
#define xmlDocCopyNodeList xmlDocCopyNodeList__internal_alias
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlDocDump
extern __typeof (xmlDocDump) xmlDocDump __attribute((alias("xmlDocDump__internal_alias")));
#else
#ifndef xmlDocDump
extern __typeof (xmlDocDump) xmlDocDump__internal_alias __attribute((visibility("hidden")));
#define xmlDocDump xmlDocDump__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlDocDumpFormatMemory
extern __typeof (xmlDocDumpFormatMemory) xmlDocDumpFormatMemory __attribute((alias("xmlDocDumpFormatMemory__internal_alias")));
#else
#ifndef xmlDocDumpFormatMemory
extern __typeof (xmlDocDumpFormatMemory) xmlDocDumpFormatMemory__internal_alias __attribute((visibility("hidden")));
#define xmlDocDumpFormatMemory xmlDocDumpFormatMemory__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlDocDumpFormatMemoryEnc
extern __typeof (xmlDocDumpFormatMemoryEnc) xmlDocDumpFormatMemoryEnc __attribute((alias("xmlDocDumpFormatMemoryEnc__internal_alias")));
#else
#ifndef xmlDocDumpFormatMemoryEnc
extern __typeof (xmlDocDumpFormatMemoryEnc) xmlDocDumpFormatMemoryEnc__internal_alias __attribute((visibility("hidden")));
#define xmlDocDumpFormatMemoryEnc xmlDocDumpFormatMemoryEnc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlDocDumpMemory
extern __typeof (xmlDocDumpMemory) xmlDocDumpMemory __attribute((alias("xmlDocDumpMemory__internal_alias")));
#else
#ifndef xmlDocDumpMemory
extern __typeof (xmlDocDumpMemory) xmlDocDumpMemory__internal_alias __attribute((visibility("hidden")));
#define xmlDocDumpMemory xmlDocDumpMemory__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlDocDumpMemoryEnc
extern __typeof (xmlDocDumpMemoryEnc) xmlDocDumpMemoryEnc __attribute((alias("xmlDocDumpMemoryEnc__internal_alias")));
#else
#ifndef xmlDocDumpMemoryEnc
extern __typeof (xmlDocDumpMemoryEnc) xmlDocDumpMemoryEnc__internal_alias __attribute((visibility("hidden")));
#define xmlDocDumpMemoryEnc xmlDocDumpMemoryEnc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlDocFormatDump
extern __typeof (xmlDocFormatDump) xmlDocFormatDump __attribute((alias("xmlDocFormatDump__internal_alias")));
#else
#ifndef xmlDocFormatDump
extern __typeof (xmlDocFormatDump) xmlDocFormatDump__internal_alias __attribute((visibility("hidden")));
#define xmlDocFormatDump xmlDocFormatDump__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlDocGetRootElement
extern __typeof (xmlDocGetRootElement) xmlDocGetRootElement __attribute((alias("xmlDocGetRootElement__internal_alias")));
#else
#ifndef xmlDocGetRootElement
extern __typeof (xmlDocGetRootElement) xmlDocGetRootElement__internal_alias __attribute((visibility("hidden")));
#define xmlDocGetRootElement xmlDocGetRootElement__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_tree
#undef xmlDocSetRootElement
extern __typeof (xmlDocSetRootElement) xmlDocSetRootElement __attribute((alias("xmlDocSetRootElement__internal_alias")));
#else
#ifndef xmlDocSetRootElement
extern __typeof (xmlDocSetRootElement) xmlDocSetRootElement__internal_alias __attribute((visibility("hidden")));
#define xmlDocSetRootElement xmlDocSetRootElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_valid
#undef xmlDumpAttributeDecl
extern __typeof (xmlDumpAttributeDecl) xmlDumpAttributeDecl __attribute((alias("xmlDumpAttributeDecl__internal_alias")));
#else
#ifndef xmlDumpAttributeDecl
extern __typeof (xmlDumpAttributeDecl) xmlDumpAttributeDecl__internal_alias __attribute((visibility("hidden")));
#define xmlDumpAttributeDecl xmlDumpAttributeDecl__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_valid
#undef xmlDumpAttributeTable
extern __typeof (xmlDumpAttributeTable) xmlDumpAttributeTable __attribute((alias("xmlDumpAttributeTable__internal_alias")));
#else
#ifndef xmlDumpAttributeTable
extern __typeof (xmlDumpAttributeTable) xmlDumpAttributeTable__internal_alias __attribute((visibility("hidden")));
#define xmlDumpAttributeTable xmlDumpAttributeTable__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_valid
#undef xmlDumpElementDecl
extern __typeof (xmlDumpElementDecl) xmlDumpElementDecl __attribute((alias("xmlDumpElementDecl__internal_alias")));
#else
#ifndef xmlDumpElementDecl
extern __typeof (xmlDumpElementDecl) xmlDumpElementDecl__internal_alias __attribute((visibility("hidden")));
#define xmlDumpElementDecl xmlDumpElementDecl__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_valid
#undef xmlDumpElementTable
extern __typeof (xmlDumpElementTable) xmlDumpElementTable __attribute((alias("xmlDumpElementTable__internal_alias")));
#else
#ifndef xmlDumpElementTable
extern __typeof (xmlDumpElementTable) xmlDumpElementTable__internal_alias __attribute((visibility("hidden")));
#define xmlDumpElementTable xmlDumpElementTable__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_entities
#undef xmlDumpEntitiesTable
extern __typeof (xmlDumpEntitiesTable) xmlDumpEntitiesTable __attribute((alias("xmlDumpEntitiesTable__internal_alias")));
#else
#ifndef xmlDumpEntitiesTable
extern __typeof (xmlDumpEntitiesTable) xmlDumpEntitiesTable__internal_alias __attribute((visibility("hidden")));
#define xmlDumpEntitiesTable xmlDumpEntitiesTable__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_entities
#undef xmlDumpEntityDecl
extern __typeof (xmlDumpEntityDecl) xmlDumpEntityDecl __attribute((alias("xmlDumpEntityDecl__internal_alias")));
#else
#ifndef xmlDumpEntityDecl
extern __typeof (xmlDumpEntityDecl) xmlDumpEntityDecl__internal_alias __attribute((visibility("hidden")));
#define xmlDumpEntityDecl xmlDumpEntityDecl__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_valid
#undef xmlDumpNotationDecl
extern __typeof (xmlDumpNotationDecl) xmlDumpNotationDecl __attribute((alias("xmlDumpNotationDecl__internal_alias")));
#else
#ifndef xmlDumpNotationDecl
extern __typeof (xmlDumpNotationDecl) xmlDumpNotationDecl__internal_alias __attribute((visibility("hidden")));
#define xmlDumpNotationDecl xmlDumpNotationDecl__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_valid
#undef xmlDumpNotationTable
extern __typeof (xmlDumpNotationTable) xmlDumpNotationTable __attribute((alias("xmlDumpNotationTable__internal_alias")));
#else
#ifndef xmlDumpNotationTable
extern __typeof (xmlDumpNotationTable) xmlDumpNotationTable__internal_alias __attribute((visibility("hidden")));
#define xmlDumpNotationTable xmlDumpNotationTable__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlElemDump
extern __typeof (xmlElemDump) xmlElemDump __attribute((alias("xmlElemDump__internal_alias")));
#else
#ifndef xmlElemDump
extern __typeof (xmlElemDump) xmlElemDump__internal_alias __attribute((visibility("hidden")));
#define xmlElemDump xmlElemDump__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlEncodeEntities
extern __typeof (xmlEncodeEntities) xmlEncodeEntities __attribute((alias("xmlEncodeEntities__internal_alias")));
#else
#ifndef xmlEncodeEntities
extern __typeof (xmlEncodeEntities) xmlEncodeEntities__internal_alias __attribute((visibility("hidden")));
#define xmlEncodeEntities xmlEncodeEntities__internal_alias
#endif
#endif
#endif

#ifdef bottom_entities
#undef xmlEncodeEntitiesReentrant
extern __typeof (xmlEncodeEntitiesReentrant) xmlEncodeEntitiesReentrant __attribute((alias("xmlEncodeEntitiesReentrant__internal_alias")));
#else
#ifndef xmlEncodeEntitiesReentrant
extern __typeof (xmlEncodeEntitiesReentrant) xmlEncodeEntitiesReentrant__internal_alias __attribute((visibility("hidden")));
#define xmlEncodeEntitiesReentrant xmlEncodeEntitiesReentrant__internal_alias
#endif
#endif

#ifdef bottom_entities
#undef xmlEncodeSpecialChars
extern __typeof (xmlEncodeSpecialChars) xmlEncodeSpecialChars __attribute((alias("xmlEncodeSpecialChars__internal_alias")));
#else
#ifndef xmlEncodeSpecialChars
extern __typeof (xmlEncodeSpecialChars) xmlEncodeSpecialChars__internal_alias __attribute((visibility("hidden")));
#define xmlEncodeSpecialChars xmlEncodeSpecialChars__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlErrMemory
extern __typeof (xmlErrMemory) xmlErrMemory __attribute((alias("xmlErrMemory__internal_alias")));
#else
#ifndef xmlErrMemory
extern __typeof (xmlErrMemory) xmlErrMemory__internal_alias __attribute((visibility("hidden")));
#define xmlErrMemory xmlErrMemory__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlFileClose
extern __typeof (xmlFileClose) xmlFileClose __attribute((alias("xmlFileClose__internal_alias")));
#else
#ifndef xmlFileClose
extern __typeof (xmlFileClose) xmlFileClose__internal_alias __attribute((visibility("hidden")));
#define xmlFileClose xmlFileClose__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlFileMatch
extern __typeof (xmlFileMatch) xmlFileMatch __attribute((alias("xmlFileMatch__internal_alias")));
#else
#ifndef xmlFileMatch
extern __typeof (xmlFileMatch) xmlFileMatch__internal_alias __attribute((visibility("hidden")));
#define xmlFileMatch xmlFileMatch__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlFileOpen
extern __typeof (xmlFileOpen) xmlFileOpen __attribute((alias("xmlFileOpen__internal_alias")));
#else
#ifndef xmlFileOpen
extern __typeof (xmlFileOpen) xmlFileOpen__internal_alias __attribute((visibility("hidden")));
#define xmlFileOpen xmlFileOpen__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlFileRead
extern __typeof (xmlFileRead) xmlFileRead __attribute((alias("xmlFileRead__internal_alias")));
#else
#ifndef xmlFileRead
extern __typeof (xmlFileRead) xmlFileRead__internal_alias __attribute((visibility("hidden")));
#define xmlFileRead xmlFileRead__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlFindCharEncodingHandler
extern __typeof (xmlFindCharEncodingHandler) xmlFindCharEncodingHandler __attribute((alias("xmlFindCharEncodingHandler__internal_alias")));
#else
#ifndef xmlFindCharEncodingHandler
extern __typeof (xmlFindCharEncodingHandler) xmlFindCharEncodingHandler__internal_alias __attribute((visibility("hidden")));
#define xmlFindCharEncodingHandler xmlFindCharEncodingHandler__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlFreeAttributeTable
extern __typeof (xmlFreeAttributeTable) xmlFreeAttributeTable __attribute((alias("xmlFreeAttributeTable__internal_alias")));
#else
#ifndef xmlFreeAttributeTable
extern __typeof (xmlFreeAttributeTable) xmlFreeAttributeTable__internal_alias __attribute((visibility("hidden")));
#define xmlFreeAttributeTable xmlFreeAttributeTable__internal_alias
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlFreeAutomata
extern __typeof (xmlFreeAutomata) xmlFreeAutomata __attribute((alias("xmlFreeAutomata__internal_alias")));
#else
#ifndef xmlFreeAutomata
extern __typeof (xmlFreeAutomata) xmlFreeAutomata__internal_alias __attribute((visibility("hidden")));
#define xmlFreeAutomata xmlFreeAutomata__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlFreeCatalog
extern __typeof (xmlFreeCatalog) xmlFreeCatalog __attribute((alias("xmlFreeCatalog__internal_alias")));
#else
#ifndef xmlFreeCatalog
extern __typeof (xmlFreeCatalog) xmlFreeCatalog__internal_alias __attribute((visibility("hidden")));
#define xmlFreeCatalog xmlFreeCatalog__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlFreeDoc
extern __typeof (xmlFreeDoc) xmlFreeDoc __attribute((alias("xmlFreeDoc__internal_alias")));
#else
#ifndef xmlFreeDoc
extern __typeof (xmlFreeDoc) xmlFreeDoc__internal_alias __attribute((visibility("hidden")));
#define xmlFreeDoc xmlFreeDoc__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlFreeDocElementContent
extern __typeof (xmlFreeDocElementContent) xmlFreeDocElementContent __attribute((alias("xmlFreeDocElementContent__internal_alias")));
#else
#ifndef xmlFreeDocElementContent
extern __typeof (xmlFreeDocElementContent) xmlFreeDocElementContent__internal_alias __attribute((visibility("hidden")));
#define xmlFreeDocElementContent xmlFreeDocElementContent__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlFreeDtd
extern __typeof (xmlFreeDtd) xmlFreeDtd __attribute((alias("xmlFreeDtd__internal_alias")));
#else
#ifndef xmlFreeDtd
extern __typeof (xmlFreeDtd) xmlFreeDtd__internal_alias __attribute((visibility("hidden")));
#define xmlFreeDtd xmlFreeDtd__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlFreeElementContent
extern __typeof (xmlFreeElementContent) xmlFreeElementContent __attribute((alias("xmlFreeElementContent__internal_alias")));
#else
#ifndef xmlFreeElementContent
extern __typeof (xmlFreeElementContent) xmlFreeElementContent__internal_alias __attribute((visibility("hidden")));
#define xmlFreeElementContent xmlFreeElementContent__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlFreeElementTable
extern __typeof (xmlFreeElementTable) xmlFreeElementTable __attribute((alias("xmlFreeElementTable__internal_alias")));
#else
#ifndef xmlFreeElementTable
extern __typeof (xmlFreeElementTable) xmlFreeElementTable__internal_alias __attribute((visibility("hidden")));
#define xmlFreeElementTable xmlFreeElementTable__internal_alias
#endif
#endif

#ifdef bottom_entities
#undef xmlFreeEntitiesTable
extern __typeof (xmlFreeEntitiesTable) xmlFreeEntitiesTable __attribute((alias("xmlFreeEntitiesTable__internal_alias")));
#else
#ifndef xmlFreeEntitiesTable
extern __typeof (xmlFreeEntitiesTable) xmlFreeEntitiesTable__internal_alias __attribute((visibility("hidden")));
#define xmlFreeEntitiesTable xmlFreeEntitiesTable__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlFreeEnumeration
extern __typeof (xmlFreeEnumeration) xmlFreeEnumeration __attribute((alias("xmlFreeEnumeration__internal_alias")));
#else
#ifndef xmlFreeEnumeration
extern __typeof (xmlFreeEnumeration) xmlFreeEnumeration__internal_alias __attribute((visibility("hidden")));
#define xmlFreeEnumeration xmlFreeEnumeration__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlFreeIDTable
extern __typeof (xmlFreeIDTable) xmlFreeIDTable __attribute((alias("xmlFreeIDTable__internal_alias")));
#else
#ifndef xmlFreeIDTable
extern __typeof (xmlFreeIDTable) xmlFreeIDTable__internal_alias __attribute((visibility("hidden")));
#define xmlFreeIDTable xmlFreeIDTable__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlFreeInputStream
extern __typeof (xmlFreeInputStream) xmlFreeInputStream __attribute((alias("xmlFreeInputStream__internal_alias")));
#else
#ifndef xmlFreeInputStream
extern __typeof (xmlFreeInputStream) xmlFreeInputStream__internal_alias __attribute((visibility("hidden")));
#define xmlFreeInputStream xmlFreeInputStream__internal_alias
#endif
#endif

#ifdef bottom_threads
#undef xmlFreeMutex
extern __typeof (xmlFreeMutex) xmlFreeMutex __attribute((alias("xmlFreeMutex__internal_alias")));
#else
#ifndef xmlFreeMutex
extern __typeof (xmlFreeMutex) xmlFreeMutex__internal_alias __attribute((visibility("hidden")));
#define xmlFreeMutex xmlFreeMutex__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlFreeNode
extern __typeof (xmlFreeNode) xmlFreeNode __attribute((alias("xmlFreeNode__internal_alias")));
#else
#ifndef xmlFreeNode
extern __typeof (xmlFreeNode) xmlFreeNode__internal_alias __attribute((visibility("hidden")));
#define xmlFreeNode xmlFreeNode__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlFreeNodeList
extern __typeof (xmlFreeNodeList) xmlFreeNodeList __attribute((alias("xmlFreeNodeList__internal_alias")));
#else
#ifndef xmlFreeNodeList
extern __typeof (xmlFreeNodeList) xmlFreeNodeList__internal_alias __attribute((visibility("hidden")));
#define xmlFreeNodeList xmlFreeNodeList__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlFreeNotationTable
extern __typeof (xmlFreeNotationTable) xmlFreeNotationTable __attribute((alias("xmlFreeNotationTable__internal_alias")));
#else
#ifndef xmlFreeNotationTable
extern __typeof (xmlFreeNotationTable) xmlFreeNotationTable__internal_alias __attribute((visibility("hidden")));
#define xmlFreeNotationTable xmlFreeNotationTable__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlFreeNs
extern __typeof (xmlFreeNs) xmlFreeNs __attribute((alias("xmlFreeNs__internal_alias")));
#else
#ifndef xmlFreeNs
extern __typeof (xmlFreeNs) xmlFreeNs__internal_alias __attribute((visibility("hidden")));
#define xmlFreeNs xmlFreeNs__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlFreeNsList
extern __typeof (xmlFreeNsList) xmlFreeNsList __attribute((alias("xmlFreeNsList__internal_alias")));
#else
#ifndef xmlFreeNsList
extern __typeof (xmlFreeNsList) xmlFreeNsList__internal_alias __attribute((visibility("hidden")));
#define xmlFreeNsList xmlFreeNsList__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlFreeParserCtxt
extern __typeof (xmlFreeParserCtxt) xmlFreeParserCtxt __attribute((alias("xmlFreeParserCtxt__internal_alias")));
#else
#ifndef xmlFreeParserCtxt
extern __typeof (xmlFreeParserCtxt) xmlFreeParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlFreeParserCtxt xmlFreeParserCtxt__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlFreeParserInputBuffer
extern __typeof (xmlFreeParserInputBuffer) xmlFreeParserInputBuffer __attribute((alias("xmlFreeParserInputBuffer__internal_alias")));
#else
#ifndef xmlFreeParserInputBuffer
extern __typeof (xmlFreeParserInputBuffer) xmlFreeParserInputBuffer__internal_alias __attribute((visibility("hidden")));
#define xmlFreeParserInputBuffer xmlFreeParserInputBuffer__internal_alias
#endif
#endif

#if defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_pattern
#undef xmlFreePattern
extern __typeof (xmlFreePattern) xmlFreePattern __attribute((alias("xmlFreePattern__internal_alias")));
#else
#ifndef xmlFreePattern
extern __typeof (xmlFreePattern) xmlFreePattern__internal_alias __attribute((visibility("hidden")));
#define xmlFreePattern xmlFreePattern__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_pattern
#undef xmlFreePatternList
extern __typeof (xmlFreePatternList) xmlFreePatternList __attribute((alias("xmlFreePatternList__internal_alias")));
#else
#ifndef xmlFreePatternList
extern __typeof (xmlFreePatternList) xmlFreePatternList__internal_alias __attribute((visibility("hidden")));
#define xmlFreePatternList xmlFreePatternList__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlFreeProp
extern __typeof (xmlFreeProp) xmlFreeProp __attribute((alias("xmlFreeProp__internal_alias")));
#else
#ifndef xmlFreeProp
extern __typeof (xmlFreeProp) xmlFreeProp__internal_alias __attribute((visibility("hidden")));
#define xmlFreeProp xmlFreeProp__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlFreePropList
extern __typeof (xmlFreePropList) xmlFreePropList __attribute((alias("xmlFreePropList__internal_alias")));
#else
#ifndef xmlFreePropList
extern __typeof (xmlFreePropList) xmlFreePropList__internal_alias __attribute((visibility("hidden")));
#define xmlFreePropList xmlFreePropList__internal_alias
#endif
#endif

#ifdef bottom_threads
#undef xmlFreeRMutex
extern __typeof (xmlFreeRMutex) xmlFreeRMutex __attribute((alias("xmlFreeRMutex__internal_alias")));
#else
#ifndef xmlFreeRMutex
extern __typeof (xmlFreeRMutex) xmlFreeRMutex__internal_alias __attribute((visibility("hidden")));
#define xmlFreeRMutex xmlFreeRMutex__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlFreeRefTable
extern __typeof (xmlFreeRefTable) xmlFreeRefTable __attribute((alias("xmlFreeRefTable__internal_alias")));
#else
#ifndef xmlFreeRefTable
extern __typeof (xmlFreeRefTable) xmlFreeRefTable__internal_alias __attribute((visibility("hidden")));
#define xmlFreeRefTable xmlFreeRefTable__internal_alias
#endif
#endif

#if defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_pattern
#undef xmlFreeStreamCtxt
extern __typeof (xmlFreeStreamCtxt) xmlFreeStreamCtxt __attribute((alias("xmlFreeStreamCtxt__internal_alias")));
#else
#ifndef xmlFreeStreamCtxt
extern __typeof (xmlFreeStreamCtxt) xmlFreeStreamCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlFreeStreamCtxt xmlFreeStreamCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlFreeTextReader
extern __typeof (xmlFreeTextReader) xmlFreeTextReader __attribute((alias("xmlFreeTextReader__internal_alias")));
#else
#ifndef xmlFreeTextReader
extern __typeof (xmlFreeTextReader) xmlFreeTextReader__internal_alias __attribute((visibility("hidden")));
#define xmlFreeTextReader xmlFreeTextReader__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlFreeTextWriter
extern __typeof (xmlFreeTextWriter) xmlFreeTextWriter __attribute((alias("xmlFreeTextWriter__internal_alias")));
#else
#ifndef xmlFreeTextWriter
extern __typeof (xmlFreeTextWriter) xmlFreeTextWriter__internal_alias __attribute((visibility("hidden")));
#define xmlFreeTextWriter xmlFreeTextWriter__internal_alias
#endif
#endif
#endif

#ifdef bottom_uri
#undef xmlFreeURI
extern __typeof (xmlFreeURI) xmlFreeURI __attribute((alias("xmlFreeURI__internal_alias")));
#else
#ifndef xmlFreeURI
extern __typeof (xmlFreeURI) xmlFreeURI__internal_alias __attribute((visibility("hidden")));
#define xmlFreeURI xmlFreeURI__internal_alias
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlFreeValidCtxt
extern __typeof (xmlFreeValidCtxt) xmlFreeValidCtxt __attribute((alias("xmlFreeValidCtxt__internal_alias")));
#else
#ifndef xmlFreeValidCtxt
extern __typeof (xmlFreeValidCtxt) xmlFreeValidCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlFreeValidCtxt xmlFreeValidCtxt__internal_alias
#endif
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlGcMemGet
extern __typeof (xmlGcMemGet) xmlGcMemGet __attribute((alias("xmlGcMemGet__internal_alias")));
#else
#ifndef xmlGcMemGet
extern __typeof (xmlGcMemGet) xmlGcMemGet__internal_alias __attribute((visibility("hidden")));
#define xmlGcMemGet xmlGcMemGet__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlGcMemSetup
extern __typeof (xmlGcMemSetup) xmlGcMemSetup __attribute((alias("xmlGcMemSetup__internal_alias")));
#else
#ifndef xmlGcMemSetup
extern __typeof (xmlGcMemSetup) xmlGcMemSetup__internal_alias __attribute((visibility("hidden")));
#define xmlGcMemSetup xmlGcMemSetup__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlGetBufferAllocationScheme
extern __typeof (xmlGetBufferAllocationScheme) xmlGetBufferAllocationScheme __attribute((alias("xmlGetBufferAllocationScheme__internal_alias")));
#else
#ifndef xmlGetBufferAllocationScheme
extern __typeof (xmlGetBufferAllocationScheme) xmlGetBufferAllocationScheme__internal_alias __attribute((visibility("hidden")));
#define xmlGetBufferAllocationScheme xmlGetBufferAllocationScheme__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlGetCharEncodingHandler
extern __typeof (xmlGetCharEncodingHandler) xmlGetCharEncodingHandler __attribute((alias("xmlGetCharEncodingHandler__internal_alias")));
#else
#ifndef xmlGetCharEncodingHandler
extern __typeof (xmlGetCharEncodingHandler) xmlGetCharEncodingHandler__internal_alias __attribute((visibility("hidden")));
#define xmlGetCharEncodingHandler xmlGetCharEncodingHandler__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlGetCharEncodingName
extern __typeof (xmlGetCharEncodingName) xmlGetCharEncodingName __attribute((alias("xmlGetCharEncodingName__internal_alias")));
#else
#ifndef xmlGetCharEncodingName
extern __typeof (xmlGetCharEncodingName) xmlGetCharEncodingName__internal_alias __attribute((visibility("hidden")));
#define xmlGetCharEncodingName xmlGetCharEncodingName__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlGetCompressMode
extern __typeof (xmlGetCompressMode) xmlGetCompressMode __attribute((alias("xmlGetCompressMode__internal_alias")));
#else
#ifndef xmlGetCompressMode
extern __typeof (xmlGetCompressMode) xmlGetCompressMode__internal_alias __attribute((visibility("hidden")));
#define xmlGetCompressMode xmlGetCompressMode__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlGetDocCompressMode
extern __typeof (xmlGetDocCompressMode) xmlGetDocCompressMode __attribute((alias("xmlGetDocCompressMode__internal_alias")));
#else
#ifndef xmlGetDocCompressMode
extern __typeof (xmlGetDocCompressMode) xmlGetDocCompressMode__internal_alias __attribute((visibility("hidden")));
#define xmlGetDocCompressMode xmlGetDocCompressMode__internal_alias
#endif
#endif

#ifdef bottom_entities
#undef xmlGetDocEntity
extern __typeof (xmlGetDocEntity) xmlGetDocEntity __attribute((alias("xmlGetDocEntity__internal_alias")));
#else
#ifndef xmlGetDocEntity
extern __typeof (xmlGetDocEntity) xmlGetDocEntity__internal_alias __attribute((visibility("hidden")));
#define xmlGetDocEntity xmlGetDocEntity__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlGetDtdAttrDesc
extern __typeof (xmlGetDtdAttrDesc) xmlGetDtdAttrDesc __attribute((alias("xmlGetDtdAttrDesc__internal_alias")));
#else
#ifndef xmlGetDtdAttrDesc
extern __typeof (xmlGetDtdAttrDesc) xmlGetDtdAttrDesc__internal_alias __attribute((visibility("hidden")));
#define xmlGetDtdAttrDesc xmlGetDtdAttrDesc__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlGetDtdElementDesc
extern __typeof (xmlGetDtdElementDesc) xmlGetDtdElementDesc __attribute((alias("xmlGetDtdElementDesc__internal_alias")));
#else
#ifndef xmlGetDtdElementDesc
extern __typeof (xmlGetDtdElementDesc) xmlGetDtdElementDesc__internal_alias __attribute((visibility("hidden")));
#define xmlGetDtdElementDesc xmlGetDtdElementDesc__internal_alias
#endif
#endif

#ifdef bottom_entities
#undef xmlGetDtdEntity
extern __typeof (xmlGetDtdEntity) xmlGetDtdEntity __attribute((alias("xmlGetDtdEntity__internal_alias")));
#else
#ifndef xmlGetDtdEntity
extern __typeof (xmlGetDtdEntity) xmlGetDtdEntity__internal_alias __attribute((visibility("hidden")));
#define xmlGetDtdEntity xmlGetDtdEntity__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlGetDtdNotationDesc
extern __typeof (xmlGetDtdNotationDesc) xmlGetDtdNotationDesc __attribute((alias("xmlGetDtdNotationDesc__internal_alias")));
#else
#ifndef xmlGetDtdNotationDesc
extern __typeof (xmlGetDtdNotationDesc) xmlGetDtdNotationDesc__internal_alias __attribute((visibility("hidden")));
#define xmlGetDtdNotationDesc xmlGetDtdNotationDesc__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlGetDtdQAttrDesc
extern __typeof (xmlGetDtdQAttrDesc) xmlGetDtdQAttrDesc __attribute((alias("xmlGetDtdQAttrDesc__internal_alias")));
#else
#ifndef xmlGetDtdQAttrDesc
extern __typeof (xmlGetDtdQAttrDesc) xmlGetDtdQAttrDesc__internal_alias __attribute((visibility("hidden")));
#define xmlGetDtdQAttrDesc xmlGetDtdQAttrDesc__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlGetDtdQElementDesc
extern __typeof (xmlGetDtdQElementDesc) xmlGetDtdQElementDesc __attribute((alias("xmlGetDtdQElementDesc__internal_alias")));
#else
#ifndef xmlGetDtdQElementDesc
extern __typeof (xmlGetDtdQElementDesc) xmlGetDtdQElementDesc__internal_alias __attribute((visibility("hidden")));
#define xmlGetDtdQElementDesc xmlGetDtdQElementDesc__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlGetEncodingAlias
extern __typeof (xmlGetEncodingAlias) xmlGetEncodingAlias __attribute((alias("xmlGetEncodingAlias__internal_alias")));
#else
#ifndef xmlGetEncodingAlias
extern __typeof (xmlGetEncodingAlias) xmlGetEncodingAlias__internal_alias __attribute((visibility("hidden")));
#define xmlGetEncodingAlias xmlGetEncodingAlias__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlGetExternalEntityLoader
extern __typeof (xmlGetExternalEntityLoader) xmlGetExternalEntityLoader __attribute((alias("xmlGetExternalEntityLoader__internal_alias")));
#else
#ifndef xmlGetExternalEntityLoader
extern __typeof (xmlGetExternalEntityLoader) xmlGetExternalEntityLoader__internal_alias __attribute((visibility("hidden")));
#define xmlGetExternalEntityLoader xmlGetExternalEntityLoader__internal_alias
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlGetFeature
extern __typeof (xmlGetFeature) xmlGetFeature __attribute((alias("xmlGetFeature__internal_alias")));
#else
#ifndef xmlGetFeature
extern __typeof (xmlGetFeature) xmlGetFeature__internal_alias __attribute((visibility("hidden")));
#define xmlGetFeature xmlGetFeature__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlGetFeaturesList
extern __typeof (xmlGetFeaturesList) xmlGetFeaturesList __attribute((alias("xmlGetFeaturesList__internal_alias")));
#else
#ifndef xmlGetFeaturesList
extern __typeof (xmlGetFeaturesList) xmlGetFeaturesList__internal_alias __attribute((visibility("hidden")));
#define xmlGetFeaturesList xmlGetFeaturesList__internal_alias
#endif
#endif
#endif

#ifdef bottom_threads
#undef xmlGetGlobalState
extern __typeof (xmlGetGlobalState) xmlGetGlobalState __attribute((alias("xmlGetGlobalState__internal_alias")));
#else
#ifndef xmlGetGlobalState
extern __typeof (xmlGetGlobalState) xmlGetGlobalState__internal_alias __attribute((visibility("hidden")));
#define xmlGetGlobalState xmlGetGlobalState__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlGetID
extern __typeof (xmlGetID) xmlGetID __attribute((alias("xmlGetID__internal_alias")));
#else
#ifndef xmlGetID
extern __typeof (xmlGetID) xmlGetID__internal_alias __attribute((visibility("hidden")));
#define xmlGetID xmlGetID__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlGetIntSubset
extern __typeof (xmlGetIntSubset) xmlGetIntSubset __attribute((alias("xmlGetIntSubset__internal_alias")));
#else
#ifndef xmlGetIntSubset
extern __typeof (xmlGetIntSubset) xmlGetIntSubset__internal_alias __attribute((visibility("hidden")));
#define xmlGetIntSubset xmlGetIntSubset__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlGetLastChild
extern __typeof (xmlGetLastChild) xmlGetLastChild __attribute((alias("xmlGetLastChild__internal_alias")));
#else
#ifndef xmlGetLastChild
extern __typeof (xmlGetLastChild) xmlGetLastChild__internal_alias __attribute((visibility("hidden")));
#define xmlGetLastChild xmlGetLastChild__internal_alias
#endif
#endif

#ifdef bottom_error
#undef xmlGetLastError
extern __typeof (xmlGetLastError) xmlGetLastError __attribute((alias("xmlGetLastError__internal_alias")));
#else
#ifndef xmlGetLastError
extern __typeof (xmlGetLastError) xmlGetLastError__internal_alias __attribute((visibility("hidden")));
#define xmlGetLastError xmlGetLastError__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlGetLineNo
extern __typeof (xmlGetLineNo) xmlGetLineNo __attribute((alias("xmlGetLineNo__internal_alias")));
#else
#ifndef xmlGetLineNo
extern __typeof (xmlGetLineNo) xmlGetLineNo__internal_alias __attribute((visibility("hidden")));
#define xmlGetLineNo xmlGetLineNo__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlGetNoNsProp
extern __typeof (xmlGetNoNsProp) xmlGetNoNsProp __attribute((alias("xmlGetNoNsProp__internal_alias")));
#else
#ifndef xmlGetNoNsProp
extern __typeof (xmlGetNoNsProp) xmlGetNoNsProp__internal_alias __attribute((visibility("hidden")));
#define xmlGetNoNsProp xmlGetNoNsProp__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_tree
#undef xmlGetNodePath
extern __typeof (xmlGetNodePath) xmlGetNodePath __attribute((alias("xmlGetNodePath__internal_alias")));
#else
#ifndef xmlGetNodePath
extern __typeof (xmlGetNodePath) xmlGetNodePath__internal_alias __attribute((visibility("hidden")));
#define xmlGetNodePath xmlGetNodePath__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_tree
#undef xmlGetNsList
extern __typeof (xmlGetNsList) xmlGetNsList __attribute((alias("xmlGetNsList__internal_alias")));
#else
#ifndef xmlGetNsList
extern __typeof (xmlGetNsList) xmlGetNsList__internal_alias __attribute((visibility("hidden")));
#define xmlGetNsList xmlGetNsList__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlGetNsProp
extern __typeof (xmlGetNsProp) xmlGetNsProp __attribute((alias("xmlGetNsProp__internal_alias")));
#else
#ifndef xmlGetNsProp
extern __typeof (xmlGetNsProp) xmlGetNsProp__internal_alias __attribute((visibility("hidden")));
#define xmlGetNsProp xmlGetNsProp__internal_alias
#endif
#endif

#ifdef bottom_entities
#undef xmlGetParameterEntity
extern __typeof (xmlGetParameterEntity) xmlGetParameterEntity __attribute((alias("xmlGetParameterEntity__internal_alias")));
#else
#ifndef xmlGetParameterEntity
extern __typeof (xmlGetParameterEntity) xmlGetParameterEntity__internal_alias __attribute((visibility("hidden")));
#define xmlGetParameterEntity xmlGetParameterEntity__internal_alias
#endif
#endif

#ifdef bottom_entities
#undef xmlGetPredefinedEntity
extern __typeof (xmlGetPredefinedEntity) xmlGetPredefinedEntity __attribute((alias("xmlGetPredefinedEntity__internal_alias")));
#else
#ifndef xmlGetPredefinedEntity
extern __typeof (xmlGetPredefinedEntity) xmlGetPredefinedEntity__internal_alias __attribute((visibility("hidden")));
#define xmlGetPredefinedEntity xmlGetPredefinedEntity__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlGetProp
extern __typeof (xmlGetProp) xmlGetProp __attribute((alias("xmlGetProp__internal_alias")));
#else
#ifndef xmlGetProp
extern __typeof (xmlGetProp) xmlGetProp__internal_alias __attribute((visibility("hidden")));
#define xmlGetProp xmlGetProp__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlGetRefs
extern __typeof (xmlGetRefs) xmlGetRefs __attribute((alias("xmlGetRefs__internal_alias")));
#else
#ifndef xmlGetRefs
extern __typeof (xmlGetRefs) xmlGetRefs__internal_alias __attribute((visibility("hidden")));
#define xmlGetRefs xmlGetRefs__internal_alias
#endif
#endif

#ifdef bottom_threads
#undef xmlGetThreadId
extern __typeof (xmlGetThreadId) xmlGetThreadId __attribute((alias("xmlGetThreadId__internal_alias")));
#else
#ifndef xmlGetThreadId
extern __typeof (xmlGetThreadId) xmlGetThreadId__internal_alias __attribute((visibility("hidden")));
#define xmlGetThreadId xmlGetThreadId__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlGetUTF8Char
extern __typeof (xmlGetUTF8Char) xmlGetUTF8Char __attribute((alias("xmlGetUTF8Char__internal_alias")));
#else
#ifndef xmlGetUTF8Char
extern __typeof (xmlGetUTF8Char) xmlGetUTF8Char__internal_alias __attribute((visibility("hidden")));
#define xmlGetUTF8Char xmlGetUTF8Char__internal_alias
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlHandleEntity
extern __typeof (xmlHandleEntity) xmlHandleEntity __attribute((alias("xmlHandleEntity__internal_alias")));
#else
#ifndef xmlHandleEntity
extern __typeof (xmlHandleEntity) xmlHandleEntity__internal_alias __attribute((visibility("hidden")));
#define xmlHandleEntity xmlHandleEntity__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlHasNsProp
extern __typeof (xmlHasNsProp) xmlHasNsProp __attribute((alias("xmlHasNsProp__internal_alias")));
#else
#ifndef xmlHasNsProp
extern __typeof (xmlHasNsProp) xmlHasNsProp__internal_alias __attribute((visibility("hidden")));
#define xmlHasNsProp xmlHasNsProp__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlHasProp
extern __typeof (xmlHasProp) xmlHasProp __attribute((alias("xmlHasProp__internal_alias")));
#else
#ifndef xmlHasProp
extern __typeof (xmlHasProp) xmlHasProp__internal_alias __attribute((visibility("hidden")));
#define xmlHasProp xmlHasProp__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashAddEntry
extern __typeof (xmlHashAddEntry) xmlHashAddEntry __attribute((alias("xmlHashAddEntry__internal_alias")));
#else
#ifndef xmlHashAddEntry
extern __typeof (xmlHashAddEntry) xmlHashAddEntry__internal_alias __attribute((visibility("hidden")));
#define xmlHashAddEntry xmlHashAddEntry__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashAddEntry2
extern __typeof (xmlHashAddEntry2) xmlHashAddEntry2 __attribute((alias("xmlHashAddEntry2__internal_alias")));
#else
#ifndef xmlHashAddEntry2
extern __typeof (xmlHashAddEntry2) xmlHashAddEntry2__internal_alias __attribute((visibility("hidden")));
#define xmlHashAddEntry2 xmlHashAddEntry2__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashAddEntry3
extern __typeof (xmlHashAddEntry3) xmlHashAddEntry3 __attribute((alias("xmlHashAddEntry3__internal_alias")));
#else
#ifndef xmlHashAddEntry3
extern __typeof (xmlHashAddEntry3) xmlHashAddEntry3__internal_alias __attribute((visibility("hidden")));
#define xmlHashAddEntry3 xmlHashAddEntry3__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashCopy
extern __typeof (xmlHashCopy) xmlHashCopy __attribute((alias("xmlHashCopy__internal_alias")));
#else
#ifndef xmlHashCopy
extern __typeof (xmlHashCopy) xmlHashCopy__internal_alias __attribute((visibility("hidden")));
#define xmlHashCopy xmlHashCopy__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashCreate
extern __typeof (xmlHashCreate) xmlHashCreate __attribute((alias("xmlHashCreate__internal_alias")));
#else
#ifndef xmlHashCreate
extern __typeof (xmlHashCreate) xmlHashCreate__internal_alias __attribute((visibility("hidden")));
#define xmlHashCreate xmlHashCreate__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashCreateDict
extern __typeof (xmlHashCreateDict) xmlHashCreateDict __attribute((alias("xmlHashCreateDict__internal_alias")));
#else
#ifndef xmlHashCreateDict
extern __typeof (xmlHashCreateDict) xmlHashCreateDict__internal_alias __attribute((visibility("hidden")));
#define xmlHashCreateDict xmlHashCreateDict__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashFree
extern __typeof (xmlHashFree) xmlHashFree __attribute((alias("xmlHashFree__internal_alias")));
#else
#ifndef xmlHashFree
extern __typeof (xmlHashFree) xmlHashFree__internal_alias __attribute((visibility("hidden")));
#define xmlHashFree xmlHashFree__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashLookup
extern __typeof (xmlHashLookup) xmlHashLookup __attribute((alias("xmlHashLookup__internal_alias")));
#else
#ifndef xmlHashLookup
extern __typeof (xmlHashLookup) xmlHashLookup__internal_alias __attribute((visibility("hidden")));
#define xmlHashLookup xmlHashLookup__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashLookup2
extern __typeof (xmlHashLookup2) xmlHashLookup2 __attribute((alias("xmlHashLookup2__internal_alias")));
#else
#ifndef xmlHashLookup2
extern __typeof (xmlHashLookup2) xmlHashLookup2__internal_alias __attribute((visibility("hidden")));
#define xmlHashLookup2 xmlHashLookup2__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashLookup3
extern __typeof (xmlHashLookup3) xmlHashLookup3 __attribute((alias("xmlHashLookup3__internal_alias")));
#else
#ifndef xmlHashLookup3
extern __typeof (xmlHashLookup3) xmlHashLookup3__internal_alias __attribute((visibility("hidden")));
#define xmlHashLookup3 xmlHashLookup3__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashQLookup
extern __typeof (xmlHashQLookup) xmlHashQLookup __attribute((alias("xmlHashQLookup__internal_alias")));
#else
#ifndef xmlHashQLookup
extern __typeof (xmlHashQLookup) xmlHashQLookup__internal_alias __attribute((visibility("hidden")));
#define xmlHashQLookup xmlHashQLookup__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashQLookup2
extern __typeof (xmlHashQLookup2) xmlHashQLookup2 __attribute((alias("xmlHashQLookup2__internal_alias")));
#else
#ifndef xmlHashQLookup2
extern __typeof (xmlHashQLookup2) xmlHashQLookup2__internal_alias __attribute((visibility("hidden")));
#define xmlHashQLookup2 xmlHashQLookup2__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashQLookup3
extern __typeof (xmlHashQLookup3) xmlHashQLookup3 __attribute((alias("xmlHashQLookup3__internal_alias")));
#else
#ifndef xmlHashQLookup3
extern __typeof (xmlHashQLookup3) xmlHashQLookup3__internal_alias __attribute((visibility("hidden")));
#define xmlHashQLookup3 xmlHashQLookup3__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashRemoveEntry
extern __typeof (xmlHashRemoveEntry) xmlHashRemoveEntry __attribute((alias("xmlHashRemoveEntry__internal_alias")));
#else
#ifndef xmlHashRemoveEntry
extern __typeof (xmlHashRemoveEntry) xmlHashRemoveEntry__internal_alias __attribute((visibility("hidden")));
#define xmlHashRemoveEntry xmlHashRemoveEntry__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashRemoveEntry2
extern __typeof (xmlHashRemoveEntry2) xmlHashRemoveEntry2 __attribute((alias("xmlHashRemoveEntry2__internal_alias")));
#else
#ifndef xmlHashRemoveEntry2
extern __typeof (xmlHashRemoveEntry2) xmlHashRemoveEntry2__internal_alias __attribute((visibility("hidden")));
#define xmlHashRemoveEntry2 xmlHashRemoveEntry2__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashRemoveEntry3
extern __typeof (xmlHashRemoveEntry3) xmlHashRemoveEntry3 __attribute((alias("xmlHashRemoveEntry3__internal_alias")));
#else
#ifndef xmlHashRemoveEntry3
extern __typeof (xmlHashRemoveEntry3) xmlHashRemoveEntry3__internal_alias __attribute((visibility("hidden")));
#define xmlHashRemoveEntry3 xmlHashRemoveEntry3__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashScan
extern __typeof (xmlHashScan) xmlHashScan __attribute((alias("xmlHashScan__internal_alias")));
#else
#ifndef xmlHashScan
extern __typeof (xmlHashScan) xmlHashScan__internal_alias __attribute((visibility("hidden")));
#define xmlHashScan xmlHashScan__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashScan3
extern __typeof (xmlHashScan3) xmlHashScan3 __attribute((alias("xmlHashScan3__internal_alias")));
#else
#ifndef xmlHashScan3
extern __typeof (xmlHashScan3) xmlHashScan3__internal_alias __attribute((visibility("hidden")));
#define xmlHashScan3 xmlHashScan3__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashScanFull
extern __typeof (xmlHashScanFull) xmlHashScanFull __attribute((alias("xmlHashScanFull__internal_alias")));
#else
#ifndef xmlHashScanFull
extern __typeof (xmlHashScanFull) xmlHashScanFull__internal_alias __attribute((visibility("hidden")));
#define xmlHashScanFull xmlHashScanFull__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashScanFull3
extern __typeof (xmlHashScanFull3) xmlHashScanFull3 __attribute((alias("xmlHashScanFull3__internal_alias")));
#else
#ifndef xmlHashScanFull3
extern __typeof (xmlHashScanFull3) xmlHashScanFull3__internal_alias __attribute((visibility("hidden")));
#define xmlHashScanFull3 xmlHashScanFull3__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashSize
extern __typeof (xmlHashSize) xmlHashSize __attribute((alias("xmlHashSize__internal_alias")));
#else
#ifndef xmlHashSize
extern __typeof (xmlHashSize) xmlHashSize__internal_alias __attribute((visibility("hidden")));
#define xmlHashSize xmlHashSize__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashUpdateEntry
extern __typeof (xmlHashUpdateEntry) xmlHashUpdateEntry __attribute((alias("xmlHashUpdateEntry__internal_alias")));
#else
#ifndef xmlHashUpdateEntry
extern __typeof (xmlHashUpdateEntry) xmlHashUpdateEntry__internal_alias __attribute((visibility("hidden")));
#define xmlHashUpdateEntry xmlHashUpdateEntry__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashUpdateEntry2
extern __typeof (xmlHashUpdateEntry2) xmlHashUpdateEntry2 __attribute((alias("xmlHashUpdateEntry2__internal_alias")));
#else
#ifndef xmlHashUpdateEntry2
extern __typeof (xmlHashUpdateEntry2) xmlHashUpdateEntry2__internal_alias __attribute((visibility("hidden")));
#define xmlHashUpdateEntry2 xmlHashUpdateEntry2__internal_alias
#endif
#endif

#ifdef bottom_hash
#undef xmlHashUpdateEntry3
extern __typeof (xmlHashUpdateEntry3) xmlHashUpdateEntry3 __attribute((alias("xmlHashUpdateEntry3__internal_alias")));
#else
#ifndef xmlHashUpdateEntry3
extern __typeof (xmlHashUpdateEntry3) xmlHashUpdateEntry3__internal_alias __attribute((visibility("hidden")));
#define xmlHashUpdateEntry3 xmlHashUpdateEntry3__internal_alias
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_xmlIO
#undef xmlIOFTPClose
extern __typeof (xmlIOFTPClose) xmlIOFTPClose __attribute((alias("xmlIOFTPClose__internal_alias")));
#else
#ifndef xmlIOFTPClose
extern __typeof (xmlIOFTPClose) xmlIOFTPClose__internal_alias __attribute((visibility("hidden")));
#define xmlIOFTPClose xmlIOFTPClose__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_xmlIO
#undef xmlIOFTPMatch
extern __typeof (xmlIOFTPMatch) xmlIOFTPMatch __attribute((alias("xmlIOFTPMatch__internal_alias")));
#else
#ifndef xmlIOFTPMatch
extern __typeof (xmlIOFTPMatch) xmlIOFTPMatch__internal_alias __attribute((visibility("hidden")));
#define xmlIOFTPMatch xmlIOFTPMatch__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_xmlIO
#undef xmlIOFTPOpen
extern __typeof (xmlIOFTPOpen) xmlIOFTPOpen __attribute((alias("xmlIOFTPOpen__internal_alias")));
#else
#ifndef xmlIOFTPOpen
extern __typeof (xmlIOFTPOpen) xmlIOFTPOpen__internal_alias __attribute((visibility("hidden")));
#define xmlIOFTPOpen xmlIOFTPOpen__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_xmlIO
#undef xmlIOFTPRead
extern __typeof (xmlIOFTPRead) xmlIOFTPRead __attribute((alias("xmlIOFTPRead__internal_alias")));
#else
#ifndef xmlIOFTPRead
extern __typeof (xmlIOFTPRead) xmlIOFTPRead__internal_alias __attribute((visibility("hidden")));
#define xmlIOFTPRead xmlIOFTPRead__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_xmlIO
#undef xmlIOHTTPClose
extern __typeof (xmlIOHTTPClose) xmlIOHTTPClose __attribute((alias("xmlIOHTTPClose__internal_alias")));
#else
#ifndef xmlIOHTTPClose
extern __typeof (xmlIOHTTPClose) xmlIOHTTPClose__internal_alias __attribute((visibility("hidden")));
#define xmlIOHTTPClose xmlIOHTTPClose__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_xmlIO
#undef xmlIOHTTPMatch
extern __typeof (xmlIOHTTPMatch) xmlIOHTTPMatch __attribute((alias("xmlIOHTTPMatch__internal_alias")));
#else
#ifndef xmlIOHTTPMatch
extern __typeof (xmlIOHTTPMatch) xmlIOHTTPMatch__internal_alias __attribute((visibility("hidden")));
#define xmlIOHTTPMatch xmlIOHTTPMatch__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_xmlIO
#undef xmlIOHTTPOpen
extern __typeof (xmlIOHTTPOpen) xmlIOHTTPOpen __attribute((alias("xmlIOHTTPOpen__internal_alias")));
#else
#ifndef xmlIOHTTPOpen
extern __typeof (xmlIOHTTPOpen) xmlIOHTTPOpen__internal_alias __attribute((visibility("hidden")));
#define xmlIOHTTPOpen xmlIOHTTPOpen__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlIOHTTPOpenW
extern __typeof (xmlIOHTTPOpenW) xmlIOHTTPOpenW __attribute((alias("xmlIOHTTPOpenW__internal_alias")));
#else
#ifndef xmlIOHTTPOpenW
extern __typeof (xmlIOHTTPOpenW) xmlIOHTTPOpenW__internal_alias __attribute((visibility("hidden")));
#define xmlIOHTTPOpenW xmlIOHTTPOpenW__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_xmlIO
#undef xmlIOHTTPRead
extern __typeof (xmlIOHTTPRead) xmlIOHTTPRead __attribute((alias("xmlIOHTTPRead__internal_alias")));
#else
#ifndef xmlIOHTTPRead
extern __typeof (xmlIOHTTPRead) xmlIOHTTPRead__internal_alias __attribute((visibility("hidden")));
#define xmlIOHTTPRead xmlIOHTTPRead__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_parser
#undef xmlIOParseDTD
extern __typeof (xmlIOParseDTD) xmlIOParseDTD __attribute((alias("xmlIOParseDTD__internal_alias")));
#else
#ifndef xmlIOParseDTD
extern __typeof (xmlIOParseDTD) xmlIOParseDTD__internal_alias __attribute((visibility("hidden")));
#define xmlIOParseDTD xmlIOParseDTD__internal_alias
#endif
#endif
#endif

#ifdef bottom_encoding
#undef xmlInitCharEncodingHandlers
extern __typeof (xmlInitCharEncodingHandlers) xmlInitCharEncodingHandlers __attribute((alias("xmlInitCharEncodingHandlers__internal_alias")));
#else
#ifndef xmlInitCharEncodingHandlers
extern __typeof (xmlInitCharEncodingHandlers) xmlInitCharEncodingHandlers__internal_alias __attribute((visibility("hidden")));
#define xmlInitCharEncodingHandlers xmlInitCharEncodingHandlers__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlInitGlobals
extern __typeof (xmlInitGlobals) xmlInitGlobals __attribute((alias("xmlInitGlobals__internal_alias")));
#else
#ifndef xmlInitGlobals
extern __typeof (xmlInitGlobals) xmlInitGlobals__internal_alias __attribute((visibility("hidden")));
#define xmlInitGlobals xmlInitGlobals__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlInitMemory
extern __typeof (xmlInitMemory) xmlInitMemory __attribute((alias("xmlInitMemory__internal_alias")));
#else
#ifndef xmlInitMemory
extern __typeof (xmlInitMemory) xmlInitMemory__internal_alias __attribute((visibility("hidden")));
#define xmlInitMemory xmlInitMemory__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlInitNodeInfoSeq
extern __typeof (xmlInitNodeInfoSeq) xmlInitNodeInfoSeq __attribute((alias("xmlInitNodeInfoSeq__internal_alias")));
#else
#ifndef xmlInitNodeInfoSeq
extern __typeof (xmlInitNodeInfoSeq) xmlInitNodeInfoSeq__internal_alias __attribute((visibility("hidden")));
#define xmlInitNodeInfoSeq xmlInitNodeInfoSeq__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlInitParser
extern __typeof (xmlInitParser) xmlInitParser __attribute((alias("xmlInitParser__internal_alias")));
#else
#ifndef xmlInitParser
extern __typeof (xmlInitParser) xmlInitParser__internal_alias __attribute((visibility("hidden")));
#define xmlInitParser xmlInitParser__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlInitParserCtxt
extern __typeof (xmlInitParserCtxt) xmlInitParserCtxt __attribute((alias("xmlInitParserCtxt__internal_alias")));
#else
#ifndef xmlInitParserCtxt
extern __typeof (xmlInitParserCtxt) xmlInitParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlInitParserCtxt xmlInitParserCtxt__internal_alias
#endif
#endif

#ifdef bottom_threads
#undef xmlInitThreads
extern __typeof (xmlInitThreads) xmlInitThreads __attribute((alias("xmlInitThreads__internal_alias")));
#else
#ifndef xmlInitThreads
extern __typeof (xmlInitThreads) xmlInitThreads__internal_alias __attribute((visibility("hidden")));
#define xmlInitThreads xmlInitThreads__internal_alias
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlInitializeCatalog
extern __typeof (xmlInitializeCatalog) xmlInitializeCatalog __attribute((alias("xmlInitializeCatalog__internal_alias")));
#else
#ifndef xmlInitializeCatalog
extern __typeof (xmlInitializeCatalog) xmlInitializeCatalog__internal_alias __attribute((visibility("hidden")));
#define xmlInitializeCatalog xmlInitializeCatalog__internal_alias
#endif
#endif
#endif

#ifdef bottom_globals
#undef xmlInitializeGlobalState
extern __typeof (xmlInitializeGlobalState) xmlInitializeGlobalState __attribute((alias("xmlInitializeGlobalState__internal_alias")));
#else
#ifndef xmlInitializeGlobalState
extern __typeof (xmlInitializeGlobalState) xmlInitializeGlobalState__internal_alias __attribute((visibility("hidden")));
#define xmlInitializeGlobalState xmlInitializeGlobalState__internal_alias
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlInitializePredefinedEntities
extern __typeof (xmlInitializePredefinedEntities) xmlInitializePredefinedEntities __attribute((alias("xmlInitializePredefinedEntities__internal_alias")));
#else
#ifndef xmlInitializePredefinedEntities
extern __typeof (xmlInitializePredefinedEntities) xmlInitializePredefinedEntities__internal_alias __attribute((visibility("hidden")));
#define xmlInitializePredefinedEntities xmlInitializePredefinedEntities__internal_alias
#endif
#endif
#endif

#ifdef bottom_chvalid
#undef xmlIsBaseChar
extern __typeof (xmlIsBaseChar) xmlIsBaseChar __attribute((alias("xmlIsBaseChar__internal_alias")));
#else
#ifndef xmlIsBaseChar
extern __typeof (xmlIsBaseChar) xmlIsBaseChar__internal_alias __attribute((visibility("hidden")));
#define xmlIsBaseChar xmlIsBaseChar__internal_alias
#endif
#endif

#ifdef bottom_chvalid
#undef xmlIsBlank
extern __typeof (xmlIsBlank) xmlIsBlank __attribute((alias("xmlIsBlank__internal_alias")));
#else
#ifndef xmlIsBlank
extern __typeof (xmlIsBlank) xmlIsBlank__internal_alias __attribute((visibility("hidden")));
#define xmlIsBlank xmlIsBlank__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlIsBlankNode
extern __typeof (xmlIsBlankNode) xmlIsBlankNode __attribute((alias("xmlIsBlankNode__internal_alias")));
#else
#ifndef xmlIsBlankNode
extern __typeof (xmlIsBlankNode) xmlIsBlankNode__internal_alias __attribute((visibility("hidden")));
#define xmlIsBlankNode xmlIsBlankNode__internal_alias
#endif
#endif

#ifdef bottom_chvalid
#undef xmlIsChar
extern __typeof (xmlIsChar) xmlIsChar __attribute((alias("xmlIsChar__internal_alias")));
#else
#ifndef xmlIsChar
extern __typeof (xmlIsChar) xmlIsChar__internal_alias __attribute((visibility("hidden")));
#define xmlIsChar xmlIsChar__internal_alias
#endif
#endif

#ifdef bottom_chvalid
#undef xmlIsCombining
extern __typeof (xmlIsCombining) xmlIsCombining __attribute((alias("xmlIsCombining__internal_alias")));
#else
#ifndef xmlIsCombining
extern __typeof (xmlIsCombining) xmlIsCombining__internal_alias __attribute((visibility("hidden")));
#define xmlIsCombining xmlIsCombining__internal_alias
#endif
#endif

#ifdef bottom_chvalid
#undef xmlIsDigit
extern __typeof (xmlIsDigit) xmlIsDigit __attribute((alias("xmlIsDigit__internal_alias")));
#else
#ifndef xmlIsDigit
extern __typeof (xmlIsDigit) xmlIsDigit__internal_alias __attribute((visibility("hidden")));
#define xmlIsDigit xmlIsDigit__internal_alias
#endif
#endif

#ifdef bottom_chvalid
#undef xmlIsExtender
extern __typeof (xmlIsExtender) xmlIsExtender __attribute((alias("xmlIsExtender__internal_alias")));
#else
#ifndef xmlIsExtender
extern __typeof (xmlIsExtender) xmlIsExtender__internal_alias __attribute((visibility("hidden")));
#define xmlIsExtender xmlIsExtender__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlIsID
extern __typeof (xmlIsID) xmlIsID __attribute((alias("xmlIsID__internal_alias")));
#else
#ifndef xmlIsID
extern __typeof (xmlIsID) xmlIsID__internal_alias __attribute((visibility("hidden")));
#define xmlIsID xmlIsID__internal_alias
#endif
#endif

#ifdef bottom_chvalid
#undef xmlIsIdeographic
extern __typeof (xmlIsIdeographic) xmlIsIdeographic __attribute((alias("xmlIsIdeographic__internal_alias")));
#else
#ifndef xmlIsIdeographic
extern __typeof (xmlIsIdeographic) xmlIsIdeographic__internal_alias __attribute((visibility("hidden")));
#define xmlIsIdeographic xmlIsIdeographic__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlIsLetter
extern __typeof (xmlIsLetter) xmlIsLetter __attribute((alias("xmlIsLetter__internal_alias")));
#else
#ifndef xmlIsLetter
extern __typeof (xmlIsLetter) xmlIsLetter__internal_alias __attribute((visibility("hidden")));
#define xmlIsLetter xmlIsLetter__internal_alias
#endif
#endif

#ifdef bottom_threads
#undef xmlIsMainThread
extern __typeof (xmlIsMainThread) xmlIsMainThread __attribute((alias("xmlIsMainThread__internal_alias")));
#else
#ifndef xmlIsMainThread
extern __typeof (xmlIsMainThread) xmlIsMainThread__internal_alias __attribute((visibility("hidden")));
#define xmlIsMainThread xmlIsMainThread__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlIsMixedElement
extern __typeof (xmlIsMixedElement) xmlIsMixedElement __attribute((alias("xmlIsMixedElement__internal_alias")));
#else
#ifndef xmlIsMixedElement
extern __typeof (xmlIsMixedElement) xmlIsMixedElement__internal_alias __attribute((visibility("hidden")));
#define xmlIsMixedElement xmlIsMixedElement__internal_alias
#endif
#endif

#ifdef bottom_chvalid
#undef xmlIsPubidChar
extern __typeof (xmlIsPubidChar) xmlIsPubidChar __attribute((alias("xmlIsPubidChar__internal_alias")));
#else
#ifndef xmlIsPubidChar
extern __typeof (xmlIsPubidChar) xmlIsPubidChar__internal_alias __attribute((visibility("hidden")));
#define xmlIsPubidChar xmlIsPubidChar__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlIsRef
extern __typeof (xmlIsRef) xmlIsRef __attribute((alias("xmlIsRef__internal_alias")));
#else
#ifndef xmlIsRef
extern __typeof (xmlIsRef) xmlIsRef__internal_alias __attribute((visibility("hidden")));
#define xmlIsRef xmlIsRef__internal_alias
#endif
#endif

#ifdef bottom_xmlsave
#undef xmlIsXHTML
extern __typeof (xmlIsXHTML) xmlIsXHTML __attribute((alias("xmlIsXHTML__internal_alias")));
#else
#ifndef xmlIsXHTML
extern __typeof (xmlIsXHTML) xmlIsXHTML__internal_alias __attribute((visibility("hidden")));
#define xmlIsXHTML xmlIsXHTML__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlKeepBlanksDefault
extern __typeof (xmlKeepBlanksDefault) xmlKeepBlanksDefault __attribute((alias("xmlKeepBlanksDefault__internal_alias")));
#else
#ifndef xmlKeepBlanksDefault
extern __typeof (xmlKeepBlanksDefault) xmlKeepBlanksDefault__internal_alias __attribute((visibility("hidden")));
#define xmlKeepBlanksDefault xmlKeepBlanksDefault__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlLineNumbersDefault
extern __typeof (xmlLineNumbersDefault) xmlLineNumbersDefault __attribute((alias("xmlLineNumbersDefault__internal_alias")));
#else
#ifndef xmlLineNumbersDefault
extern __typeof (xmlLineNumbersDefault) xmlLineNumbersDefault__internal_alias __attribute((visibility("hidden")));
#define xmlLineNumbersDefault xmlLineNumbersDefault__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlLinkGetData
extern __typeof (xmlLinkGetData) xmlLinkGetData __attribute((alias("xmlLinkGetData__internal_alias")));
#else
#ifndef xmlLinkGetData
extern __typeof (xmlLinkGetData) xmlLinkGetData__internal_alias __attribute((visibility("hidden")));
#define xmlLinkGetData xmlLinkGetData__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListAppend
extern __typeof (xmlListAppend) xmlListAppend __attribute((alias("xmlListAppend__internal_alias")));
#else
#ifndef xmlListAppend
extern __typeof (xmlListAppend) xmlListAppend__internal_alias __attribute((visibility("hidden")));
#define xmlListAppend xmlListAppend__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListClear
extern __typeof (xmlListClear) xmlListClear __attribute((alias("xmlListClear__internal_alias")));
#else
#ifndef xmlListClear
extern __typeof (xmlListClear) xmlListClear__internal_alias __attribute((visibility("hidden")));
#define xmlListClear xmlListClear__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListCopy
extern __typeof (xmlListCopy) xmlListCopy __attribute((alias("xmlListCopy__internal_alias")));
#else
#ifndef xmlListCopy
extern __typeof (xmlListCopy) xmlListCopy__internal_alias __attribute((visibility("hidden")));
#define xmlListCopy xmlListCopy__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListCreate
extern __typeof (xmlListCreate) xmlListCreate __attribute((alias("xmlListCreate__internal_alias")));
#else
#ifndef xmlListCreate
extern __typeof (xmlListCreate) xmlListCreate__internal_alias __attribute((visibility("hidden")));
#define xmlListCreate xmlListCreate__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListDelete
extern __typeof (xmlListDelete) xmlListDelete __attribute((alias("xmlListDelete__internal_alias")));
#else
#ifndef xmlListDelete
extern __typeof (xmlListDelete) xmlListDelete__internal_alias __attribute((visibility("hidden")));
#define xmlListDelete xmlListDelete__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListDup
extern __typeof (xmlListDup) xmlListDup __attribute((alias("xmlListDup__internal_alias")));
#else
#ifndef xmlListDup
extern __typeof (xmlListDup) xmlListDup__internal_alias __attribute((visibility("hidden")));
#define xmlListDup xmlListDup__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListEmpty
extern __typeof (xmlListEmpty) xmlListEmpty __attribute((alias("xmlListEmpty__internal_alias")));
#else
#ifndef xmlListEmpty
extern __typeof (xmlListEmpty) xmlListEmpty__internal_alias __attribute((visibility("hidden")));
#define xmlListEmpty xmlListEmpty__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListEnd
extern __typeof (xmlListEnd) xmlListEnd __attribute((alias("xmlListEnd__internal_alias")));
#else
#ifndef xmlListEnd
extern __typeof (xmlListEnd) xmlListEnd__internal_alias __attribute((visibility("hidden")));
#define xmlListEnd xmlListEnd__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListFront
extern __typeof (xmlListFront) xmlListFront __attribute((alias("xmlListFront__internal_alias")));
#else
#ifndef xmlListFront
extern __typeof (xmlListFront) xmlListFront__internal_alias __attribute((visibility("hidden")));
#define xmlListFront xmlListFront__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListInsert
extern __typeof (xmlListInsert) xmlListInsert __attribute((alias("xmlListInsert__internal_alias")));
#else
#ifndef xmlListInsert
extern __typeof (xmlListInsert) xmlListInsert__internal_alias __attribute((visibility("hidden")));
#define xmlListInsert xmlListInsert__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListMerge
extern __typeof (xmlListMerge) xmlListMerge __attribute((alias("xmlListMerge__internal_alias")));
#else
#ifndef xmlListMerge
extern __typeof (xmlListMerge) xmlListMerge__internal_alias __attribute((visibility("hidden")));
#define xmlListMerge xmlListMerge__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListPopBack
extern __typeof (xmlListPopBack) xmlListPopBack __attribute((alias("xmlListPopBack__internal_alias")));
#else
#ifndef xmlListPopBack
extern __typeof (xmlListPopBack) xmlListPopBack__internal_alias __attribute((visibility("hidden")));
#define xmlListPopBack xmlListPopBack__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListPopFront
extern __typeof (xmlListPopFront) xmlListPopFront __attribute((alias("xmlListPopFront__internal_alias")));
#else
#ifndef xmlListPopFront
extern __typeof (xmlListPopFront) xmlListPopFront__internal_alias __attribute((visibility("hidden")));
#define xmlListPopFront xmlListPopFront__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListPushBack
extern __typeof (xmlListPushBack) xmlListPushBack __attribute((alias("xmlListPushBack__internal_alias")));
#else
#ifndef xmlListPushBack
extern __typeof (xmlListPushBack) xmlListPushBack__internal_alias __attribute((visibility("hidden")));
#define xmlListPushBack xmlListPushBack__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListPushFront
extern __typeof (xmlListPushFront) xmlListPushFront __attribute((alias("xmlListPushFront__internal_alias")));
#else
#ifndef xmlListPushFront
extern __typeof (xmlListPushFront) xmlListPushFront__internal_alias __attribute((visibility("hidden")));
#define xmlListPushFront xmlListPushFront__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListRemoveAll
extern __typeof (xmlListRemoveAll) xmlListRemoveAll __attribute((alias("xmlListRemoveAll__internal_alias")));
#else
#ifndef xmlListRemoveAll
extern __typeof (xmlListRemoveAll) xmlListRemoveAll__internal_alias __attribute((visibility("hidden")));
#define xmlListRemoveAll xmlListRemoveAll__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListRemoveFirst
extern __typeof (xmlListRemoveFirst) xmlListRemoveFirst __attribute((alias("xmlListRemoveFirst__internal_alias")));
#else
#ifndef xmlListRemoveFirst
extern __typeof (xmlListRemoveFirst) xmlListRemoveFirst__internal_alias __attribute((visibility("hidden")));
#define xmlListRemoveFirst xmlListRemoveFirst__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListRemoveLast
extern __typeof (xmlListRemoveLast) xmlListRemoveLast __attribute((alias("xmlListRemoveLast__internal_alias")));
#else
#ifndef xmlListRemoveLast
extern __typeof (xmlListRemoveLast) xmlListRemoveLast__internal_alias __attribute((visibility("hidden")));
#define xmlListRemoveLast xmlListRemoveLast__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListReverse
extern __typeof (xmlListReverse) xmlListReverse __attribute((alias("xmlListReverse__internal_alias")));
#else
#ifndef xmlListReverse
extern __typeof (xmlListReverse) xmlListReverse__internal_alias __attribute((visibility("hidden")));
#define xmlListReverse xmlListReverse__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListReverseSearch
extern __typeof (xmlListReverseSearch) xmlListReverseSearch __attribute((alias("xmlListReverseSearch__internal_alias")));
#else
#ifndef xmlListReverseSearch
extern __typeof (xmlListReverseSearch) xmlListReverseSearch__internal_alias __attribute((visibility("hidden")));
#define xmlListReverseSearch xmlListReverseSearch__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListReverseWalk
extern __typeof (xmlListReverseWalk) xmlListReverseWalk __attribute((alias("xmlListReverseWalk__internal_alias")));
#else
#ifndef xmlListReverseWalk
extern __typeof (xmlListReverseWalk) xmlListReverseWalk__internal_alias __attribute((visibility("hidden")));
#define xmlListReverseWalk xmlListReverseWalk__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListSearch
extern __typeof (xmlListSearch) xmlListSearch __attribute((alias("xmlListSearch__internal_alias")));
#else
#ifndef xmlListSearch
extern __typeof (xmlListSearch) xmlListSearch__internal_alias __attribute((visibility("hidden")));
#define xmlListSearch xmlListSearch__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListSize
extern __typeof (xmlListSize) xmlListSize __attribute((alias("xmlListSize__internal_alias")));
#else
#ifndef xmlListSize
extern __typeof (xmlListSize) xmlListSize__internal_alias __attribute((visibility("hidden")));
#define xmlListSize xmlListSize__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListSort
extern __typeof (xmlListSort) xmlListSort __attribute((alias("xmlListSort__internal_alias")));
#else
#ifndef xmlListSort
extern __typeof (xmlListSort) xmlListSort__internal_alias __attribute((visibility("hidden")));
#define xmlListSort xmlListSort__internal_alias
#endif
#endif

#ifdef bottom_list
#undef xmlListWalk
extern __typeof (xmlListWalk) xmlListWalk __attribute((alias("xmlListWalk__internal_alias")));
#else
#ifndef xmlListWalk
extern __typeof (xmlListWalk) xmlListWalk__internal_alias __attribute((visibility("hidden")));
#define xmlListWalk xmlListWalk__internal_alias
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlLoadACatalog
extern __typeof (xmlLoadACatalog) xmlLoadACatalog __attribute((alias("xmlLoadACatalog__internal_alias")));
#else
#ifndef xmlLoadACatalog
extern __typeof (xmlLoadACatalog) xmlLoadACatalog__internal_alias __attribute((visibility("hidden")));
#define xmlLoadACatalog xmlLoadACatalog__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlLoadCatalog
extern __typeof (xmlLoadCatalog) xmlLoadCatalog __attribute((alias("xmlLoadCatalog__internal_alias")));
#else
#ifndef xmlLoadCatalog
extern __typeof (xmlLoadCatalog) xmlLoadCatalog__internal_alias __attribute((visibility("hidden")));
#define xmlLoadCatalog xmlLoadCatalog__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlLoadCatalogs
extern __typeof (xmlLoadCatalogs) xmlLoadCatalogs __attribute((alias("xmlLoadCatalogs__internal_alias")));
#else
#ifndef xmlLoadCatalogs
extern __typeof (xmlLoadCatalogs) xmlLoadCatalogs__internal_alias __attribute((visibility("hidden")));
#define xmlLoadCatalogs xmlLoadCatalogs__internal_alias
#endif
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlLoadExternalEntity
extern __typeof (xmlLoadExternalEntity) xmlLoadExternalEntity __attribute((alias("xmlLoadExternalEntity__internal_alias")));
#else
#ifndef xmlLoadExternalEntity
extern __typeof (xmlLoadExternalEntity) xmlLoadExternalEntity__internal_alias __attribute((visibility("hidden")));
#define xmlLoadExternalEntity xmlLoadExternalEntity__internal_alias
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlLoadSGMLSuperCatalog
extern __typeof (xmlLoadSGMLSuperCatalog) xmlLoadSGMLSuperCatalog __attribute((alias("xmlLoadSGMLSuperCatalog__internal_alias")));
#else
#ifndef xmlLoadSGMLSuperCatalog
extern __typeof (xmlLoadSGMLSuperCatalog) xmlLoadSGMLSuperCatalog__internal_alias __attribute((visibility("hidden")));
#define xmlLoadSGMLSuperCatalog xmlLoadSGMLSuperCatalog__internal_alias
#endif
#endif
#endif

#ifdef bottom_threads
#undef xmlLockLibrary
extern __typeof (xmlLockLibrary) xmlLockLibrary __attribute((alias("xmlLockLibrary__internal_alias")));
#else
#ifndef xmlLockLibrary
extern __typeof (xmlLockLibrary) xmlLockLibrary__internal_alias __attribute((visibility("hidden")));
#define xmlLockLibrary xmlLockLibrary__internal_alias
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlLsCountNode
extern __typeof (xmlLsCountNode) xmlLsCountNode __attribute((alias("xmlLsCountNode__internal_alias")));
#else
#ifndef xmlLsCountNode
extern __typeof (xmlLsCountNode) xmlLsCountNode__internal_alias __attribute((visibility("hidden")));
#define xmlLsCountNode xmlLsCountNode__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_debugXML
#undef xmlLsOneNode
extern __typeof (xmlLsOneNode) xmlLsOneNode __attribute((alias("xmlLsOneNode__internal_alias")));
#else
#ifndef xmlLsOneNode
extern __typeof (xmlLsOneNode) xmlLsOneNode__internal_alias __attribute((visibility("hidden")));
#define xmlLsOneNode xmlLsOneNode__internal_alias
#endif
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMallocAtomicLoc
extern __typeof (xmlMallocAtomicLoc) xmlMallocAtomicLoc __attribute((alias("xmlMallocAtomicLoc__internal_alias")));
#else
#ifndef xmlMallocAtomicLoc
extern __typeof (xmlMallocAtomicLoc) xmlMallocAtomicLoc__internal_alias __attribute((visibility("hidden")));
#define xmlMallocAtomicLoc xmlMallocAtomicLoc__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMallocLoc
extern __typeof (xmlMallocLoc) xmlMallocLoc __attribute((alias("xmlMallocLoc__internal_alias")));
#else
#ifndef xmlMallocLoc
extern __typeof (xmlMallocLoc) xmlMallocLoc__internal_alias __attribute((visibility("hidden")));
#define xmlMallocLoc xmlMallocLoc__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMemBlocks
extern __typeof (xmlMemBlocks) xmlMemBlocks __attribute((alias("xmlMemBlocks__internal_alias")));
#else
#ifndef xmlMemBlocks
extern __typeof (xmlMemBlocks) xmlMemBlocks__internal_alias __attribute((visibility("hidden")));
#define xmlMemBlocks xmlMemBlocks__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMemDisplay
extern __typeof (xmlMemDisplay) xmlMemDisplay __attribute((alias("xmlMemDisplay__internal_alias")));
#else
#ifndef xmlMemDisplay
extern __typeof (xmlMemDisplay) xmlMemDisplay__internal_alias __attribute((visibility("hidden")));
#define xmlMemDisplay xmlMemDisplay__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMemFree
extern __typeof (xmlMemFree) xmlMemFree __attribute((alias("xmlMemFree__internal_alias")));
#else
#ifndef xmlMemFree
extern __typeof (xmlMemFree) xmlMemFree__internal_alias __attribute((visibility("hidden")));
#define xmlMemFree xmlMemFree__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMemGet
extern __typeof (xmlMemGet) xmlMemGet __attribute((alias("xmlMemGet__internal_alias")));
#else
#ifndef xmlMemGet
extern __typeof (xmlMemGet) xmlMemGet__internal_alias __attribute((visibility("hidden")));
#define xmlMemGet xmlMemGet__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMemMalloc
extern __typeof (xmlMemMalloc) xmlMemMalloc __attribute((alias("xmlMemMalloc__internal_alias")));
#else
#ifndef xmlMemMalloc
extern __typeof (xmlMemMalloc) xmlMemMalloc__internal_alias __attribute((visibility("hidden")));
#define xmlMemMalloc xmlMemMalloc__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMemRealloc
extern __typeof (xmlMemRealloc) xmlMemRealloc __attribute((alias("xmlMemRealloc__internal_alias")));
#else
#ifndef xmlMemRealloc
extern __typeof (xmlMemRealloc) xmlMemRealloc__internal_alias __attribute((visibility("hidden")));
#define xmlMemRealloc xmlMemRealloc__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMemSetup
extern __typeof (xmlMemSetup) xmlMemSetup __attribute((alias("xmlMemSetup__internal_alias")));
#else
#ifndef xmlMemSetup
extern __typeof (xmlMemSetup) xmlMemSetup__internal_alias __attribute((visibility("hidden")));
#define xmlMemSetup xmlMemSetup__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMemShow
extern __typeof (xmlMemShow) xmlMemShow __attribute((alias("xmlMemShow__internal_alias")));
#else
#ifndef xmlMemShow
extern __typeof (xmlMemShow) xmlMemShow__internal_alias __attribute((visibility("hidden")));
#define xmlMemShow xmlMemShow__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMemStrdupLoc
extern __typeof (xmlMemStrdupLoc) xmlMemStrdupLoc __attribute((alias("xmlMemStrdupLoc__internal_alias")));
#else
#ifndef xmlMemStrdupLoc
extern __typeof (xmlMemStrdupLoc) xmlMemStrdupLoc__internal_alias __attribute((visibility("hidden")));
#define xmlMemStrdupLoc xmlMemStrdupLoc__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMemUsed
extern __typeof (xmlMemUsed) xmlMemUsed __attribute((alias("xmlMemUsed__internal_alias")));
#else
#ifndef xmlMemUsed
extern __typeof (xmlMemUsed) xmlMemUsed__internal_alias __attribute((visibility("hidden")));
#define xmlMemUsed xmlMemUsed__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMemoryDump
extern __typeof (xmlMemoryDump) xmlMemoryDump __attribute((alias("xmlMemoryDump__internal_alias")));
#else
#ifndef xmlMemoryDump
extern __typeof (xmlMemoryDump) xmlMemoryDump__internal_alias __attribute((visibility("hidden")));
#define xmlMemoryDump xmlMemoryDump__internal_alias
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlMemoryStrdup
extern __typeof (xmlMemoryStrdup) xmlMemoryStrdup __attribute((alias("xmlMemoryStrdup__internal_alias")));
#else
#ifndef xmlMemoryStrdup
extern __typeof (xmlMemoryStrdup) xmlMemoryStrdup__internal_alias __attribute((visibility("hidden")));
#define xmlMemoryStrdup xmlMemoryStrdup__internal_alias
#endif
#endif

#if defined(LIBXML_MODULES_ENABLED)
#ifdef bottom_xmlmodule
#undef xmlModuleClose
extern __typeof (xmlModuleClose) xmlModuleClose __attribute((alias("xmlModuleClose__internal_alias")));
#else
#ifndef xmlModuleClose
extern __typeof (xmlModuleClose) xmlModuleClose__internal_alias __attribute((visibility("hidden")));
#define xmlModuleClose xmlModuleClose__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_MODULES_ENABLED)
#ifdef bottom_xmlmodule
#undef xmlModuleFree
extern __typeof (xmlModuleFree) xmlModuleFree __attribute((alias("xmlModuleFree__internal_alias")));
#else
#ifndef xmlModuleFree
extern __typeof (xmlModuleFree) xmlModuleFree__internal_alias __attribute((visibility("hidden")));
#define xmlModuleFree xmlModuleFree__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_MODULES_ENABLED)
#ifdef bottom_xmlmodule
#undef xmlModuleOpen
extern __typeof (xmlModuleOpen) xmlModuleOpen __attribute((alias("xmlModuleOpen__internal_alias")));
#else
#ifndef xmlModuleOpen
extern __typeof (xmlModuleOpen) xmlModuleOpen__internal_alias __attribute((visibility("hidden")));
#define xmlModuleOpen xmlModuleOpen__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_MODULES_ENABLED)
#ifdef bottom_xmlmodule
#undef xmlModuleSymbol
extern __typeof (xmlModuleSymbol) xmlModuleSymbol __attribute((alias("xmlModuleSymbol__internal_alias")));
#else
#ifndef xmlModuleSymbol
extern __typeof (xmlModuleSymbol) xmlModuleSymbol__internal_alias __attribute((visibility("hidden")));
#define xmlModuleSymbol xmlModuleSymbol__internal_alias
#endif
#endif
#endif

#ifdef bottom_threads
#undef xmlMutexLock
extern __typeof (xmlMutexLock) xmlMutexLock __attribute((alias("xmlMutexLock__internal_alias")));
#else
#ifndef xmlMutexLock
extern __typeof (xmlMutexLock) xmlMutexLock__internal_alias __attribute((visibility("hidden")));
#define xmlMutexLock xmlMutexLock__internal_alias
#endif
#endif

#ifdef bottom_threads
#undef xmlMutexUnlock
extern __typeof (xmlMutexUnlock) xmlMutexUnlock __attribute((alias("xmlMutexUnlock__internal_alias")));
#else
#ifndef xmlMutexUnlock
extern __typeof (xmlMutexUnlock) xmlMutexUnlock__internal_alias __attribute((visibility("hidden")));
#define xmlMutexUnlock xmlMutexUnlock__internal_alias
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlNamespaceParseNCName
extern __typeof (xmlNamespaceParseNCName) xmlNamespaceParseNCName __attribute((alias("xmlNamespaceParseNCName__internal_alias")));
#else
#ifndef xmlNamespaceParseNCName
extern __typeof (xmlNamespaceParseNCName) xmlNamespaceParseNCName__internal_alias __attribute((visibility("hidden")));
#define xmlNamespaceParseNCName xmlNamespaceParseNCName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlNamespaceParseNSDef
extern __typeof (xmlNamespaceParseNSDef) xmlNamespaceParseNSDef __attribute((alias("xmlNamespaceParseNSDef__internal_alias")));
#else
#ifndef xmlNamespaceParseNSDef
extern __typeof (xmlNamespaceParseNSDef) xmlNamespaceParseNSDef__internal_alias __attribute((visibility("hidden")));
#define xmlNamespaceParseNSDef xmlNamespaceParseNSDef__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlNamespaceParseQName
extern __typeof (xmlNamespaceParseQName) xmlNamespaceParseQName __attribute((alias("xmlNamespaceParseQName__internal_alias")));
#else
#ifndef xmlNamespaceParseQName
extern __typeof (xmlNamespaceParseQName) xmlNamespaceParseQName__internal_alias __attribute((visibility("hidden")));
#define xmlNamespaceParseQName xmlNamespaceParseQName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPCheckResponse
extern __typeof (xmlNanoFTPCheckResponse) xmlNanoFTPCheckResponse __attribute((alias("xmlNanoFTPCheckResponse__internal_alias")));
#else
#ifndef xmlNanoFTPCheckResponse
extern __typeof (xmlNanoFTPCheckResponse) xmlNanoFTPCheckResponse__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPCheckResponse xmlNanoFTPCheckResponse__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPCleanup
extern __typeof (xmlNanoFTPCleanup) xmlNanoFTPCleanup __attribute((alias("xmlNanoFTPCleanup__internal_alias")));
#else
#ifndef xmlNanoFTPCleanup
extern __typeof (xmlNanoFTPCleanup) xmlNanoFTPCleanup__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPCleanup xmlNanoFTPCleanup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPClose
extern __typeof (xmlNanoFTPClose) xmlNanoFTPClose __attribute((alias("xmlNanoFTPClose__internal_alias")));
#else
#ifndef xmlNanoFTPClose
extern __typeof (xmlNanoFTPClose) xmlNanoFTPClose__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPClose xmlNanoFTPClose__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPCloseConnection
extern __typeof (xmlNanoFTPCloseConnection) xmlNanoFTPCloseConnection __attribute((alias("xmlNanoFTPCloseConnection__internal_alias")));
#else
#ifndef xmlNanoFTPCloseConnection
extern __typeof (xmlNanoFTPCloseConnection) xmlNanoFTPCloseConnection__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPCloseConnection xmlNanoFTPCloseConnection__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPConnect
extern __typeof (xmlNanoFTPConnect) xmlNanoFTPConnect __attribute((alias("xmlNanoFTPConnect__internal_alias")));
#else
#ifndef xmlNanoFTPConnect
extern __typeof (xmlNanoFTPConnect) xmlNanoFTPConnect__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPConnect xmlNanoFTPConnect__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPConnectTo
extern __typeof (xmlNanoFTPConnectTo) xmlNanoFTPConnectTo __attribute((alias("xmlNanoFTPConnectTo__internal_alias")));
#else
#ifndef xmlNanoFTPConnectTo
extern __typeof (xmlNanoFTPConnectTo) xmlNanoFTPConnectTo__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPConnectTo xmlNanoFTPConnectTo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPCwd
extern __typeof (xmlNanoFTPCwd) xmlNanoFTPCwd __attribute((alias("xmlNanoFTPCwd__internal_alias")));
#else
#ifndef xmlNanoFTPCwd
extern __typeof (xmlNanoFTPCwd) xmlNanoFTPCwd__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPCwd xmlNanoFTPCwd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPDele
extern __typeof (xmlNanoFTPDele) xmlNanoFTPDele __attribute((alias("xmlNanoFTPDele__internal_alias")));
#else
#ifndef xmlNanoFTPDele
extern __typeof (xmlNanoFTPDele) xmlNanoFTPDele__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPDele xmlNanoFTPDele__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPFreeCtxt
extern __typeof (xmlNanoFTPFreeCtxt) xmlNanoFTPFreeCtxt __attribute((alias("xmlNanoFTPFreeCtxt__internal_alias")));
#else
#ifndef xmlNanoFTPFreeCtxt
extern __typeof (xmlNanoFTPFreeCtxt) xmlNanoFTPFreeCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPFreeCtxt xmlNanoFTPFreeCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPGet
extern __typeof (xmlNanoFTPGet) xmlNanoFTPGet __attribute((alias("xmlNanoFTPGet__internal_alias")));
#else
#ifndef xmlNanoFTPGet
extern __typeof (xmlNanoFTPGet) xmlNanoFTPGet__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPGet xmlNanoFTPGet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPGetConnection
extern __typeof (xmlNanoFTPGetConnection) xmlNanoFTPGetConnection __attribute((alias("xmlNanoFTPGetConnection__internal_alias")));
#else
#ifndef xmlNanoFTPGetConnection
extern __typeof (xmlNanoFTPGetConnection) xmlNanoFTPGetConnection__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPGetConnection xmlNanoFTPGetConnection__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPGetResponse
extern __typeof (xmlNanoFTPGetResponse) xmlNanoFTPGetResponse __attribute((alias("xmlNanoFTPGetResponse__internal_alias")));
#else
#ifndef xmlNanoFTPGetResponse
extern __typeof (xmlNanoFTPGetResponse) xmlNanoFTPGetResponse__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPGetResponse xmlNanoFTPGetResponse__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPGetSocket
extern __typeof (xmlNanoFTPGetSocket) xmlNanoFTPGetSocket __attribute((alias("xmlNanoFTPGetSocket__internal_alias")));
#else
#ifndef xmlNanoFTPGetSocket
extern __typeof (xmlNanoFTPGetSocket) xmlNanoFTPGetSocket__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPGetSocket xmlNanoFTPGetSocket__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPInit
extern __typeof (xmlNanoFTPInit) xmlNanoFTPInit __attribute((alias("xmlNanoFTPInit__internal_alias")));
#else
#ifndef xmlNanoFTPInit
extern __typeof (xmlNanoFTPInit) xmlNanoFTPInit__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPInit xmlNanoFTPInit__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPList
extern __typeof (xmlNanoFTPList) xmlNanoFTPList __attribute((alias("xmlNanoFTPList__internal_alias")));
#else
#ifndef xmlNanoFTPList
extern __typeof (xmlNanoFTPList) xmlNanoFTPList__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPList xmlNanoFTPList__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPNewCtxt
extern __typeof (xmlNanoFTPNewCtxt) xmlNanoFTPNewCtxt __attribute((alias("xmlNanoFTPNewCtxt__internal_alias")));
#else
#ifndef xmlNanoFTPNewCtxt
extern __typeof (xmlNanoFTPNewCtxt) xmlNanoFTPNewCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPNewCtxt xmlNanoFTPNewCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPOpen
extern __typeof (xmlNanoFTPOpen) xmlNanoFTPOpen __attribute((alias("xmlNanoFTPOpen__internal_alias")));
#else
#ifndef xmlNanoFTPOpen
extern __typeof (xmlNanoFTPOpen) xmlNanoFTPOpen__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPOpen xmlNanoFTPOpen__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPProxy
extern __typeof (xmlNanoFTPProxy) xmlNanoFTPProxy __attribute((alias("xmlNanoFTPProxy__internal_alias")));
#else
#ifndef xmlNanoFTPProxy
extern __typeof (xmlNanoFTPProxy) xmlNanoFTPProxy__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPProxy xmlNanoFTPProxy__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPQuit
extern __typeof (xmlNanoFTPQuit) xmlNanoFTPQuit __attribute((alias("xmlNanoFTPQuit__internal_alias")));
#else
#ifndef xmlNanoFTPQuit
extern __typeof (xmlNanoFTPQuit) xmlNanoFTPQuit__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPQuit xmlNanoFTPQuit__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPRead
extern __typeof (xmlNanoFTPRead) xmlNanoFTPRead __attribute((alias("xmlNanoFTPRead__internal_alias")));
#else
#ifndef xmlNanoFTPRead
extern __typeof (xmlNanoFTPRead) xmlNanoFTPRead__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPRead xmlNanoFTPRead__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPScanProxy
extern __typeof (xmlNanoFTPScanProxy) xmlNanoFTPScanProxy __attribute((alias("xmlNanoFTPScanProxy__internal_alias")));
#else
#ifndef xmlNanoFTPScanProxy
extern __typeof (xmlNanoFTPScanProxy) xmlNanoFTPScanProxy__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPScanProxy xmlNanoFTPScanProxy__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_FTP_ENABLED)
#ifdef bottom_nanoftp
#undef xmlNanoFTPUpdateURL
extern __typeof (xmlNanoFTPUpdateURL) xmlNanoFTPUpdateURL __attribute((alias("xmlNanoFTPUpdateURL__internal_alias")));
#else
#ifndef xmlNanoFTPUpdateURL
extern __typeof (xmlNanoFTPUpdateURL) xmlNanoFTPUpdateURL__internal_alias __attribute((visibility("hidden")));
#define xmlNanoFTPUpdateURL xmlNanoFTPUpdateURL__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPAuthHeader
extern __typeof (xmlNanoHTTPAuthHeader) xmlNanoHTTPAuthHeader __attribute((alias("xmlNanoHTTPAuthHeader__internal_alias")));
#else
#ifndef xmlNanoHTTPAuthHeader
extern __typeof (xmlNanoHTTPAuthHeader) xmlNanoHTTPAuthHeader__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPAuthHeader xmlNanoHTTPAuthHeader__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPCleanup
extern __typeof (xmlNanoHTTPCleanup) xmlNanoHTTPCleanup __attribute((alias("xmlNanoHTTPCleanup__internal_alias")));
#else
#ifndef xmlNanoHTTPCleanup
extern __typeof (xmlNanoHTTPCleanup) xmlNanoHTTPCleanup__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPCleanup xmlNanoHTTPCleanup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPClose
extern __typeof (xmlNanoHTTPClose) xmlNanoHTTPClose __attribute((alias("xmlNanoHTTPClose__internal_alias")));
#else
#ifndef xmlNanoHTTPClose
extern __typeof (xmlNanoHTTPClose) xmlNanoHTTPClose__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPClose xmlNanoHTTPClose__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPContentLength
extern __typeof (xmlNanoHTTPContentLength) xmlNanoHTTPContentLength __attribute((alias("xmlNanoHTTPContentLength__internal_alias")));
#else
#ifndef xmlNanoHTTPContentLength
extern __typeof (xmlNanoHTTPContentLength) xmlNanoHTTPContentLength__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPContentLength xmlNanoHTTPContentLength__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPEncoding
extern __typeof (xmlNanoHTTPEncoding) xmlNanoHTTPEncoding __attribute((alias("xmlNanoHTTPEncoding__internal_alias")));
#else
#ifndef xmlNanoHTTPEncoding
extern __typeof (xmlNanoHTTPEncoding) xmlNanoHTTPEncoding__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPEncoding xmlNanoHTTPEncoding__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPFetch
extern __typeof (xmlNanoHTTPFetch) xmlNanoHTTPFetch __attribute((alias("xmlNanoHTTPFetch__internal_alias")));
#else
#ifndef xmlNanoHTTPFetch
extern __typeof (xmlNanoHTTPFetch) xmlNanoHTTPFetch__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPFetch xmlNanoHTTPFetch__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPInit
extern __typeof (xmlNanoHTTPInit) xmlNanoHTTPInit __attribute((alias("xmlNanoHTTPInit__internal_alias")));
#else
#ifndef xmlNanoHTTPInit
extern __typeof (xmlNanoHTTPInit) xmlNanoHTTPInit__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPInit xmlNanoHTTPInit__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPMethod
extern __typeof (xmlNanoHTTPMethod) xmlNanoHTTPMethod __attribute((alias("xmlNanoHTTPMethod__internal_alias")));
#else
#ifndef xmlNanoHTTPMethod
extern __typeof (xmlNanoHTTPMethod) xmlNanoHTTPMethod__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPMethod xmlNanoHTTPMethod__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPMethodRedir
extern __typeof (xmlNanoHTTPMethodRedir) xmlNanoHTTPMethodRedir __attribute((alias("xmlNanoHTTPMethodRedir__internal_alias")));
#else
#ifndef xmlNanoHTTPMethodRedir
extern __typeof (xmlNanoHTTPMethodRedir) xmlNanoHTTPMethodRedir__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPMethodRedir xmlNanoHTTPMethodRedir__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPMimeType
extern __typeof (xmlNanoHTTPMimeType) xmlNanoHTTPMimeType __attribute((alias("xmlNanoHTTPMimeType__internal_alias")));
#else
#ifndef xmlNanoHTTPMimeType
extern __typeof (xmlNanoHTTPMimeType) xmlNanoHTTPMimeType__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPMimeType xmlNanoHTTPMimeType__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPOpen
extern __typeof (xmlNanoHTTPOpen) xmlNanoHTTPOpen __attribute((alias("xmlNanoHTTPOpen__internal_alias")));
#else
#ifndef xmlNanoHTTPOpen
extern __typeof (xmlNanoHTTPOpen) xmlNanoHTTPOpen__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPOpen xmlNanoHTTPOpen__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPOpenRedir
extern __typeof (xmlNanoHTTPOpenRedir) xmlNanoHTTPOpenRedir __attribute((alias("xmlNanoHTTPOpenRedir__internal_alias")));
#else
#ifndef xmlNanoHTTPOpenRedir
extern __typeof (xmlNanoHTTPOpenRedir) xmlNanoHTTPOpenRedir__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPOpenRedir xmlNanoHTTPOpenRedir__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPRead
extern __typeof (xmlNanoHTTPRead) xmlNanoHTTPRead __attribute((alias("xmlNanoHTTPRead__internal_alias")));
#else
#ifndef xmlNanoHTTPRead
extern __typeof (xmlNanoHTTPRead) xmlNanoHTTPRead__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPRead xmlNanoHTTPRead__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPRedir
extern __typeof (xmlNanoHTTPRedir) xmlNanoHTTPRedir __attribute((alias("xmlNanoHTTPRedir__internal_alias")));
#else
#ifndef xmlNanoHTTPRedir
extern __typeof (xmlNanoHTTPRedir) xmlNanoHTTPRedir__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPRedir xmlNanoHTTPRedir__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPReturnCode
extern __typeof (xmlNanoHTTPReturnCode) xmlNanoHTTPReturnCode __attribute((alias("xmlNanoHTTPReturnCode__internal_alias")));
#else
#ifndef xmlNanoHTTPReturnCode
extern __typeof (xmlNanoHTTPReturnCode) xmlNanoHTTPReturnCode__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPReturnCode xmlNanoHTTPReturnCode__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPSave
extern __typeof (xmlNanoHTTPSave) xmlNanoHTTPSave __attribute((alias("xmlNanoHTTPSave__internal_alias")));
#else
#ifndef xmlNanoHTTPSave
extern __typeof (xmlNanoHTTPSave) xmlNanoHTTPSave__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPSave xmlNanoHTTPSave__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_nanohttp
#undef xmlNanoHTTPScanProxy
extern __typeof (xmlNanoHTTPScanProxy) xmlNanoHTTPScanProxy __attribute((alias("xmlNanoHTTPScanProxy__internal_alias")));
#else
#ifndef xmlNanoHTTPScanProxy
extern __typeof (xmlNanoHTTPScanProxy) xmlNanoHTTPScanProxy__internal_alias __attribute((visibility("hidden")));
#define xmlNanoHTTPScanProxy xmlNanoHTTPScanProxy__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED) && defined(LIBXML_AUTOMATA_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlNewAutomata
extern __typeof (xmlNewAutomata) xmlNewAutomata __attribute((alias("xmlNewAutomata__internal_alias")));
#else
#ifndef xmlNewAutomata
extern __typeof (xmlNewAutomata) xmlNewAutomata__internal_alias __attribute((visibility("hidden")));
#define xmlNewAutomata xmlNewAutomata__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlNewCDataBlock
extern __typeof (xmlNewCDataBlock) xmlNewCDataBlock __attribute((alias("xmlNewCDataBlock__internal_alias")));
#else
#ifndef xmlNewCDataBlock
extern __typeof (xmlNewCDataBlock) xmlNewCDataBlock__internal_alias __attribute((visibility("hidden")));
#define xmlNewCDataBlock xmlNewCDataBlock__internal_alias
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlNewCatalog
extern __typeof (xmlNewCatalog) xmlNewCatalog __attribute((alias("xmlNewCatalog__internal_alias")));
#else
#ifndef xmlNewCatalog
extern __typeof (xmlNewCatalog) xmlNewCatalog__internal_alias __attribute((visibility("hidden")));
#define xmlNewCatalog xmlNewCatalog__internal_alias
#endif
#endif
#endif

#ifdef bottom_encoding
#undef xmlNewCharEncodingHandler
extern __typeof (xmlNewCharEncodingHandler) xmlNewCharEncodingHandler __attribute((alias("xmlNewCharEncodingHandler__internal_alias")));
#else
#ifndef xmlNewCharEncodingHandler
extern __typeof (xmlNewCharEncodingHandler) xmlNewCharEncodingHandler__internal_alias __attribute((visibility("hidden")));
#define xmlNewCharEncodingHandler xmlNewCharEncodingHandler__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewCharRef
extern __typeof (xmlNewCharRef) xmlNewCharRef __attribute((alias("xmlNewCharRef__internal_alias")));
#else
#ifndef xmlNewCharRef
extern __typeof (xmlNewCharRef) xmlNewCharRef__internal_alias __attribute((visibility("hidden")));
#define xmlNewCharRef xmlNewCharRef__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_tree
#undef xmlNewChild
extern __typeof (xmlNewChild) xmlNewChild __attribute((alias("xmlNewChild__internal_alias")));
#else
#ifndef xmlNewChild
extern __typeof (xmlNewChild) xmlNewChild__internal_alias __attribute((visibility("hidden")));
#define xmlNewChild xmlNewChild__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlNewComment
extern __typeof (xmlNewComment) xmlNewComment __attribute((alias("xmlNewComment__internal_alias")));
#else
#ifndef xmlNewComment
extern __typeof (xmlNewComment) xmlNewComment__internal_alias __attribute((visibility("hidden")));
#define xmlNewComment xmlNewComment__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewDoc
extern __typeof (xmlNewDoc) xmlNewDoc __attribute((alias("xmlNewDoc__internal_alias")));
#else
#ifndef xmlNewDoc
extern __typeof (xmlNewDoc) xmlNewDoc__internal_alias __attribute((visibility("hidden")));
#define xmlNewDoc xmlNewDoc__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewDocComment
extern __typeof (xmlNewDocComment) xmlNewDocComment __attribute((alias("xmlNewDocComment__internal_alias")));
#else
#ifndef xmlNewDocComment
extern __typeof (xmlNewDocComment) xmlNewDocComment__internal_alias __attribute((visibility("hidden")));
#define xmlNewDocComment xmlNewDocComment__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlNewDocElementContent
extern __typeof (xmlNewDocElementContent) xmlNewDocElementContent __attribute((alias("xmlNewDocElementContent__internal_alias")));
#else
#ifndef xmlNewDocElementContent
extern __typeof (xmlNewDocElementContent) xmlNewDocElementContent__internal_alias __attribute((visibility("hidden")));
#define xmlNewDocElementContent xmlNewDocElementContent__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_tree
#undef xmlNewDocFragment
extern __typeof (xmlNewDocFragment) xmlNewDocFragment __attribute((alias("xmlNewDocFragment__internal_alias")));
#else
#ifndef xmlNewDocFragment
extern __typeof (xmlNewDocFragment) xmlNewDocFragment__internal_alias __attribute((visibility("hidden")));
#define xmlNewDocFragment xmlNewDocFragment__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlNewDocNode
extern __typeof (xmlNewDocNode) xmlNewDocNode __attribute((alias("xmlNewDocNode__internal_alias")));
#else
#ifndef xmlNewDocNode
extern __typeof (xmlNewDocNode) xmlNewDocNode__internal_alias __attribute((visibility("hidden")));
#define xmlNewDocNode xmlNewDocNode__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewDocNodeEatName
extern __typeof (xmlNewDocNodeEatName) xmlNewDocNodeEatName __attribute((alias("xmlNewDocNodeEatName__internal_alias")));
#else
#ifndef xmlNewDocNodeEatName
extern __typeof (xmlNewDocNodeEatName) xmlNewDocNodeEatName__internal_alias __attribute((visibility("hidden")));
#define xmlNewDocNodeEatName xmlNewDocNodeEatName__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewDocPI
extern __typeof (xmlNewDocPI) xmlNewDocPI __attribute((alias("xmlNewDocPI__internal_alias")));
#else
#ifndef xmlNewDocPI
extern __typeof (xmlNewDocPI) xmlNewDocPI__internal_alias __attribute((visibility("hidden")));
#define xmlNewDocPI xmlNewDocPI__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewDocProp
extern __typeof (xmlNewDocProp) xmlNewDocProp __attribute((alias("xmlNewDocProp__internal_alias")));
#else
#ifndef xmlNewDocProp
extern __typeof (xmlNewDocProp) xmlNewDocProp__internal_alias __attribute((visibility("hidden")));
#define xmlNewDocProp xmlNewDocProp__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_tree
#undef xmlNewDocRawNode
extern __typeof (xmlNewDocRawNode) xmlNewDocRawNode __attribute((alias("xmlNewDocRawNode__internal_alias")));
#else
#ifndef xmlNewDocRawNode
extern __typeof (xmlNewDocRawNode) xmlNewDocRawNode__internal_alias __attribute((visibility("hidden")));
#define xmlNewDocRawNode xmlNewDocRawNode__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlNewDocText
extern __typeof (xmlNewDocText) xmlNewDocText __attribute((alias("xmlNewDocText__internal_alias")));
#else
#ifndef xmlNewDocText
extern __typeof (xmlNewDocText) xmlNewDocText__internal_alias __attribute((visibility("hidden")));
#define xmlNewDocText xmlNewDocText__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewDocTextLen
extern __typeof (xmlNewDocTextLen) xmlNewDocTextLen __attribute((alias("xmlNewDocTextLen__internal_alias")));
#else
#ifndef xmlNewDocTextLen
extern __typeof (xmlNewDocTextLen) xmlNewDocTextLen__internal_alias __attribute((visibility("hidden")));
#define xmlNewDocTextLen xmlNewDocTextLen__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewDtd
extern __typeof (xmlNewDtd) xmlNewDtd __attribute((alias("xmlNewDtd__internal_alias")));
#else
#ifndef xmlNewDtd
extern __typeof (xmlNewDtd) xmlNewDtd__internal_alias __attribute((visibility("hidden")));
#define xmlNewDtd xmlNewDtd__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlNewElementContent
extern __typeof (xmlNewElementContent) xmlNewElementContent __attribute((alias("xmlNewElementContent__internal_alias")));
#else
#ifndef xmlNewElementContent
extern __typeof (xmlNewElementContent) xmlNewElementContent__internal_alias __attribute((visibility("hidden")));
#define xmlNewElementContent xmlNewElementContent__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlNewEntityInputStream
extern __typeof (xmlNewEntityInputStream) xmlNewEntityInputStream __attribute((alias("xmlNewEntityInputStream__internal_alias")));
#else
#ifndef xmlNewEntityInputStream
extern __typeof (xmlNewEntityInputStream) xmlNewEntityInputStream__internal_alias __attribute((visibility("hidden")));
#define xmlNewEntityInputStream xmlNewEntityInputStream__internal_alias
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlNewGlobalNs
extern __typeof (xmlNewGlobalNs) xmlNewGlobalNs __attribute((alias("xmlNewGlobalNs__internal_alias")));
#else
#ifndef xmlNewGlobalNs
extern __typeof (xmlNewGlobalNs) xmlNewGlobalNs__internal_alias __attribute((visibility("hidden")));
#define xmlNewGlobalNs xmlNewGlobalNs__internal_alias
#endif
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlNewIOInputStream
extern __typeof (xmlNewIOInputStream) xmlNewIOInputStream __attribute((alias("xmlNewIOInputStream__internal_alias")));
#else
#ifndef xmlNewIOInputStream
extern __typeof (xmlNewIOInputStream) xmlNewIOInputStream__internal_alias __attribute((visibility("hidden")));
#define xmlNewIOInputStream xmlNewIOInputStream__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlNewInputFromFile
extern __typeof (xmlNewInputFromFile) xmlNewInputFromFile __attribute((alias("xmlNewInputFromFile__internal_alias")));
#else
#ifndef xmlNewInputFromFile
extern __typeof (xmlNewInputFromFile) xmlNewInputFromFile__internal_alias __attribute((visibility("hidden")));
#define xmlNewInputFromFile xmlNewInputFromFile__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlNewInputStream
extern __typeof (xmlNewInputStream) xmlNewInputStream __attribute((alias("xmlNewInputStream__internal_alias")));
#else
#ifndef xmlNewInputStream
extern __typeof (xmlNewInputStream) xmlNewInputStream__internal_alias __attribute((visibility("hidden")));
#define xmlNewInputStream xmlNewInputStream__internal_alias
#endif
#endif

#ifdef bottom_threads
#undef xmlNewMutex
extern __typeof (xmlNewMutex) xmlNewMutex __attribute((alias("xmlNewMutex__internal_alias")));
#else
#ifndef xmlNewMutex
extern __typeof (xmlNewMutex) xmlNewMutex__internal_alias __attribute((visibility("hidden")));
#define xmlNewMutex xmlNewMutex__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewNode
extern __typeof (xmlNewNode) xmlNewNode __attribute((alias("xmlNewNode__internal_alias")));
#else
#ifndef xmlNewNode
extern __typeof (xmlNewNode) xmlNewNode__internal_alias __attribute((visibility("hidden")));
#define xmlNewNode xmlNewNode__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewNodeEatName
extern __typeof (xmlNewNodeEatName) xmlNewNodeEatName __attribute((alias("xmlNewNodeEatName__internal_alias")));
#else
#ifndef xmlNewNodeEatName
extern __typeof (xmlNewNodeEatName) xmlNewNodeEatName__internal_alias __attribute((visibility("hidden")));
#define xmlNewNodeEatName xmlNewNodeEatName__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewNs
extern __typeof (xmlNewNs) xmlNewNs __attribute((alias("xmlNewNs__internal_alias")));
#else
#ifndef xmlNewNs
extern __typeof (xmlNewNs) xmlNewNs__internal_alias __attribute((visibility("hidden")));
#define xmlNewNs xmlNewNs__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewNsProp
extern __typeof (xmlNewNsProp) xmlNewNsProp __attribute((alias("xmlNewNsProp__internal_alias")));
#else
#ifndef xmlNewNsProp
extern __typeof (xmlNewNsProp) xmlNewNsProp__internal_alias __attribute((visibility("hidden")));
#define xmlNewNsProp xmlNewNsProp__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewNsPropEatName
extern __typeof (xmlNewNsPropEatName) xmlNewNsPropEatName __attribute((alias("xmlNewNsPropEatName__internal_alias")));
#else
#ifndef xmlNewNsPropEatName
extern __typeof (xmlNewNsPropEatName) xmlNewNsPropEatName__internal_alias __attribute((visibility("hidden")));
#define xmlNewNsPropEatName xmlNewNsPropEatName__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewPI
extern __typeof (xmlNewPI) xmlNewPI __attribute((alias("xmlNewPI__internal_alias")));
#else
#ifndef xmlNewPI
extern __typeof (xmlNewPI) xmlNewPI__internal_alias __attribute((visibility("hidden")));
#define xmlNewPI xmlNewPI__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlNewParserCtxt
extern __typeof (xmlNewParserCtxt) xmlNewParserCtxt __attribute((alias("xmlNewParserCtxt__internal_alias")));
#else
#ifndef xmlNewParserCtxt
extern __typeof (xmlNewParserCtxt) xmlNewParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlNewParserCtxt xmlNewParserCtxt__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_HTML_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_tree
#undef xmlNewProp
extern __typeof (xmlNewProp) xmlNewProp __attribute((alias("xmlNewProp__internal_alias")));
#else
#ifndef xmlNewProp
extern __typeof (xmlNewProp) xmlNewProp__internal_alias __attribute((visibility("hidden")));
#define xmlNewProp xmlNewProp__internal_alias
#endif
#endif
#endif

#ifdef bottom_threads
#undef xmlNewRMutex
extern __typeof (xmlNewRMutex) xmlNewRMutex __attribute((alias("xmlNewRMutex__internal_alias")));
#else
#ifndef xmlNewRMutex
extern __typeof (xmlNewRMutex) xmlNewRMutex__internal_alias __attribute((visibility("hidden")));
#define xmlNewRMutex xmlNewRMutex__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewReference
extern __typeof (xmlNewReference) xmlNewReference __attribute((alias("xmlNewReference__internal_alias")));
#else
#ifndef xmlNewReference
extern __typeof (xmlNewReference) xmlNewReference__internal_alias __attribute((visibility("hidden")));
#define xmlNewReference xmlNewReference__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlNewStringInputStream
extern __typeof (xmlNewStringInputStream) xmlNewStringInputStream __attribute((alias("xmlNewStringInputStream__internal_alias")));
#else
#ifndef xmlNewStringInputStream
extern __typeof (xmlNewStringInputStream) xmlNewStringInputStream__internal_alias __attribute((visibility("hidden")));
#define xmlNewStringInputStream xmlNewStringInputStream__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNewText
extern __typeof (xmlNewText) xmlNewText __attribute((alias("xmlNewText__internal_alias")));
#else
#ifndef xmlNewText
extern __typeof (xmlNewText) xmlNewText__internal_alias __attribute((visibility("hidden")));
#define xmlNewText xmlNewText__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_tree
#undef xmlNewTextChild
extern __typeof (xmlNewTextChild) xmlNewTextChild __attribute((alias("xmlNewTextChild__internal_alias")));
#else
#ifndef xmlNewTextChild
extern __typeof (xmlNewTextChild) xmlNewTextChild__internal_alias __attribute((visibility("hidden")));
#define xmlNewTextChild xmlNewTextChild__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlNewTextLen
extern __typeof (xmlNewTextLen) xmlNewTextLen __attribute((alias("xmlNewTextLen__internal_alias")));
#else
#ifndef xmlNewTextLen
extern __typeof (xmlNewTextLen) xmlNewTextLen__internal_alias __attribute((visibility("hidden")));
#define xmlNewTextLen xmlNewTextLen__internal_alias
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlNewTextReader
extern __typeof (xmlNewTextReader) xmlNewTextReader __attribute((alias("xmlNewTextReader__internal_alias")));
#else
#ifndef xmlNewTextReader
extern __typeof (xmlNewTextReader) xmlNewTextReader__internal_alias __attribute((visibility("hidden")));
#define xmlNewTextReader xmlNewTextReader__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlNewTextReaderFilename
extern __typeof (xmlNewTextReaderFilename) xmlNewTextReaderFilename __attribute((alias("xmlNewTextReaderFilename__internal_alias")));
#else
#ifndef xmlNewTextReaderFilename
extern __typeof (xmlNewTextReaderFilename) xmlNewTextReaderFilename__internal_alias __attribute((visibility("hidden")));
#define xmlNewTextReaderFilename xmlNewTextReaderFilename__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlNewTextWriter
extern __typeof (xmlNewTextWriter) xmlNewTextWriter __attribute((alias("xmlNewTextWriter__internal_alias")));
#else
#ifndef xmlNewTextWriter
extern __typeof (xmlNewTextWriter) xmlNewTextWriter__internal_alias __attribute((visibility("hidden")));
#define xmlNewTextWriter xmlNewTextWriter__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlNewTextWriterDoc
extern __typeof (xmlNewTextWriterDoc) xmlNewTextWriterDoc __attribute((alias("xmlNewTextWriterDoc__internal_alias")));
#else
#ifndef xmlNewTextWriterDoc
extern __typeof (xmlNewTextWriterDoc) xmlNewTextWriterDoc__internal_alias __attribute((visibility("hidden")));
#define xmlNewTextWriterDoc xmlNewTextWriterDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlNewTextWriterFilename
extern __typeof (xmlNewTextWriterFilename) xmlNewTextWriterFilename __attribute((alias("xmlNewTextWriterFilename__internal_alias")));
#else
#ifndef xmlNewTextWriterFilename
extern __typeof (xmlNewTextWriterFilename) xmlNewTextWriterFilename__internal_alias __attribute((visibility("hidden")));
#define xmlNewTextWriterFilename xmlNewTextWriterFilename__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlNewTextWriterMemory
extern __typeof (xmlNewTextWriterMemory) xmlNewTextWriterMemory __attribute((alias("xmlNewTextWriterMemory__internal_alias")));
#else
#ifndef xmlNewTextWriterMemory
extern __typeof (xmlNewTextWriterMemory) xmlNewTextWriterMemory__internal_alias __attribute((visibility("hidden")));
#define xmlNewTextWriterMemory xmlNewTextWriterMemory__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlNewTextWriterPushParser
extern __typeof (xmlNewTextWriterPushParser) xmlNewTextWriterPushParser __attribute((alias("xmlNewTextWriterPushParser__internal_alias")));
#else
#ifndef xmlNewTextWriterPushParser
extern __typeof (xmlNewTextWriterPushParser) xmlNewTextWriterPushParser__internal_alias __attribute((visibility("hidden")));
#define xmlNewTextWriterPushParser xmlNewTextWriterPushParser__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlNewTextWriterTree
extern __typeof (xmlNewTextWriterTree) xmlNewTextWriterTree __attribute((alias("xmlNewTextWriterTree__internal_alias")));
#else
#ifndef xmlNewTextWriterTree
extern __typeof (xmlNewTextWriterTree) xmlNewTextWriterTree__internal_alias __attribute((visibility("hidden")));
#define xmlNewTextWriterTree xmlNewTextWriterTree__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlNewValidCtxt
extern __typeof (xmlNewValidCtxt) xmlNewValidCtxt __attribute((alias("xmlNewValidCtxt__internal_alias")));
#else
#ifndef xmlNewValidCtxt
extern __typeof (xmlNewValidCtxt) xmlNewValidCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlNewValidCtxt xmlNewValidCtxt__internal_alias
#endif
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlNextChar
extern __typeof (xmlNextChar) xmlNextChar __attribute((alias("xmlNextChar__internal_alias")));
#else
#ifndef xmlNextChar
extern __typeof (xmlNextChar) xmlNextChar__internal_alias __attribute((visibility("hidden")));
#define xmlNextChar xmlNextChar__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlNoNetExternalEntityLoader
extern __typeof (xmlNoNetExternalEntityLoader) xmlNoNetExternalEntityLoader __attribute((alias("xmlNoNetExternalEntityLoader__internal_alias")));
#else
#ifndef xmlNoNetExternalEntityLoader
extern __typeof (xmlNoNetExternalEntityLoader) xmlNoNetExternalEntityLoader__internal_alias __attribute((visibility("hidden")));
#define xmlNoNetExternalEntityLoader xmlNoNetExternalEntityLoader__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNodeAddContent
extern __typeof (xmlNodeAddContent) xmlNodeAddContent __attribute((alias("xmlNodeAddContent__internal_alias")));
#else
#ifndef xmlNodeAddContent
extern __typeof (xmlNodeAddContent) xmlNodeAddContent__internal_alias __attribute((visibility("hidden")));
#define xmlNodeAddContent xmlNodeAddContent__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNodeAddContentLen
extern __typeof (xmlNodeAddContentLen) xmlNodeAddContentLen __attribute((alias("xmlNodeAddContentLen__internal_alias")));
#else
#ifndef xmlNodeAddContentLen
extern __typeof (xmlNodeAddContentLen) xmlNodeAddContentLen__internal_alias __attribute((visibility("hidden")));
#define xmlNodeAddContentLen xmlNodeAddContentLen__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNodeBufGetContent
extern __typeof (xmlNodeBufGetContent) xmlNodeBufGetContent __attribute((alias("xmlNodeBufGetContent__internal_alias")));
#else
#ifndef xmlNodeBufGetContent
extern __typeof (xmlNodeBufGetContent) xmlNodeBufGetContent__internal_alias __attribute((visibility("hidden")));
#define xmlNodeBufGetContent xmlNodeBufGetContent__internal_alias
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlNodeDump
extern __typeof (xmlNodeDump) xmlNodeDump __attribute((alias("xmlNodeDump__internal_alias")));
#else
#ifndef xmlNodeDump
extern __typeof (xmlNodeDump) xmlNodeDump__internal_alias __attribute((visibility("hidden")));
#define xmlNodeDump xmlNodeDump__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlNodeDumpOutput
extern __typeof (xmlNodeDumpOutput) xmlNodeDumpOutput __attribute((alias("xmlNodeDumpOutput__internal_alias")));
#else
#ifndef xmlNodeDumpOutput
extern __typeof (xmlNodeDumpOutput) xmlNodeDumpOutput__internal_alias __attribute((visibility("hidden")));
#define xmlNodeDumpOutput xmlNodeDumpOutput__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlNodeGetBase
extern __typeof (xmlNodeGetBase) xmlNodeGetBase __attribute((alias("xmlNodeGetBase__internal_alias")));
#else
#ifndef xmlNodeGetBase
extern __typeof (xmlNodeGetBase) xmlNodeGetBase__internal_alias __attribute((visibility("hidden")));
#define xmlNodeGetBase xmlNodeGetBase__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNodeGetContent
extern __typeof (xmlNodeGetContent) xmlNodeGetContent __attribute((alias("xmlNodeGetContent__internal_alias")));
#else
#ifndef xmlNodeGetContent
extern __typeof (xmlNodeGetContent) xmlNodeGetContent__internal_alias __attribute((visibility("hidden")));
#define xmlNodeGetContent xmlNodeGetContent__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNodeGetLang
extern __typeof (xmlNodeGetLang) xmlNodeGetLang __attribute((alias("xmlNodeGetLang__internal_alias")));
#else
#ifndef xmlNodeGetLang
extern __typeof (xmlNodeGetLang) xmlNodeGetLang__internal_alias __attribute((visibility("hidden")));
#define xmlNodeGetLang xmlNodeGetLang__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNodeGetSpacePreserve
extern __typeof (xmlNodeGetSpacePreserve) xmlNodeGetSpacePreserve __attribute((alias("xmlNodeGetSpacePreserve__internal_alias")));
#else
#ifndef xmlNodeGetSpacePreserve
extern __typeof (xmlNodeGetSpacePreserve) xmlNodeGetSpacePreserve__internal_alias __attribute((visibility("hidden")));
#define xmlNodeGetSpacePreserve xmlNodeGetSpacePreserve__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlNodeIsText
extern __typeof (xmlNodeIsText) xmlNodeIsText __attribute((alias("xmlNodeIsText__internal_alias")));
#else
#ifndef xmlNodeIsText
extern __typeof (xmlNodeIsText) xmlNodeIsText__internal_alias __attribute((visibility("hidden")));
#define xmlNodeIsText xmlNodeIsText__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_tree
#undef xmlNodeListGetRawString
extern __typeof (xmlNodeListGetRawString) xmlNodeListGetRawString __attribute((alias("xmlNodeListGetRawString__internal_alias")));
#else
#ifndef xmlNodeListGetRawString
extern __typeof (xmlNodeListGetRawString) xmlNodeListGetRawString__internal_alias __attribute((visibility("hidden")));
#define xmlNodeListGetRawString xmlNodeListGetRawString__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlNodeListGetString
extern __typeof (xmlNodeListGetString) xmlNodeListGetString __attribute((alias("xmlNodeListGetString__internal_alias")));
#else
#ifndef xmlNodeListGetString
extern __typeof (xmlNodeListGetString) xmlNodeListGetString__internal_alias __attribute((visibility("hidden")));
#define xmlNodeListGetString xmlNodeListGetString__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_XINCLUDE_ENABLED)
#ifdef bottom_tree
#undef xmlNodeSetBase
extern __typeof (xmlNodeSetBase) xmlNodeSetBase __attribute((alias("xmlNodeSetBase__internal_alias")));
#else
#ifndef xmlNodeSetBase
extern __typeof (xmlNodeSetBase) xmlNodeSetBase__internal_alias __attribute((visibility("hidden")));
#define xmlNodeSetBase xmlNodeSetBase__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlNodeSetContent
extern __typeof (xmlNodeSetContent) xmlNodeSetContent __attribute((alias("xmlNodeSetContent__internal_alias")));
#else
#ifndef xmlNodeSetContent
extern __typeof (xmlNodeSetContent) xmlNodeSetContent__internal_alias __attribute((visibility("hidden")));
#define xmlNodeSetContent xmlNodeSetContent__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_tree
#undef xmlNodeSetContentLen
extern __typeof (xmlNodeSetContentLen) xmlNodeSetContentLen __attribute((alias("xmlNodeSetContentLen__internal_alias")));
#else
#ifndef xmlNodeSetContentLen
extern __typeof (xmlNodeSetContentLen) xmlNodeSetContentLen__internal_alias __attribute((visibility("hidden")));
#define xmlNodeSetContentLen xmlNodeSetContentLen__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_tree
#undef xmlNodeSetLang
extern __typeof (xmlNodeSetLang) xmlNodeSetLang __attribute((alias("xmlNodeSetLang__internal_alias")));
#else
#ifndef xmlNodeSetLang
extern __typeof (xmlNodeSetLang) xmlNodeSetLang__internal_alias __attribute((visibility("hidden")));
#define xmlNodeSetLang xmlNodeSetLang__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_tree
#undef xmlNodeSetName
extern __typeof (xmlNodeSetName) xmlNodeSetName __attribute((alias("xmlNodeSetName__internal_alias")));
#else
#ifndef xmlNodeSetName
extern __typeof (xmlNodeSetName) xmlNodeSetName__internal_alias __attribute((visibility("hidden")));
#define xmlNodeSetName xmlNodeSetName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_tree
#undef xmlNodeSetSpacePreserve
extern __typeof (xmlNodeSetSpacePreserve) xmlNodeSetSpacePreserve __attribute((alias("xmlNodeSetSpacePreserve__internal_alias")));
#else
#ifndef xmlNodeSetSpacePreserve
extern __typeof (xmlNodeSetSpacePreserve) xmlNodeSetSpacePreserve__internal_alias __attribute((visibility("hidden")));
#define xmlNodeSetSpacePreserve xmlNodeSetSpacePreserve__internal_alias
#endif
#endif
#endif

#ifdef bottom_uri
#undef xmlNormalizeURIPath
extern __typeof (xmlNormalizeURIPath) xmlNormalizeURIPath __attribute((alias("xmlNormalizeURIPath__internal_alias")));
#else
#ifndef xmlNormalizeURIPath
extern __typeof (xmlNormalizeURIPath) xmlNormalizeURIPath__internal_alias __attribute((visibility("hidden")));
#define xmlNormalizeURIPath xmlNormalizeURIPath__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlNormalizeWindowsPath
extern __typeof (xmlNormalizeWindowsPath) xmlNormalizeWindowsPath __attribute((alias("xmlNormalizeWindowsPath__internal_alias")));
#else
#ifndef xmlNormalizeWindowsPath
extern __typeof (xmlNormalizeWindowsPath) xmlNormalizeWindowsPath__internal_alias __attribute((visibility("hidden")));
#define xmlNormalizeWindowsPath xmlNormalizeWindowsPath__internal_alias
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlOutputBufferClose
extern __typeof (xmlOutputBufferClose) xmlOutputBufferClose __attribute((alias("xmlOutputBufferClose__internal_alias")));
#else
#ifndef xmlOutputBufferClose
extern __typeof (xmlOutputBufferClose) xmlOutputBufferClose__internal_alias __attribute((visibility("hidden")));
#define xmlOutputBufferClose xmlOutputBufferClose__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlOutputBufferCreateFd
extern __typeof (xmlOutputBufferCreateFd) xmlOutputBufferCreateFd __attribute((alias("xmlOutputBufferCreateFd__internal_alias")));
#else
#ifndef xmlOutputBufferCreateFd
extern __typeof (xmlOutputBufferCreateFd) xmlOutputBufferCreateFd__internal_alias __attribute((visibility("hidden")));
#define xmlOutputBufferCreateFd xmlOutputBufferCreateFd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlOutputBufferCreateFile
extern __typeof (xmlOutputBufferCreateFile) xmlOutputBufferCreateFile __attribute((alias("xmlOutputBufferCreateFile__internal_alias")));
#else
#ifndef xmlOutputBufferCreateFile
extern __typeof (xmlOutputBufferCreateFile) xmlOutputBufferCreateFile__internal_alias __attribute((visibility("hidden")));
#define xmlOutputBufferCreateFile xmlOutputBufferCreateFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlOutputBufferCreateFilename
extern __typeof (xmlOutputBufferCreateFilename) xmlOutputBufferCreateFilename __attribute((alias("xmlOutputBufferCreateFilename__internal_alias")));
#else
#ifndef xmlOutputBufferCreateFilename
extern __typeof (xmlOutputBufferCreateFilename) xmlOutputBufferCreateFilename__internal_alias __attribute((visibility("hidden")));
#define xmlOutputBufferCreateFilename xmlOutputBufferCreateFilename__internal_alias
#endif
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlOutputBufferCreateFilenameDefault
extern __typeof (xmlOutputBufferCreateFilenameDefault) xmlOutputBufferCreateFilenameDefault __attribute((alias("xmlOutputBufferCreateFilenameDefault__internal_alias")));
#else
#ifndef xmlOutputBufferCreateFilenameDefault
extern __typeof (xmlOutputBufferCreateFilenameDefault) xmlOutputBufferCreateFilenameDefault__internal_alias __attribute((visibility("hidden")));
#define xmlOutputBufferCreateFilenameDefault xmlOutputBufferCreateFilenameDefault__internal_alias
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlOutputBufferCreateIO
extern __typeof (xmlOutputBufferCreateIO) xmlOutputBufferCreateIO __attribute((alias("xmlOutputBufferCreateIO__internal_alias")));
#else
#ifndef xmlOutputBufferCreateIO
extern __typeof (xmlOutputBufferCreateIO) xmlOutputBufferCreateIO__internal_alias __attribute((visibility("hidden")));
#define xmlOutputBufferCreateIO xmlOutputBufferCreateIO__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlOutputBufferFlush
extern __typeof (xmlOutputBufferFlush) xmlOutputBufferFlush __attribute((alias("xmlOutputBufferFlush__internal_alias")));
#else
#ifndef xmlOutputBufferFlush
extern __typeof (xmlOutputBufferFlush) xmlOutputBufferFlush__internal_alias __attribute((visibility("hidden")));
#define xmlOutputBufferFlush xmlOutputBufferFlush__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlOutputBufferWrite
extern __typeof (xmlOutputBufferWrite) xmlOutputBufferWrite __attribute((alias("xmlOutputBufferWrite__internal_alias")));
#else
#ifndef xmlOutputBufferWrite
extern __typeof (xmlOutputBufferWrite) xmlOutputBufferWrite__internal_alias __attribute((visibility("hidden")));
#define xmlOutputBufferWrite xmlOutputBufferWrite__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlOutputBufferWriteEscape
extern __typeof (xmlOutputBufferWriteEscape) xmlOutputBufferWriteEscape __attribute((alias("xmlOutputBufferWriteEscape__internal_alias")));
#else
#ifndef xmlOutputBufferWriteEscape
extern __typeof (xmlOutputBufferWriteEscape) xmlOutputBufferWriteEscape__internal_alias __attribute((visibility("hidden")));
#define xmlOutputBufferWriteEscape xmlOutputBufferWriteEscape__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlOutputBufferWriteString
extern __typeof (xmlOutputBufferWriteString) xmlOutputBufferWriteString __attribute((alias("xmlOutputBufferWriteString__internal_alias")));
#else
#ifndef xmlOutputBufferWriteString
extern __typeof (xmlOutputBufferWriteString) xmlOutputBufferWriteString__internal_alias __attribute((visibility("hidden")));
#define xmlOutputBufferWriteString xmlOutputBufferWriteString__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseAttValue
extern __typeof (xmlParseAttValue) xmlParseAttValue __attribute((alias("xmlParseAttValue__internal_alias")));
#else
#ifndef xmlParseAttValue
extern __typeof (xmlParseAttValue) xmlParseAttValue__internal_alias __attribute((visibility("hidden")));
#define xmlParseAttValue xmlParseAttValue__internal_alias
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlParseAttribute
extern __typeof (xmlParseAttribute) xmlParseAttribute __attribute((alias("xmlParseAttribute__internal_alias")));
#else
#ifndef xmlParseAttribute
extern __typeof (xmlParseAttribute) xmlParseAttribute__internal_alias __attribute((visibility("hidden")));
#define xmlParseAttribute xmlParseAttribute__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseAttributeListDecl
extern __typeof (xmlParseAttributeListDecl) xmlParseAttributeListDecl __attribute((alias("xmlParseAttributeListDecl__internal_alias")));
#else
#ifndef xmlParseAttributeListDecl
extern __typeof (xmlParseAttributeListDecl) xmlParseAttributeListDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseAttributeListDecl xmlParseAttributeListDecl__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseAttributeType
extern __typeof (xmlParseAttributeType) xmlParseAttributeType __attribute((alias("xmlParseAttributeType__internal_alias")));
#else
#ifndef xmlParseAttributeType
extern __typeof (xmlParseAttributeType) xmlParseAttributeType__internal_alias __attribute((visibility("hidden")));
#define xmlParseAttributeType xmlParseAttributeType__internal_alias
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlParseBalancedChunkMemory
extern __typeof (xmlParseBalancedChunkMemory) xmlParseBalancedChunkMemory __attribute((alias("xmlParseBalancedChunkMemory__internal_alias")));
#else
#ifndef xmlParseBalancedChunkMemory
extern __typeof (xmlParseBalancedChunkMemory) xmlParseBalancedChunkMemory__internal_alias __attribute((visibility("hidden")));
#define xmlParseBalancedChunkMemory xmlParseBalancedChunkMemory__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlParseBalancedChunkMemoryRecover
extern __typeof (xmlParseBalancedChunkMemoryRecover) xmlParseBalancedChunkMemoryRecover __attribute((alias("xmlParseBalancedChunkMemoryRecover__internal_alias")));
#else
#ifndef xmlParseBalancedChunkMemoryRecover
extern __typeof (xmlParseBalancedChunkMemoryRecover) xmlParseBalancedChunkMemoryRecover__internal_alias __attribute((visibility("hidden")));
#define xmlParseBalancedChunkMemoryRecover xmlParseBalancedChunkMemoryRecover__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseCDSect
extern __typeof (xmlParseCDSect) xmlParseCDSect __attribute((alias("xmlParseCDSect__internal_alias")));
#else
#ifndef xmlParseCDSect
extern __typeof (xmlParseCDSect) xmlParseCDSect__internal_alias __attribute((visibility("hidden")));
#define xmlParseCDSect xmlParseCDSect__internal_alias
#endif
#endif

#if defined(LIBXML_CATALOG_ENABLED)
#ifdef bottom_catalog
#undef xmlParseCatalogFile
extern __typeof (xmlParseCatalogFile) xmlParseCatalogFile __attribute((alias("xmlParseCatalogFile__internal_alias")));
#else
#ifndef xmlParseCatalogFile
extern __typeof (xmlParseCatalogFile) xmlParseCatalogFile__internal_alias __attribute((visibility("hidden")));
#define xmlParseCatalogFile xmlParseCatalogFile__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseCharData
extern __typeof (xmlParseCharData) xmlParseCharData __attribute((alias("xmlParseCharData__internal_alias")));
#else
#ifndef xmlParseCharData
extern __typeof (xmlParseCharData) xmlParseCharData__internal_alias __attribute((visibility("hidden")));
#define xmlParseCharData xmlParseCharData__internal_alias
#endif
#endif

#ifdef bottom_encoding
#undef xmlParseCharEncoding
extern __typeof (xmlParseCharEncoding) xmlParseCharEncoding __attribute((alias("xmlParseCharEncoding__internal_alias")));
#else
#ifndef xmlParseCharEncoding
extern __typeof (xmlParseCharEncoding) xmlParseCharEncoding__internal_alias __attribute((visibility("hidden")));
#define xmlParseCharEncoding xmlParseCharEncoding__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseCharRef
extern __typeof (xmlParseCharRef) xmlParseCharRef __attribute((alias("xmlParseCharRef__internal_alias")));
#else
#ifndef xmlParseCharRef
extern __typeof (xmlParseCharRef) xmlParseCharRef__internal_alias __attribute((visibility("hidden")));
#define xmlParseCharRef xmlParseCharRef__internal_alias
#endif
#endif

#if defined(LIBXML_PUSH_ENABLED)
#ifdef bottom_parser
#undef xmlParseChunk
extern __typeof (xmlParseChunk) xmlParseChunk __attribute((alias("xmlParseChunk__internal_alias")));
#else
#ifndef xmlParseChunk
extern __typeof (xmlParseChunk) xmlParseChunk__internal_alias __attribute((visibility("hidden")));
#define xmlParseChunk xmlParseChunk__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseComment
extern __typeof (xmlParseComment) xmlParseComment __attribute((alias("xmlParseComment__internal_alias")));
#else
#ifndef xmlParseComment
extern __typeof (xmlParseComment) xmlParseComment__internal_alias __attribute((visibility("hidden")));
#define xmlParseComment xmlParseComment__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseContent
extern __typeof (xmlParseContent) xmlParseContent __attribute((alias("xmlParseContent__internal_alias")));
#else
#ifndef xmlParseContent
extern __typeof (xmlParseContent) xmlParseContent__internal_alias __attribute((visibility("hidden")));
#define xmlParseContent xmlParseContent__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseCtxtExternalEntity
extern __typeof (xmlParseCtxtExternalEntity) xmlParseCtxtExternalEntity __attribute((alias("xmlParseCtxtExternalEntity__internal_alias")));
#else
#ifndef xmlParseCtxtExternalEntity
extern __typeof (xmlParseCtxtExternalEntity) xmlParseCtxtExternalEntity__internal_alias __attribute((visibility("hidden")));
#define xmlParseCtxtExternalEntity xmlParseCtxtExternalEntity__internal_alias
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_parser
#undef xmlParseDTD
extern __typeof (xmlParseDTD) xmlParseDTD __attribute((alias("xmlParseDTD__internal_alias")));
#else
#ifndef xmlParseDTD
extern __typeof (xmlParseDTD) xmlParseDTD__internal_alias __attribute((visibility("hidden")));
#define xmlParseDTD xmlParseDTD__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseDefaultDecl
extern __typeof (xmlParseDefaultDecl) xmlParseDefaultDecl __attribute((alias("xmlParseDefaultDecl__internal_alias")));
#else
#ifndef xmlParseDefaultDecl
extern __typeof (xmlParseDefaultDecl) xmlParseDefaultDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseDefaultDecl xmlParseDefaultDecl__internal_alias
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlParseDoc
extern __typeof (xmlParseDoc) xmlParseDoc __attribute((alias("xmlParseDoc__internal_alias")));
#else
#ifndef xmlParseDoc
extern __typeof (xmlParseDoc) xmlParseDoc__internal_alias __attribute((visibility("hidden")));
#define xmlParseDoc xmlParseDoc__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseDocTypeDecl
extern __typeof (xmlParseDocTypeDecl) xmlParseDocTypeDecl __attribute((alias("xmlParseDocTypeDecl__internal_alias")));
#else
#ifndef xmlParseDocTypeDecl
extern __typeof (xmlParseDocTypeDecl) xmlParseDocTypeDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseDocTypeDecl xmlParseDocTypeDecl__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseDocument
extern __typeof (xmlParseDocument) xmlParseDocument __attribute((alias("xmlParseDocument__internal_alias")));
#else
#ifndef xmlParseDocument
extern __typeof (xmlParseDocument) xmlParseDocument__internal_alias __attribute((visibility("hidden")));
#define xmlParseDocument xmlParseDocument__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseElement
extern __typeof (xmlParseElement) xmlParseElement __attribute((alias("xmlParseElement__internal_alias")));
#else
#ifndef xmlParseElement
extern __typeof (xmlParseElement) xmlParseElement__internal_alias __attribute((visibility("hidden")));
#define xmlParseElement xmlParseElement__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseElementChildrenContentDecl
extern __typeof (xmlParseElementChildrenContentDecl) xmlParseElementChildrenContentDecl __attribute((alias("xmlParseElementChildrenContentDecl__internal_alias")));
#else
#ifndef xmlParseElementChildrenContentDecl
extern __typeof (xmlParseElementChildrenContentDecl) xmlParseElementChildrenContentDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseElementChildrenContentDecl xmlParseElementChildrenContentDecl__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseElementContentDecl
extern __typeof (xmlParseElementContentDecl) xmlParseElementContentDecl __attribute((alias("xmlParseElementContentDecl__internal_alias")));
#else
#ifndef xmlParseElementContentDecl
extern __typeof (xmlParseElementContentDecl) xmlParseElementContentDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseElementContentDecl xmlParseElementContentDecl__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseElementDecl
extern __typeof (xmlParseElementDecl) xmlParseElementDecl __attribute((alias("xmlParseElementDecl__internal_alias")));
#else
#ifndef xmlParseElementDecl
extern __typeof (xmlParseElementDecl) xmlParseElementDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseElementDecl xmlParseElementDecl__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseElementMixedContentDecl
extern __typeof (xmlParseElementMixedContentDecl) xmlParseElementMixedContentDecl __attribute((alias("xmlParseElementMixedContentDecl__internal_alias")));
#else
#ifndef xmlParseElementMixedContentDecl
extern __typeof (xmlParseElementMixedContentDecl) xmlParseElementMixedContentDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseElementMixedContentDecl xmlParseElementMixedContentDecl__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseEncName
extern __typeof (xmlParseEncName) xmlParseEncName __attribute((alias("xmlParseEncName__internal_alias")));
#else
#ifndef xmlParseEncName
extern __typeof (xmlParseEncName) xmlParseEncName__internal_alias __attribute((visibility("hidden")));
#define xmlParseEncName xmlParseEncName__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseEncodingDecl
extern __typeof (xmlParseEncodingDecl) xmlParseEncodingDecl __attribute((alias("xmlParseEncodingDecl__internal_alias")));
#else
#ifndef xmlParseEncodingDecl
extern __typeof (xmlParseEncodingDecl) xmlParseEncodingDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseEncodingDecl xmlParseEncodingDecl__internal_alias
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlParseEndTag
extern __typeof (xmlParseEndTag) xmlParseEndTag __attribute((alias("xmlParseEndTag__internal_alias")));
#else
#ifndef xmlParseEndTag
extern __typeof (xmlParseEndTag) xmlParseEndTag__internal_alias __attribute((visibility("hidden")));
#define xmlParseEndTag xmlParseEndTag__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlParseEntity
extern __typeof (xmlParseEntity) xmlParseEntity __attribute((alias("xmlParseEntity__internal_alias")));
#else
#ifndef xmlParseEntity
extern __typeof (xmlParseEntity) xmlParseEntity__internal_alias __attribute((visibility("hidden")));
#define xmlParseEntity xmlParseEntity__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseEntityDecl
extern __typeof (xmlParseEntityDecl) xmlParseEntityDecl __attribute((alias("xmlParseEntityDecl__internal_alias")));
#else
#ifndef xmlParseEntityDecl
extern __typeof (xmlParseEntityDecl) xmlParseEntityDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseEntityDecl xmlParseEntityDecl__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseEntityRef
extern __typeof (xmlParseEntityRef) xmlParseEntityRef __attribute((alias("xmlParseEntityRef__internal_alias")));
#else
#ifndef xmlParseEntityRef
extern __typeof (xmlParseEntityRef) xmlParseEntityRef__internal_alias __attribute((visibility("hidden")));
#define xmlParseEntityRef xmlParseEntityRef__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseEntityValue
extern __typeof (xmlParseEntityValue) xmlParseEntityValue __attribute((alias("xmlParseEntityValue__internal_alias")));
#else
#ifndef xmlParseEntityValue
extern __typeof (xmlParseEntityValue) xmlParseEntityValue__internal_alias __attribute((visibility("hidden")));
#define xmlParseEntityValue xmlParseEntityValue__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseEnumeratedType
extern __typeof (xmlParseEnumeratedType) xmlParseEnumeratedType __attribute((alias("xmlParseEnumeratedType__internal_alias")));
#else
#ifndef xmlParseEnumeratedType
extern __typeof (xmlParseEnumeratedType) xmlParseEnumeratedType__internal_alias __attribute((visibility("hidden")));
#define xmlParseEnumeratedType xmlParseEnumeratedType__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseEnumerationType
extern __typeof (xmlParseEnumerationType) xmlParseEnumerationType __attribute((alias("xmlParseEnumerationType__internal_alias")));
#else
#ifndef xmlParseEnumerationType
extern __typeof (xmlParseEnumerationType) xmlParseEnumerationType__internal_alias __attribute((visibility("hidden")));
#define xmlParseEnumerationType xmlParseEnumerationType__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseExtParsedEnt
extern __typeof (xmlParseExtParsedEnt) xmlParseExtParsedEnt __attribute((alias("xmlParseExtParsedEnt__internal_alias")));
#else
#ifndef xmlParseExtParsedEnt
extern __typeof (xmlParseExtParsedEnt) xmlParseExtParsedEnt__internal_alias __attribute((visibility("hidden")));
#define xmlParseExtParsedEnt xmlParseExtParsedEnt__internal_alias
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlParseExternalEntity
extern __typeof (xmlParseExternalEntity) xmlParseExternalEntity __attribute((alias("xmlParseExternalEntity__internal_alias")));
#else
#ifndef xmlParseExternalEntity
extern __typeof (xmlParseExternalEntity) xmlParseExternalEntity__internal_alias __attribute((visibility("hidden")));
#define xmlParseExternalEntity xmlParseExternalEntity__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseExternalID
extern __typeof (xmlParseExternalID) xmlParseExternalID __attribute((alias("xmlParseExternalID__internal_alias")));
#else
#ifndef xmlParseExternalID
extern __typeof (xmlParseExternalID) xmlParseExternalID__internal_alias __attribute((visibility("hidden")));
#define xmlParseExternalID xmlParseExternalID__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseExternalSubset
extern __typeof (xmlParseExternalSubset) xmlParseExternalSubset __attribute((alias("xmlParseExternalSubset__internal_alias")));
#else
#ifndef xmlParseExternalSubset
extern __typeof (xmlParseExternalSubset) xmlParseExternalSubset__internal_alias __attribute((visibility("hidden")));
#define xmlParseExternalSubset xmlParseExternalSubset__internal_alias
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlParseFile
extern __typeof (xmlParseFile) xmlParseFile __attribute((alias("xmlParseFile__internal_alias")));
#else
#ifndef xmlParseFile
extern __typeof (xmlParseFile) xmlParseFile__internal_alias __attribute((visibility("hidden")));
#define xmlParseFile xmlParseFile__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseInNodeContext
extern __typeof (xmlParseInNodeContext) xmlParseInNodeContext __attribute((alias("xmlParseInNodeContext__internal_alias")));
#else
#ifndef xmlParseInNodeContext
extern __typeof (xmlParseInNodeContext) xmlParseInNodeContext__internal_alias __attribute((visibility("hidden")));
#define xmlParseInNodeContext xmlParseInNodeContext__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseMarkupDecl
extern __typeof (xmlParseMarkupDecl) xmlParseMarkupDecl __attribute((alias("xmlParseMarkupDecl__internal_alias")));
#else
#ifndef xmlParseMarkupDecl
extern __typeof (xmlParseMarkupDecl) xmlParseMarkupDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseMarkupDecl xmlParseMarkupDecl__internal_alias
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlParseMemory
extern __typeof (xmlParseMemory) xmlParseMemory __attribute((alias("xmlParseMemory__internal_alias")));
#else
#ifndef xmlParseMemory
extern __typeof (xmlParseMemory) xmlParseMemory__internal_alias __attribute((visibility("hidden")));
#define xmlParseMemory xmlParseMemory__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseMisc
extern __typeof (xmlParseMisc) xmlParseMisc __attribute((alias("xmlParseMisc__internal_alias")));
#else
#ifndef xmlParseMisc
extern __typeof (xmlParseMisc) xmlParseMisc__internal_alias __attribute((visibility("hidden")));
#define xmlParseMisc xmlParseMisc__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseName
extern __typeof (xmlParseName) xmlParseName __attribute((alias("xmlParseName__internal_alias")));
#else
#ifndef xmlParseName
extern __typeof (xmlParseName) xmlParseName__internal_alias __attribute((visibility("hidden")));
#define xmlParseName xmlParseName__internal_alias
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlParseNamespace
extern __typeof (xmlParseNamespace) xmlParseNamespace __attribute((alias("xmlParseNamespace__internal_alias")));
#else
#ifndef xmlParseNamespace
extern __typeof (xmlParseNamespace) xmlParseNamespace__internal_alias __attribute((visibility("hidden")));
#define xmlParseNamespace xmlParseNamespace__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseNmtoken
extern __typeof (xmlParseNmtoken) xmlParseNmtoken __attribute((alias("xmlParseNmtoken__internal_alias")));
#else
#ifndef xmlParseNmtoken
extern __typeof (xmlParseNmtoken) xmlParseNmtoken__internal_alias __attribute((visibility("hidden")));
#define xmlParseNmtoken xmlParseNmtoken__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseNotationDecl
extern __typeof (xmlParseNotationDecl) xmlParseNotationDecl __attribute((alias("xmlParseNotationDecl__internal_alias")));
#else
#ifndef xmlParseNotationDecl
extern __typeof (xmlParseNotationDecl) xmlParseNotationDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseNotationDecl xmlParseNotationDecl__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseNotationType
extern __typeof (xmlParseNotationType) xmlParseNotationType __attribute((alias("xmlParseNotationType__internal_alias")));
#else
#ifndef xmlParseNotationType
extern __typeof (xmlParseNotationType) xmlParseNotationType__internal_alias __attribute((visibility("hidden")));
#define xmlParseNotationType xmlParseNotationType__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParsePEReference
extern __typeof (xmlParsePEReference) xmlParsePEReference __attribute((alias("xmlParsePEReference__internal_alias")));
#else
#ifndef xmlParsePEReference
extern __typeof (xmlParsePEReference) xmlParsePEReference__internal_alias __attribute((visibility("hidden")));
#define xmlParsePEReference xmlParsePEReference__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParsePI
extern __typeof (xmlParsePI) xmlParsePI __attribute((alias("xmlParsePI__internal_alias")));
#else
#ifndef xmlParsePI
extern __typeof (xmlParsePI) xmlParsePI__internal_alias __attribute((visibility("hidden")));
#define xmlParsePI xmlParsePI__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParsePITarget
extern __typeof (xmlParsePITarget) xmlParsePITarget __attribute((alias("xmlParsePITarget__internal_alias")));
#else
#ifndef xmlParsePITarget
extern __typeof (xmlParsePITarget) xmlParsePITarget__internal_alias __attribute((visibility("hidden")));
#define xmlParsePITarget xmlParsePITarget__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParsePubidLiteral
extern __typeof (xmlParsePubidLiteral) xmlParsePubidLiteral __attribute((alias("xmlParsePubidLiteral__internal_alias")));
#else
#ifndef xmlParsePubidLiteral
extern __typeof (xmlParsePubidLiteral) xmlParsePubidLiteral__internal_alias __attribute((visibility("hidden")));
#define xmlParsePubidLiteral xmlParsePubidLiteral__internal_alias
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlParseQuotedString
extern __typeof (xmlParseQuotedString) xmlParseQuotedString __attribute((alias("xmlParseQuotedString__internal_alias")));
#else
#ifndef xmlParseQuotedString
extern __typeof (xmlParseQuotedString) xmlParseQuotedString__internal_alias __attribute((visibility("hidden")));
#define xmlParseQuotedString xmlParseQuotedString__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseReference
extern __typeof (xmlParseReference) xmlParseReference __attribute((alias("xmlParseReference__internal_alias")));
#else
#ifndef xmlParseReference
extern __typeof (xmlParseReference) xmlParseReference__internal_alias __attribute((visibility("hidden")));
#define xmlParseReference xmlParseReference__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseSDDecl
extern __typeof (xmlParseSDDecl) xmlParseSDDecl __attribute((alias("xmlParseSDDecl__internal_alias")));
#else
#ifndef xmlParseSDDecl
extern __typeof (xmlParseSDDecl) xmlParseSDDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseSDDecl xmlParseSDDecl__internal_alias
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlParseStartTag
extern __typeof (xmlParseStartTag) xmlParseStartTag __attribute((alias("xmlParseStartTag__internal_alias")));
#else
#ifndef xmlParseStartTag
extern __typeof (xmlParseStartTag) xmlParseStartTag__internal_alias __attribute((visibility("hidden")));
#define xmlParseStartTag xmlParseStartTag__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlParseSystemLiteral
extern __typeof (xmlParseSystemLiteral) xmlParseSystemLiteral __attribute((alias("xmlParseSystemLiteral__internal_alias")));
#else
#ifndef xmlParseSystemLiteral
extern __typeof (xmlParseSystemLiteral) xmlParseSystemLiteral__internal_alias __attribute((visibility("hidden")));
#define xmlParseSystemLiteral xmlParseSystemLiteral__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseTextDecl
extern __typeof (xmlParseTextDecl) xmlParseTextDecl __attribute((alias("xmlParseTextDecl__internal_alias")));
#else
#ifndef xmlParseTextDecl
extern __typeof (xmlParseTextDecl) xmlParseTextDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseTextDecl xmlParseTextDecl__internal_alias
#endif
#endif

#ifdef bottom_uri
#undef xmlParseURI
extern __typeof (xmlParseURI) xmlParseURI __attribute((alias("xmlParseURI__internal_alias")));
#else
#ifndef xmlParseURI
extern __typeof (xmlParseURI) xmlParseURI__internal_alias __attribute((visibility("hidden")));
#define xmlParseURI xmlParseURI__internal_alias
#endif
#endif

#ifdef bottom_uri
#undef xmlParseURIReference
extern __typeof (xmlParseURIReference) xmlParseURIReference __attribute((alias("xmlParseURIReference__internal_alias")));
#else
#ifndef xmlParseURIReference
extern __typeof (xmlParseURIReference) xmlParseURIReference__internal_alias __attribute((visibility("hidden")));
#define xmlParseURIReference xmlParseURIReference__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseVersionInfo
extern __typeof (xmlParseVersionInfo) xmlParseVersionInfo __attribute((alias("xmlParseVersionInfo__internal_alias")));
#else
#ifndef xmlParseVersionInfo
extern __typeof (xmlParseVersionInfo) xmlParseVersionInfo__internal_alias __attribute((visibility("hidden")));
#define xmlParseVersionInfo xmlParseVersionInfo__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseVersionNum
extern __typeof (xmlParseVersionNum) xmlParseVersionNum __attribute((alias("xmlParseVersionNum__internal_alias")));
#else
#ifndef xmlParseVersionNum
extern __typeof (xmlParseVersionNum) xmlParseVersionNum__internal_alias __attribute((visibility("hidden")));
#define xmlParseVersionNum xmlParseVersionNum__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParseXMLDecl
extern __typeof (xmlParseXMLDecl) xmlParseXMLDecl __attribute((alias("xmlParseXMLDecl__internal_alias")));
#else
#ifndef xmlParseXMLDecl
extern __typeof (xmlParseXMLDecl) xmlParseXMLDecl__internal_alias __attribute((visibility("hidden")));
#define xmlParseXMLDecl xmlParseXMLDecl__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlParserAddNodeInfo
extern __typeof (xmlParserAddNodeInfo) xmlParserAddNodeInfo __attribute((alias("xmlParserAddNodeInfo__internal_alias")));
#else
#ifndef xmlParserAddNodeInfo
extern __typeof (xmlParserAddNodeInfo) xmlParserAddNodeInfo__internal_alias __attribute((visibility("hidden")));
#define xmlParserAddNodeInfo xmlParserAddNodeInfo__internal_alias
#endif
#endif

#ifdef bottom_error
#undef xmlParserError
extern __typeof (xmlParserError) xmlParserError __attribute((alias("xmlParserError__internal_alias")));
#else
#ifndef xmlParserError
extern __typeof (xmlParserError) xmlParserError__internal_alias __attribute((visibility("hidden")));
#define xmlParserError xmlParserError__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlParserFindNodeInfo
extern __typeof (xmlParserFindNodeInfo) xmlParserFindNodeInfo __attribute((alias("xmlParserFindNodeInfo__internal_alias")));
#else
#ifndef xmlParserFindNodeInfo
extern __typeof (xmlParserFindNodeInfo) xmlParserFindNodeInfo__internal_alias __attribute((visibility("hidden")));
#define xmlParserFindNodeInfo xmlParserFindNodeInfo__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlParserFindNodeInfoIndex
extern __typeof (xmlParserFindNodeInfoIndex) xmlParserFindNodeInfoIndex __attribute((alias("xmlParserFindNodeInfoIndex__internal_alias")));
#else
#ifndef xmlParserFindNodeInfoIndex
extern __typeof (xmlParserFindNodeInfoIndex) xmlParserFindNodeInfoIndex__internal_alias __attribute((visibility("hidden")));
#define xmlParserFindNodeInfoIndex xmlParserFindNodeInfoIndex__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlParserGetDirectory
extern __typeof (xmlParserGetDirectory) xmlParserGetDirectory __attribute((alias("xmlParserGetDirectory__internal_alias")));
#else
#ifndef xmlParserGetDirectory
extern __typeof (xmlParserGetDirectory) xmlParserGetDirectory__internal_alias __attribute((visibility("hidden")));
#define xmlParserGetDirectory xmlParserGetDirectory__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlParserHandlePEReference
extern __typeof (xmlParserHandlePEReference) xmlParserHandlePEReference __attribute((alias("xmlParserHandlePEReference__internal_alias")));
#else
#ifndef xmlParserHandlePEReference
extern __typeof (xmlParserHandlePEReference) xmlParserHandlePEReference__internal_alias __attribute((visibility("hidden")));
#define xmlParserHandlePEReference xmlParserHandlePEReference__internal_alias
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlParserHandleReference
extern __typeof (xmlParserHandleReference) xmlParserHandleReference __attribute((alias("xmlParserHandleReference__internal_alias")));
#else
#ifndef xmlParserHandleReference
extern __typeof (xmlParserHandleReference) xmlParserHandleReference__internal_alias __attribute((visibility("hidden")));
#define xmlParserHandleReference xmlParserHandleReference__internal_alias
#endif
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlParserInputBufferCreateFd
extern __typeof (xmlParserInputBufferCreateFd) xmlParserInputBufferCreateFd __attribute((alias("xmlParserInputBufferCreateFd__internal_alias")));
#else
#ifndef xmlParserInputBufferCreateFd
extern __typeof (xmlParserInputBufferCreateFd) xmlParserInputBufferCreateFd__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputBufferCreateFd xmlParserInputBufferCreateFd__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlParserInputBufferCreateFile
extern __typeof (xmlParserInputBufferCreateFile) xmlParserInputBufferCreateFile __attribute((alias("xmlParserInputBufferCreateFile__internal_alias")));
#else
#ifndef xmlParserInputBufferCreateFile
extern __typeof (xmlParserInputBufferCreateFile) xmlParserInputBufferCreateFile__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputBufferCreateFile xmlParserInputBufferCreateFile__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlParserInputBufferCreateFilename
extern __typeof (xmlParserInputBufferCreateFilename) xmlParserInputBufferCreateFilename __attribute((alias("xmlParserInputBufferCreateFilename__internal_alias")));
#else
#ifndef xmlParserInputBufferCreateFilename
extern __typeof (xmlParserInputBufferCreateFilename) xmlParserInputBufferCreateFilename__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputBufferCreateFilename xmlParserInputBufferCreateFilename__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlParserInputBufferCreateFilenameDefault
extern __typeof (xmlParserInputBufferCreateFilenameDefault) xmlParserInputBufferCreateFilenameDefault __attribute((alias("xmlParserInputBufferCreateFilenameDefault__internal_alias")));
#else
#ifndef xmlParserInputBufferCreateFilenameDefault
extern __typeof (xmlParserInputBufferCreateFilenameDefault) xmlParserInputBufferCreateFilenameDefault__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputBufferCreateFilenameDefault xmlParserInputBufferCreateFilenameDefault__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlParserInputBufferCreateIO
extern __typeof (xmlParserInputBufferCreateIO) xmlParserInputBufferCreateIO __attribute((alias("xmlParserInputBufferCreateIO__internal_alias")));
#else
#ifndef xmlParserInputBufferCreateIO
extern __typeof (xmlParserInputBufferCreateIO) xmlParserInputBufferCreateIO__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputBufferCreateIO xmlParserInputBufferCreateIO__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlParserInputBufferCreateMem
extern __typeof (xmlParserInputBufferCreateMem) xmlParserInputBufferCreateMem __attribute((alias("xmlParserInputBufferCreateMem__internal_alias")));
#else
#ifndef xmlParserInputBufferCreateMem
extern __typeof (xmlParserInputBufferCreateMem) xmlParserInputBufferCreateMem__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputBufferCreateMem xmlParserInputBufferCreateMem__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlParserInputBufferCreateStatic
extern __typeof (xmlParserInputBufferCreateStatic) xmlParserInputBufferCreateStatic __attribute((alias("xmlParserInputBufferCreateStatic__internal_alias")));
#else
#ifndef xmlParserInputBufferCreateStatic
extern __typeof (xmlParserInputBufferCreateStatic) xmlParserInputBufferCreateStatic__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputBufferCreateStatic xmlParserInputBufferCreateStatic__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlParserInputBufferGrow
extern __typeof (xmlParserInputBufferGrow) xmlParserInputBufferGrow __attribute((alias("xmlParserInputBufferGrow__internal_alias")));
#else
#ifndef xmlParserInputBufferGrow
extern __typeof (xmlParserInputBufferGrow) xmlParserInputBufferGrow__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputBufferGrow xmlParserInputBufferGrow__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlParserInputBufferPush
extern __typeof (xmlParserInputBufferPush) xmlParserInputBufferPush __attribute((alias("xmlParserInputBufferPush__internal_alias")));
#else
#ifndef xmlParserInputBufferPush
extern __typeof (xmlParserInputBufferPush) xmlParserInputBufferPush__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputBufferPush xmlParserInputBufferPush__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlParserInputBufferRead
extern __typeof (xmlParserInputBufferRead) xmlParserInputBufferRead __attribute((alias("xmlParserInputBufferRead__internal_alias")));
#else
#ifndef xmlParserInputBufferRead
extern __typeof (xmlParserInputBufferRead) xmlParserInputBufferRead__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputBufferRead xmlParserInputBufferRead__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlParserInputGrow
extern __typeof (xmlParserInputGrow) xmlParserInputGrow __attribute((alias("xmlParserInputGrow__internal_alias")));
#else
#ifndef xmlParserInputGrow
extern __typeof (xmlParserInputGrow) xmlParserInputGrow__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputGrow xmlParserInputGrow__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlParserInputRead
extern __typeof (xmlParserInputRead) xmlParserInputRead __attribute((alias("xmlParserInputRead__internal_alias")));
#else
#ifndef xmlParserInputRead
extern __typeof (xmlParserInputRead) xmlParserInputRead__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputRead xmlParserInputRead__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlParserInputShrink
extern __typeof (xmlParserInputShrink) xmlParserInputShrink __attribute((alias("xmlParserInputShrink__internal_alias")));
#else
#ifndef xmlParserInputShrink
extern __typeof (xmlParserInputShrink) xmlParserInputShrink__internal_alias __attribute((visibility("hidden")));
#define xmlParserInputShrink xmlParserInputShrink__internal_alias
#endif
#endif

#ifdef bottom_error
#undef xmlParserPrintFileContext
extern __typeof (xmlParserPrintFileContext) xmlParserPrintFileContext __attribute((alias("xmlParserPrintFileContext__internal_alias")));
#else
#ifndef xmlParserPrintFileContext
extern __typeof (xmlParserPrintFileContext) xmlParserPrintFileContext__internal_alias __attribute((visibility("hidden")));
#define xmlParserPrintFileContext xmlParserPrintFileContext__internal_alias
#endif
#endif

#ifdef bottom_error
#undef xmlParserPrintFileInfo
extern __typeof (xmlParserPrintFileInfo) xmlParserPrintFileInfo __attribute((alias("xmlParserPrintFileInfo__internal_alias")));
#else
#ifndef xmlParserPrintFileInfo
extern __typeof (xmlParserPrintFileInfo) xmlParserPrintFileInfo__internal_alias __attribute((visibility("hidden")));
#define xmlParserPrintFileInfo xmlParserPrintFileInfo__internal_alias
#endif
#endif

#ifdef bottom_error
#undef xmlParserValidityError
extern __typeof (xmlParserValidityError) xmlParserValidityError __attribute((alias("xmlParserValidityError__internal_alias")));
#else
#ifndef xmlParserValidityError
extern __typeof (xmlParserValidityError) xmlParserValidityError__internal_alias __attribute((visibility("hidden")));
#define xmlParserValidityError xmlParserValidityError__internal_alias
#endif
#endif

#ifdef bottom_error
#undef xmlParserValidityWarning
extern __typeof (xmlParserValidityWarning) xmlParserValidityWarning __attribute((alias("xmlParserValidityWarning__internal_alias")));
#else
#ifndef xmlParserValidityWarning
extern __typeof (xmlParserValidityWarning) xmlParserValidityWarning__internal_alias __attribute((visibility("hidden")));
#define xmlParserValidityWarning xmlParserValidityWarning__internal_alias
#endif
#endif

#ifdef bottom_error
#undef xmlParserWarning
extern __typeof (xmlParserWarning) xmlParserWarning __attribute((alias("xmlParserWarning__internal_alias")));
#else
#ifndef xmlParserWarning
extern __typeof (xmlParserWarning) xmlParserWarning__internal_alias __attribute((visibility("hidden")));
#define xmlParserWarning xmlParserWarning__internal_alias
#endif
#endif

#if defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_pattern
#undef xmlPatternFromRoot
extern __typeof (xmlPatternFromRoot) xmlPatternFromRoot __attribute((alias("xmlPatternFromRoot__internal_alias")));
#else
#ifndef xmlPatternFromRoot
extern __typeof (xmlPatternFromRoot) xmlPatternFromRoot__internal_alias __attribute((visibility("hidden")));
#define xmlPatternFromRoot xmlPatternFromRoot__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_pattern
#undef xmlPatternGetStreamCtxt
extern __typeof (xmlPatternGetStreamCtxt) xmlPatternGetStreamCtxt __attribute((alias("xmlPatternGetStreamCtxt__internal_alias")));
#else
#ifndef xmlPatternGetStreamCtxt
extern __typeof (xmlPatternGetStreamCtxt) xmlPatternGetStreamCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlPatternGetStreamCtxt xmlPatternGetStreamCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_pattern
#undef xmlPatternMatch
extern __typeof (xmlPatternMatch) xmlPatternMatch __attribute((alias("xmlPatternMatch__internal_alias")));
#else
#ifndef xmlPatternMatch
extern __typeof (xmlPatternMatch) xmlPatternMatch__internal_alias __attribute((visibility("hidden")));
#define xmlPatternMatch xmlPatternMatch__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_pattern
#undef xmlPatternMaxDepth
extern __typeof (xmlPatternMaxDepth) xmlPatternMaxDepth __attribute((alias("xmlPatternMaxDepth__internal_alias")));
#else
#ifndef xmlPatternMaxDepth
extern __typeof (xmlPatternMaxDepth) xmlPatternMaxDepth__internal_alias __attribute((visibility("hidden")));
#define xmlPatternMaxDepth xmlPatternMaxDepth__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_pattern
#undef xmlPatternStreamable
extern __typeof (xmlPatternStreamable) xmlPatternStreamable __attribute((alias("xmlPatternStreamable__internal_alias")));
#else
#ifndef xmlPatternStreamable
extern __typeof (xmlPatternStreamable) xmlPatternStreamable__internal_alias __attribute((visibility("hidden")));
#define xmlPatternStreamable xmlPatternStreamable__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_pattern
#undef xmlPatterncompile
extern __typeof (xmlPatterncompile) xmlPatterncompile __attribute((alias("xmlPatterncompile__internal_alias")));
#else
#ifndef xmlPatterncompile
extern __typeof (xmlPatterncompile) xmlPatterncompile__internal_alias __attribute((visibility("hidden")));
#define xmlPatterncompile xmlPatterncompile__internal_alias
#endif
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlPedanticParserDefault
extern __typeof (xmlPedanticParserDefault) xmlPedanticParserDefault __attribute((alias("xmlPedanticParserDefault__internal_alias")));
#else
#ifndef xmlPedanticParserDefault
extern __typeof (xmlPedanticParserDefault) xmlPedanticParserDefault__internal_alias __attribute((visibility("hidden")));
#define xmlPedanticParserDefault xmlPedanticParserDefault__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlPopInput
extern __typeof (xmlPopInput) xmlPopInput __attribute((alias("xmlPopInput__internal_alias")));
#else
#ifndef xmlPopInput
extern __typeof (xmlPopInput) xmlPopInput__internal_alias __attribute((visibility("hidden")));
#define xmlPopInput xmlPopInput__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlPopInputCallbacks
extern __typeof (xmlPopInputCallbacks) xmlPopInputCallbacks __attribute((alias("xmlPopInputCallbacks__internal_alias")));
#else
#ifndef xmlPopInputCallbacks
extern __typeof (xmlPopInputCallbacks) xmlPopInputCallbacks__internal_alias __attribute((visibility("hidden")));
#define xmlPopInputCallbacks xmlPopInputCallbacks__internal_alias
#endif
#endif

#ifdef bottom_uri
#undef xmlPrintURI
extern __typeof (xmlPrintURI) xmlPrintURI __attribute((alias("xmlPrintURI__internal_alias")));
#else
#ifndef xmlPrintURI
extern __typeof (xmlPrintURI) xmlPrintURI__internal_alias __attribute((visibility("hidden")));
#define xmlPrintURI xmlPrintURI__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlPushInput
extern __typeof (xmlPushInput) xmlPushInput __attribute((alias("xmlPushInput__internal_alias")));
#else
#ifndef xmlPushInput
extern __typeof (xmlPushInput) xmlPushInput__internal_alias __attribute((visibility("hidden")));
#define xmlPushInput xmlPushInput__internal_alias
#endif
#endif

#ifdef bottom_threads
#undef xmlRMutexLock
extern __typeof (xmlRMutexLock) xmlRMutexLock __attribute((alias("xmlRMutexLock__internal_alias")));
#else
#ifndef xmlRMutexLock
extern __typeof (xmlRMutexLock) xmlRMutexLock__internal_alias __attribute((visibility("hidden")));
#define xmlRMutexLock xmlRMutexLock__internal_alias
#endif
#endif

#ifdef bottom_threads
#undef xmlRMutexUnlock
extern __typeof (xmlRMutexUnlock) xmlRMutexUnlock __attribute((alias("xmlRMutexUnlock__internal_alias")));
#else
#ifndef xmlRMutexUnlock
extern __typeof (xmlRMutexUnlock) xmlRMutexUnlock__internal_alias __attribute((visibility("hidden")));
#define xmlRMutexUnlock xmlRMutexUnlock__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlReadDoc
extern __typeof (xmlReadDoc) xmlReadDoc __attribute((alias("xmlReadDoc__internal_alias")));
#else
#ifndef xmlReadDoc
extern __typeof (xmlReadDoc) xmlReadDoc__internal_alias __attribute((visibility("hidden")));
#define xmlReadDoc xmlReadDoc__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlReadFd
extern __typeof (xmlReadFd) xmlReadFd __attribute((alias("xmlReadFd__internal_alias")));
#else
#ifndef xmlReadFd
extern __typeof (xmlReadFd) xmlReadFd__internal_alias __attribute((visibility("hidden")));
#define xmlReadFd xmlReadFd__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlReadFile
extern __typeof (xmlReadFile) xmlReadFile __attribute((alias("xmlReadFile__internal_alias")));
#else
#ifndef xmlReadFile
extern __typeof (xmlReadFile) xmlReadFile__internal_alias __attribute((visibility("hidden")));
#define xmlReadFile xmlReadFile__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlReadIO
extern __typeof (xmlReadIO) xmlReadIO __attribute((alias("xmlReadIO__internal_alias")));
#else
#ifndef xmlReadIO
extern __typeof (xmlReadIO) xmlReadIO__internal_alias __attribute((visibility("hidden")));
#define xmlReadIO xmlReadIO__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlReadMemory
extern __typeof (xmlReadMemory) xmlReadMemory __attribute((alias("xmlReadMemory__internal_alias")));
#else
#ifndef xmlReadMemory
extern __typeof (xmlReadMemory) xmlReadMemory__internal_alias __attribute((visibility("hidden")));
#define xmlReadMemory xmlReadMemory__internal_alias
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlReaderForDoc
extern __typeof (xmlReaderForDoc) xmlReaderForDoc __attribute((alias("xmlReaderForDoc__internal_alias")));
#else
#ifndef xmlReaderForDoc
extern __typeof (xmlReaderForDoc) xmlReaderForDoc__internal_alias __attribute((visibility("hidden")));
#define xmlReaderForDoc xmlReaderForDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlReaderForFd
extern __typeof (xmlReaderForFd) xmlReaderForFd __attribute((alias("xmlReaderForFd__internal_alias")));
#else
#ifndef xmlReaderForFd
extern __typeof (xmlReaderForFd) xmlReaderForFd__internal_alias __attribute((visibility("hidden")));
#define xmlReaderForFd xmlReaderForFd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlReaderForFile
extern __typeof (xmlReaderForFile) xmlReaderForFile __attribute((alias("xmlReaderForFile__internal_alias")));
#else
#ifndef xmlReaderForFile
extern __typeof (xmlReaderForFile) xmlReaderForFile__internal_alias __attribute((visibility("hidden")));
#define xmlReaderForFile xmlReaderForFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlReaderForIO
extern __typeof (xmlReaderForIO) xmlReaderForIO __attribute((alias("xmlReaderForIO__internal_alias")));
#else
#ifndef xmlReaderForIO
extern __typeof (xmlReaderForIO) xmlReaderForIO__internal_alias __attribute((visibility("hidden")));
#define xmlReaderForIO xmlReaderForIO__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlReaderForMemory
extern __typeof (xmlReaderForMemory) xmlReaderForMemory __attribute((alias("xmlReaderForMemory__internal_alias")));
#else
#ifndef xmlReaderForMemory
extern __typeof (xmlReaderForMemory) xmlReaderForMemory__internal_alias __attribute((visibility("hidden")));
#define xmlReaderForMemory xmlReaderForMemory__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlReaderNewDoc
extern __typeof (xmlReaderNewDoc) xmlReaderNewDoc __attribute((alias("xmlReaderNewDoc__internal_alias")));
#else
#ifndef xmlReaderNewDoc
extern __typeof (xmlReaderNewDoc) xmlReaderNewDoc__internal_alias __attribute((visibility("hidden")));
#define xmlReaderNewDoc xmlReaderNewDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlReaderNewFd
extern __typeof (xmlReaderNewFd) xmlReaderNewFd __attribute((alias("xmlReaderNewFd__internal_alias")));
#else
#ifndef xmlReaderNewFd
extern __typeof (xmlReaderNewFd) xmlReaderNewFd__internal_alias __attribute((visibility("hidden")));
#define xmlReaderNewFd xmlReaderNewFd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlReaderNewFile
extern __typeof (xmlReaderNewFile) xmlReaderNewFile __attribute((alias("xmlReaderNewFile__internal_alias")));
#else
#ifndef xmlReaderNewFile
extern __typeof (xmlReaderNewFile) xmlReaderNewFile__internal_alias __attribute((visibility("hidden")));
#define xmlReaderNewFile xmlReaderNewFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlReaderNewIO
extern __typeof (xmlReaderNewIO) xmlReaderNewIO __attribute((alias("xmlReaderNewIO__internal_alias")));
#else
#ifndef xmlReaderNewIO
extern __typeof (xmlReaderNewIO) xmlReaderNewIO__internal_alias __attribute((visibility("hidden")));
#define xmlReaderNewIO xmlReaderNewIO__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlReaderNewMemory
extern __typeof (xmlReaderNewMemory) xmlReaderNewMemory __attribute((alias("xmlReaderNewMemory__internal_alias")));
#else
#ifndef xmlReaderNewMemory
extern __typeof (xmlReaderNewMemory) xmlReaderNewMemory__internal_alias __attribute((visibility("hidden")));
#define xmlReaderNewMemory xmlReaderNewMemory__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlReaderNewWalker
extern __typeof (xmlReaderNewWalker) xmlReaderNewWalker __attribute((alias("xmlReaderNewWalker__internal_alias")));
#else
#ifndef xmlReaderNewWalker
extern __typeof (xmlReaderNewWalker) xmlReaderNewWalker__internal_alias __attribute((visibility("hidden")));
#define xmlReaderNewWalker xmlReaderNewWalker__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlReaderWalker
extern __typeof (xmlReaderWalker) xmlReaderWalker __attribute((alias("xmlReaderWalker__internal_alias")));
#else
#ifndef xmlReaderWalker
extern __typeof (xmlReaderWalker) xmlReaderWalker__internal_alias __attribute((visibility("hidden")));
#define xmlReaderWalker xmlReaderWalker__internal_alias
#endif
#endif
#endif

#ifdef bottom_xmlmemory
#undef xmlReallocLoc
extern __typeof (xmlReallocLoc) xmlReallocLoc __attribute((alias("xmlReallocLoc__internal_alias")));
#else
#ifndef xmlReallocLoc
extern __typeof (xmlReallocLoc) xmlReallocLoc__internal_alias __attribute((visibility("hidden")));
#define xmlReallocLoc xmlReallocLoc__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_tree
#undef xmlReconciliateNs
extern __typeof (xmlReconciliateNs) xmlReconciliateNs __attribute((alias("xmlReconciliateNs__internal_alias")));
#else
#ifndef xmlReconciliateNs
extern __typeof (xmlReconciliateNs) xmlReconciliateNs__internal_alias __attribute((visibility("hidden")));
#define xmlReconciliateNs xmlReconciliateNs__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlRecoverDoc
extern __typeof (xmlRecoverDoc) xmlRecoverDoc __attribute((alias("xmlRecoverDoc__internal_alias")));
#else
#ifndef xmlRecoverDoc
extern __typeof (xmlRecoverDoc) xmlRecoverDoc__internal_alias __attribute((visibility("hidden")));
#define xmlRecoverDoc xmlRecoverDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlRecoverFile
extern __typeof (xmlRecoverFile) xmlRecoverFile __attribute((alias("xmlRecoverFile__internal_alias")));
#else
#ifndef xmlRecoverFile
extern __typeof (xmlRecoverFile) xmlRecoverFile__internal_alias __attribute((visibility("hidden")));
#define xmlRecoverFile xmlRecoverFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlRecoverMemory
extern __typeof (xmlRecoverMemory) xmlRecoverMemory __attribute((alias("xmlRecoverMemory__internal_alias")));
#else
#ifndef xmlRecoverMemory
extern __typeof (xmlRecoverMemory) xmlRecoverMemory__internal_alias __attribute((visibility("hidden")));
#define xmlRecoverMemory xmlRecoverMemory__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlRegExecErrInfo
extern __typeof (xmlRegExecErrInfo) xmlRegExecErrInfo __attribute((alias("xmlRegExecErrInfo__internal_alias")));
#else
#ifndef xmlRegExecErrInfo
extern __typeof (xmlRegExecErrInfo) xmlRegExecErrInfo__internal_alias __attribute((visibility("hidden")));
#define xmlRegExecErrInfo xmlRegExecErrInfo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlRegExecNextValues
extern __typeof (xmlRegExecNextValues) xmlRegExecNextValues __attribute((alias("xmlRegExecNextValues__internal_alias")));
#else
#ifndef xmlRegExecNextValues
extern __typeof (xmlRegExecNextValues) xmlRegExecNextValues__internal_alias __attribute((visibility("hidden")));
#define xmlRegExecNextValues xmlRegExecNextValues__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlRegExecPushString
extern __typeof (xmlRegExecPushString) xmlRegExecPushString __attribute((alias("xmlRegExecPushString__internal_alias")));
#else
#ifndef xmlRegExecPushString
extern __typeof (xmlRegExecPushString) xmlRegExecPushString__internal_alias __attribute((visibility("hidden")));
#define xmlRegExecPushString xmlRegExecPushString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlRegExecPushString2
extern __typeof (xmlRegExecPushString2) xmlRegExecPushString2 __attribute((alias("xmlRegExecPushString2__internal_alias")));
#else
#ifndef xmlRegExecPushString2
extern __typeof (xmlRegExecPushString2) xmlRegExecPushString2__internal_alias __attribute((visibility("hidden")));
#define xmlRegExecPushString2 xmlRegExecPushString2__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlRegFreeExecCtxt
extern __typeof (xmlRegFreeExecCtxt) xmlRegFreeExecCtxt __attribute((alias("xmlRegFreeExecCtxt__internal_alias")));
#else
#ifndef xmlRegFreeExecCtxt
extern __typeof (xmlRegFreeExecCtxt) xmlRegFreeExecCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlRegFreeExecCtxt xmlRegFreeExecCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlRegFreeRegexp
extern __typeof (xmlRegFreeRegexp) xmlRegFreeRegexp __attribute((alias("xmlRegFreeRegexp__internal_alias")));
#else
#ifndef xmlRegFreeRegexp
extern __typeof (xmlRegFreeRegexp) xmlRegFreeRegexp__internal_alias __attribute((visibility("hidden")));
#define xmlRegFreeRegexp xmlRegFreeRegexp__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlRegNewExecCtxt
extern __typeof (xmlRegNewExecCtxt) xmlRegNewExecCtxt __attribute((alias("xmlRegNewExecCtxt__internal_alias")));
#else
#ifndef xmlRegNewExecCtxt
extern __typeof (xmlRegNewExecCtxt) xmlRegNewExecCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlRegNewExecCtxt xmlRegNewExecCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlRegexpCompile
extern __typeof (xmlRegexpCompile) xmlRegexpCompile __attribute((alias("xmlRegexpCompile__internal_alias")));
#else
#ifndef xmlRegexpCompile
extern __typeof (xmlRegexpCompile) xmlRegexpCompile__internal_alias __attribute((visibility("hidden")));
#define xmlRegexpCompile xmlRegexpCompile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlRegexpExec
extern __typeof (xmlRegexpExec) xmlRegexpExec __attribute((alias("xmlRegexpExec__internal_alias")));
#else
#ifndef xmlRegexpExec
extern __typeof (xmlRegexpExec) xmlRegexpExec__internal_alias __attribute((visibility("hidden")));
#define xmlRegexpExec xmlRegexpExec__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlRegexpIsDeterminist
extern __typeof (xmlRegexpIsDeterminist) xmlRegexpIsDeterminist __attribute((alias("xmlRegexpIsDeterminist__internal_alias")));
#else
#ifndef xmlRegexpIsDeterminist
extern __typeof (xmlRegexpIsDeterminist) xmlRegexpIsDeterminist__internal_alias __attribute((visibility("hidden")));
#define xmlRegexpIsDeterminist xmlRegexpIsDeterminist__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_xmlregexp
#undef xmlRegexpPrint
extern __typeof (xmlRegexpPrint) xmlRegexpPrint __attribute((alias("xmlRegexpPrint__internal_alias")));
#else
#ifndef xmlRegexpPrint
extern __typeof (xmlRegexpPrint) xmlRegexpPrint__internal_alias __attribute((visibility("hidden")));
#define xmlRegexpPrint xmlRegexpPrint__internal_alias
#endif
#endif
#endif

#ifdef bottom_encoding
#undef xmlRegisterCharEncodingHandler
extern __typeof (xmlRegisterCharEncodingHandler) xmlRegisterCharEncodingHandler __attribute((alias("xmlRegisterCharEncodingHandler__internal_alias")));
#else
#ifndef xmlRegisterCharEncodingHandler
extern __typeof (xmlRegisterCharEncodingHandler) xmlRegisterCharEncodingHandler__internal_alias __attribute((visibility("hidden")));
#define xmlRegisterCharEncodingHandler xmlRegisterCharEncodingHandler__internal_alias
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlRegisterDefaultInputCallbacks
extern __typeof (xmlRegisterDefaultInputCallbacks) xmlRegisterDefaultInputCallbacks __attribute((alias("xmlRegisterDefaultInputCallbacks__internal_alias")));
#else
#ifndef xmlRegisterDefaultInputCallbacks
extern __typeof (xmlRegisterDefaultInputCallbacks) xmlRegisterDefaultInputCallbacks__internal_alias __attribute((visibility("hidden")));
#define xmlRegisterDefaultInputCallbacks xmlRegisterDefaultInputCallbacks__internal_alias
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlRegisterDefaultOutputCallbacks
extern __typeof (xmlRegisterDefaultOutputCallbacks) xmlRegisterDefaultOutputCallbacks __attribute((alias("xmlRegisterDefaultOutputCallbacks__internal_alias")));
#else
#ifndef xmlRegisterDefaultOutputCallbacks
extern __typeof (xmlRegisterDefaultOutputCallbacks) xmlRegisterDefaultOutputCallbacks__internal_alias __attribute((visibility("hidden")));
#define xmlRegisterDefaultOutputCallbacks xmlRegisterDefaultOutputCallbacks__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED) && defined(LIBXML_HTTP_ENABLED)
#ifdef bottom_xmlIO
#undef xmlRegisterHTTPPostCallbacks
extern __typeof (xmlRegisterHTTPPostCallbacks) xmlRegisterHTTPPostCallbacks __attribute((alias("xmlRegisterHTTPPostCallbacks__internal_alias")));
#else
#ifndef xmlRegisterHTTPPostCallbacks
extern __typeof (xmlRegisterHTTPPostCallbacks) xmlRegisterHTTPPostCallbacks__internal_alias __attribute((visibility("hidden")));
#define xmlRegisterHTTPPostCallbacks xmlRegisterHTTPPostCallbacks__internal_alias
#endif
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlRegisterInputCallbacks
extern __typeof (xmlRegisterInputCallbacks) xmlRegisterInputCallbacks __attribute((alias("xmlRegisterInputCallbacks__internal_alias")));
#else
#ifndef xmlRegisterInputCallbacks
extern __typeof (xmlRegisterInputCallbacks) xmlRegisterInputCallbacks__internal_alias __attribute((visibility("hidden")));
#define xmlRegisterInputCallbacks xmlRegisterInputCallbacks__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlRegisterNodeDefault
extern __typeof (xmlRegisterNodeDefault) xmlRegisterNodeDefault __attribute((alias("xmlRegisterNodeDefault__internal_alias")));
#else
#ifndef xmlRegisterNodeDefault
extern __typeof (xmlRegisterNodeDefault) xmlRegisterNodeDefault__internal_alias __attribute((visibility("hidden")));
#define xmlRegisterNodeDefault xmlRegisterNodeDefault__internal_alias
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlIO
#undef xmlRegisterOutputCallbacks
extern __typeof (xmlRegisterOutputCallbacks) xmlRegisterOutputCallbacks __attribute((alias("xmlRegisterOutputCallbacks__internal_alias")));
#else
#ifndef xmlRegisterOutputCallbacks
extern __typeof (xmlRegisterOutputCallbacks) xmlRegisterOutputCallbacks__internal_alias __attribute((visibility("hidden")));
#define xmlRegisterOutputCallbacks xmlRegisterOutputCallbacks__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGCleanupTypes
extern __typeof (xmlRelaxNGCleanupTypes) xmlRelaxNGCleanupTypes __attribute((alias("xmlRelaxNGCleanupTypes__internal_alias")));
#else
#ifndef xmlRelaxNGCleanupTypes
extern __typeof (xmlRelaxNGCleanupTypes) xmlRelaxNGCleanupTypes__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGCleanupTypes xmlRelaxNGCleanupTypes__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGDump
extern __typeof (xmlRelaxNGDump) xmlRelaxNGDump __attribute((alias("xmlRelaxNGDump__internal_alias")));
#else
#ifndef xmlRelaxNGDump
extern __typeof (xmlRelaxNGDump) xmlRelaxNGDump__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGDump xmlRelaxNGDump__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGDumpTree
extern __typeof (xmlRelaxNGDumpTree) xmlRelaxNGDumpTree __attribute((alias("xmlRelaxNGDumpTree__internal_alias")));
#else
#ifndef xmlRelaxNGDumpTree
extern __typeof (xmlRelaxNGDumpTree) xmlRelaxNGDumpTree__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGDumpTree xmlRelaxNGDumpTree__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGFree
extern __typeof (xmlRelaxNGFree) xmlRelaxNGFree __attribute((alias("xmlRelaxNGFree__internal_alias")));
#else
#ifndef xmlRelaxNGFree
extern __typeof (xmlRelaxNGFree) xmlRelaxNGFree__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGFree xmlRelaxNGFree__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGFreeParserCtxt
extern __typeof (xmlRelaxNGFreeParserCtxt) xmlRelaxNGFreeParserCtxt __attribute((alias("xmlRelaxNGFreeParserCtxt__internal_alias")));
#else
#ifndef xmlRelaxNGFreeParserCtxt
extern __typeof (xmlRelaxNGFreeParserCtxt) xmlRelaxNGFreeParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGFreeParserCtxt xmlRelaxNGFreeParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGFreeValidCtxt
extern __typeof (xmlRelaxNGFreeValidCtxt) xmlRelaxNGFreeValidCtxt __attribute((alias("xmlRelaxNGFreeValidCtxt__internal_alias")));
#else
#ifndef xmlRelaxNGFreeValidCtxt
extern __typeof (xmlRelaxNGFreeValidCtxt) xmlRelaxNGFreeValidCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGFreeValidCtxt xmlRelaxNGFreeValidCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGGetParserErrors
extern __typeof (xmlRelaxNGGetParserErrors) xmlRelaxNGGetParserErrors __attribute((alias("xmlRelaxNGGetParserErrors__internal_alias")));
#else
#ifndef xmlRelaxNGGetParserErrors
extern __typeof (xmlRelaxNGGetParserErrors) xmlRelaxNGGetParserErrors__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGGetParserErrors xmlRelaxNGGetParserErrors__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGGetValidErrors
extern __typeof (xmlRelaxNGGetValidErrors) xmlRelaxNGGetValidErrors __attribute((alias("xmlRelaxNGGetValidErrors__internal_alias")));
#else
#ifndef xmlRelaxNGGetValidErrors
extern __typeof (xmlRelaxNGGetValidErrors) xmlRelaxNGGetValidErrors__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGGetValidErrors xmlRelaxNGGetValidErrors__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGInitTypes
extern __typeof (xmlRelaxNGInitTypes) xmlRelaxNGInitTypes __attribute((alias("xmlRelaxNGInitTypes__internal_alias")));
#else
#ifndef xmlRelaxNGInitTypes
extern __typeof (xmlRelaxNGInitTypes) xmlRelaxNGInitTypes__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGInitTypes xmlRelaxNGInitTypes__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGNewDocParserCtxt
extern __typeof (xmlRelaxNGNewDocParserCtxt) xmlRelaxNGNewDocParserCtxt __attribute((alias("xmlRelaxNGNewDocParserCtxt__internal_alias")));
#else
#ifndef xmlRelaxNGNewDocParserCtxt
extern __typeof (xmlRelaxNGNewDocParserCtxt) xmlRelaxNGNewDocParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGNewDocParserCtxt xmlRelaxNGNewDocParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGNewMemParserCtxt
extern __typeof (xmlRelaxNGNewMemParserCtxt) xmlRelaxNGNewMemParserCtxt __attribute((alias("xmlRelaxNGNewMemParserCtxt__internal_alias")));
#else
#ifndef xmlRelaxNGNewMemParserCtxt
extern __typeof (xmlRelaxNGNewMemParserCtxt) xmlRelaxNGNewMemParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGNewMemParserCtxt xmlRelaxNGNewMemParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGNewParserCtxt
extern __typeof (xmlRelaxNGNewParserCtxt) xmlRelaxNGNewParserCtxt __attribute((alias("xmlRelaxNGNewParserCtxt__internal_alias")));
#else
#ifndef xmlRelaxNGNewParserCtxt
extern __typeof (xmlRelaxNGNewParserCtxt) xmlRelaxNGNewParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGNewParserCtxt xmlRelaxNGNewParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGNewValidCtxt
extern __typeof (xmlRelaxNGNewValidCtxt) xmlRelaxNGNewValidCtxt __attribute((alias("xmlRelaxNGNewValidCtxt__internal_alias")));
#else
#ifndef xmlRelaxNGNewValidCtxt
extern __typeof (xmlRelaxNGNewValidCtxt) xmlRelaxNGNewValidCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGNewValidCtxt xmlRelaxNGNewValidCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGParse
extern __typeof (xmlRelaxNGParse) xmlRelaxNGParse __attribute((alias("xmlRelaxNGParse__internal_alias")));
#else
#ifndef xmlRelaxNGParse
extern __typeof (xmlRelaxNGParse) xmlRelaxNGParse__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGParse xmlRelaxNGParse__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGSetParserErrors
extern __typeof (xmlRelaxNGSetParserErrors) xmlRelaxNGSetParserErrors __attribute((alias("xmlRelaxNGSetParserErrors__internal_alias")));
#else
#ifndef xmlRelaxNGSetParserErrors
extern __typeof (xmlRelaxNGSetParserErrors) xmlRelaxNGSetParserErrors__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGSetParserErrors xmlRelaxNGSetParserErrors__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGSetValidErrors
extern __typeof (xmlRelaxNGSetValidErrors) xmlRelaxNGSetValidErrors __attribute((alias("xmlRelaxNGSetValidErrors__internal_alias")));
#else
#ifndef xmlRelaxNGSetValidErrors
extern __typeof (xmlRelaxNGSetValidErrors) xmlRelaxNGSetValidErrors__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGSetValidErrors xmlRelaxNGSetValidErrors__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGValidateDoc
extern __typeof (xmlRelaxNGValidateDoc) xmlRelaxNGValidateDoc __attribute((alias("xmlRelaxNGValidateDoc__internal_alias")));
#else
#ifndef xmlRelaxNGValidateDoc
extern __typeof (xmlRelaxNGValidateDoc) xmlRelaxNGValidateDoc__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGValidateDoc xmlRelaxNGValidateDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGValidateFullElement
extern __typeof (xmlRelaxNGValidateFullElement) xmlRelaxNGValidateFullElement __attribute((alias("xmlRelaxNGValidateFullElement__internal_alias")));
#else
#ifndef xmlRelaxNGValidateFullElement
extern __typeof (xmlRelaxNGValidateFullElement) xmlRelaxNGValidateFullElement__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGValidateFullElement xmlRelaxNGValidateFullElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGValidatePopElement
extern __typeof (xmlRelaxNGValidatePopElement) xmlRelaxNGValidatePopElement __attribute((alias("xmlRelaxNGValidatePopElement__internal_alias")));
#else
#ifndef xmlRelaxNGValidatePopElement
extern __typeof (xmlRelaxNGValidatePopElement) xmlRelaxNGValidatePopElement__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGValidatePopElement xmlRelaxNGValidatePopElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGValidatePushCData
extern __typeof (xmlRelaxNGValidatePushCData) xmlRelaxNGValidatePushCData __attribute((alias("xmlRelaxNGValidatePushCData__internal_alias")));
#else
#ifndef xmlRelaxNGValidatePushCData
extern __typeof (xmlRelaxNGValidatePushCData) xmlRelaxNGValidatePushCData__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGValidatePushCData xmlRelaxNGValidatePushCData__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxNGValidatePushElement
extern __typeof (xmlRelaxNGValidatePushElement) xmlRelaxNGValidatePushElement __attribute((alias("xmlRelaxNGValidatePushElement__internal_alias")));
#else
#ifndef xmlRelaxNGValidatePushElement
extern __typeof (xmlRelaxNGValidatePushElement) xmlRelaxNGValidatePushElement__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxNGValidatePushElement xmlRelaxNGValidatePushElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_relaxng
#undef xmlRelaxParserSetFlag
extern __typeof (xmlRelaxParserSetFlag) xmlRelaxParserSetFlag __attribute((alias("xmlRelaxParserSetFlag__internal_alias")));
#else
#ifndef xmlRelaxParserSetFlag
extern __typeof (xmlRelaxParserSetFlag) xmlRelaxParserSetFlag__internal_alias __attribute((visibility("hidden")));
#define xmlRelaxParserSetFlag xmlRelaxParserSetFlag__internal_alias
#endif
#endif
#endif

#ifdef bottom_valid
#undef xmlRemoveID
extern __typeof (xmlRemoveID) xmlRemoveID __attribute((alias("xmlRemoveID__internal_alias")));
#else
#ifndef xmlRemoveID
extern __typeof (xmlRemoveID) xmlRemoveID__internal_alias __attribute((visibility("hidden")));
#define xmlRemoveID xmlRemoveID__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED)
#ifdef bottom_tree
#undef xmlRemoveProp
extern __typeof (xmlRemoveProp) xmlRemoveProp __attribute((alias("xmlRemoveProp__internal_alias")));
#else
#ifndef xmlRemoveProp
extern __typeof (xmlRemoveProp) xmlRemoveProp__internal_alias __attribute((visibility("hidden")));
#define xmlRemoveProp xmlRemoveProp__internal_alias
#endif
#endif
#endif

#ifdef bottom_valid
#undef xmlRemoveRef
extern __typeof (xmlRemoveRef) xmlRemoveRef __attribute((alias("xmlRemoveRef__internal_alias")));
#else
#ifndef xmlRemoveRef
extern __typeof (xmlRemoveRef) xmlRemoveRef__internal_alias __attribute((visibility("hidden")));
#define xmlRemoveRef xmlRemoveRef__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_tree
#undef xmlReplaceNode
extern __typeof (xmlReplaceNode) xmlReplaceNode __attribute((alias("xmlReplaceNode__internal_alias")));
#else
#ifndef xmlReplaceNode
extern __typeof (xmlReplaceNode) xmlReplaceNode__internal_alias __attribute((visibility("hidden")));
#define xmlReplaceNode xmlReplaceNode__internal_alias
#endif
#endif
#endif

#ifdef bottom_error
#undef xmlResetError
extern __typeof (xmlResetError) xmlResetError __attribute((alias("xmlResetError__internal_alias")));
#else
#ifndef xmlResetError
extern __typeof (xmlResetError) xmlResetError__internal_alias __attribute((visibility("hidden")));
#define xmlResetError xmlResetError__internal_alias
#endif
#endif

#ifdef bottom_error
#undef xmlResetLastError
extern __typeof (xmlResetLastError) xmlResetLastError __attribute((alias("xmlResetLastError__internal_alias")));
#else
#ifndef xmlResetLastError
extern __typeof (xmlResetLastError) xmlResetLastError__internal_alias __attribute((visibility("hidden")));
#define xmlResetLastError xmlResetLastError__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2AttributeDecl
extern __typeof (xmlSAX2AttributeDecl) xmlSAX2AttributeDecl __attribute((alias("xmlSAX2AttributeDecl__internal_alias")));
#else
#ifndef xmlSAX2AttributeDecl
extern __typeof (xmlSAX2AttributeDecl) xmlSAX2AttributeDecl__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2AttributeDecl xmlSAX2AttributeDecl__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2CDataBlock
extern __typeof (xmlSAX2CDataBlock) xmlSAX2CDataBlock __attribute((alias("xmlSAX2CDataBlock__internal_alias")));
#else
#ifndef xmlSAX2CDataBlock
extern __typeof (xmlSAX2CDataBlock) xmlSAX2CDataBlock__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2CDataBlock xmlSAX2CDataBlock__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2Characters
extern __typeof (xmlSAX2Characters) xmlSAX2Characters __attribute((alias("xmlSAX2Characters__internal_alias")));
#else
#ifndef xmlSAX2Characters
extern __typeof (xmlSAX2Characters) xmlSAX2Characters__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2Characters xmlSAX2Characters__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2Comment
extern __typeof (xmlSAX2Comment) xmlSAX2Comment __attribute((alias("xmlSAX2Comment__internal_alias")));
#else
#ifndef xmlSAX2Comment
extern __typeof (xmlSAX2Comment) xmlSAX2Comment__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2Comment xmlSAX2Comment__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2ElementDecl
extern __typeof (xmlSAX2ElementDecl) xmlSAX2ElementDecl __attribute((alias("xmlSAX2ElementDecl__internal_alias")));
#else
#ifndef xmlSAX2ElementDecl
extern __typeof (xmlSAX2ElementDecl) xmlSAX2ElementDecl__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2ElementDecl xmlSAX2ElementDecl__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2EndDocument
extern __typeof (xmlSAX2EndDocument) xmlSAX2EndDocument __attribute((alias("xmlSAX2EndDocument__internal_alias")));
#else
#ifndef xmlSAX2EndDocument
extern __typeof (xmlSAX2EndDocument) xmlSAX2EndDocument__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2EndDocument xmlSAX2EndDocument__internal_alias
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED) || defined(LIBXML_HTML_ENABLED)
#ifdef bottom_SAX2
#undef xmlSAX2EndElement
extern __typeof (xmlSAX2EndElement) xmlSAX2EndElement __attribute((alias("xmlSAX2EndElement__internal_alias")));
#else
#ifndef xmlSAX2EndElement
extern __typeof (xmlSAX2EndElement) xmlSAX2EndElement__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2EndElement xmlSAX2EndElement__internal_alias
#endif
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2EndElementNs
extern __typeof (xmlSAX2EndElementNs) xmlSAX2EndElementNs __attribute((alias("xmlSAX2EndElementNs__internal_alias")));
#else
#ifndef xmlSAX2EndElementNs
extern __typeof (xmlSAX2EndElementNs) xmlSAX2EndElementNs__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2EndElementNs xmlSAX2EndElementNs__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2EntityDecl
extern __typeof (xmlSAX2EntityDecl) xmlSAX2EntityDecl __attribute((alias("xmlSAX2EntityDecl__internal_alias")));
#else
#ifndef xmlSAX2EntityDecl
extern __typeof (xmlSAX2EntityDecl) xmlSAX2EntityDecl__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2EntityDecl xmlSAX2EntityDecl__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2ExternalSubset
extern __typeof (xmlSAX2ExternalSubset) xmlSAX2ExternalSubset __attribute((alias("xmlSAX2ExternalSubset__internal_alias")));
#else
#ifndef xmlSAX2ExternalSubset
extern __typeof (xmlSAX2ExternalSubset) xmlSAX2ExternalSubset__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2ExternalSubset xmlSAX2ExternalSubset__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2GetColumnNumber
extern __typeof (xmlSAX2GetColumnNumber) xmlSAX2GetColumnNumber __attribute((alias("xmlSAX2GetColumnNumber__internal_alias")));
#else
#ifndef xmlSAX2GetColumnNumber
extern __typeof (xmlSAX2GetColumnNumber) xmlSAX2GetColumnNumber__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2GetColumnNumber xmlSAX2GetColumnNumber__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2GetEntity
extern __typeof (xmlSAX2GetEntity) xmlSAX2GetEntity __attribute((alias("xmlSAX2GetEntity__internal_alias")));
#else
#ifndef xmlSAX2GetEntity
extern __typeof (xmlSAX2GetEntity) xmlSAX2GetEntity__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2GetEntity xmlSAX2GetEntity__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2GetLineNumber
extern __typeof (xmlSAX2GetLineNumber) xmlSAX2GetLineNumber __attribute((alias("xmlSAX2GetLineNumber__internal_alias")));
#else
#ifndef xmlSAX2GetLineNumber
extern __typeof (xmlSAX2GetLineNumber) xmlSAX2GetLineNumber__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2GetLineNumber xmlSAX2GetLineNumber__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2GetParameterEntity
extern __typeof (xmlSAX2GetParameterEntity) xmlSAX2GetParameterEntity __attribute((alias("xmlSAX2GetParameterEntity__internal_alias")));
#else
#ifndef xmlSAX2GetParameterEntity
extern __typeof (xmlSAX2GetParameterEntity) xmlSAX2GetParameterEntity__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2GetParameterEntity xmlSAX2GetParameterEntity__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2GetPublicId
extern __typeof (xmlSAX2GetPublicId) xmlSAX2GetPublicId __attribute((alias("xmlSAX2GetPublicId__internal_alias")));
#else
#ifndef xmlSAX2GetPublicId
extern __typeof (xmlSAX2GetPublicId) xmlSAX2GetPublicId__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2GetPublicId xmlSAX2GetPublicId__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2GetSystemId
extern __typeof (xmlSAX2GetSystemId) xmlSAX2GetSystemId __attribute((alias("xmlSAX2GetSystemId__internal_alias")));
#else
#ifndef xmlSAX2GetSystemId
extern __typeof (xmlSAX2GetSystemId) xmlSAX2GetSystemId__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2GetSystemId xmlSAX2GetSystemId__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2HasExternalSubset
extern __typeof (xmlSAX2HasExternalSubset) xmlSAX2HasExternalSubset __attribute((alias("xmlSAX2HasExternalSubset__internal_alias")));
#else
#ifndef xmlSAX2HasExternalSubset
extern __typeof (xmlSAX2HasExternalSubset) xmlSAX2HasExternalSubset__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2HasExternalSubset xmlSAX2HasExternalSubset__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2HasInternalSubset
extern __typeof (xmlSAX2HasInternalSubset) xmlSAX2HasInternalSubset __attribute((alias("xmlSAX2HasInternalSubset__internal_alias")));
#else
#ifndef xmlSAX2HasInternalSubset
extern __typeof (xmlSAX2HasInternalSubset) xmlSAX2HasInternalSubset__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2HasInternalSubset xmlSAX2HasInternalSubset__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2IgnorableWhitespace
extern __typeof (xmlSAX2IgnorableWhitespace) xmlSAX2IgnorableWhitespace __attribute((alias("xmlSAX2IgnorableWhitespace__internal_alias")));
#else
#ifndef xmlSAX2IgnorableWhitespace
extern __typeof (xmlSAX2IgnorableWhitespace) xmlSAX2IgnorableWhitespace__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2IgnorableWhitespace xmlSAX2IgnorableWhitespace__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2InitDefaultSAXHandler
extern __typeof (xmlSAX2InitDefaultSAXHandler) xmlSAX2InitDefaultSAXHandler __attribute((alias("xmlSAX2InitDefaultSAXHandler__internal_alias")));
#else
#ifndef xmlSAX2InitDefaultSAXHandler
extern __typeof (xmlSAX2InitDefaultSAXHandler) xmlSAX2InitDefaultSAXHandler__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2InitDefaultSAXHandler xmlSAX2InitDefaultSAXHandler__internal_alias
#endif
#endif

#if defined(LIBXML_DOCB_ENABLED)
#ifdef bottom_SAX2
#undef xmlSAX2InitDocbDefaultSAXHandler
extern __typeof (xmlSAX2InitDocbDefaultSAXHandler) xmlSAX2InitDocbDefaultSAXHandler __attribute((alias("xmlSAX2InitDocbDefaultSAXHandler__internal_alias")));
#else
#ifndef xmlSAX2InitDocbDefaultSAXHandler
extern __typeof (xmlSAX2InitDocbDefaultSAXHandler) xmlSAX2InitDocbDefaultSAXHandler__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2InitDocbDefaultSAXHandler xmlSAX2InitDocbDefaultSAXHandler__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_HTML_ENABLED)
#ifdef bottom_SAX2
#undef xmlSAX2InitHtmlDefaultSAXHandler
extern __typeof (xmlSAX2InitHtmlDefaultSAXHandler) xmlSAX2InitHtmlDefaultSAXHandler __attribute((alias("xmlSAX2InitHtmlDefaultSAXHandler__internal_alias")));
#else
#ifndef xmlSAX2InitHtmlDefaultSAXHandler
extern __typeof (xmlSAX2InitHtmlDefaultSAXHandler) xmlSAX2InitHtmlDefaultSAXHandler__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2InitHtmlDefaultSAXHandler xmlSAX2InitHtmlDefaultSAXHandler__internal_alias
#endif
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2InternalSubset
extern __typeof (xmlSAX2InternalSubset) xmlSAX2InternalSubset __attribute((alias("xmlSAX2InternalSubset__internal_alias")));
#else
#ifndef xmlSAX2InternalSubset
extern __typeof (xmlSAX2InternalSubset) xmlSAX2InternalSubset__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2InternalSubset xmlSAX2InternalSubset__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2IsStandalone
extern __typeof (xmlSAX2IsStandalone) xmlSAX2IsStandalone __attribute((alias("xmlSAX2IsStandalone__internal_alias")));
#else
#ifndef xmlSAX2IsStandalone
extern __typeof (xmlSAX2IsStandalone) xmlSAX2IsStandalone__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2IsStandalone xmlSAX2IsStandalone__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2NotationDecl
extern __typeof (xmlSAX2NotationDecl) xmlSAX2NotationDecl __attribute((alias("xmlSAX2NotationDecl__internal_alias")));
#else
#ifndef xmlSAX2NotationDecl
extern __typeof (xmlSAX2NotationDecl) xmlSAX2NotationDecl__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2NotationDecl xmlSAX2NotationDecl__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2ProcessingInstruction
extern __typeof (xmlSAX2ProcessingInstruction) xmlSAX2ProcessingInstruction __attribute((alias("xmlSAX2ProcessingInstruction__internal_alias")));
#else
#ifndef xmlSAX2ProcessingInstruction
extern __typeof (xmlSAX2ProcessingInstruction) xmlSAX2ProcessingInstruction__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2ProcessingInstruction xmlSAX2ProcessingInstruction__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2Reference
extern __typeof (xmlSAX2Reference) xmlSAX2Reference __attribute((alias("xmlSAX2Reference__internal_alias")));
#else
#ifndef xmlSAX2Reference
extern __typeof (xmlSAX2Reference) xmlSAX2Reference__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2Reference xmlSAX2Reference__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2ResolveEntity
extern __typeof (xmlSAX2ResolveEntity) xmlSAX2ResolveEntity __attribute((alias("xmlSAX2ResolveEntity__internal_alias")));
#else
#ifndef xmlSAX2ResolveEntity
extern __typeof (xmlSAX2ResolveEntity) xmlSAX2ResolveEntity__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2ResolveEntity xmlSAX2ResolveEntity__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2SetDocumentLocator
extern __typeof (xmlSAX2SetDocumentLocator) xmlSAX2SetDocumentLocator __attribute((alias("xmlSAX2SetDocumentLocator__internal_alias")));
#else
#ifndef xmlSAX2SetDocumentLocator
extern __typeof (xmlSAX2SetDocumentLocator) xmlSAX2SetDocumentLocator__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2SetDocumentLocator xmlSAX2SetDocumentLocator__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2StartDocument
extern __typeof (xmlSAX2StartDocument) xmlSAX2StartDocument __attribute((alias("xmlSAX2StartDocument__internal_alias")));
#else
#ifndef xmlSAX2StartDocument
extern __typeof (xmlSAX2StartDocument) xmlSAX2StartDocument__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2StartDocument xmlSAX2StartDocument__internal_alias
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED) || defined(LIBXML_HTML_ENABLED)
#ifdef bottom_SAX2
#undef xmlSAX2StartElement
extern __typeof (xmlSAX2StartElement) xmlSAX2StartElement __attribute((alias("xmlSAX2StartElement__internal_alias")));
#else
#ifndef xmlSAX2StartElement
extern __typeof (xmlSAX2StartElement) xmlSAX2StartElement__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2StartElement xmlSAX2StartElement__internal_alias
#endif
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2StartElementNs
extern __typeof (xmlSAX2StartElementNs) xmlSAX2StartElementNs __attribute((alias("xmlSAX2StartElementNs__internal_alias")));
#else
#ifndef xmlSAX2StartElementNs
extern __typeof (xmlSAX2StartElementNs) xmlSAX2StartElementNs__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2StartElementNs xmlSAX2StartElementNs__internal_alias
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAX2UnparsedEntityDecl
extern __typeof (xmlSAX2UnparsedEntityDecl) xmlSAX2UnparsedEntityDecl __attribute((alias("xmlSAX2UnparsedEntityDecl__internal_alias")));
#else
#ifndef xmlSAX2UnparsedEntityDecl
extern __typeof (xmlSAX2UnparsedEntityDecl) xmlSAX2UnparsedEntityDecl__internal_alias __attribute((visibility("hidden")));
#define xmlSAX2UnparsedEntityDecl xmlSAX2UnparsedEntityDecl__internal_alias
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_SAX2
#undef xmlSAXDefaultVersion
extern __typeof (xmlSAXDefaultVersion) xmlSAXDefaultVersion __attribute((alias("xmlSAXDefaultVersion__internal_alias")));
#else
#ifndef xmlSAXDefaultVersion
extern __typeof (xmlSAXDefaultVersion) xmlSAXDefaultVersion__internal_alias __attribute((visibility("hidden")));
#define xmlSAXDefaultVersion xmlSAXDefaultVersion__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_parser
#undef xmlSAXParseDTD
extern __typeof (xmlSAXParseDTD) xmlSAXParseDTD __attribute((alias("xmlSAXParseDTD__internal_alias")));
#else
#ifndef xmlSAXParseDTD
extern __typeof (xmlSAXParseDTD) xmlSAXParseDTD__internal_alias __attribute((visibility("hidden")));
#define xmlSAXParseDTD xmlSAXParseDTD__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlSAXParseDoc
extern __typeof (xmlSAXParseDoc) xmlSAXParseDoc __attribute((alias("xmlSAXParseDoc__internal_alias")));
#else
#ifndef xmlSAXParseDoc
extern __typeof (xmlSAXParseDoc) xmlSAXParseDoc__internal_alias __attribute((visibility("hidden")));
#define xmlSAXParseDoc xmlSAXParseDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlSAXParseEntity
extern __typeof (xmlSAXParseEntity) xmlSAXParseEntity __attribute((alias("xmlSAXParseEntity__internal_alias")));
#else
#ifndef xmlSAXParseEntity
extern __typeof (xmlSAXParseEntity) xmlSAXParseEntity__internal_alias __attribute((visibility("hidden")));
#define xmlSAXParseEntity xmlSAXParseEntity__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlSAXParseFile
extern __typeof (xmlSAXParseFile) xmlSAXParseFile __attribute((alias("xmlSAXParseFile__internal_alias")));
#else
#ifndef xmlSAXParseFile
extern __typeof (xmlSAXParseFile) xmlSAXParseFile__internal_alias __attribute((visibility("hidden")));
#define xmlSAXParseFile xmlSAXParseFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlSAXParseFileWithData
extern __typeof (xmlSAXParseFileWithData) xmlSAXParseFileWithData __attribute((alias("xmlSAXParseFileWithData__internal_alias")));
#else
#ifndef xmlSAXParseFileWithData
extern __typeof (xmlSAXParseFileWithData) xmlSAXParseFileWithData__internal_alias __attribute((visibility("hidden")));
#define xmlSAXParseFileWithData xmlSAXParseFileWithData__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlSAXParseMemory
extern __typeof (xmlSAXParseMemory) xmlSAXParseMemory __attribute((alias("xmlSAXParseMemory__internal_alias")));
#else
#ifndef xmlSAXParseMemory
extern __typeof (xmlSAXParseMemory) xmlSAXParseMemory__internal_alias __attribute((visibility("hidden")));
#define xmlSAXParseMemory xmlSAXParseMemory__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlSAXParseMemoryWithData
extern __typeof (xmlSAXParseMemoryWithData) xmlSAXParseMemoryWithData __attribute((alias("xmlSAXParseMemoryWithData__internal_alias")));
#else
#ifndef xmlSAXParseMemoryWithData
extern __typeof (xmlSAXParseMemoryWithData) xmlSAXParseMemoryWithData__internal_alias __attribute((visibility("hidden")));
#define xmlSAXParseMemoryWithData xmlSAXParseMemoryWithData__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlSAXUserParseFile
extern __typeof (xmlSAXUserParseFile) xmlSAXUserParseFile __attribute((alias("xmlSAXUserParseFile__internal_alias")));
#else
#ifndef xmlSAXUserParseFile
extern __typeof (xmlSAXUserParseFile) xmlSAXUserParseFile__internal_alias __attribute((visibility("hidden")));
#define xmlSAXUserParseFile xmlSAXUserParseFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlSAXUserParseMemory
extern __typeof (xmlSAXUserParseMemory) xmlSAXUserParseMemory __attribute((alias("xmlSAXUserParseMemory__internal_alias")));
#else
#ifndef xmlSAXUserParseMemory
extern __typeof (xmlSAXUserParseMemory) xmlSAXUserParseMemory__internal_alias __attribute((visibility("hidden")));
#define xmlSAXUserParseMemory xmlSAXUserParseMemory__internal_alias
#endif
#endif
#endif

#ifdef bottom_SAX2
#undef xmlSAXVersion
extern __typeof (xmlSAXVersion) xmlSAXVersion __attribute((alias("xmlSAXVersion__internal_alias")));
#else
#ifndef xmlSAXVersion
extern __typeof (xmlSAXVersion) xmlSAXVersion__internal_alias __attribute((visibility("hidden")));
#define xmlSAXVersion xmlSAXVersion__internal_alias
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveClose
extern __typeof (xmlSaveClose) xmlSaveClose __attribute((alias("xmlSaveClose__internal_alias")));
#else
#ifndef xmlSaveClose
extern __typeof (xmlSaveClose) xmlSaveClose__internal_alias __attribute((visibility("hidden")));
#define xmlSaveClose xmlSaveClose__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveDoc
extern __typeof (xmlSaveDoc) xmlSaveDoc __attribute((alias("xmlSaveDoc__internal_alias")));
#else
#ifndef xmlSaveDoc
extern __typeof (xmlSaveDoc) xmlSaveDoc__internal_alias __attribute((visibility("hidden")));
#define xmlSaveDoc xmlSaveDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveFile
extern __typeof (xmlSaveFile) xmlSaveFile __attribute((alias("xmlSaveFile__internal_alias")));
#else
#ifndef xmlSaveFile
extern __typeof (xmlSaveFile) xmlSaveFile__internal_alias __attribute((visibility("hidden")));
#define xmlSaveFile xmlSaveFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveFileEnc
extern __typeof (xmlSaveFileEnc) xmlSaveFileEnc __attribute((alias("xmlSaveFileEnc__internal_alias")));
#else
#ifndef xmlSaveFileEnc
extern __typeof (xmlSaveFileEnc) xmlSaveFileEnc__internal_alias __attribute((visibility("hidden")));
#define xmlSaveFileEnc xmlSaveFileEnc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveFileTo
extern __typeof (xmlSaveFileTo) xmlSaveFileTo __attribute((alias("xmlSaveFileTo__internal_alias")));
#else
#ifndef xmlSaveFileTo
extern __typeof (xmlSaveFileTo) xmlSaveFileTo__internal_alias __attribute((visibility("hidden")));
#define xmlSaveFileTo xmlSaveFileTo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveFlush
extern __typeof (xmlSaveFlush) xmlSaveFlush __attribute((alias("xmlSaveFlush__internal_alias")));
#else
#ifndef xmlSaveFlush
extern __typeof (xmlSaveFlush) xmlSaveFlush__internal_alias __attribute((visibility("hidden")));
#define xmlSaveFlush xmlSaveFlush__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveFormatFile
extern __typeof (xmlSaveFormatFile) xmlSaveFormatFile __attribute((alias("xmlSaveFormatFile__internal_alias")));
#else
#ifndef xmlSaveFormatFile
extern __typeof (xmlSaveFormatFile) xmlSaveFormatFile__internal_alias __attribute((visibility("hidden")));
#define xmlSaveFormatFile xmlSaveFormatFile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveFormatFileEnc
extern __typeof (xmlSaveFormatFileEnc) xmlSaveFormatFileEnc __attribute((alias("xmlSaveFormatFileEnc__internal_alias")));
#else
#ifndef xmlSaveFormatFileEnc
extern __typeof (xmlSaveFormatFileEnc) xmlSaveFormatFileEnc__internal_alias __attribute((visibility("hidden")));
#define xmlSaveFormatFileEnc xmlSaveFormatFileEnc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveFormatFileTo
extern __typeof (xmlSaveFormatFileTo) xmlSaveFormatFileTo __attribute((alias("xmlSaveFormatFileTo__internal_alias")));
#else
#ifndef xmlSaveFormatFileTo
extern __typeof (xmlSaveFormatFileTo) xmlSaveFormatFileTo__internal_alias __attribute((visibility("hidden")));
#define xmlSaveFormatFileTo xmlSaveFormatFileTo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveSetAttrEscape
extern __typeof (xmlSaveSetAttrEscape) xmlSaveSetAttrEscape __attribute((alias("xmlSaveSetAttrEscape__internal_alias")));
#else
#ifndef xmlSaveSetAttrEscape
extern __typeof (xmlSaveSetAttrEscape) xmlSaveSetAttrEscape__internal_alias __attribute((visibility("hidden")));
#define xmlSaveSetAttrEscape xmlSaveSetAttrEscape__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveSetEscape
extern __typeof (xmlSaveSetEscape) xmlSaveSetEscape __attribute((alias("xmlSaveSetEscape__internal_alias")));
#else
#ifndef xmlSaveSetEscape
extern __typeof (xmlSaveSetEscape) xmlSaveSetEscape__internal_alias __attribute((visibility("hidden")));
#define xmlSaveSetEscape xmlSaveSetEscape__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveToFd
extern __typeof (xmlSaveToFd) xmlSaveToFd __attribute((alias("xmlSaveToFd__internal_alias")));
#else
#ifndef xmlSaveToFd
extern __typeof (xmlSaveToFd) xmlSaveToFd__internal_alias __attribute((visibility("hidden")));
#define xmlSaveToFd xmlSaveToFd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveToFilename
extern __typeof (xmlSaveToFilename) xmlSaveToFilename __attribute((alias("xmlSaveToFilename__internal_alias")));
#else
#ifndef xmlSaveToFilename
extern __typeof (xmlSaveToFilename) xmlSaveToFilename__internal_alias __attribute((visibility("hidden")));
#define xmlSaveToFilename xmlSaveToFilename__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveToIO
extern __typeof (xmlSaveToIO) xmlSaveToIO __attribute((alias("xmlSaveToIO__internal_alias")));
#else
#ifndef xmlSaveToIO
extern __typeof (xmlSaveToIO) xmlSaveToIO__internal_alias __attribute((visibility("hidden")));
#define xmlSaveToIO xmlSaveToIO__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlsave
#undef xmlSaveTree
extern __typeof (xmlSaveTree) xmlSaveTree __attribute((alias("xmlSaveTree__internal_alias")));
#else
#ifndef xmlSaveTree
extern __typeof (xmlSaveTree) xmlSaveTree__internal_alias __attribute((visibility("hidden")));
#define xmlSaveTree xmlSaveTree__internal_alias
#endif
#endif
#endif

#ifdef bottom_uri
#undef xmlSaveUri
extern __typeof (xmlSaveUri) xmlSaveUri __attribute((alias("xmlSaveUri__internal_alias")));
#else
#ifndef xmlSaveUri
extern __typeof (xmlSaveUri) xmlSaveUri__internal_alias __attribute((visibility("hidden")));
#define xmlSaveUri xmlSaveUri__internal_alias
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlScanName
extern __typeof (xmlScanName) xmlScanName __attribute((alias("xmlScanName__internal_alias")));
#else
#ifndef xmlScanName
extern __typeof (xmlScanName) xmlScanName__internal_alias __attribute((visibility("hidden")));
#define xmlScanName xmlScanName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaCheckFacet
extern __typeof (xmlSchemaCheckFacet) xmlSchemaCheckFacet __attribute((alias("xmlSchemaCheckFacet__internal_alias")));
#else
#ifndef xmlSchemaCheckFacet
extern __typeof (xmlSchemaCheckFacet) xmlSchemaCheckFacet__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaCheckFacet xmlSchemaCheckFacet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaCleanupTypes
extern __typeof (xmlSchemaCleanupTypes) xmlSchemaCleanupTypes __attribute((alias("xmlSchemaCleanupTypes__internal_alias")));
#else
#ifndef xmlSchemaCleanupTypes
extern __typeof (xmlSchemaCleanupTypes) xmlSchemaCleanupTypes__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaCleanupTypes xmlSchemaCleanupTypes__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaCollapseString
extern __typeof (xmlSchemaCollapseString) xmlSchemaCollapseString __attribute((alias("xmlSchemaCollapseString__internal_alias")));
#else
#ifndef xmlSchemaCollapseString
extern __typeof (xmlSchemaCollapseString) xmlSchemaCollapseString__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaCollapseString xmlSchemaCollapseString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaCompareValues
extern __typeof (xmlSchemaCompareValues) xmlSchemaCompareValues __attribute((alias("xmlSchemaCompareValues__internal_alias")));
#else
#ifndef xmlSchemaCompareValues
extern __typeof (xmlSchemaCompareValues) xmlSchemaCompareValues__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaCompareValues xmlSchemaCompareValues__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaCompareValuesWhtsp
extern __typeof (xmlSchemaCompareValuesWhtsp) xmlSchemaCompareValuesWhtsp __attribute((alias("xmlSchemaCompareValuesWhtsp__internal_alias")));
#else
#ifndef xmlSchemaCompareValuesWhtsp
extern __typeof (xmlSchemaCompareValuesWhtsp) xmlSchemaCompareValuesWhtsp__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaCompareValuesWhtsp xmlSchemaCompareValuesWhtsp__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaCopyValue
extern __typeof (xmlSchemaCopyValue) xmlSchemaCopyValue __attribute((alias("xmlSchemaCopyValue__internal_alias")));
#else
#ifndef xmlSchemaCopyValue
extern __typeof (xmlSchemaCopyValue) xmlSchemaCopyValue__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaCopyValue xmlSchemaCopyValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaDump
extern __typeof (xmlSchemaDump) xmlSchemaDump __attribute((alias("xmlSchemaDump__internal_alias")));
#else
#ifndef xmlSchemaDump
extern __typeof (xmlSchemaDump) xmlSchemaDump__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaDump xmlSchemaDump__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaFree
extern __typeof (xmlSchemaFree) xmlSchemaFree __attribute((alias("xmlSchemaFree__internal_alias")));
#else
#ifndef xmlSchemaFree
extern __typeof (xmlSchemaFree) xmlSchemaFree__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaFree xmlSchemaFree__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaFreeFacet
extern __typeof (xmlSchemaFreeFacet) xmlSchemaFreeFacet __attribute((alias("xmlSchemaFreeFacet__internal_alias")));
#else
#ifndef xmlSchemaFreeFacet
extern __typeof (xmlSchemaFreeFacet) xmlSchemaFreeFacet__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaFreeFacet xmlSchemaFreeFacet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaFreeParserCtxt
extern __typeof (xmlSchemaFreeParserCtxt) xmlSchemaFreeParserCtxt __attribute((alias("xmlSchemaFreeParserCtxt__internal_alias")));
#else
#ifndef xmlSchemaFreeParserCtxt
extern __typeof (xmlSchemaFreeParserCtxt) xmlSchemaFreeParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaFreeParserCtxt xmlSchemaFreeParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaFreeType
extern __typeof (xmlSchemaFreeType) xmlSchemaFreeType __attribute((alias("xmlSchemaFreeType__internal_alias")));
#else
#ifndef xmlSchemaFreeType
extern __typeof (xmlSchemaFreeType) xmlSchemaFreeType__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaFreeType xmlSchemaFreeType__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaFreeValidCtxt
extern __typeof (xmlSchemaFreeValidCtxt) xmlSchemaFreeValidCtxt __attribute((alias("xmlSchemaFreeValidCtxt__internal_alias")));
#else
#ifndef xmlSchemaFreeValidCtxt
extern __typeof (xmlSchemaFreeValidCtxt) xmlSchemaFreeValidCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaFreeValidCtxt xmlSchemaFreeValidCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaFreeValue
extern __typeof (xmlSchemaFreeValue) xmlSchemaFreeValue __attribute((alias("xmlSchemaFreeValue__internal_alias")));
#else
#ifndef xmlSchemaFreeValue
extern __typeof (xmlSchemaFreeValue) xmlSchemaFreeValue__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaFreeValue xmlSchemaFreeValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaFreeWildcard
extern __typeof (xmlSchemaFreeWildcard) xmlSchemaFreeWildcard __attribute((alias("xmlSchemaFreeWildcard__internal_alias")));
#else
#ifndef xmlSchemaFreeWildcard
extern __typeof (xmlSchemaFreeWildcard) xmlSchemaFreeWildcard__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaFreeWildcard xmlSchemaFreeWildcard__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaGetBuiltInListSimpleTypeItemType
extern __typeof (xmlSchemaGetBuiltInListSimpleTypeItemType) xmlSchemaGetBuiltInListSimpleTypeItemType __attribute((alias("xmlSchemaGetBuiltInListSimpleTypeItemType__internal_alias")));
#else
#ifndef xmlSchemaGetBuiltInListSimpleTypeItemType
extern __typeof (xmlSchemaGetBuiltInListSimpleTypeItemType) xmlSchemaGetBuiltInListSimpleTypeItemType__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaGetBuiltInListSimpleTypeItemType xmlSchemaGetBuiltInListSimpleTypeItemType__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaGetBuiltInType
extern __typeof (xmlSchemaGetBuiltInType) xmlSchemaGetBuiltInType __attribute((alias("xmlSchemaGetBuiltInType__internal_alias")));
#else
#ifndef xmlSchemaGetBuiltInType
extern __typeof (xmlSchemaGetBuiltInType) xmlSchemaGetBuiltInType__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaGetBuiltInType xmlSchemaGetBuiltInType__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaGetCanonValue
extern __typeof (xmlSchemaGetCanonValue) xmlSchemaGetCanonValue __attribute((alias("xmlSchemaGetCanonValue__internal_alias")));
#else
#ifndef xmlSchemaGetCanonValue
extern __typeof (xmlSchemaGetCanonValue) xmlSchemaGetCanonValue__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaGetCanonValue xmlSchemaGetCanonValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaGetFacetValueAsULong
extern __typeof (xmlSchemaGetFacetValueAsULong) xmlSchemaGetFacetValueAsULong __attribute((alias("xmlSchemaGetFacetValueAsULong__internal_alias")));
#else
#ifndef xmlSchemaGetFacetValueAsULong
extern __typeof (xmlSchemaGetFacetValueAsULong) xmlSchemaGetFacetValueAsULong__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaGetFacetValueAsULong xmlSchemaGetFacetValueAsULong__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaGetParserErrors
extern __typeof (xmlSchemaGetParserErrors) xmlSchemaGetParserErrors __attribute((alias("xmlSchemaGetParserErrors__internal_alias")));
#else
#ifndef xmlSchemaGetParserErrors
extern __typeof (xmlSchemaGetParserErrors) xmlSchemaGetParserErrors__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaGetParserErrors xmlSchemaGetParserErrors__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaGetPredefinedType
extern __typeof (xmlSchemaGetPredefinedType) xmlSchemaGetPredefinedType __attribute((alias("xmlSchemaGetPredefinedType__internal_alias")));
#else
#ifndef xmlSchemaGetPredefinedType
extern __typeof (xmlSchemaGetPredefinedType) xmlSchemaGetPredefinedType__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaGetPredefinedType xmlSchemaGetPredefinedType__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaGetValType
extern __typeof (xmlSchemaGetValType) xmlSchemaGetValType __attribute((alias("xmlSchemaGetValType__internal_alias")));
#else
#ifndef xmlSchemaGetValType
extern __typeof (xmlSchemaGetValType) xmlSchemaGetValType__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaGetValType xmlSchemaGetValType__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaGetValidErrors
extern __typeof (xmlSchemaGetValidErrors) xmlSchemaGetValidErrors __attribute((alias("xmlSchemaGetValidErrors__internal_alias")));
#else
#ifndef xmlSchemaGetValidErrors
extern __typeof (xmlSchemaGetValidErrors) xmlSchemaGetValidErrors__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaGetValidErrors xmlSchemaGetValidErrors__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaInitTypes
extern __typeof (xmlSchemaInitTypes) xmlSchemaInitTypes __attribute((alias("xmlSchemaInitTypes__internal_alias")));
#else
#ifndef xmlSchemaInitTypes
extern __typeof (xmlSchemaInitTypes) xmlSchemaInitTypes__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaInitTypes xmlSchemaInitTypes__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaIsBuiltInTypeFacet
extern __typeof (xmlSchemaIsBuiltInTypeFacet) xmlSchemaIsBuiltInTypeFacet __attribute((alias("xmlSchemaIsBuiltInTypeFacet__internal_alias")));
#else
#ifndef xmlSchemaIsBuiltInTypeFacet
extern __typeof (xmlSchemaIsBuiltInTypeFacet) xmlSchemaIsBuiltInTypeFacet__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaIsBuiltInTypeFacet xmlSchemaIsBuiltInTypeFacet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaNewDocParserCtxt
extern __typeof (xmlSchemaNewDocParserCtxt) xmlSchemaNewDocParserCtxt __attribute((alias("xmlSchemaNewDocParserCtxt__internal_alias")));
#else
#ifndef xmlSchemaNewDocParserCtxt
extern __typeof (xmlSchemaNewDocParserCtxt) xmlSchemaNewDocParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaNewDocParserCtxt xmlSchemaNewDocParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaNewFacet
extern __typeof (xmlSchemaNewFacet) xmlSchemaNewFacet __attribute((alias("xmlSchemaNewFacet__internal_alias")));
#else
#ifndef xmlSchemaNewFacet
extern __typeof (xmlSchemaNewFacet) xmlSchemaNewFacet__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaNewFacet xmlSchemaNewFacet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaNewMemParserCtxt
extern __typeof (xmlSchemaNewMemParserCtxt) xmlSchemaNewMemParserCtxt __attribute((alias("xmlSchemaNewMemParserCtxt__internal_alias")));
#else
#ifndef xmlSchemaNewMemParserCtxt
extern __typeof (xmlSchemaNewMemParserCtxt) xmlSchemaNewMemParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaNewMemParserCtxt xmlSchemaNewMemParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaNewNOTATIONValue
extern __typeof (xmlSchemaNewNOTATIONValue) xmlSchemaNewNOTATIONValue __attribute((alias("xmlSchemaNewNOTATIONValue__internal_alias")));
#else
#ifndef xmlSchemaNewNOTATIONValue
extern __typeof (xmlSchemaNewNOTATIONValue) xmlSchemaNewNOTATIONValue__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaNewNOTATIONValue xmlSchemaNewNOTATIONValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaNewParserCtxt
extern __typeof (xmlSchemaNewParserCtxt) xmlSchemaNewParserCtxt __attribute((alias("xmlSchemaNewParserCtxt__internal_alias")));
#else
#ifndef xmlSchemaNewParserCtxt
extern __typeof (xmlSchemaNewParserCtxt) xmlSchemaNewParserCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaNewParserCtxt xmlSchemaNewParserCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaNewStringValue
extern __typeof (xmlSchemaNewStringValue) xmlSchemaNewStringValue __attribute((alias("xmlSchemaNewStringValue__internal_alias")));
#else
#ifndef xmlSchemaNewStringValue
extern __typeof (xmlSchemaNewStringValue) xmlSchemaNewStringValue__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaNewStringValue xmlSchemaNewStringValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaNewValidCtxt
extern __typeof (xmlSchemaNewValidCtxt) xmlSchemaNewValidCtxt __attribute((alias("xmlSchemaNewValidCtxt__internal_alias")));
#else
#ifndef xmlSchemaNewValidCtxt
extern __typeof (xmlSchemaNewValidCtxt) xmlSchemaNewValidCtxt__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaNewValidCtxt xmlSchemaNewValidCtxt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaParse
extern __typeof (xmlSchemaParse) xmlSchemaParse __attribute((alias("xmlSchemaParse__internal_alias")));
#else
#ifndef xmlSchemaParse
extern __typeof (xmlSchemaParse) xmlSchemaParse__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaParse xmlSchemaParse__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaSetParserErrors
extern __typeof (xmlSchemaSetParserErrors) xmlSchemaSetParserErrors __attribute((alias("xmlSchemaSetParserErrors__internal_alias")));
#else
#ifndef xmlSchemaSetParserErrors
extern __typeof (xmlSchemaSetParserErrors) xmlSchemaSetParserErrors__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaSetParserErrors xmlSchemaSetParserErrors__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaSetValidErrors
extern __typeof (xmlSchemaSetValidErrors) xmlSchemaSetValidErrors __attribute((alias("xmlSchemaSetValidErrors__internal_alias")));
#else
#ifndef xmlSchemaSetValidErrors
extern __typeof (xmlSchemaSetValidErrors) xmlSchemaSetValidErrors__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaSetValidErrors xmlSchemaSetValidErrors__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaSetValidOptions
extern __typeof (xmlSchemaSetValidOptions) xmlSchemaSetValidOptions __attribute((alias("xmlSchemaSetValidOptions__internal_alias")));
#else
#ifndef xmlSchemaSetValidOptions
extern __typeof (xmlSchemaSetValidOptions) xmlSchemaSetValidOptions__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaSetValidOptions xmlSchemaSetValidOptions__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaValPredefTypeNode
extern __typeof (xmlSchemaValPredefTypeNode) xmlSchemaValPredefTypeNode __attribute((alias("xmlSchemaValPredefTypeNode__internal_alias")));
#else
#ifndef xmlSchemaValPredefTypeNode
extern __typeof (xmlSchemaValPredefTypeNode) xmlSchemaValPredefTypeNode__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaValPredefTypeNode xmlSchemaValPredefTypeNode__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaValPredefTypeNodeNoNorm
extern __typeof (xmlSchemaValPredefTypeNodeNoNorm) xmlSchemaValPredefTypeNodeNoNorm __attribute((alias("xmlSchemaValPredefTypeNodeNoNorm__internal_alias")));
#else
#ifndef xmlSchemaValPredefTypeNodeNoNorm
extern __typeof (xmlSchemaValPredefTypeNodeNoNorm) xmlSchemaValPredefTypeNodeNoNorm__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaValPredefTypeNodeNoNorm xmlSchemaValPredefTypeNodeNoNorm__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaValidCtxtGetOptions
extern __typeof (xmlSchemaValidCtxtGetOptions) xmlSchemaValidCtxtGetOptions __attribute((alias("xmlSchemaValidCtxtGetOptions__internal_alias")));
#else
#ifndef xmlSchemaValidCtxtGetOptions
extern __typeof (xmlSchemaValidCtxtGetOptions) xmlSchemaValidCtxtGetOptions__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaValidCtxtGetOptions xmlSchemaValidCtxtGetOptions__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaValidateDoc
extern __typeof (xmlSchemaValidateDoc) xmlSchemaValidateDoc __attribute((alias("xmlSchemaValidateDoc__internal_alias")));
#else
#ifndef xmlSchemaValidateDoc
extern __typeof (xmlSchemaValidateDoc) xmlSchemaValidateDoc__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaValidateDoc xmlSchemaValidateDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaValidateFacet
extern __typeof (xmlSchemaValidateFacet) xmlSchemaValidateFacet __attribute((alias("xmlSchemaValidateFacet__internal_alias")));
#else
#ifndef xmlSchemaValidateFacet
extern __typeof (xmlSchemaValidateFacet) xmlSchemaValidateFacet__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaValidateFacet xmlSchemaValidateFacet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaValidateFacetWhtsp
extern __typeof (xmlSchemaValidateFacetWhtsp) xmlSchemaValidateFacetWhtsp __attribute((alias("xmlSchemaValidateFacetWhtsp__internal_alias")));
#else
#ifndef xmlSchemaValidateFacetWhtsp
extern __typeof (xmlSchemaValidateFacetWhtsp) xmlSchemaValidateFacetWhtsp__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaValidateFacetWhtsp xmlSchemaValidateFacetWhtsp__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaValidateLengthFacet
extern __typeof (xmlSchemaValidateLengthFacet) xmlSchemaValidateLengthFacet __attribute((alias("xmlSchemaValidateLengthFacet__internal_alias")));
#else
#ifndef xmlSchemaValidateLengthFacet
extern __typeof (xmlSchemaValidateLengthFacet) xmlSchemaValidateLengthFacet__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaValidateLengthFacet xmlSchemaValidateLengthFacet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaValidateLengthFacetWhtsp
extern __typeof (xmlSchemaValidateLengthFacetWhtsp) xmlSchemaValidateLengthFacetWhtsp __attribute((alias("xmlSchemaValidateLengthFacetWhtsp__internal_alias")));
#else
#ifndef xmlSchemaValidateLengthFacetWhtsp
extern __typeof (xmlSchemaValidateLengthFacetWhtsp) xmlSchemaValidateLengthFacetWhtsp__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaValidateLengthFacetWhtsp xmlSchemaValidateLengthFacetWhtsp__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaValidateListSimpleTypeFacet
extern __typeof (xmlSchemaValidateListSimpleTypeFacet) xmlSchemaValidateListSimpleTypeFacet __attribute((alias("xmlSchemaValidateListSimpleTypeFacet__internal_alias")));
#else
#ifndef xmlSchemaValidateListSimpleTypeFacet
extern __typeof (xmlSchemaValidateListSimpleTypeFacet) xmlSchemaValidateListSimpleTypeFacet__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaValidateListSimpleTypeFacet xmlSchemaValidateListSimpleTypeFacet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaValidateOneElement
extern __typeof (xmlSchemaValidateOneElement) xmlSchemaValidateOneElement __attribute((alias("xmlSchemaValidateOneElement__internal_alias")));
#else
#ifndef xmlSchemaValidateOneElement
extern __typeof (xmlSchemaValidateOneElement) xmlSchemaValidateOneElement__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaValidateOneElement xmlSchemaValidateOneElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaValidatePredefinedType
extern __typeof (xmlSchemaValidatePredefinedType) xmlSchemaValidatePredefinedType __attribute((alias("xmlSchemaValidatePredefinedType__internal_alias")));
#else
#ifndef xmlSchemaValidatePredefinedType
extern __typeof (xmlSchemaValidatePredefinedType) xmlSchemaValidatePredefinedType__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaValidatePredefinedType xmlSchemaValidatePredefinedType__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemas
#undef xmlSchemaValidateStream
extern __typeof (xmlSchemaValidateStream) xmlSchemaValidateStream __attribute((alias("xmlSchemaValidateStream__internal_alias")));
#else
#ifndef xmlSchemaValidateStream
extern __typeof (xmlSchemaValidateStream) xmlSchemaValidateStream__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaValidateStream xmlSchemaValidateStream__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlschemastypes
#undef xmlSchemaWhiteSpaceReplace
extern __typeof (xmlSchemaWhiteSpaceReplace) xmlSchemaWhiteSpaceReplace __attribute((alias("xmlSchemaWhiteSpaceReplace__internal_alias")));
#else
#ifndef xmlSchemaWhiteSpaceReplace
extern __typeof (xmlSchemaWhiteSpaceReplace) xmlSchemaWhiteSpaceReplace__internal_alias __attribute((visibility("hidden")));
#define xmlSchemaWhiteSpaceReplace xmlSchemaWhiteSpaceReplace__internal_alias
#endif
#endif
#endif

#ifdef bottom_tree
#undef xmlSearchNs
extern __typeof (xmlSearchNs) xmlSearchNs __attribute((alias("xmlSearchNs__internal_alias")));
#else
#ifndef xmlSearchNs
extern __typeof (xmlSearchNs) xmlSearchNs__internal_alias __attribute((visibility("hidden")));
#define xmlSearchNs xmlSearchNs__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlSearchNsByHref
extern __typeof (xmlSearchNsByHref) xmlSearchNsByHref __attribute((alias("xmlSearchNsByHref__internal_alias")));
#else
#ifndef xmlSearchNsByHref
extern __typeof (xmlSearchNsByHref) xmlSearchNsByHref__internal_alias __attribute((visibility("hidden")));
#define xmlSearchNsByHref xmlSearchNsByHref__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlSetBufferAllocationScheme
extern __typeof (xmlSetBufferAllocationScheme) xmlSetBufferAllocationScheme __attribute((alias("xmlSetBufferAllocationScheme__internal_alias")));
#else
#ifndef xmlSetBufferAllocationScheme
extern __typeof (xmlSetBufferAllocationScheme) xmlSetBufferAllocationScheme__internal_alias __attribute((visibility("hidden")));
#define xmlSetBufferAllocationScheme xmlSetBufferAllocationScheme__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlSetCompressMode
extern __typeof (xmlSetCompressMode) xmlSetCompressMode __attribute((alias("xmlSetCompressMode__internal_alias")));
#else
#ifndef xmlSetCompressMode
extern __typeof (xmlSetCompressMode) xmlSetCompressMode__internal_alias __attribute((visibility("hidden")));
#define xmlSetCompressMode xmlSetCompressMode__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlSetDocCompressMode
extern __typeof (xmlSetDocCompressMode) xmlSetDocCompressMode __attribute((alias("xmlSetDocCompressMode__internal_alias")));
#else
#ifndef xmlSetDocCompressMode
extern __typeof (xmlSetDocCompressMode) xmlSetDocCompressMode__internal_alias __attribute((visibility("hidden")));
#define xmlSetDocCompressMode xmlSetDocCompressMode__internal_alias
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_parser
#undef xmlSetEntityReferenceFunc
extern __typeof (xmlSetEntityReferenceFunc) xmlSetEntityReferenceFunc __attribute((alias("xmlSetEntityReferenceFunc__internal_alias")));
#else
#ifndef xmlSetEntityReferenceFunc
extern __typeof (xmlSetEntityReferenceFunc) xmlSetEntityReferenceFunc__internal_alias __attribute((visibility("hidden")));
#define xmlSetEntityReferenceFunc xmlSetEntityReferenceFunc__internal_alias
#endif
#endif
#endif

#ifdef bottom_xmlIO
#undef xmlSetExternalEntityLoader
extern __typeof (xmlSetExternalEntityLoader) xmlSetExternalEntityLoader __attribute((alias("xmlSetExternalEntityLoader__internal_alias")));
#else
#ifndef xmlSetExternalEntityLoader
extern __typeof (xmlSetExternalEntityLoader) xmlSetExternalEntityLoader__internal_alias __attribute((visibility("hidden")));
#define xmlSetExternalEntityLoader xmlSetExternalEntityLoader__internal_alias
#endif
#endif

#if defined(LIBXML_LEGACY_ENABLED)
#ifdef bottom_legacy
#undef xmlSetFeature
extern __typeof (xmlSetFeature) xmlSetFeature __attribute((alias("xmlSetFeature__internal_alias")));
#else
#ifndef xmlSetFeature
extern __typeof (xmlSetFeature) xmlSetFeature__internal_alias __attribute((visibility("hidden")));
#define xmlSetFeature xmlSetFeature__internal_alias
#endif
#endif
#endif

#ifdef bottom_error
#undef xmlSetGenericErrorFunc
extern __typeof (xmlSetGenericErrorFunc) xmlSetGenericErrorFunc __attribute((alias("xmlSetGenericErrorFunc__internal_alias")));
#else
#ifndef xmlSetGenericErrorFunc
extern __typeof (xmlSetGenericErrorFunc) xmlSetGenericErrorFunc__internal_alias __attribute((visibility("hidden")));
#define xmlSetGenericErrorFunc xmlSetGenericErrorFunc__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlSetListDoc
extern __typeof (xmlSetListDoc) xmlSetListDoc __attribute((alias("xmlSetListDoc__internal_alias")));
#else
#ifndef xmlSetListDoc
extern __typeof (xmlSetListDoc) xmlSetListDoc__internal_alias __attribute((visibility("hidden")));
#define xmlSetListDoc xmlSetListDoc__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlSetNs
extern __typeof (xmlSetNs) xmlSetNs __attribute((alias("xmlSetNs__internal_alias")));
#else
#ifndef xmlSetNs
extern __typeof (xmlSetNs) xmlSetNs__internal_alias __attribute((visibility("hidden")));
#define xmlSetNs xmlSetNs__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_XINCLUDE_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED) || defined(LIBXML_HTML_ENABLED)
#ifdef bottom_tree
#undef xmlSetNsProp
extern __typeof (xmlSetNsProp) xmlSetNsProp __attribute((alias("xmlSetNsProp__internal_alias")));
#else
#ifndef xmlSetNsProp
extern __typeof (xmlSetNsProp) xmlSetNsProp__internal_alias __attribute((visibility("hidden")));
#define xmlSetNsProp xmlSetNsProp__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_XINCLUDE_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED) || defined(LIBXML_HTML_ENABLED)
#ifdef bottom_tree
#undef xmlSetProp
extern __typeof (xmlSetProp) xmlSetProp __attribute((alias("xmlSetProp__internal_alias")));
#else
#ifndef xmlSetProp
extern __typeof (xmlSetProp) xmlSetProp__internal_alias __attribute((visibility("hidden")));
#define xmlSetProp xmlSetProp__internal_alias
#endif
#endif
#endif

#ifdef bottom_error
#undef xmlSetStructuredErrorFunc
extern __typeof (xmlSetStructuredErrorFunc) xmlSetStructuredErrorFunc __attribute((alias("xmlSetStructuredErrorFunc__internal_alias")));
#else
#ifndef xmlSetStructuredErrorFunc
extern __typeof (xmlSetStructuredErrorFunc) xmlSetStructuredErrorFunc__internal_alias __attribute((visibility("hidden")));
#define xmlSetStructuredErrorFunc xmlSetStructuredErrorFunc__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlSetTreeDoc
extern __typeof (xmlSetTreeDoc) xmlSetTreeDoc __attribute((alias("xmlSetTreeDoc__internal_alias")));
#else
#ifndef xmlSetTreeDoc
extern __typeof (xmlSetTreeDoc) xmlSetTreeDoc__internal_alias __attribute((visibility("hidden")));
#define xmlSetTreeDoc xmlSetTreeDoc__internal_alias
#endif
#endif

#if defined(LIBXML_SAX1_ENABLED)
#ifdef bottom_parser
#undef xmlSetupParserForBuffer
extern __typeof (xmlSetupParserForBuffer) xmlSetupParserForBuffer __attribute((alias("xmlSetupParserForBuffer__internal_alias")));
#else
#ifndef xmlSetupParserForBuffer
extern __typeof (xmlSetupParserForBuffer) xmlSetupParserForBuffer__internal_alias __attribute((visibility("hidden")));
#define xmlSetupParserForBuffer xmlSetupParserForBuffer__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_debugXML
#undef xmlShell
extern __typeof (xmlShell) xmlShell __attribute((alias("xmlShell__internal_alias")));
#else
#ifndef xmlShell
extern __typeof (xmlShell) xmlShell__internal_alias __attribute((visibility("hidden")));
#define xmlShell xmlShell__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellBase
extern __typeof (xmlShellBase) xmlShellBase __attribute((alias("xmlShellBase__internal_alias")));
#else
#ifndef xmlShellBase
extern __typeof (xmlShellBase) xmlShellBase__internal_alias __attribute((visibility("hidden")));
#define xmlShellBase xmlShellBase__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellCat
extern __typeof (xmlShellCat) xmlShellCat __attribute((alias("xmlShellCat__internal_alias")));
#else
#ifndef xmlShellCat
extern __typeof (xmlShellCat) xmlShellCat__internal_alias __attribute((visibility("hidden")));
#define xmlShellCat xmlShellCat__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellDir
extern __typeof (xmlShellDir) xmlShellDir __attribute((alias("xmlShellDir__internal_alias")));
#else
#ifndef xmlShellDir
extern __typeof (xmlShellDir) xmlShellDir__internal_alias __attribute((visibility("hidden")));
#define xmlShellDir xmlShellDir__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellDu
extern __typeof (xmlShellDu) xmlShellDu __attribute((alias("xmlShellDu__internal_alias")));
#else
#ifndef xmlShellDu
extern __typeof (xmlShellDu) xmlShellDu__internal_alias __attribute((visibility("hidden")));
#define xmlShellDu xmlShellDu__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellList
extern __typeof (xmlShellList) xmlShellList __attribute((alias("xmlShellList__internal_alias")));
#else
#ifndef xmlShellList
extern __typeof (xmlShellList) xmlShellList__internal_alias __attribute((visibility("hidden")));
#define xmlShellList xmlShellList__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellLoad
extern __typeof (xmlShellLoad) xmlShellLoad __attribute((alias("xmlShellLoad__internal_alias")));
#else
#ifndef xmlShellLoad
extern __typeof (xmlShellLoad) xmlShellLoad__internal_alias __attribute((visibility("hidden")));
#define xmlShellLoad xmlShellLoad__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellPrintNode
extern __typeof (xmlShellPrintNode) xmlShellPrintNode __attribute((alias("xmlShellPrintNode__internal_alias")));
#else
#ifndef xmlShellPrintNode
extern __typeof (xmlShellPrintNode) xmlShellPrintNode__internal_alias __attribute((visibility("hidden")));
#define xmlShellPrintNode xmlShellPrintNode__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellPrintXPathError
extern __typeof (xmlShellPrintXPathError) xmlShellPrintXPathError __attribute((alias("xmlShellPrintXPathError__internal_alias")));
#else
#ifndef xmlShellPrintXPathError
extern __typeof (xmlShellPrintXPathError) xmlShellPrintXPathError__internal_alias __attribute((visibility("hidden")));
#define xmlShellPrintXPathError xmlShellPrintXPathError__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellPrintXPathResult
extern __typeof (xmlShellPrintXPathResult) xmlShellPrintXPathResult __attribute((alias("xmlShellPrintXPathResult__internal_alias")));
#else
#ifndef xmlShellPrintXPathResult
extern __typeof (xmlShellPrintXPathResult) xmlShellPrintXPathResult__internal_alias __attribute((visibility("hidden")));
#define xmlShellPrintXPathResult xmlShellPrintXPathResult__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellPwd
extern __typeof (xmlShellPwd) xmlShellPwd __attribute((alias("xmlShellPwd__internal_alias")));
#else
#ifndef xmlShellPwd
extern __typeof (xmlShellPwd) xmlShellPwd__internal_alias __attribute((visibility("hidden")));
#define xmlShellPwd xmlShellPwd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellSave
extern __typeof (xmlShellSave) xmlShellSave __attribute((alias("xmlShellSave__internal_alias")));
#else
#ifndef xmlShellSave
extern __typeof (xmlShellSave) xmlShellSave__internal_alias __attribute((visibility("hidden")));
#define xmlShellSave xmlShellSave__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_VALID_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellValidate
extern __typeof (xmlShellValidate) xmlShellValidate __attribute((alias("xmlShellValidate__internal_alias")));
#else
#ifndef xmlShellValidate
extern __typeof (xmlShellValidate) xmlShellValidate__internal_alias __attribute((visibility("hidden")));
#define xmlShellValidate xmlShellValidate__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_DEBUG_ENABLED) && defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_debugXML
#undef xmlShellWrite
extern __typeof (xmlShellWrite) xmlShellWrite __attribute((alias("xmlShellWrite__internal_alias")));
#else
#ifndef xmlShellWrite
extern __typeof (xmlShellWrite) xmlShellWrite__internal_alias __attribute((visibility("hidden")));
#define xmlShellWrite xmlShellWrite__internal_alias
#endif
#endif
#endif

#ifdef bottom_parser
#undef xmlSkipBlankChars
extern __typeof (xmlSkipBlankChars) xmlSkipBlankChars __attribute((alias("xmlSkipBlankChars__internal_alias")));
#else
#ifndef xmlSkipBlankChars
extern __typeof (xmlSkipBlankChars) xmlSkipBlankChars__internal_alias __attribute((visibility("hidden")));
#define xmlSkipBlankChars xmlSkipBlankChars__internal_alias
#endif
#endif

#ifdef bottom_valid
#undef xmlSnprintfElementContent
extern __typeof (xmlSnprintfElementContent) xmlSnprintfElementContent __attribute((alias("xmlSnprintfElementContent__internal_alias")));
#else
#ifndef xmlSnprintfElementContent
extern __typeof (xmlSnprintfElementContent) xmlSnprintfElementContent__internal_alias __attribute((visibility("hidden")));
#define xmlSnprintfElementContent xmlSnprintfElementContent__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlSplitQName
extern __typeof (xmlSplitQName) xmlSplitQName __attribute((alias("xmlSplitQName__internal_alias")));
#else
#ifndef xmlSplitQName
extern __typeof (xmlSplitQName) xmlSplitQName__internal_alias __attribute((visibility("hidden")));
#define xmlSplitQName xmlSplitQName__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlSplitQName2
extern __typeof (xmlSplitQName2) xmlSplitQName2 __attribute((alias("xmlSplitQName2__internal_alias")));
#else
#ifndef xmlSplitQName2
extern __typeof (xmlSplitQName2) xmlSplitQName2__internal_alias __attribute((visibility("hidden")));
#define xmlSplitQName2 xmlSplitQName2__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlSplitQName3
extern __typeof (xmlSplitQName3) xmlSplitQName3 __attribute((alias("xmlSplitQName3__internal_alias")));
#else
#ifndef xmlSplitQName3
extern __typeof (xmlSplitQName3) xmlSplitQName3__internal_alias __attribute((visibility("hidden")));
#define xmlSplitQName3 xmlSplitQName3__internal_alias
#endif
#endif

#if defined(LIBXML_OUTPUT_ENABLED)
#ifdef bottom_valid
#undef xmlSprintfElementContent
extern __typeof (xmlSprintfElementContent) xmlSprintfElementContent __attribute((alias("xmlSprintfElementContent__internal_alias")));
#else
#ifndef xmlSprintfElementContent
extern __typeof (xmlSprintfElementContent) xmlSprintfElementContent__internal_alias __attribute((visibility("hidden")));
#define xmlSprintfElementContent xmlSprintfElementContent__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_PUSH_ENABLED)
#ifdef bottom_parser
#undef xmlStopParser
extern __typeof (xmlStopParser) xmlStopParser __attribute((alias("xmlStopParser__internal_alias")));
#else
#ifndef xmlStopParser
extern __typeof (xmlStopParser) xmlStopParser__internal_alias __attribute((visibility("hidden")));
#define xmlStopParser xmlStopParser__internal_alias
#endif
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrEqual
extern __typeof (xmlStrEqual) xmlStrEqual __attribute((alias("xmlStrEqual__internal_alias")));
#else
#ifndef xmlStrEqual
extern __typeof (xmlStrEqual) xmlStrEqual__internal_alias __attribute((visibility("hidden")));
#define xmlStrEqual xmlStrEqual__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrPrintf
extern __typeof (xmlStrPrintf) xmlStrPrintf __attribute((alias("xmlStrPrintf__internal_alias")));
#else
#ifndef xmlStrPrintf
extern __typeof (xmlStrPrintf) xmlStrPrintf__internal_alias __attribute((visibility("hidden")));
#define xmlStrPrintf xmlStrPrintf__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrQEqual
extern __typeof (xmlStrQEqual) xmlStrQEqual __attribute((alias("xmlStrQEqual__internal_alias")));
#else
#ifndef xmlStrQEqual
extern __typeof (xmlStrQEqual) xmlStrQEqual__internal_alias __attribute((visibility("hidden")));
#define xmlStrQEqual xmlStrQEqual__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrVPrintf
extern __typeof (xmlStrVPrintf) xmlStrVPrintf __attribute((alias("xmlStrVPrintf__internal_alias")));
#else
#ifndef xmlStrVPrintf
extern __typeof (xmlStrVPrintf) xmlStrVPrintf__internal_alias __attribute((visibility("hidden")));
#define xmlStrVPrintf xmlStrVPrintf__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrcasecmp
extern __typeof (xmlStrcasecmp) xmlStrcasecmp __attribute((alias("xmlStrcasecmp__internal_alias")));
#else
#ifndef xmlStrcasecmp
extern __typeof (xmlStrcasecmp) xmlStrcasecmp__internal_alias __attribute((visibility("hidden")));
#define xmlStrcasecmp xmlStrcasecmp__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrcasestr
extern __typeof (xmlStrcasestr) xmlStrcasestr __attribute((alias("xmlStrcasestr__internal_alias")));
#else
#ifndef xmlStrcasestr
extern __typeof (xmlStrcasestr) xmlStrcasestr__internal_alias __attribute((visibility("hidden")));
#define xmlStrcasestr xmlStrcasestr__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrcat
extern __typeof (xmlStrcat) xmlStrcat __attribute((alias("xmlStrcat__internal_alias")));
#else
#ifndef xmlStrcat
extern __typeof (xmlStrcat) xmlStrcat__internal_alias __attribute((visibility("hidden")));
#define xmlStrcat xmlStrcat__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrchr
extern __typeof (xmlStrchr) xmlStrchr __attribute((alias("xmlStrchr__internal_alias")));
#else
#ifndef xmlStrchr
extern __typeof (xmlStrchr) xmlStrchr__internal_alias __attribute((visibility("hidden")));
#define xmlStrchr xmlStrchr__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrcmp
extern __typeof (xmlStrcmp) xmlStrcmp __attribute((alias("xmlStrcmp__internal_alias")));
#else
#ifndef xmlStrcmp
extern __typeof (xmlStrcmp) xmlStrcmp__internal_alias __attribute((visibility("hidden")));
#define xmlStrcmp xmlStrcmp__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrdup
extern __typeof (xmlStrdup) xmlStrdup __attribute((alias("xmlStrdup__internal_alias")));
#else
#ifndef xmlStrdup
extern __typeof (xmlStrdup) xmlStrdup__internal_alias __attribute((visibility("hidden")));
#define xmlStrdup xmlStrdup__internal_alias
#endif
#endif

#if defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_pattern
#undef xmlStreamPop
extern __typeof (xmlStreamPop) xmlStreamPop __attribute((alias("xmlStreamPop__internal_alias")));
#else
#ifndef xmlStreamPop
extern __typeof (xmlStreamPop) xmlStreamPop__internal_alias __attribute((visibility("hidden")));
#define xmlStreamPop xmlStreamPop__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_pattern
#undef xmlStreamPush
extern __typeof (xmlStreamPush) xmlStreamPush __attribute((alias("xmlStreamPush__internal_alias")));
#else
#ifndef xmlStreamPush
extern __typeof (xmlStreamPush) xmlStreamPush__internal_alias __attribute((visibility("hidden")));
#define xmlStreamPush xmlStreamPush__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_pattern
#undef xmlStreamPushAttr
extern __typeof (xmlStreamPushAttr) xmlStreamPushAttr __attribute((alias("xmlStreamPushAttr__internal_alias")));
#else
#ifndef xmlStreamPushAttr
extern __typeof (xmlStreamPushAttr) xmlStreamPushAttr__internal_alias __attribute((visibility("hidden")));
#define xmlStreamPushAttr xmlStreamPushAttr__internal_alias
#endif
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlStringCurrentChar
extern __typeof (xmlStringCurrentChar) xmlStringCurrentChar __attribute((alias("xmlStringCurrentChar__internal_alias")));
#else
#ifndef xmlStringCurrentChar
extern __typeof (xmlStringCurrentChar) xmlStringCurrentChar__internal_alias __attribute((visibility("hidden")));
#define xmlStringCurrentChar xmlStringCurrentChar__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlStringDecodeEntities
extern __typeof (xmlStringDecodeEntities) xmlStringDecodeEntities __attribute((alias("xmlStringDecodeEntities__internal_alias")));
#else
#ifndef xmlStringDecodeEntities
extern __typeof (xmlStringDecodeEntities) xmlStringDecodeEntities__internal_alias __attribute((visibility("hidden")));
#define xmlStringDecodeEntities xmlStringDecodeEntities__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlStringGetNodeList
extern __typeof (xmlStringGetNodeList) xmlStringGetNodeList __attribute((alias("xmlStringGetNodeList__internal_alias")));
#else
#ifndef xmlStringGetNodeList
extern __typeof (xmlStringGetNodeList) xmlStringGetNodeList__internal_alias __attribute((visibility("hidden")));
#define xmlStringGetNodeList xmlStringGetNodeList__internal_alias
#endif
#endif

#ifdef bottom_parser
#undef xmlStringLenDecodeEntities
extern __typeof (xmlStringLenDecodeEntities) xmlStringLenDecodeEntities __attribute((alias("xmlStringLenDecodeEntities__internal_alias")));
#else
#ifndef xmlStringLenDecodeEntities
extern __typeof (xmlStringLenDecodeEntities) xmlStringLenDecodeEntities__internal_alias __attribute((visibility("hidden")));
#define xmlStringLenDecodeEntities xmlStringLenDecodeEntities__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlStringLenGetNodeList
extern __typeof (xmlStringLenGetNodeList) xmlStringLenGetNodeList __attribute((alias("xmlStringLenGetNodeList__internal_alias")));
#else
#ifndef xmlStringLenGetNodeList
extern __typeof (xmlStringLenGetNodeList) xmlStringLenGetNodeList__internal_alias __attribute((visibility("hidden")));
#define xmlStringLenGetNodeList xmlStringLenGetNodeList__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrlen
extern __typeof (xmlStrlen) xmlStrlen __attribute((alias("xmlStrlen__internal_alias")));
#else
#ifndef xmlStrlen
extern __typeof (xmlStrlen) xmlStrlen__internal_alias __attribute((visibility("hidden")));
#define xmlStrlen xmlStrlen__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrncasecmp
extern __typeof (xmlStrncasecmp) xmlStrncasecmp __attribute((alias("xmlStrncasecmp__internal_alias")));
#else
#ifndef xmlStrncasecmp
extern __typeof (xmlStrncasecmp) xmlStrncasecmp__internal_alias __attribute((visibility("hidden")));
#define xmlStrncasecmp xmlStrncasecmp__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrncat
extern __typeof (xmlStrncat) xmlStrncat __attribute((alias("xmlStrncat__internal_alias")));
#else
#ifndef xmlStrncat
extern __typeof (xmlStrncat) xmlStrncat__internal_alias __attribute((visibility("hidden")));
#define xmlStrncat xmlStrncat__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrncatNew
extern __typeof (xmlStrncatNew) xmlStrncatNew __attribute((alias("xmlStrncatNew__internal_alias")));
#else
#ifndef xmlStrncatNew
extern __typeof (xmlStrncatNew) xmlStrncatNew__internal_alias __attribute((visibility("hidden")));
#define xmlStrncatNew xmlStrncatNew__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrncmp
extern __typeof (xmlStrncmp) xmlStrncmp __attribute((alias("xmlStrncmp__internal_alias")));
#else
#ifndef xmlStrncmp
extern __typeof (xmlStrncmp) xmlStrncmp__internal_alias __attribute((visibility("hidden")));
#define xmlStrncmp xmlStrncmp__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrndup
extern __typeof (xmlStrndup) xmlStrndup __attribute((alias("xmlStrndup__internal_alias")));
#else
#ifndef xmlStrndup
extern __typeof (xmlStrndup) xmlStrndup__internal_alias __attribute((visibility("hidden")));
#define xmlStrndup xmlStrndup__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrstr
extern __typeof (xmlStrstr) xmlStrstr __attribute((alias("xmlStrstr__internal_alias")));
#else
#ifndef xmlStrstr
extern __typeof (xmlStrstr) xmlStrstr__internal_alias __attribute((visibility("hidden")));
#define xmlStrstr xmlStrstr__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlStrsub
extern __typeof (xmlStrsub) xmlStrsub __attribute((alias("xmlStrsub__internal_alias")));
#else
#ifndef xmlStrsub
extern __typeof (xmlStrsub) xmlStrsub__internal_alias __attribute((visibility("hidden")));
#define xmlStrsub xmlStrsub__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlSubstituteEntitiesDefault
extern __typeof (xmlSubstituteEntitiesDefault) xmlSubstituteEntitiesDefault __attribute((alias("xmlSubstituteEntitiesDefault__internal_alias")));
#else
#ifndef xmlSubstituteEntitiesDefault
extern __typeof (xmlSubstituteEntitiesDefault) xmlSubstituteEntitiesDefault__internal_alias __attribute((visibility("hidden")));
#define xmlSubstituteEntitiesDefault xmlSubstituteEntitiesDefault__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlSwitchEncoding
extern __typeof (xmlSwitchEncoding) xmlSwitchEncoding __attribute((alias("xmlSwitchEncoding__internal_alias")));
#else
#ifndef xmlSwitchEncoding
extern __typeof (xmlSwitchEncoding) xmlSwitchEncoding__internal_alias __attribute((visibility("hidden")));
#define xmlSwitchEncoding xmlSwitchEncoding__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlSwitchInputEncoding
extern __typeof (xmlSwitchInputEncoding) xmlSwitchInputEncoding __attribute((alias("xmlSwitchInputEncoding__internal_alias")));
#else
#ifndef xmlSwitchInputEncoding
extern __typeof (xmlSwitchInputEncoding) xmlSwitchInputEncoding__internal_alias __attribute((visibility("hidden")));
#define xmlSwitchInputEncoding xmlSwitchInputEncoding__internal_alias
#endif
#endif

#ifdef bottom_parserInternals
#undef xmlSwitchToEncoding
extern __typeof (xmlSwitchToEncoding) xmlSwitchToEncoding __attribute((alias("xmlSwitchToEncoding__internal_alias")));
#else
#ifndef xmlSwitchToEncoding
extern __typeof (xmlSwitchToEncoding) xmlSwitchToEncoding__internal_alias __attribute((visibility("hidden")));
#define xmlSwitchToEncoding xmlSwitchToEncoding__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlTextConcat
extern __typeof (xmlTextConcat) xmlTextConcat __attribute((alias("xmlTextConcat__internal_alias")));
#else
#ifndef xmlTextConcat
extern __typeof (xmlTextConcat) xmlTextConcat__internal_alias __attribute((visibility("hidden")));
#define xmlTextConcat xmlTextConcat__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlTextMerge
extern __typeof (xmlTextMerge) xmlTextMerge __attribute((alias("xmlTextMerge__internal_alias")));
#else
#ifndef xmlTextMerge
extern __typeof (xmlTextMerge) xmlTextMerge__internal_alias __attribute((visibility("hidden")));
#define xmlTextMerge xmlTextMerge__internal_alias
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderAttributeCount
extern __typeof (xmlTextReaderAttributeCount) xmlTextReaderAttributeCount __attribute((alias("xmlTextReaderAttributeCount__internal_alias")));
#else
#ifndef xmlTextReaderAttributeCount
extern __typeof (xmlTextReaderAttributeCount) xmlTextReaderAttributeCount__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderAttributeCount xmlTextReaderAttributeCount__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderBaseUri
extern __typeof (xmlTextReaderBaseUri) xmlTextReaderBaseUri __attribute((alias("xmlTextReaderBaseUri__internal_alias")));
#else
#ifndef xmlTextReaderBaseUri
extern __typeof (xmlTextReaderBaseUri) xmlTextReaderBaseUri__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderBaseUri xmlTextReaderBaseUri__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderByteConsumed
extern __typeof (xmlTextReaderByteConsumed) xmlTextReaderByteConsumed __attribute((alias("xmlTextReaderByteConsumed__internal_alias")));
#else
#ifndef xmlTextReaderByteConsumed
extern __typeof (xmlTextReaderByteConsumed) xmlTextReaderByteConsumed__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderByteConsumed xmlTextReaderByteConsumed__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderClose
extern __typeof (xmlTextReaderClose) xmlTextReaderClose __attribute((alias("xmlTextReaderClose__internal_alias")));
#else
#ifndef xmlTextReaderClose
extern __typeof (xmlTextReaderClose) xmlTextReaderClose__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderClose xmlTextReaderClose__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderConstBaseUri
extern __typeof (xmlTextReaderConstBaseUri) xmlTextReaderConstBaseUri __attribute((alias("xmlTextReaderConstBaseUri__internal_alias")));
#else
#ifndef xmlTextReaderConstBaseUri
extern __typeof (xmlTextReaderConstBaseUri) xmlTextReaderConstBaseUri__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderConstBaseUri xmlTextReaderConstBaseUri__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderConstEncoding
extern __typeof (xmlTextReaderConstEncoding) xmlTextReaderConstEncoding __attribute((alias("xmlTextReaderConstEncoding__internal_alias")));
#else
#ifndef xmlTextReaderConstEncoding
extern __typeof (xmlTextReaderConstEncoding) xmlTextReaderConstEncoding__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderConstEncoding xmlTextReaderConstEncoding__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderConstLocalName
extern __typeof (xmlTextReaderConstLocalName) xmlTextReaderConstLocalName __attribute((alias("xmlTextReaderConstLocalName__internal_alias")));
#else
#ifndef xmlTextReaderConstLocalName
extern __typeof (xmlTextReaderConstLocalName) xmlTextReaderConstLocalName__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderConstLocalName xmlTextReaderConstLocalName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderConstName
extern __typeof (xmlTextReaderConstName) xmlTextReaderConstName __attribute((alias("xmlTextReaderConstName__internal_alias")));
#else
#ifndef xmlTextReaderConstName
extern __typeof (xmlTextReaderConstName) xmlTextReaderConstName__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderConstName xmlTextReaderConstName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderConstNamespaceUri
extern __typeof (xmlTextReaderConstNamespaceUri) xmlTextReaderConstNamespaceUri __attribute((alias("xmlTextReaderConstNamespaceUri__internal_alias")));
#else
#ifndef xmlTextReaderConstNamespaceUri
extern __typeof (xmlTextReaderConstNamespaceUri) xmlTextReaderConstNamespaceUri__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderConstNamespaceUri xmlTextReaderConstNamespaceUri__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderConstPrefix
extern __typeof (xmlTextReaderConstPrefix) xmlTextReaderConstPrefix __attribute((alias("xmlTextReaderConstPrefix__internal_alias")));
#else
#ifndef xmlTextReaderConstPrefix
extern __typeof (xmlTextReaderConstPrefix) xmlTextReaderConstPrefix__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderConstPrefix xmlTextReaderConstPrefix__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderConstString
extern __typeof (xmlTextReaderConstString) xmlTextReaderConstString __attribute((alias("xmlTextReaderConstString__internal_alias")));
#else
#ifndef xmlTextReaderConstString
extern __typeof (xmlTextReaderConstString) xmlTextReaderConstString__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderConstString xmlTextReaderConstString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderConstValue
extern __typeof (xmlTextReaderConstValue) xmlTextReaderConstValue __attribute((alias("xmlTextReaderConstValue__internal_alias")));
#else
#ifndef xmlTextReaderConstValue
extern __typeof (xmlTextReaderConstValue) xmlTextReaderConstValue__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderConstValue xmlTextReaderConstValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderConstXmlLang
extern __typeof (xmlTextReaderConstXmlLang) xmlTextReaderConstXmlLang __attribute((alias("xmlTextReaderConstXmlLang__internal_alias")));
#else
#ifndef xmlTextReaderConstXmlLang
extern __typeof (xmlTextReaderConstXmlLang) xmlTextReaderConstXmlLang__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderConstXmlLang xmlTextReaderConstXmlLang__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderConstXmlVersion
extern __typeof (xmlTextReaderConstXmlVersion) xmlTextReaderConstXmlVersion __attribute((alias("xmlTextReaderConstXmlVersion__internal_alias")));
#else
#ifndef xmlTextReaderConstXmlVersion
extern __typeof (xmlTextReaderConstXmlVersion) xmlTextReaderConstXmlVersion__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderConstXmlVersion xmlTextReaderConstXmlVersion__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderCurrentDoc
extern __typeof (xmlTextReaderCurrentDoc) xmlTextReaderCurrentDoc __attribute((alias("xmlTextReaderCurrentDoc__internal_alias")));
#else
#ifndef xmlTextReaderCurrentDoc
extern __typeof (xmlTextReaderCurrentDoc) xmlTextReaderCurrentDoc__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderCurrentDoc xmlTextReaderCurrentDoc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderCurrentNode
extern __typeof (xmlTextReaderCurrentNode) xmlTextReaderCurrentNode __attribute((alias("xmlTextReaderCurrentNode__internal_alias")));
#else
#ifndef xmlTextReaderCurrentNode
extern __typeof (xmlTextReaderCurrentNode) xmlTextReaderCurrentNode__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderCurrentNode xmlTextReaderCurrentNode__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderDepth
extern __typeof (xmlTextReaderDepth) xmlTextReaderDepth __attribute((alias("xmlTextReaderDepth__internal_alias")));
#else
#ifndef xmlTextReaderDepth
extern __typeof (xmlTextReaderDepth) xmlTextReaderDepth__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderDepth xmlTextReaderDepth__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderExpand
extern __typeof (xmlTextReaderExpand) xmlTextReaderExpand __attribute((alias("xmlTextReaderExpand__internal_alias")));
#else
#ifndef xmlTextReaderExpand
extern __typeof (xmlTextReaderExpand) xmlTextReaderExpand__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderExpand xmlTextReaderExpand__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderGetAttribute
extern __typeof (xmlTextReaderGetAttribute) xmlTextReaderGetAttribute __attribute((alias("xmlTextReaderGetAttribute__internal_alias")));
#else
#ifndef xmlTextReaderGetAttribute
extern __typeof (xmlTextReaderGetAttribute) xmlTextReaderGetAttribute__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderGetAttribute xmlTextReaderGetAttribute__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderGetAttributeNo
extern __typeof (xmlTextReaderGetAttributeNo) xmlTextReaderGetAttributeNo __attribute((alias("xmlTextReaderGetAttributeNo__internal_alias")));
#else
#ifndef xmlTextReaderGetAttributeNo
extern __typeof (xmlTextReaderGetAttributeNo) xmlTextReaderGetAttributeNo__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderGetAttributeNo xmlTextReaderGetAttributeNo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderGetAttributeNs
extern __typeof (xmlTextReaderGetAttributeNs) xmlTextReaderGetAttributeNs __attribute((alias("xmlTextReaderGetAttributeNs__internal_alias")));
#else
#ifndef xmlTextReaderGetAttributeNs
extern __typeof (xmlTextReaderGetAttributeNs) xmlTextReaderGetAttributeNs__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderGetAttributeNs xmlTextReaderGetAttributeNs__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderGetErrorHandler
extern __typeof (xmlTextReaderGetErrorHandler) xmlTextReaderGetErrorHandler __attribute((alias("xmlTextReaderGetErrorHandler__internal_alias")));
#else
#ifndef xmlTextReaderGetErrorHandler
extern __typeof (xmlTextReaderGetErrorHandler) xmlTextReaderGetErrorHandler__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderGetErrorHandler xmlTextReaderGetErrorHandler__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderGetParserColumnNumber
extern __typeof (xmlTextReaderGetParserColumnNumber) xmlTextReaderGetParserColumnNumber __attribute((alias("xmlTextReaderGetParserColumnNumber__internal_alias")));
#else
#ifndef xmlTextReaderGetParserColumnNumber
extern __typeof (xmlTextReaderGetParserColumnNumber) xmlTextReaderGetParserColumnNumber__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderGetParserColumnNumber xmlTextReaderGetParserColumnNumber__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderGetParserLineNumber
extern __typeof (xmlTextReaderGetParserLineNumber) xmlTextReaderGetParserLineNumber __attribute((alias("xmlTextReaderGetParserLineNumber__internal_alias")));
#else
#ifndef xmlTextReaderGetParserLineNumber
extern __typeof (xmlTextReaderGetParserLineNumber) xmlTextReaderGetParserLineNumber__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderGetParserLineNumber xmlTextReaderGetParserLineNumber__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderGetParserProp
extern __typeof (xmlTextReaderGetParserProp) xmlTextReaderGetParserProp __attribute((alias("xmlTextReaderGetParserProp__internal_alias")));
#else
#ifndef xmlTextReaderGetParserProp
extern __typeof (xmlTextReaderGetParserProp) xmlTextReaderGetParserProp__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderGetParserProp xmlTextReaderGetParserProp__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderGetRemainder
extern __typeof (xmlTextReaderGetRemainder) xmlTextReaderGetRemainder __attribute((alias("xmlTextReaderGetRemainder__internal_alias")));
#else
#ifndef xmlTextReaderGetRemainder
extern __typeof (xmlTextReaderGetRemainder) xmlTextReaderGetRemainder__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderGetRemainder xmlTextReaderGetRemainder__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderHasAttributes
extern __typeof (xmlTextReaderHasAttributes) xmlTextReaderHasAttributes __attribute((alias("xmlTextReaderHasAttributes__internal_alias")));
#else
#ifndef xmlTextReaderHasAttributes
extern __typeof (xmlTextReaderHasAttributes) xmlTextReaderHasAttributes__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderHasAttributes xmlTextReaderHasAttributes__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderHasValue
extern __typeof (xmlTextReaderHasValue) xmlTextReaderHasValue __attribute((alias("xmlTextReaderHasValue__internal_alias")));
#else
#ifndef xmlTextReaderHasValue
extern __typeof (xmlTextReaderHasValue) xmlTextReaderHasValue__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderHasValue xmlTextReaderHasValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderIsDefault
extern __typeof (xmlTextReaderIsDefault) xmlTextReaderIsDefault __attribute((alias("xmlTextReaderIsDefault__internal_alias")));
#else
#ifndef xmlTextReaderIsDefault
extern __typeof (xmlTextReaderIsDefault) xmlTextReaderIsDefault__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderIsDefault xmlTextReaderIsDefault__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderIsEmptyElement
extern __typeof (xmlTextReaderIsEmptyElement) xmlTextReaderIsEmptyElement __attribute((alias("xmlTextReaderIsEmptyElement__internal_alias")));
#else
#ifndef xmlTextReaderIsEmptyElement
extern __typeof (xmlTextReaderIsEmptyElement) xmlTextReaderIsEmptyElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderIsEmptyElement xmlTextReaderIsEmptyElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderIsNamespaceDecl
extern __typeof (xmlTextReaderIsNamespaceDecl) xmlTextReaderIsNamespaceDecl __attribute((alias("xmlTextReaderIsNamespaceDecl__internal_alias")));
#else
#ifndef xmlTextReaderIsNamespaceDecl
extern __typeof (xmlTextReaderIsNamespaceDecl) xmlTextReaderIsNamespaceDecl__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderIsNamespaceDecl xmlTextReaderIsNamespaceDecl__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderIsValid
extern __typeof (xmlTextReaderIsValid) xmlTextReaderIsValid __attribute((alias("xmlTextReaderIsValid__internal_alias")));
#else
#ifndef xmlTextReaderIsValid
extern __typeof (xmlTextReaderIsValid) xmlTextReaderIsValid__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderIsValid xmlTextReaderIsValid__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderLocalName
extern __typeof (xmlTextReaderLocalName) xmlTextReaderLocalName __attribute((alias("xmlTextReaderLocalName__internal_alias")));
#else
#ifndef xmlTextReaderLocalName
extern __typeof (xmlTextReaderLocalName) xmlTextReaderLocalName__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderLocalName xmlTextReaderLocalName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderLocatorBaseURI
extern __typeof (xmlTextReaderLocatorBaseURI) xmlTextReaderLocatorBaseURI __attribute((alias("xmlTextReaderLocatorBaseURI__internal_alias")));
#else
#ifndef xmlTextReaderLocatorBaseURI
extern __typeof (xmlTextReaderLocatorBaseURI) xmlTextReaderLocatorBaseURI__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderLocatorBaseURI xmlTextReaderLocatorBaseURI__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderLocatorLineNumber
extern __typeof (xmlTextReaderLocatorLineNumber) xmlTextReaderLocatorLineNumber __attribute((alias("xmlTextReaderLocatorLineNumber__internal_alias")));
#else
#ifndef xmlTextReaderLocatorLineNumber
extern __typeof (xmlTextReaderLocatorLineNumber) xmlTextReaderLocatorLineNumber__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderLocatorLineNumber xmlTextReaderLocatorLineNumber__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderLookupNamespace
extern __typeof (xmlTextReaderLookupNamespace) xmlTextReaderLookupNamespace __attribute((alias("xmlTextReaderLookupNamespace__internal_alias")));
#else
#ifndef xmlTextReaderLookupNamespace
extern __typeof (xmlTextReaderLookupNamespace) xmlTextReaderLookupNamespace__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderLookupNamespace xmlTextReaderLookupNamespace__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderMoveToAttribute
extern __typeof (xmlTextReaderMoveToAttribute) xmlTextReaderMoveToAttribute __attribute((alias("xmlTextReaderMoveToAttribute__internal_alias")));
#else
#ifndef xmlTextReaderMoveToAttribute
extern __typeof (xmlTextReaderMoveToAttribute) xmlTextReaderMoveToAttribute__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderMoveToAttribute xmlTextReaderMoveToAttribute__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderMoveToAttributeNo
extern __typeof (xmlTextReaderMoveToAttributeNo) xmlTextReaderMoveToAttributeNo __attribute((alias("xmlTextReaderMoveToAttributeNo__internal_alias")));
#else
#ifndef xmlTextReaderMoveToAttributeNo
extern __typeof (xmlTextReaderMoveToAttributeNo) xmlTextReaderMoveToAttributeNo__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderMoveToAttributeNo xmlTextReaderMoveToAttributeNo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderMoveToAttributeNs
extern __typeof (xmlTextReaderMoveToAttributeNs) xmlTextReaderMoveToAttributeNs __attribute((alias("xmlTextReaderMoveToAttributeNs__internal_alias")));
#else
#ifndef xmlTextReaderMoveToAttributeNs
extern __typeof (xmlTextReaderMoveToAttributeNs) xmlTextReaderMoveToAttributeNs__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderMoveToAttributeNs xmlTextReaderMoveToAttributeNs__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderMoveToElement
extern __typeof (xmlTextReaderMoveToElement) xmlTextReaderMoveToElement __attribute((alias("xmlTextReaderMoveToElement__internal_alias")));
#else
#ifndef xmlTextReaderMoveToElement
extern __typeof (xmlTextReaderMoveToElement) xmlTextReaderMoveToElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderMoveToElement xmlTextReaderMoveToElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderMoveToFirstAttribute
extern __typeof (xmlTextReaderMoveToFirstAttribute) xmlTextReaderMoveToFirstAttribute __attribute((alias("xmlTextReaderMoveToFirstAttribute__internal_alias")));
#else
#ifndef xmlTextReaderMoveToFirstAttribute
extern __typeof (xmlTextReaderMoveToFirstAttribute) xmlTextReaderMoveToFirstAttribute__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderMoveToFirstAttribute xmlTextReaderMoveToFirstAttribute__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderMoveToNextAttribute
extern __typeof (xmlTextReaderMoveToNextAttribute) xmlTextReaderMoveToNextAttribute __attribute((alias("xmlTextReaderMoveToNextAttribute__internal_alias")));
#else
#ifndef xmlTextReaderMoveToNextAttribute
extern __typeof (xmlTextReaderMoveToNextAttribute) xmlTextReaderMoveToNextAttribute__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderMoveToNextAttribute xmlTextReaderMoveToNextAttribute__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderName
extern __typeof (xmlTextReaderName) xmlTextReaderName __attribute((alias("xmlTextReaderName__internal_alias")));
#else
#ifndef xmlTextReaderName
extern __typeof (xmlTextReaderName) xmlTextReaderName__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderName xmlTextReaderName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderNamespaceUri
extern __typeof (xmlTextReaderNamespaceUri) xmlTextReaderNamespaceUri __attribute((alias("xmlTextReaderNamespaceUri__internal_alias")));
#else
#ifndef xmlTextReaderNamespaceUri
extern __typeof (xmlTextReaderNamespaceUri) xmlTextReaderNamespaceUri__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderNamespaceUri xmlTextReaderNamespaceUri__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderNext
extern __typeof (xmlTextReaderNext) xmlTextReaderNext __attribute((alias("xmlTextReaderNext__internal_alias")));
#else
#ifndef xmlTextReaderNext
extern __typeof (xmlTextReaderNext) xmlTextReaderNext__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderNext xmlTextReaderNext__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderNextSibling
extern __typeof (xmlTextReaderNextSibling) xmlTextReaderNextSibling __attribute((alias("xmlTextReaderNextSibling__internal_alias")));
#else
#ifndef xmlTextReaderNextSibling
extern __typeof (xmlTextReaderNextSibling) xmlTextReaderNextSibling__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderNextSibling xmlTextReaderNextSibling__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderNodeType
extern __typeof (xmlTextReaderNodeType) xmlTextReaderNodeType __attribute((alias("xmlTextReaderNodeType__internal_alias")));
#else
#ifndef xmlTextReaderNodeType
extern __typeof (xmlTextReaderNodeType) xmlTextReaderNodeType__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderNodeType xmlTextReaderNodeType__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderNormalization
extern __typeof (xmlTextReaderNormalization) xmlTextReaderNormalization __attribute((alias("xmlTextReaderNormalization__internal_alias")));
#else
#ifndef xmlTextReaderNormalization
extern __typeof (xmlTextReaderNormalization) xmlTextReaderNormalization__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderNormalization xmlTextReaderNormalization__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderPrefix
extern __typeof (xmlTextReaderPrefix) xmlTextReaderPrefix __attribute((alias("xmlTextReaderPrefix__internal_alias")));
#else
#ifndef xmlTextReaderPrefix
extern __typeof (xmlTextReaderPrefix) xmlTextReaderPrefix__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderPrefix xmlTextReaderPrefix__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderPreserve
extern __typeof (xmlTextReaderPreserve) xmlTextReaderPreserve __attribute((alias("xmlTextReaderPreserve__internal_alias")));
#else
#ifndef xmlTextReaderPreserve
extern __typeof (xmlTextReaderPreserve) xmlTextReaderPreserve__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderPreserve xmlTextReaderPreserve__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED) && defined(LIBXML_PATTERN_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderPreservePattern
extern __typeof (xmlTextReaderPreservePattern) xmlTextReaderPreservePattern __attribute((alias("xmlTextReaderPreservePattern__internal_alias")));
#else
#ifndef xmlTextReaderPreservePattern
extern __typeof (xmlTextReaderPreservePattern) xmlTextReaderPreservePattern__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderPreservePattern xmlTextReaderPreservePattern__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderQuoteChar
extern __typeof (xmlTextReaderQuoteChar) xmlTextReaderQuoteChar __attribute((alias("xmlTextReaderQuoteChar__internal_alias")));
#else
#ifndef xmlTextReaderQuoteChar
extern __typeof (xmlTextReaderQuoteChar) xmlTextReaderQuoteChar__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderQuoteChar xmlTextReaderQuoteChar__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderRead
extern __typeof (xmlTextReaderRead) xmlTextReaderRead __attribute((alias("xmlTextReaderRead__internal_alias")));
#else
#ifndef xmlTextReaderRead
extern __typeof (xmlTextReaderRead) xmlTextReaderRead__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderRead xmlTextReaderRead__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderReadAttributeValue
extern __typeof (xmlTextReaderReadAttributeValue) xmlTextReaderReadAttributeValue __attribute((alias("xmlTextReaderReadAttributeValue__internal_alias")));
#else
#ifndef xmlTextReaderReadAttributeValue
extern __typeof (xmlTextReaderReadAttributeValue) xmlTextReaderReadAttributeValue__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderReadAttributeValue xmlTextReaderReadAttributeValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderReadInnerXml
extern __typeof (xmlTextReaderReadInnerXml) xmlTextReaderReadInnerXml __attribute((alias("xmlTextReaderReadInnerXml__internal_alias")));
#else
#ifndef xmlTextReaderReadInnerXml
extern __typeof (xmlTextReaderReadInnerXml) xmlTextReaderReadInnerXml__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderReadInnerXml xmlTextReaderReadInnerXml__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderReadOuterXml
extern __typeof (xmlTextReaderReadOuterXml) xmlTextReaderReadOuterXml __attribute((alias("xmlTextReaderReadOuterXml__internal_alias")));
#else
#ifndef xmlTextReaderReadOuterXml
extern __typeof (xmlTextReaderReadOuterXml) xmlTextReaderReadOuterXml__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderReadOuterXml xmlTextReaderReadOuterXml__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderReadState
extern __typeof (xmlTextReaderReadState) xmlTextReaderReadState __attribute((alias("xmlTextReaderReadState__internal_alias")));
#else
#ifndef xmlTextReaderReadState
extern __typeof (xmlTextReaderReadState) xmlTextReaderReadState__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderReadState xmlTextReaderReadState__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderReadString
extern __typeof (xmlTextReaderReadString) xmlTextReaderReadString __attribute((alias("xmlTextReaderReadString__internal_alias")));
#else
#ifndef xmlTextReaderReadString
extern __typeof (xmlTextReaderReadString) xmlTextReaderReadString__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderReadString xmlTextReaderReadString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED) && defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderRelaxNGSetSchema
extern __typeof (xmlTextReaderRelaxNGSetSchema) xmlTextReaderRelaxNGSetSchema __attribute((alias("xmlTextReaderRelaxNGSetSchema__internal_alias")));
#else
#ifndef xmlTextReaderRelaxNGSetSchema
extern __typeof (xmlTextReaderRelaxNGSetSchema) xmlTextReaderRelaxNGSetSchema__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderRelaxNGSetSchema xmlTextReaderRelaxNGSetSchema__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED) && defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderRelaxNGValidate
extern __typeof (xmlTextReaderRelaxNGValidate) xmlTextReaderRelaxNGValidate __attribute((alias("xmlTextReaderRelaxNGValidate__internal_alias")));
#else
#ifndef xmlTextReaderRelaxNGValidate
extern __typeof (xmlTextReaderRelaxNGValidate) xmlTextReaderRelaxNGValidate__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderRelaxNGValidate xmlTextReaderRelaxNGValidate__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderSetErrorHandler
extern __typeof (xmlTextReaderSetErrorHandler) xmlTextReaderSetErrorHandler __attribute((alias("xmlTextReaderSetErrorHandler__internal_alias")));
#else
#ifndef xmlTextReaderSetErrorHandler
extern __typeof (xmlTextReaderSetErrorHandler) xmlTextReaderSetErrorHandler__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderSetErrorHandler xmlTextReaderSetErrorHandler__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderSetParserProp
extern __typeof (xmlTextReaderSetParserProp) xmlTextReaderSetParserProp __attribute((alias("xmlTextReaderSetParserProp__internal_alias")));
#else
#ifndef xmlTextReaderSetParserProp
extern __typeof (xmlTextReaderSetParserProp) xmlTextReaderSetParserProp__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderSetParserProp xmlTextReaderSetParserProp__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderSetStructuredErrorHandler
extern __typeof (xmlTextReaderSetStructuredErrorHandler) xmlTextReaderSetStructuredErrorHandler __attribute((alias("xmlTextReaderSetStructuredErrorHandler__internal_alias")));
#else
#ifndef xmlTextReaderSetStructuredErrorHandler
extern __typeof (xmlTextReaderSetStructuredErrorHandler) xmlTextReaderSetStructuredErrorHandler__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderSetStructuredErrorHandler xmlTextReaderSetStructuredErrorHandler__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderStandalone
extern __typeof (xmlTextReaderStandalone) xmlTextReaderStandalone __attribute((alias("xmlTextReaderStandalone__internal_alias")));
#else
#ifndef xmlTextReaderStandalone
extern __typeof (xmlTextReaderStandalone) xmlTextReaderStandalone__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderStandalone xmlTextReaderStandalone__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderValue
extern __typeof (xmlTextReaderValue) xmlTextReaderValue __attribute((alias("xmlTextReaderValue__internal_alias")));
#else
#ifndef xmlTextReaderValue
extern __typeof (xmlTextReaderValue) xmlTextReaderValue__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderValue xmlTextReaderValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_READER_ENABLED)
#ifdef bottom_xmlreader
#undef xmlTextReaderXmlLang
extern __typeof (xmlTextReaderXmlLang) xmlTextReaderXmlLang __attribute((alias("xmlTextReaderXmlLang__internal_alias")));
#else
#ifndef xmlTextReaderXmlLang
extern __typeof (xmlTextReaderXmlLang) xmlTextReaderXmlLang__internal_alias __attribute((visibility("hidden")));
#define xmlTextReaderXmlLang xmlTextReaderXmlLang__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterEndAttribute
extern __typeof (xmlTextWriterEndAttribute) xmlTextWriterEndAttribute __attribute((alias("xmlTextWriterEndAttribute__internal_alias")));
#else
#ifndef xmlTextWriterEndAttribute
extern __typeof (xmlTextWriterEndAttribute) xmlTextWriterEndAttribute__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterEndAttribute xmlTextWriterEndAttribute__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterEndCDATA
extern __typeof (xmlTextWriterEndCDATA) xmlTextWriterEndCDATA __attribute((alias("xmlTextWriterEndCDATA__internal_alias")));
#else
#ifndef xmlTextWriterEndCDATA
extern __typeof (xmlTextWriterEndCDATA) xmlTextWriterEndCDATA__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterEndCDATA xmlTextWriterEndCDATA__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterEndComment
extern __typeof (xmlTextWriterEndComment) xmlTextWriterEndComment __attribute((alias("xmlTextWriterEndComment__internal_alias")));
#else
#ifndef xmlTextWriterEndComment
extern __typeof (xmlTextWriterEndComment) xmlTextWriterEndComment__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterEndComment xmlTextWriterEndComment__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterEndDTD
extern __typeof (xmlTextWriterEndDTD) xmlTextWriterEndDTD __attribute((alias("xmlTextWriterEndDTD__internal_alias")));
#else
#ifndef xmlTextWriterEndDTD
extern __typeof (xmlTextWriterEndDTD) xmlTextWriterEndDTD__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterEndDTD xmlTextWriterEndDTD__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterEndDTDAttlist
extern __typeof (xmlTextWriterEndDTDAttlist) xmlTextWriterEndDTDAttlist __attribute((alias("xmlTextWriterEndDTDAttlist__internal_alias")));
#else
#ifndef xmlTextWriterEndDTDAttlist
extern __typeof (xmlTextWriterEndDTDAttlist) xmlTextWriterEndDTDAttlist__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterEndDTDAttlist xmlTextWriterEndDTDAttlist__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterEndDTDElement
extern __typeof (xmlTextWriterEndDTDElement) xmlTextWriterEndDTDElement __attribute((alias("xmlTextWriterEndDTDElement__internal_alias")));
#else
#ifndef xmlTextWriterEndDTDElement
extern __typeof (xmlTextWriterEndDTDElement) xmlTextWriterEndDTDElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterEndDTDElement xmlTextWriterEndDTDElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterEndDTDEntity
extern __typeof (xmlTextWriterEndDTDEntity) xmlTextWriterEndDTDEntity __attribute((alias("xmlTextWriterEndDTDEntity__internal_alias")));
#else
#ifndef xmlTextWriterEndDTDEntity
extern __typeof (xmlTextWriterEndDTDEntity) xmlTextWriterEndDTDEntity__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterEndDTDEntity xmlTextWriterEndDTDEntity__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterEndDocument
extern __typeof (xmlTextWriterEndDocument) xmlTextWriterEndDocument __attribute((alias("xmlTextWriterEndDocument__internal_alias")));
#else
#ifndef xmlTextWriterEndDocument
extern __typeof (xmlTextWriterEndDocument) xmlTextWriterEndDocument__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterEndDocument xmlTextWriterEndDocument__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterEndElement
extern __typeof (xmlTextWriterEndElement) xmlTextWriterEndElement __attribute((alias("xmlTextWriterEndElement__internal_alias")));
#else
#ifndef xmlTextWriterEndElement
extern __typeof (xmlTextWriterEndElement) xmlTextWriterEndElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterEndElement xmlTextWriterEndElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterEndPI
extern __typeof (xmlTextWriterEndPI) xmlTextWriterEndPI __attribute((alias("xmlTextWriterEndPI__internal_alias")));
#else
#ifndef xmlTextWriterEndPI
extern __typeof (xmlTextWriterEndPI) xmlTextWriterEndPI__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterEndPI xmlTextWriterEndPI__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterFlush
extern __typeof (xmlTextWriterFlush) xmlTextWriterFlush __attribute((alias("xmlTextWriterFlush__internal_alias")));
#else
#ifndef xmlTextWriterFlush
extern __typeof (xmlTextWriterFlush) xmlTextWriterFlush__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterFlush xmlTextWriterFlush__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterFullEndElement
extern __typeof (xmlTextWriterFullEndElement) xmlTextWriterFullEndElement __attribute((alias("xmlTextWriterFullEndElement__internal_alias")));
#else
#ifndef xmlTextWriterFullEndElement
extern __typeof (xmlTextWriterFullEndElement) xmlTextWriterFullEndElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterFullEndElement xmlTextWriterFullEndElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterSetIndent
extern __typeof (xmlTextWriterSetIndent) xmlTextWriterSetIndent __attribute((alias("xmlTextWriterSetIndent__internal_alias")));
#else
#ifndef xmlTextWriterSetIndent
extern __typeof (xmlTextWriterSetIndent) xmlTextWriterSetIndent__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterSetIndent xmlTextWriterSetIndent__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterSetIndentString
extern __typeof (xmlTextWriterSetIndentString) xmlTextWriterSetIndentString __attribute((alias("xmlTextWriterSetIndentString__internal_alias")));
#else
#ifndef xmlTextWriterSetIndentString
extern __typeof (xmlTextWriterSetIndentString) xmlTextWriterSetIndentString__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterSetIndentString xmlTextWriterSetIndentString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterStartAttribute
extern __typeof (xmlTextWriterStartAttribute) xmlTextWriterStartAttribute __attribute((alias("xmlTextWriterStartAttribute__internal_alias")));
#else
#ifndef xmlTextWriterStartAttribute
extern __typeof (xmlTextWriterStartAttribute) xmlTextWriterStartAttribute__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterStartAttribute xmlTextWriterStartAttribute__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterStartAttributeNS
extern __typeof (xmlTextWriterStartAttributeNS) xmlTextWriterStartAttributeNS __attribute((alias("xmlTextWriterStartAttributeNS__internal_alias")));
#else
#ifndef xmlTextWriterStartAttributeNS
extern __typeof (xmlTextWriterStartAttributeNS) xmlTextWriterStartAttributeNS__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterStartAttributeNS xmlTextWriterStartAttributeNS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterStartCDATA
extern __typeof (xmlTextWriterStartCDATA) xmlTextWriterStartCDATA __attribute((alias("xmlTextWriterStartCDATA__internal_alias")));
#else
#ifndef xmlTextWriterStartCDATA
extern __typeof (xmlTextWriterStartCDATA) xmlTextWriterStartCDATA__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterStartCDATA xmlTextWriterStartCDATA__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterStartComment
extern __typeof (xmlTextWriterStartComment) xmlTextWriterStartComment __attribute((alias("xmlTextWriterStartComment__internal_alias")));
#else
#ifndef xmlTextWriterStartComment
extern __typeof (xmlTextWriterStartComment) xmlTextWriterStartComment__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterStartComment xmlTextWriterStartComment__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterStartDTD
extern __typeof (xmlTextWriterStartDTD) xmlTextWriterStartDTD __attribute((alias("xmlTextWriterStartDTD__internal_alias")));
#else
#ifndef xmlTextWriterStartDTD
extern __typeof (xmlTextWriterStartDTD) xmlTextWriterStartDTD__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterStartDTD xmlTextWriterStartDTD__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterStartDTDAttlist
extern __typeof (xmlTextWriterStartDTDAttlist) xmlTextWriterStartDTDAttlist __attribute((alias("xmlTextWriterStartDTDAttlist__internal_alias")));
#else
#ifndef xmlTextWriterStartDTDAttlist
extern __typeof (xmlTextWriterStartDTDAttlist) xmlTextWriterStartDTDAttlist__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterStartDTDAttlist xmlTextWriterStartDTDAttlist__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterStartDTDElement
extern __typeof (xmlTextWriterStartDTDElement) xmlTextWriterStartDTDElement __attribute((alias("xmlTextWriterStartDTDElement__internal_alias")));
#else
#ifndef xmlTextWriterStartDTDElement
extern __typeof (xmlTextWriterStartDTDElement) xmlTextWriterStartDTDElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterStartDTDElement xmlTextWriterStartDTDElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterStartDTDEntity
extern __typeof (xmlTextWriterStartDTDEntity) xmlTextWriterStartDTDEntity __attribute((alias("xmlTextWriterStartDTDEntity__internal_alias")));
#else
#ifndef xmlTextWriterStartDTDEntity
extern __typeof (xmlTextWriterStartDTDEntity) xmlTextWriterStartDTDEntity__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterStartDTDEntity xmlTextWriterStartDTDEntity__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterStartDocument
extern __typeof (xmlTextWriterStartDocument) xmlTextWriterStartDocument __attribute((alias("xmlTextWriterStartDocument__internal_alias")));
#else
#ifndef xmlTextWriterStartDocument
extern __typeof (xmlTextWriterStartDocument) xmlTextWriterStartDocument__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterStartDocument xmlTextWriterStartDocument__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterStartElement
extern __typeof (xmlTextWriterStartElement) xmlTextWriterStartElement __attribute((alias("xmlTextWriterStartElement__internal_alias")));
#else
#ifndef xmlTextWriterStartElement
extern __typeof (xmlTextWriterStartElement) xmlTextWriterStartElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterStartElement xmlTextWriterStartElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterStartElementNS
extern __typeof (xmlTextWriterStartElementNS) xmlTextWriterStartElementNS __attribute((alias("xmlTextWriterStartElementNS__internal_alias")));
#else
#ifndef xmlTextWriterStartElementNS
extern __typeof (xmlTextWriterStartElementNS) xmlTextWriterStartElementNS__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterStartElementNS xmlTextWriterStartElementNS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterStartPI
extern __typeof (xmlTextWriterStartPI) xmlTextWriterStartPI __attribute((alias("xmlTextWriterStartPI__internal_alias")));
#else
#ifndef xmlTextWriterStartPI
extern __typeof (xmlTextWriterStartPI) xmlTextWriterStartPI__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterStartPI xmlTextWriterStartPI__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteAttribute
extern __typeof (xmlTextWriterWriteAttribute) xmlTextWriterWriteAttribute __attribute((alias("xmlTextWriterWriteAttribute__internal_alias")));
#else
#ifndef xmlTextWriterWriteAttribute
extern __typeof (xmlTextWriterWriteAttribute) xmlTextWriterWriteAttribute__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteAttribute xmlTextWriterWriteAttribute__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteAttributeNS
extern __typeof (xmlTextWriterWriteAttributeNS) xmlTextWriterWriteAttributeNS __attribute((alias("xmlTextWriterWriteAttributeNS__internal_alias")));
#else
#ifndef xmlTextWriterWriteAttributeNS
extern __typeof (xmlTextWriterWriteAttributeNS) xmlTextWriterWriteAttributeNS__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteAttributeNS xmlTextWriterWriteAttributeNS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteBase64
extern __typeof (xmlTextWriterWriteBase64) xmlTextWriterWriteBase64 __attribute((alias("xmlTextWriterWriteBase64__internal_alias")));
#else
#ifndef xmlTextWriterWriteBase64
extern __typeof (xmlTextWriterWriteBase64) xmlTextWriterWriteBase64__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteBase64 xmlTextWriterWriteBase64__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteBinHex
extern __typeof (xmlTextWriterWriteBinHex) xmlTextWriterWriteBinHex __attribute((alias("xmlTextWriterWriteBinHex__internal_alias")));
#else
#ifndef xmlTextWriterWriteBinHex
extern __typeof (xmlTextWriterWriteBinHex) xmlTextWriterWriteBinHex__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteBinHex xmlTextWriterWriteBinHex__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteCDATA
extern __typeof (xmlTextWriterWriteCDATA) xmlTextWriterWriteCDATA __attribute((alias("xmlTextWriterWriteCDATA__internal_alias")));
#else
#ifndef xmlTextWriterWriteCDATA
extern __typeof (xmlTextWriterWriteCDATA) xmlTextWriterWriteCDATA__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteCDATA xmlTextWriterWriteCDATA__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteComment
extern __typeof (xmlTextWriterWriteComment) xmlTextWriterWriteComment __attribute((alias("xmlTextWriterWriteComment__internal_alias")));
#else
#ifndef xmlTextWriterWriteComment
extern __typeof (xmlTextWriterWriteComment) xmlTextWriterWriteComment__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteComment xmlTextWriterWriteComment__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteDTD
extern __typeof (xmlTextWriterWriteDTD) xmlTextWriterWriteDTD __attribute((alias("xmlTextWriterWriteDTD__internal_alias")));
#else
#ifndef xmlTextWriterWriteDTD
extern __typeof (xmlTextWriterWriteDTD) xmlTextWriterWriteDTD__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteDTD xmlTextWriterWriteDTD__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteDTDAttlist
extern __typeof (xmlTextWriterWriteDTDAttlist) xmlTextWriterWriteDTDAttlist __attribute((alias("xmlTextWriterWriteDTDAttlist__internal_alias")));
#else
#ifndef xmlTextWriterWriteDTDAttlist
extern __typeof (xmlTextWriterWriteDTDAttlist) xmlTextWriterWriteDTDAttlist__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteDTDAttlist xmlTextWriterWriteDTDAttlist__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteDTDElement
extern __typeof (xmlTextWriterWriteDTDElement) xmlTextWriterWriteDTDElement __attribute((alias("xmlTextWriterWriteDTDElement__internal_alias")));
#else
#ifndef xmlTextWriterWriteDTDElement
extern __typeof (xmlTextWriterWriteDTDElement) xmlTextWriterWriteDTDElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteDTDElement xmlTextWriterWriteDTDElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteDTDEntity
extern __typeof (xmlTextWriterWriteDTDEntity) xmlTextWriterWriteDTDEntity __attribute((alias("xmlTextWriterWriteDTDEntity__internal_alias")));
#else
#ifndef xmlTextWriterWriteDTDEntity
extern __typeof (xmlTextWriterWriteDTDEntity) xmlTextWriterWriteDTDEntity__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteDTDEntity xmlTextWriterWriteDTDEntity__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteDTDExternalEntity
extern __typeof (xmlTextWriterWriteDTDExternalEntity) xmlTextWriterWriteDTDExternalEntity __attribute((alias("xmlTextWriterWriteDTDExternalEntity__internal_alias")));
#else
#ifndef xmlTextWriterWriteDTDExternalEntity
extern __typeof (xmlTextWriterWriteDTDExternalEntity) xmlTextWriterWriteDTDExternalEntity__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteDTDExternalEntity xmlTextWriterWriteDTDExternalEntity__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteDTDExternalEntityContents
extern __typeof (xmlTextWriterWriteDTDExternalEntityContents) xmlTextWriterWriteDTDExternalEntityContents __attribute((alias("xmlTextWriterWriteDTDExternalEntityContents__internal_alias")));
#else
#ifndef xmlTextWriterWriteDTDExternalEntityContents
extern __typeof (xmlTextWriterWriteDTDExternalEntityContents) xmlTextWriterWriteDTDExternalEntityContents__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteDTDExternalEntityContents xmlTextWriterWriteDTDExternalEntityContents__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteDTDInternalEntity
extern __typeof (xmlTextWriterWriteDTDInternalEntity) xmlTextWriterWriteDTDInternalEntity __attribute((alias("xmlTextWriterWriteDTDInternalEntity__internal_alias")));
#else
#ifndef xmlTextWriterWriteDTDInternalEntity
extern __typeof (xmlTextWriterWriteDTDInternalEntity) xmlTextWriterWriteDTDInternalEntity__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteDTDInternalEntity xmlTextWriterWriteDTDInternalEntity__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteDTDNotation
extern __typeof (xmlTextWriterWriteDTDNotation) xmlTextWriterWriteDTDNotation __attribute((alias("xmlTextWriterWriteDTDNotation__internal_alias")));
#else
#ifndef xmlTextWriterWriteDTDNotation
extern __typeof (xmlTextWriterWriteDTDNotation) xmlTextWriterWriteDTDNotation__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteDTDNotation xmlTextWriterWriteDTDNotation__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteElement
extern __typeof (xmlTextWriterWriteElement) xmlTextWriterWriteElement __attribute((alias("xmlTextWriterWriteElement__internal_alias")));
#else
#ifndef xmlTextWriterWriteElement
extern __typeof (xmlTextWriterWriteElement) xmlTextWriterWriteElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteElement xmlTextWriterWriteElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteElementNS
extern __typeof (xmlTextWriterWriteElementNS) xmlTextWriterWriteElementNS __attribute((alias("xmlTextWriterWriteElementNS__internal_alias")));
#else
#ifndef xmlTextWriterWriteElementNS
extern __typeof (xmlTextWriterWriteElementNS) xmlTextWriterWriteElementNS__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteElementNS xmlTextWriterWriteElementNS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatAttribute
extern __typeof (xmlTextWriterWriteFormatAttribute) xmlTextWriterWriteFormatAttribute __attribute((alias("xmlTextWriterWriteFormatAttribute__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatAttribute
extern __typeof (xmlTextWriterWriteFormatAttribute) xmlTextWriterWriteFormatAttribute__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatAttribute xmlTextWriterWriteFormatAttribute__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatAttributeNS
extern __typeof (xmlTextWriterWriteFormatAttributeNS) xmlTextWriterWriteFormatAttributeNS __attribute((alias("xmlTextWriterWriteFormatAttributeNS__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatAttributeNS
extern __typeof (xmlTextWriterWriteFormatAttributeNS) xmlTextWriterWriteFormatAttributeNS__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatAttributeNS xmlTextWriterWriteFormatAttributeNS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatCDATA
extern __typeof (xmlTextWriterWriteFormatCDATA) xmlTextWriterWriteFormatCDATA __attribute((alias("xmlTextWriterWriteFormatCDATA__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatCDATA
extern __typeof (xmlTextWriterWriteFormatCDATA) xmlTextWriterWriteFormatCDATA__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatCDATA xmlTextWriterWriteFormatCDATA__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatComment
extern __typeof (xmlTextWriterWriteFormatComment) xmlTextWriterWriteFormatComment __attribute((alias("xmlTextWriterWriteFormatComment__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatComment
extern __typeof (xmlTextWriterWriteFormatComment) xmlTextWriterWriteFormatComment__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatComment xmlTextWriterWriteFormatComment__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatDTD
extern __typeof (xmlTextWriterWriteFormatDTD) xmlTextWriterWriteFormatDTD __attribute((alias("xmlTextWriterWriteFormatDTD__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatDTD
extern __typeof (xmlTextWriterWriteFormatDTD) xmlTextWriterWriteFormatDTD__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatDTD xmlTextWriterWriteFormatDTD__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatDTDAttlist
extern __typeof (xmlTextWriterWriteFormatDTDAttlist) xmlTextWriterWriteFormatDTDAttlist __attribute((alias("xmlTextWriterWriteFormatDTDAttlist__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatDTDAttlist
extern __typeof (xmlTextWriterWriteFormatDTDAttlist) xmlTextWriterWriteFormatDTDAttlist__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatDTDAttlist xmlTextWriterWriteFormatDTDAttlist__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatDTDElement
extern __typeof (xmlTextWriterWriteFormatDTDElement) xmlTextWriterWriteFormatDTDElement __attribute((alias("xmlTextWriterWriteFormatDTDElement__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatDTDElement
extern __typeof (xmlTextWriterWriteFormatDTDElement) xmlTextWriterWriteFormatDTDElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatDTDElement xmlTextWriterWriteFormatDTDElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatDTDInternalEntity
extern __typeof (xmlTextWriterWriteFormatDTDInternalEntity) xmlTextWriterWriteFormatDTDInternalEntity __attribute((alias("xmlTextWriterWriteFormatDTDInternalEntity__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatDTDInternalEntity
extern __typeof (xmlTextWriterWriteFormatDTDInternalEntity) xmlTextWriterWriteFormatDTDInternalEntity__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatDTDInternalEntity xmlTextWriterWriteFormatDTDInternalEntity__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatElement
extern __typeof (xmlTextWriterWriteFormatElement) xmlTextWriterWriteFormatElement __attribute((alias("xmlTextWriterWriteFormatElement__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatElement
extern __typeof (xmlTextWriterWriteFormatElement) xmlTextWriterWriteFormatElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatElement xmlTextWriterWriteFormatElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatElementNS
extern __typeof (xmlTextWriterWriteFormatElementNS) xmlTextWriterWriteFormatElementNS __attribute((alias("xmlTextWriterWriteFormatElementNS__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatElementNS
extern __typeof (xmlTextWriterWriteFormatElementNS) xmlTextWriterWriteFormatElementNS__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatElementNS xmlTextWriterWriteFormatElementNS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatPI
extern __typeof (xmlTextWriterWriteFormatPI) xmlTextWriterWriteFormatPI __attribute((alias("xmlTextWriterWriteFormatPI__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatPI
extern __typeof (xmlTextWriterWriteFormatPI) xmlTextWriterWriteFormatPI__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatPI xmlTextWriterWriteFormatPI__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatRaw
extern __typeof (xmlTextWriterWriteFormatRaw) xmlTextWriterWriteFormatRaw __attribute((alias("xmlTextWriterWriteFormatRaw__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatRaw
extern __typeof (xmlTextWriterWriteFormatRaw) xmlTextWriterWriteFormatRaw__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatRaw xmlTextWriterWriteFormatRaw__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteFormatString
extern __typeof (xmlTextWriterWriteFormatString) xmlTextWriterWriteFormatString __attribute((alias("xmlTextWriterWriteFormatString__internal_alias")));
#else
#ifndef xmlTextWriterWriteFormatString
extern __typeof (xmlTextWriterWriteFormatString) xmlTextWriterWriteFormatString__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteFormatString xmlTextWriterWriteFormatString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWritePI
extern __typeof (xmlTextWriterWritePI) xmlTextWriterWritePI __attribute((alias("xmlTextWriterWritePI__internal_alias")));
#else
#ifndef xmlTextWriterWritePI
extern __typeof (xmlTextWriterWritePI) xmlTextWriterWritePI__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWritePI xmlTextWriterWritePI__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteRaw
extern __typeof (xmlTextWriterWriteRaw) xmlTextWriterWriteRaw __attribute((alias("xmlTextWriterWriteRaw__internal_alias")));
#else
#ifndef xmlTextWriterWriteRaw
extern __typeof (xmlTextWriterWriteRaw) xmlTextWriterWriteRaw__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteRaw xmlTextWriterWriteRaw__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteRawLen
extern __typeof (xmlTextWriterWriteRawLen) xmlTextWriterWriteRawLen __attribute((alias("xmlTextWriterWriteRawLen__internal_alias")));
#else
#ifndef xmlTextWriterWriteRawLen
extern __typeof (xmlTextWriterWriteRawLen) xmlTextWriterWriteRawLen__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteRawLen xmlTextWriterWriteRawLen__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteString
extern __typeof (xmlTextWriterWriteString) xmlTextWriterWriteString __attribute((alias("xmlTextWriterWriteString__internal_alias")));
#else
#ifndef xmlTextWriterWriteString
extern __typeof (xmlTextWriterWriteString) xmlTextWriterWriteString__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteString xmlTextWriterWriteString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatAttribute
extern __typeof (xmlTextWriterWriteVFormatAttribute) xmlTextWriterWriteVFormatAttribute __attribute((alias("xmlTextWriterWriteVFormatAttribute__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatAttribute
extern __typeof (xmlTextWriterWriteVFormatAttribute) xmlTextWriterWriteVFormatAttribute__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatAttribute xmlTextWriterWriteVFormatAttribute__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatAttributeNS
extern __typeof (xmlTextWriterWriteVFormatAttributeNS) xmlTextWriterWriteVFormatAttributeNS __attribute((alias("xmlTextWriterWriteVFormatAttributeNS__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatAttributeNS
extern __typeof (xmlTextWriterWriteVFormatAttributeNS) xmlTextWriterWriteVFormatAttributeNS__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatAttributeNS xmlTextWriterWriteVFormatAttributeNS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatCDATA
extern __typeof (xmlTextWriterWriteVFormatCDATA) xmlTextWriterWriteVFormatCDATA __attribute((alias("xmlTextWriterWriteVFormatCDATA__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatCDATA
extern __typeof (xmlTextWriterWriteVFormatCDATA) xmlTextWriterWriteVFormatCDATA__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatCDATA xmlTextWriterWriteVFormatCDATA__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatComment
extern __typeof (xmlTextWriterWriteVFormatComment) xmlTextWriterWriteVFormatComment __attribute((alias("xmlTextWriterWriteVFormatComment__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatComment
extern __typeof (xmlTextWriterWriteVFormatComment) xmlTextWriterWriteVFormatComment__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatComment xmlTextWriterWriteVFormatComment__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatDTD
extern __typeof (xmlTextWriterWriteVFormatDTD) xmlTextWriterWriteVFormatDTD __attribute((alias("xmlTextWriterWriteVFormatDTD__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatDTD
extern __typeof (xmlTextWriterWriteVFormatDTD) xmlTextWriterWriteVFormatDTD__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatDTD xmlTextWriterWriteVFormatDTD__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatDTDAttlist
extern __typeof (xmlTextWriterWriteVFormatDTDAttlist) xmlTextWriterWriteVFormatDTDAttlist __attribute((alias("xmlTextWriterWriteVFormatDTDAttlist__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatDTDAttlist
extern __typeof (xmlTextWriterWriteVFormatDTDAttlist) xmlTextWriterWriteVFormatDTDAttlist__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatDTDAttlist xmlTextWriterWriteVFormatDTDAttlist__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatDTDElement
extern __typeof (xmlTextWriterWriteVFormatDTDElement) xmlTextWriterWriteVFormatDTDElement __attribute((alias("xmlTextWriterWriteVFormatDTDElement__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatDTDElement
extern __typeof (xmlTextWriterWriteVFormatDTDElement) xmlTextWriterWriteVFormatDTDElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatDTDElement xmlTextWriterWriteVFormatDTDElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatDTDInternalEntity
extern __typeof (xmlTextWriterWriteVFormatDTDInternalEntity) xmlTextWriterWriteVFormatDTDInternalEntity __attribute((alias("xmlTextWriterWriteVFormatDTDInternalEntity__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatDTDInternalEntity
extern __typeof (xmlTextWriterWriteVFormatDTDInternalEntity) xmlTextWriterWriteVFormatDTDInternalEntity__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatDTDInternalEntity xmlTextWriterWriteVFormatDTDInternalEntity__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatElement
extern __typeof (xmlTextWriterWriteVFormatElement) xmlTextWriterWriteVFormatElement __attribute((alias("xmlTextWriterWriteVFormatElement__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatElement
extern __typeof (xmlTextWriterWriteVFormatElement) xmlTextWriterWriteVFormatElement__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatElement xmlTextWriterWriteVFormatElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatElementNS
extern __typeof (xmlTextWriterWriteVFormatElementNS) xmlTextWriterWriteVFormatElementNS __attribute((alias("xmlTextWriterWriteVFormatElementNS__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatElementNS
extern __typeof (xmlTextWriterWriteVFormatElementNS) xmlTextWriterWriteVFormatElementNS__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatElementNS xmlTextWriterWriteVFormatElementNS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatPI
extern __typeof (xmlTextWriterWriteVFormatPI) xmlTextWriterWriteVFormatPI __attribute((alias("xmlTextWriterWriteVFormatPI__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatPI
extern __typeof (xmlTextWriterWriteVFormatPI) xmlTextWriterWriteVFormatPI__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatPI xmlTextWriterWriteVFormatPI__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatRaw
extern __typeof (xmlTextWriterWriteVFormatRaw) xmlTextWriterWriteVFormatRaw __attribute((alias("xmlTextWriterWriteVFormatRaw__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatRaw
extern __typeof (xmlTextWriterWriteVFormatRaw) xmlTextWriterWriteVFormatRaw__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatRaw xmlTextWriterWriteVFormatRaw__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_WRITER_ENABLED)
#ifdef bottom_xmlwriter
#undef xmlTextWriterWriteVFormatString
extern __typeof (xmlTextWriterWriteVFormatString) xmlTextWriterWriteVFormatString __attribute((alias("xmlTextWriterWriteVFormatString__internal_alias")));
#else
#ifndef xmlTextWriterWriteVFormatString
extern __typeof (xmlTextWriterWriteVFormatString) xmlTextWriterWriteVFormatString__internal_alias __attribute((visibility("hidden")));
#define xmlTextWriterWriteVFormatString xmlTextWriterWriteVFormatString__internal_alias
#endif
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefBufferAllocScheme
extern __typeof (xmlThrDefBufferAllocScheme) xmlThrDefBufferAllocScheme __attribute((alias("xmlThrDefBufferAllocScheme__internal_alias")));
#else
#ifndef xmlThrDefBufferAllocScheme
extern __typeof (xmlThrDefBufferAllocScheme) xmlThrDefBufferAllocScheme__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefBufferAllocScheme xmlThrDefBufferAllocScheme__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefDefaultBufferSize
extern __typeof (xmlThrDefDefaultBufferSize) xmlThrDefDefaultBufferSize __attribute((alias("xmlThrDefDefaultBufferSize__internal_alias")));
#else
#ifndef xmlThrDefDefaultBufferSize
extern __typeof (xmlThrDefDefaultBufferSize) xmlThrDefDefaultBufferSize__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefDefaultBufferSize xmlThrDefDefaultBufferSize__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefDeregisterNodeDefault
extern __typeof (xmlThrDefDeregisterNodeDefault) xmlThrDefDeregisterNodeDefault __attribute((alias("xmlThrDefDeregisterNodeDefault__internal_alias")));
#else
#ifndef xmlThrDefDeregisterNodeDefault
extern __typeof (xmlThrDefDeregisterNodeDefault) xmlThrDefDeregisterNodeDefault__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefDeregisterNodeDefault xmlThrDefDeregisterNodeDefault__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefDoValidityCheckingDefaultValue
extern __typeof (xmlThrDefDoValidityCheckingDefaultValue) xmlThrDefDoValidityCheckingDefaultValue __attribute((alias("xmlThrDefDoValidityCheckingDefaultValue__internal_alias")));
#else
#ifndef xmlThrDefDoValidityCheckingDefaultValue
extern __typeof (xmlThrDefDoValidityCheckingDefaultValue) xmlThrDefDoValidityCheckingDefaultValue__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefDoValidityCheckingDefaultValue xmlThrDefDoValidityCheckingDefaultValue__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefGetWarningsDefaultValue
extern __typeof (xmlThrDefGetWarningsDefaultValue) xmlThrDefGetWarningsDefaultValue __attribute((alias("xmlThrDefGetWarningsDefaultValue__internal_alias")));
#else
#ifndef xmlThrDefGetWarningsDefaultValue
extern __typeof (xmlThrDefGetWarningsDefaultValue) xmlThrDefGetWarningsDefaultValue__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefGetWarningsDefaultValue xmlThrDefGetWarningsDefaultValue__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefIndentTreeOutput
extern __typeof (xmlThrDefIndentTreeOutput) xmlThrDefIndentTreeOutput __attribute((alias("xmlThrDefIndentTreeOutput__internal_alias")));
#else
#ifndef xmlThrDefIndentTreeOutput
extern __typeof (xmlThrDefIndentTreeOutput) xmlThrDefIndentTreeOutput__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefIndentTreeOutput xmlThrDefIndentTreeOutput__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefKeepBlanksDefaultValue
extern __typeof (xmlThrDefKeepBlanksDefaultValue) xmlThrDefKeepBlanksDefaultValue __attribute((alias("xmlThrDefKeepBlanksDefaultValue__internal_alias")));
#else
#ifndef xmlThrDefKeepBlanksDefaultValue
extern __typeof (xmlThrDefKeepBlanksDefaultValue) xmlThrDefKeepBlanksDefaultValue__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefKeepBlanksDefaultValue xmlThrDefKeepBlanksDefaultValue__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefLineNumbersDefaultValue
extern __typeof (xmlThrDefLineNumbersDefaultValue) xmlThrDefLineNumbersDefaultValue __attribute((alias("xmlThrDefLineNumbersDefaultValue__internal_alias")));
#else
#ifndef xmlThrDefLineNumbersDefaultValue
extern __typeof (xmlThrDefLineNumbersDefaultValue) xmlThrDefLineNumbersDefaultValue__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefLineNumbersDefaultValue xmlThrDefLineNumbersDefaultValue__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefLoadExtDtdDefaultValue
extern __typeof (xmlThrDefLoadExtDtdDefaultValue) xmlThrDefLoadExtDtdDefaultValue __attribute((alias("xmlThrDefLoadExtDtdDefaultValue__internal_alias")));
#else
#ifndef xmlThrDefLoadExtDtdDefaultValue
extern __typeof (xmlThrDefLoadExtDtdDefaultValue) xmlThrDefLoadExtDtdDefaultValue__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefLoadExtDtdDefaultValue xmlThrDefLoadExtDtdDefaultValue__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefOutputBufferCreateFilenameDefault
extern __typeof (xmlThrDefOutputBufferCreateFilenameDefault) xmlThrDefOutputBufferCreateFilenameDefault __attribute((alias("xmlThrDefOutputBufferCreateFilenameDefault__internal_alias")));
#else
#ifndef xmlThrDefOutputBufferCreateFilenameDefault
extern __typeof (xmlThrDefOutputBufferCreateFilenameDefault) xmlThrDefOutputBufferCreateFilenameDefault__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefOutputBufferCreateFilenameDefault xmlThrDefOutputBufferCreateFilenameDefault__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefParserDebugEntities
extern __typeof (xmlThrDefParserDebugEntities) xmlThrDefParserDebugEntities __attribute((alias("xmlThrDefParserDebugEntities__internal_alias")));
#else
#ifndef xmlThrDefParserDebugEntities
extern __typeof (xmlThrDefParserDebugEntities) xmlThrDefParserDebugEntities__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefParserDebugEntities xmlThrDefParserDebugEntities__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefParserInputBufferCreateFilenameDefault
extern __typeof (xmlThrDefParserInputBufferCreateFilenameDefault) xmlThrDefParserInputBufferCreateFilenameDefault __attribute((alias("xmlThrDefParserInputBufferCreateFilenameDefault__internal_alias")));
#else
#ifndef xmlThrDefParserInputBufferCreateFilenameDefault
extern __typeof (xmlThrDefParserInputBufferCreateFilenameDefault) xmlThrDefParserInputBufferCreateFilenameDefault__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefParserInputBufferCreateFilenameDefault xmlThrDefParserInputBufferCreateFilenameDefault__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefPedanticParserDefaultValue
extern __typeof (xmlThrDefPedanticParserDefaultValue) xmlThrDefPedanticParserDefaultValue __attribute((alias("xmlThrDefPedanticParserDefaultValue__internal_alias")));
#else
#ifndef xmlThrDefPedanticParserDefaultValue
extern __typeof (xmlThrDefPedanticParserDefaultValue) xmlThrDefPedanticParserDefaultValue__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefPedanticParserDefaultValue xmlThrDefPedanticParserDefaultValue__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefRegisterNodeDefault
extern __typeof (xmlThrDefRegisterNodeDefault) xmlThrDefRegisterNodeDefault __attribute((alias("xmlThrDefRegisterNodeDefault__internal_alias")));
#else
#ifndef xmlThrDefRegisterNodeDefault
extern __typeof (xmlThrDefRegisterNodeDefault) xmlThrDefRegisterNodeDefault__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefRegisterNodeDefault xmlThrDefRegisterNodeDefault__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefSaveNoEmptyTags
extern __typeof (xmlThrDefSaveNoEmptyTags) xmlThrDefSaveNoEmptyTags __attribute((alias("xmlThrDefSaveNoEmptyTags__internal_alias")));
#else
#ifndef xmlThrDefSaveNoEmptyTags
extern __typeof (xmlThrDefSaveNoEmptyTags) xmlThrDefSaveNoEmptyTags__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefSaveNoEmptyTags xmlThrDefSaveNoEmptyTags__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefSetGenericErrorFunc
extern __typeof (xmlThrDefSetGenericErrorFunc) xmlThrDefSetGenericErrorFunc __attribute((alias("xmlThrDefSetGenericErrorFunc__internal_alias")));
#else
#ifndef xmlThrDefSetGenericErrorFunc
extern __typeof (xmlThrDefSetGenericErrorFunc) xmlThrDefSetGenericErrorFunc__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefSetGenericErrorFunc xmlThrDefSetGenericErrorFunc__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefSetStructuredErrorFunc
extern __typeof (xmlThrDefSetStructuredErrorFunc) xmlThrDefSetStructuredErrorFunc __attribute((alias("xmlThrDefSetStructuredErrorFunc__internal_alias")));
#else
#ifndef xmlThrDefSetStructuredErrorFunc
extern __typeof (xmlThrDefSetStructuredErrorFunc) xmlThrDefSetStructuredErrorFunc__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefSetStructuredErrorFunc xmlThrDefSetStructuredErrorFunc__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefSubstituteEntitiesDefaultValue
extern __typeof (xmlThrDefSubstituteEntitiesDefaultValue) xmlThrDefSubstituteEntitiesDefaultValue __attribute((alias("xmlThrDefSubstituteEntitiesDefaultValue__internal_alias")));
#else
#ifndef xmlThrDefSubstituteEntitiesDefaultValue
extern __typeof (xmlThrDefSubstituteEntitiesDefaultValue) xmlThrDefSubstituteEntitiesDefaultValue__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefSubstituteEntitiesDefaultValue xmlThrDefSubstituteEntitiesDefaultValue__internal_alias
#endif
#endif

#ifdef bottom_globals
#undef xmlThrDefTreeIndentString
extern __typeof (xmlThrDefTreeIndentString) xmlThrDefTreeIndentString __attribute((alias("xmlThrDefTreeIndentString__internal_alias")));
#else
#ifndef xmlThrDefTreeIndentString
extern __typeof (xmlThrDefTreeIndentString) xmlThrDefTreeIndentString__internal_alias __attribute((visibility("hidden")));
#define xmlThrDefTreeIndentString xmlThrDefTreeIndentString__internal_alias
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsAegeanNumbers
extern __typeof (xmlUCSIsAegeanNumbers) xmlUCSIsAegeanNumbers __attribute((alias("xmlUCSIsAegeanNumbers__internal_alias")));
#else
#ifndef xmlUCSIsAegeanNumbers
extern __typeof (xmlUCSIsAegeanNumbers) xmlUCSIsAegeanNumbers__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsAegeanNumbers xmlUCSIsAegeanNumbers__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsAlphabeticPresentationForms
extern __typeof (xmlUCSIsAlphabeticPresentationForms) xmlUCSIsAlphabeticPresentationForms __attribute((alias("xmlUCSIsAlphabeticPresentationForms__internal_alias")));
#else
#ifndef xmlUCSIsAlphabeticPresentationForms
extern __typeof (xmlUCSIsAlphabeticPresentationForms) xmlUCSIsAlphabeticPresentationForms__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsAlphabeticPresentationForms xmlUCSIsAlphabeticPresentationForms__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsArabic
extern __typeof (xmlUCSIsArabic) xmlUCSIsArabic __attribute((alias("xmlUCSIsArabic__internal_alias")));
#else
#ifndef xmlUCSIsArabic
extern __typeof (xmlUCSIsArabic) xmlUCSIsArabic__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsArabic xmlUCSIsArabic__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsArabicPresentationFormsA
extern __typeof (xmlUCSIsArabicPresentationFormsA) xmlUCSIsArabicPresentationFormsA __attribute((alias("xmlUCSIsArabicPresentationFormsA__internal_alias")));
#else
#ifndef xmlUCSIsArabicPresentationFormsA
extern __typeof (xmlUCSIsArabicPresentationFormsA) xmlUCSIsArabicPresentationFormsA__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsArabicPresentationFormsA xmlUCSIsArabicPresentationFormsA__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsArabicPresentationFormsB
extern __typeof (xmlUCSIsArabicPresentationFormsB) xmlUCSIsArabicPresentationFormsB __attribute((alias("xmlUCSIsArabicPresentationFormsB__internal_alias")));
#else
#ifndef xmlUCSIsArabicPresentationFormsB
extern __typeof (xmlUCSIsArabicPresentationFormsB) xmlUCSIsArabicPresentationFormsB__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsArabicPresentationFormsB xmlUCSIsArabicPresentationFormsB__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsArmenian
extern __typeof (xmlUCSIsArmenian) xmlUCSIsArmenian __attribute((alias("xmlUCSIsArmenian__internal_alias")));
#else
#ifndef xmlUCSIsArmenian
extern __typeof (xmlUCSIsArmenian) xmlUCSIsArmenian__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsArmenian xmlUCSIsArmenian__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsArrows
extern __typeof (xmlUCSIsArrows) xmlUCSIsArrows __attribute((alias("xmlUCSIsArrows__internal_alias")));
#else
#ifndef xmlUCSIsArrows
extern __typeof (xmlUCSIsArrows) xmlUCSIsArrows__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsArrows xmlUCSIsArrows__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsBasicLatin
extern __typeof (xmlUCSIsBasicLatin) xmlUCSIsBasicLatin __attribute((alias("xmlUCSIsBasicLatin__internal_alias")));
#else
#ifndef xmlUCSIsBasicLatin
extern __typeof (xmlUCSIsBasicLatin) xmlUCSIsBasicLatin__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsBasicLatin xmlUCSIsBasicLatin__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsBengali
extern __typeof (xmlUCSIsBengali) xmlUCSIsBengali __attribute((alias("xmlUCSIsBengali__internal_alias")));
#else
#ifndef xmlUCSIsBengali
extern __typeof (xmlUCSIsBengali) xmlUCSIsBengali__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsBengali xmlUCSIsBengali__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsBlock
extern __typeof (xmlUCSIsBlock) xmlUCSIsBlock __attribute((alias("xmlUCSIsBlock__internal_alias")));
#else
#ifndef xmlUCSIsBlock
extern __typeof (xmlUCSIsBlock) xmlUCSIsBlock__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsBlock xmlUCSIsBlock__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsBlockElements
extern __typeof (xmlUCSIsBlockElements) xmlUCSIsBlockElements __attribute((alias("xmlUCSIsBlockElements__internal_alias")));
#else
#ifndef xmlUCSIsBlockElements
extern __typeof (xmlUCSIsBlockElements) xmlUCSIsBlockElements__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsBlockElements xmlUCSIsBlockElements__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsBopomofo
extern __typeof (xmlUCSIsBopomofo) xmlUCSIsBopomofo __attribute((alias("xmlUCSIsBopomofo__internal_alias")));
#else
#ifndef xmlUCSIsBopomofo
extern __typeof (xmlUCSIsBopomofo) xmlUCSIsBopomofo__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsBopomofo xmlUCSIsBopomofo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsBopomofoExtended
extern __typeof (xmlUCSIsBopomofoExtended) xmlUCSIsBopomofoExtended __attribute((alias("xmlUCSIsBopomofoExtended__internal_alias")));
#else
#ifndef xmlUCSIsBopomofoExtended
extern __typeof (xmlUCSIsBopomofoExtended) xmlUCSIsBopomofoExtended__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsBopomofoExtended xmlUCSIsBopomofoExtended__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsBoxDrawing
extern __typeof (xmlUCSIsBoxDrawing) xmlUCSIsBoxDrawing __attribute((alias("xmlUCSIsBoxDrawing__internal_alias")));
#else
#ifndef xmlUCSIsBoxDrawing
extern __typeof (xmlUCSIsBoxDrawing) xmlUCSIsBoxDrawing__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsBoxDrawing xmlUCSIsBoxDrawing__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsBraillePatterns
extern __typeof (xmlUCSIsBraillePatterns) xmlUCSIsBraillePatterns __attribute((alias("xmlUCSIsBraillePatterns__internal_alias")));
#else
#ifndef xmlUCSIsBraillePatterns
extern __typeof (xmlUCSIsBraillePatterns) xmlUCSIsBraillePatterns__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsBraillePatterns xmlUCSIsBraillePatterns__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsBuhid
extern __typeof (xmlUCSIsBuhid) xmlUCSIsBuhid __attribute((alias("xmlUCSIsBuhid__internal_alias")));
#else
#ifndef xmlUCSIsBuhid
extern __typeof (xmlUCSIsBuhid) xmlUCSIsBuhid__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsBuhid xmlUCSIsBuhid__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsByzantineMusicalSymbols
extern __typeof (xmlUCSIsByzantineMusicalSymbols) xmlUCSIsByzantineMusicalSymbols __attribute((alias("xmlUCSIsByzantineMusicalSymbols__internal_alias")));
#else
#ifndef xmlUCSIsByzantineMusicalSymbols
extern __typeof (xmlUCSIsByzantineMusicalSymbols) xmlUCSIsByzantineMusicalSymbols__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsByzantineMusicalSymbols xmlUCSIsByzantineMusicalSymbols__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCJKCompatibility
extern __typeof (xmlUCSIsCJKCompatibility) xmlUCSIsCJKCompatibility __attribute((alias("xmlUCSIsCJKCompatibility__internal_alias")));
#else
#ifndef xmlUCSIsCJKCompatibility
extern __typeof (xmlUCSIsCJKCompatibility) xmlUCSIsCJKCompatibility__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCJKCompatibility xmlUCSIsCJKCompatibility__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCJKCompatibilityForms
extern __typeof (xmlUCSIsCJKCompatibilityForms) xmlUCSIsCJKCompatibilityForms __attribute((alias("xmlUCSIsCJKCompatibilityForms__internal_alias")));
#else
#ifndef xmlUCSIsCJKCompatibilityForms
extern __typeof (xmlUCSIsCJKCompatibilityForms) xmlUCSIsCJKCompatibilityForms__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCJKCompatibilityForms xmlUCSIsCJKCompatibilityForms__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCJKCompatibilityIdeographs
extern __typeof (xmlUCSIsCJKCompatibilityIdeographs) xmlUCSIsCJKCompatibilityIdeographs __attribute((alias("xmlUCSIsCJKCompatibilityIdeographs__internal_alias")));
#else
#ifndef xmlUCSIsCJKCompatibilityIdeographs
extern __typeof (xmlUCSIsCJKCompatibilityIdeographs) xmlUCSIsCJKCompatibilityIdeographs__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCJKCompatibilityIdeographs xmlUCSIsCJKCompatibilityIdeographs__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCJKCompatibilityIdeographsSupplement
extern __typeof (xmlUCSIsCJKCompatibilityIdeographsSupplement) xmlUCSIsCJKCompatibilityIdeographsSupplement __attribute((alias("xmlUCSIsCJKCompatibilityIdeographsSupplement__internal_alias")));
#else
#ifndef xmlUCSIsCJKCompatibilityIdeographsSupplement
extern __typeof (xmlUCSIsCJKCompatibilityIdeographsSupplement) xmlUCSIsCJKCompatibilityIdeographsSupplement__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCJKCompatibilityIdeographsSupplement xmlUCSIsCJKCompatibilityIdeographsSupplement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCJKRadicalsSupplement
extern __typeof (xmlUCSIsCJKRadicalsSupplement) xmlUCSIsCJKRadicalsSupplement __attribute((alias("xmlUCSIsCJKRadicalsSupplement__internal_alias")));
#else
#ifndef xmlUCSIsCJKRadicalsSupplement
extern __typeof (xmlUCSIsCJKRadicalsSupplement) xmlUCSIsCJKRadicalsSupplement__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCJKRadicalsSupplement xmlUCSIsCJKRadicalsSupplement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCJKSymbolsandPunctuation
extern __typeof (xmlUCSIsCJKSymbolsandPunctuation) xmlUCSIsCJKSymbolsandPunctuation __attribute((alias("xmlUCSIsCJKSymbolsandPunctuation__internal_alias")));
#else
#ifndef xmlUCSIsCJKSymbolsandPunctuation
extern __typeof (xmlUCSIsCJKSymbolsandPunctuation) xmlUCSIsCJKSymbolsandPunctuation__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCJKSymbolsandPunctuation xmlUCSIsCJKSymbolsandPunctuation__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCJKUnifiedIdeographs
extern __typeof (xmlUCSIsCJKUnifiedIdeographs) xmlUCSIsCJKUnifiedIdeographs __attribute((alias("xmlUCSIsCJKUnifiedIdeographs__internal_alias")));
#else
#ifndef xmlUCSIsCJKUnifiedIdeographs
extern __typeof (xmlUCSIsCJKUnifiedIdeographs) xmlUCSIsCJKUnifiedIdeographs__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCJKUnifiedIdeographs xmlUCSIsCJKUnifiedIdeographs__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCJKUnifiedIdeographsExtensionA
extern __typeof (xmlUCSIsCJKUnifiedIdeographsExtensionA) xmlUCSIsCJKUnifiedIdeographsExtensionA __attribute((alias("xmlUCSIsCJKUnifiedIdeographsExtensionA__internal_alias")));
#else
#ifndef xmlUCSIsCJKUnifiedIdeographsExtensionA
extern __typeof (xmlUCSIsCJKUnifiedIdeographsExtensionA) xmlUCSIsCJKUnifiedIdeographsExtensionA__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCJKUnifiedIdeographsExtensionA xmlUCSIsCJKUnifiedIdeographsExtensionA__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCJKUnifiedIdeographsExtensionB
extern __typeof (xmlUCSIsCJKUnifiedIdeographsExtensionB) xmlUCSIsCJKUnifiedIdeographsExtensionB __attribute((alias("xmlUCSIsCJKUnifiedIdeographsExtensionB__internal_alias")));
#else
#ifndef xmlUCSIsCJKUnifiedIdeographsExtensionB
extern __typeof (xmlUCSIsCJKUnifiedIdeographsExtensionB) xmlUCSIsCJKUnifiedIdeographsExtensionB__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCJKUnifiedIdeographsExtensionB xmlUCSIsCJKUnifiedIdeographsExtensionB__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCat
extern __typeof (xmlUCSIsCat) xmlUCSIsCat __attribute((alias("xmlUCSIsCat__internal_alias")));
#else
#ifndef xmlUCSIsCat
extern __typeof (xmlUCSIsCat) xmlUCSIsCat__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCat xmlUCSIsCat__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatC
extern __typeof (xmlUCSIsCatC) xmlUCSIsCatC __attribute((alias("xmlUCSIsCatC__internal_alias")));
#else
#ifndef xmlUCSIsCatC
extern __typeof (xmlUCSIsCatC) xmlUCSIsCatC__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatC xmlUCSIsCatC__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatCc
extern __typeof (xmlUCSIsCatCc) xmlUCSIsCatCc __attribute((alias("xmlUCSIsCatCc__internal_alias")));
#else
#ifndef xmlUCSIsCatCc
extern __typeof (xmlUCSIsCatCc) xmlUCSIsCatCc__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatCc xmlUCSIsCatCc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatCf
extern __typeof (xmlUCSIsCatCf) xmlUCSIsCatCf __attribute((alias("xmlUCSIsCatCf__internal_alias")));
#else
#ifndef xmlUCSIsCatCf
extern __typeof (xmlUCSIsCatCf) xmlUCSIsCatCf__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatCf xmlUCSIsCatCf__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatCo
extern __typeof (xmlUCSIsCatCo) xmlUCSIsCatCo __attribute((alias("xmlUCSIsCatCo__internal_alias")));
#else
#ifndef xmlUCSIsCatCo
extern __typeof (xmlUCSIsCatCo) xmlUCSIsCatCo__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatCo xmlUCSIsCatCo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatCs
extern __typeof (xmlUCSIsCatCs) xmlUCSIsCatCs __attribute((alias("xmlUCSIsCatCs__internal_alias")));
#else
#ifndef xmlUCSIsCatCs
extern __typeof (xmlUCSIsCatCs) xmlUCSIsCatCs__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatCs xmlUCSIsCatCs__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatL
extern __typeof (xmlUCSIsCatL) xmlUCSIsCatL __attribute((alias("xmlUCSIsCatL__internal_alias")));
#else
#ifndef xmlUCSIsCatL
extern __typeof (xmlUCSIsCatL) xmlUCSIsCatL__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatL xmlUCSIsCatL__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatLl
extern __typeof (xmlUCSIsCatLl) xmlUCSIsCatLl __attribute((alias("xmlUCSIsCatLl__internal_alias")));
#else
#ifndef xmlUCSIsCatLl
extern __typeof (xmlUCSIsCatLl) xmlUCSIsCatLl__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatLl xmlUCSIsCatLl__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatLm
extern __typeof (xmlUCSIsCatLm) xmlUCSIsCatLm __attribute((alias("xmlUCSIsCatLm__internal_alias")));
#else
#ifndef xmlUCSIsCatLm
extern __typeof (xmlUCSIsCatLm) xmlUCSIsCatLm__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatLm xmlUCSIsCatLm__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatLo
extern __typeof (xmlUCSIsCatLo) xmlUCSIsCatLo __attribute((alias("xmlUCSIsCatLo__internal_alias")));
#else
#ifndef xmlUCSIsCatLo
extern __typeof (xmlUCSIsCatLo) xmlUCSIsCatLo__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatLo xmlUCSIsCatLo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatLt
extern __typeof (xmlUCSIsCatLt) xmlUCSIsCatLt __attribute((alias("xmlUCSIsCatLt__internal_alias")));
#else
#ifndef xmlUCSIsCatLt
extern __typeof (xmlUCSIsCatLt) xmlUCSIsCatLt__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatLt xmlUCSIsCatLt__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatLu
extern __typeof (xmlUCSIsCatLu) xmlUCSIsCatLu __attribute((alias("xmlUCSIsCatLu__internal_alias")));
#else
#ifndef xmlUCSIsCatLu
extern __typeof (xmlUCSIsCatLu) xmlUCSIsCatLu__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatLu xmlUCSIsCatLu__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatM
extern __typeof (xmlUCSIsCatM) xmlUCSIsCatM __attribute((alias("xmlUCSIsCatM__internal_alias")));
#else
#ifndef xmlUCSIsCatM
extern __typeof (xmlUCSIsCatM) xmlUCSIsCatM__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatM xmlUCSIsCatM__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatMc
extern __typeof (xmlUCSIsCatMc) xmlUCSIsCatMc __attribute((alias("xmlUCSIsCatMc__internal_alias")));
#else
#ifndef xmlUCSIsCatMc
extern __typeof (xmlUCSIsCatMc) xmlUCSIsCatMc__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatMc xmlUCSIsCatMc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatMe
extern __typeof (xmlUCSIsCatMe) xmlUCSIsCatMe __attribute((alias("xmlUCSIsCatMe__internal_alias")));
#else
#ifndef xmlUCSIsCatMe
extern __typeof (xmlUCSIsCatMe) xmlUCSIsCatMe__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatMe xmlUCSIsCatMe__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatMn
extern __typeof (xmlUCSIsCatMn) xmlUCSIsCatMn __attribute((alias("xmlUCSIsCatMn__internal_alias")));
#else
#ifndef xmlUCSIsCatMn
extern __typeof (xmlUCSIsCatMn) xmlUCSIsCatMn__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatMn xmlUCSIsCatMn__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatN
extern __typeof (xmlUCSIsCatN) xmlUCSIsCatN __attribute((alias("xmlUCSIsCatN__internal_alias")));
#else
#ifndef xmlUCSIsCatN
extern __typeof (xmlUCSIsCatN) xmlUCSIsCatN__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatN xmlUCSIsCatN__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatNd
extern __typeof (xmlUCSIsCatNd) xmlUCSIsCatNd __attribute((alias("xmlUCSIsCatNd__internal_alias")));
#else
#ifndef xmlUCSIsCatNd
extern __typeof (xmlUCSIsCatNd) xmlUCSIsCatNd__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatNd xmlUCSIsCatNd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatNl
extern __typeof (xmlUCSIsCatNl) xmlUCSIsCatNl __attribute((alias("xmlUCSIsCatNl__internal_alias")));
#else
#ifndef xmlUCSIsCatNl
extern __typeof (xmlUCSIsCatNl) xmlUCSIsCatNl__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatNl xmlUCSIsCatNl__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatNo
extern __typeof (xmlUCSIsCatNo) xmlUCSIsCatNo __attribute((alias("xmlUCSIsCatNo__internal_alias")));
#else
#ifndef xmlUCSIsCatNo
extern __typeof (xmlUCSIsCatNo) xmlUCSIsCatNo__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatNo xmlUCSIsCatNo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatP
extern __typeof (xmlUCSIsCatP) xmlUCSIsCatP __attribute((alias("xmlUCSIsCatP__internal_alias")));
#else
#ifndef xmlUCSIsCatP
extern __typeof (xmlUCSIsCatP) xmlUCSIsCatP__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatP xmlUCSIsCatP__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatPc
extern __typeof (xmlUCSIsCatPc) xmlUCSIsCatPc __attribute((alias("xmlUCSIsCatPc__internal_alias")));
#else
#ifndef xmlUCSIsCatPc
extern __typeof (xmlUCSIsCatPc) xmlUCSIsCatPc__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatPc xmlUCSIsCatPc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatPd
extern __typeof (xmlUCSIsCatPd) xmlUCSIsCatPd __attribute((alias("xmlUCSIsCatPd__internal_alias")));
#else
#ifndef xmlUCSIsCatPd
extern __typeof (xmlUCSIsCatPd) xmlUCSIsCatPd__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatPd xmlUCSIsCatPd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatPe
extern __typeof (xmlUCSIsCatPe) xmlUCSIsCatPe __attribute((alias("xmlUCSIsCatPe__internal_alias")));
#else
#ifndef xmlUCSIsCatPe
extern __typeof (xmlUCSIsCatPe) xmlUCSIsCatPe__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatPe xmlUCSIsCatPe__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatPf
extern __typeof (xmlUCSIsCatPf) xmlUCSIsCatPf __attribute((alias("xmlUCSIsCatPf__internal_alias")));
#else
#ifndef xmlUCSIsCatPf
extern __typeof (xmlUCSIsCatPf) xmlUCSIsCatPf__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatPf xmlUCSIsCatPf__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatPi
extern __typeof (xmlUCSIsCatPi) xmlUCSIsCatPi __attribute((alias("xmlUCSIsCatPi__internal_alias")));
#else
#ifndef xmlUCSIsCatPi
extern __typeof (xmlUCSIsCatPi) xmlUCSIsCatPi__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatPi xmlUCSIsCatPi__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatPo
extern __typeof (xmlUCSIsCatPo) xmlUCSIsCatPo __attribute((alias("xmlUCSIsCatPo__internal_alias")));
#else
#ifndef xmlUCSIsCatPo
extern __typeof (xmlUCSIsCatPo) xmlUCSIsCatPo__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatPo xmlUCSIsCatPo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatPs
extern __typeof (xmlUCSIsCatPs) xmlUCSIsCatPs __attribute((alias("xmlUCSIsCatPs__internal_alias")));
#else
#ifndef xmlUCSIsCatPs
extern __typeof (xmlUCSIsCatPs) xmlUCSIsCatPs__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatPs xmlUCSIsCatPs__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatS
extern __typeof (xmlUCSIsCatS) xmlUCSIsCatS __attribute((alias("xmlUCSIsCatS__internal_alias")));
#else
#ifndef xmlUCSIsCatS
extern __typeof (xmlUCSIsCatS) xmlUCSIsCatS__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatS xmlUCSIsCatS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatSc
extern __typeof (xmlUCSIsCatSc) xmlUCSIsCatSc __attribute((alias("xmlUCSIsCatSc__internal_alias")));
#else
#ifndef xmlUCSIsCatSc
extern __typeof (xmlUCSIsCatSc) xmlUCSIsCatSc__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatSc xmlUCSIsCatSc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatSk
extern __typeof (xmlUCSIsCatSk) xmlUCSIsCatSk __attribute((alias("xmlUCSIsCatSk__internal_alias")));
#else
#ifndef xmlUCSIsCatSk
extern __typeof (xmlUCSIsCatSk) xmlUCSIsCatSk__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatSk xmlUCSIsCatSk__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatSm
extern __typeof (xmlUCSIsCatSm) xmlUCSIsCatSm __attribute((alias("xmlUCSIsCatSm__internal_alias")));
#else
#ifndef xmlUCSIsCatSm
extern __typeof (xmlUCSIsCatSm) xmlUCSIsCatSm__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatSm xmlUCSIsCatSm__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatSo
extern __typeof (xmlUCSIsCatSo) xmlUCSIsCatSo __attribute((alias("xmlUCSIsCatSo__internal_alias")));
#else
#ifndef xmlUCSIsCatSo
extern __typeof (xmlUCSIsCatSo) xmlUCSIsCatSo__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatSo xmlUCSIsCatSo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatZ
extern __typeof (xmlUCSIsCatZ) xmlUCSIsCatZ __attribute((alias("xmlUCSIsCatZ__internal_alias")));
#else
#ifndef xmlUCSIsCatZ
extern __typeof (xmlUCSIsCatZ) xmlUCSIsCatZ__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatZ xmlUCSIsCatZ__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatZl
extern __typeof (xmlUCSIsCatZl) xmlUCSIsCatZl __attribute((alias("xmlUCSIsCatZl__internal_alias")));
#else
#ifndef xmlUCSIsCatZl
extern __typeof (xmlUCSIsCatZl) xmlUCSIsCatZl__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatZl xmlUCSIsCatZl__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatZp
extern __typeof (xmlUCSIsCatZp) xmlUCSIsCatZp __attribute((alias("xmlUCSIsCatZp__internal_alias")));
#else
#ifndef xmlUCSIsCatZp
extern __typeof (xmlUCSIsCatZp) xmlUCSIsCatZp__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatZp xmlUCSIsCatZp__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCatZs
extern __typeof (xmlUCSIsCatZs) xmlUCSIsCatZs __attribute((alias("xmlUCSIsCatZs__internal_alias")));
#else
#ifndef xmlUCSIsCatZs
extern __typeof (xmlUCSIsCatZs) xmlUCSIsCatZs__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCatZs xmlUCSIsCatZs__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCherokee
extern __typeof (xmlUCSIsCherokee) xmlUCSIsCherokee __attribute((alias("xmlUCSIsCherokee__internal_alias")));
#else
#ifndef xmlUCSIsCherokee
extern __typeof (xmlUCSIsCherokee) xmlUCSIsCherokee__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCherokee xmlUCSIsCherokee__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCombiningDiacriticalMarks
extern __typeof (xmlUCSIsCombiningDiacriticalMarks) xmlUCSIsCombiningDiacriticalMarks __attribute((alias("xmlUCSIsCombiningDiacriticalMarks__internal_alias")));
#else
#ifndef xmlUCSIsCombiningDiacriticalMarks
extern __typeof (xmlUCSIsCombiningDiacriticalMarks) xmlUCSIsCombiningDiacriticalMarks__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCombiningDiacriticalMarks xmlUCSIsCombiningDiacriticalMarks__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCombiningDiacriticalMarksforSymbols
extern __typeof (xmlUCSIsCombiningDiacriticalMarksforSymbols) xmlUCSIsCombiningDiacriticalMarksforSymbols __attribute((alias("xmlUCSIsCombiningDiacriticalMarksforSymbols__internal_alias")));
#else
#ifndef xmlUCSIsCombiningDiacriticalMarksforSymbols
extern __typeof (xmlUCSIsCombiningDiacriticalMarksforSymbols) xmlUCSIsCombiningDiacriticalMarksforSymbols__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCombiningDiacriticalMarksforSymbols xmlUCSIsCombiningDiacriticalMarksforSymbols__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCombiningHalfMarks
extern __typeof (xmlUCSIsCombiningHalfMarks) xmlUCSIsCombiningHalfMarks __attribute((alias("xmlUCSIsCombiningHalfMarks__internal_alias")));
#else
#ifndef xmlUCSIsCombiningHalfMarks
extern __typeof (xmlUCSIsCombiningHalfMarks) xmlUCSIsCombiningHalfMarks__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCombiningHalfMarks xmlUCSIsCombiningHalfMarks__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCombiningMarksforSymbols
extern __typeof (xmlUCSIsCombiningMarksforSymbols) xmlUCSIsCombiningMarksforSymbols __attribute((alias("xmlUCSIsCombiningMarksforSymbols__internal_alias")));
#else
#ifndef xmlUCSIsCombiningMarksforSymbols
extern __typeof (xmlUCSIsCombiningMarksforSymbols) xmlUCSIsCombiningMarksforSymbols__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCombiningMarksforSymbols xmlUCSIsCombiningMarksforSymbols__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsControlPictures
extern __typeof (xmlUCSIsControlPictures) xmlUCSIsControlPictures __attribute((alias("xmlUCSIsControlPictures__internal_alias")));
#else
#ifndef xmlUCSIsControlPictures
extern __typeof (xmlUCSIsControlPictures) xmlUCSIsControlPictures__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsControlPictures xmlUCSIsControlPictures__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCurrencySymbols
extern __typeof (xmlUCSIsCurrencySymbols) xmlUCSIsCurrencySymbols __attribute((alias("xmlUCSIsCurrencySymbols__internal_alias")));
#else
#ifndef xmlUCSIsCurrencySymbols
extern __typeof (xmlUCSIsCurrencySymbols) xmlUCSIsCurrencySymbols__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCurrencySymbols xmlUCSIsCurrencySymbols__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCypriotSyllabary
extern __typeof (xmlUCSIsCypriotSyllabary) xmlUCSIsCypriotSyllabary __attribute((alias("xmlUCSIsCypriotSyllabary__internal_alias")));
#else
#ifndef xmlUCSIsCypriotSyllabary
extern __typeof (xmlUCSIsCypriotSyllabary) xmlUCSIsCypriotSyllabary__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCypriotSyllabary xmlUCSIsCypriotSyllabary__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCyrillic
extern __typeof (xmlUCSIsCyrillic) xmlUCSIsCyrillic __attribute((alias("xmlUCSIsCyrillic__internal_alias")));
#else
#ifndef xmlUCSIsCyrillic
extern __typeof (xmlUCSIsCyrillic) xmlUCSIsCyrillic__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCyrillic xmlUCSIsCyrillic__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsCyrillicSupplement
extern __typeof (xmlUCSIsCyrillicSupplement) xmlUCSIsCyrillicSupplement __attribute((alias("xmlUCSIsCyrillicSupplement__internal_alias")));
#else
#ifndef xmlUCSIsCyrillicSupplement
extern __typeof (xmlUCSIsCyrillicSupplement) xmlUCSIsCyrillicSupplement__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsCyrillicSupplement xmlUCSIsCyrillicSupplement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsDeseret
extern __typeof (xmlUCSIsDeseret) xmlUCSIsDeseret __attribute((alias("xmlUCSIsDeseret__internal_alias")));
#else
#ifndef xmlUCSIsDeseret
extern __typeof (xmlUCSIsDeseret) xmlUCSIsDeseret__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsDeseret xmlUCSIsDeseret__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsDevanagari
extern __typeof (xmlUCSIsDevanagari) xmlUCSIsDevanagari __attribute((alias("xmlUCSIsDevanagari__internal_alias")));
#else
#ifndef xmlUCSIsDevanagari
extern __typeof (xmlUCSIsDevanagari) xmlUCSIsDevanagari__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsDevanagari xmlUCSIsDevanagari__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsDingbats
extern __typeof (xmlUCSIsDingbats) xmlUCSIsDingbats __attribute((alias("xmlUCSIsDingbats__internal_alias")));
#else
#ifndef xmlUCSIsDingbats
extern __typeof (xmlUCSIsDingbats) xmlUCSIsDingbats__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsDingbats xmlUCSIsDingbats__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsEnclosedAlphanumerics
extern __typeof (xmlUCSIsEnclosedAlphanumerics) xmlUCSIsEnclosedAlphanumerics __attribute((alias("xmlUCSIsEnclosedAlphanumerics__internal_alias")));
#else
#ifndef xmlUCSIsEnclosedAlphanumerics
extern __typeof (xmlUCSIsEnclosedAlphanumerics) xmlUCSIsEnclosedAlphanumerics__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsEnclosedAlphanumerics xmlUCSIsEnclosedAlphanumerics__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsEnclosedCJKLettersandMonths
extern __typeof (xmlUCSIsEnclosedCJKLettersandMonths) xmlUCSIsEnclosedCJKLettersandMonths __attribute((alias("xmlUCSIsEnclosedCJKLettersandMonths__internal_alias")));
#else
#ifndef xmlUCSIsEnclosedCJKLettersandMonths
extern __typeof (xmlUCSIsEnclosedCJKLettersandMonths) xmlUCSIsEnclosedCJKLettersandMonths__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsEnclosedCJKLettersandMonths xmlUCSIsEnclosedCJKLettersandMonths__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsEthiopic
extern __typeof (xmlUCSIsEthiopic) xmlUCSIsEthiopic __attribute((alias("xmlUCSIsEthiopic__internal_alias")));
#else
#ifndef xmlUCSIsEthiopic
extern __typeof (xmlUCSIsEthiopic) xmlUCSIsEthiopic__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsEthiopic xmlUCSIsEthiopic__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsGeneralPunctuation
extern __typeof (xmlUCSIsGeneralPunctuation) xmlUCSIsGeneralPunctuation __attribute((alias("xmlUCSIsGeneralPunctuation__internal_alias")));
#else
#ifndef xmlUCSIsGeneralPunctuation
extern __typeof (xmlUCSIsGeneralPunctuation) xmlUCSIsGeneralPunctuation__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsGeneralPunctuation xmlUCSIsGeneralPunctuation__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsGeometricShapes
extern __typeof (xmlUCSIsGeometricShapes) xmlUCSIsGeometricShapes __attribute((alias("xmlUCSIsGeometricShapes__internal_alias")));
#else
#ifndef xmlUCSIsGeometricShapes
extern __typeof (xmlUCSIsGeometricShapes) xmlUCSIsGeometricShapes__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsGeometricShapes xmlUCSIsGeometricShapes__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsGeorgian
extern __typeof (xmlUCSIsGeorgian) xmlUCSIsGeorgian __attribute((alias("xmlUCSIsGeorgian__internal_alias")));
#else
#ifndef xmlUCSIsGeorgian
extern __typeof (xmlUCSIsGeorgian) xmlUCSIsGeorgian__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsGeorgian xmlUCSIsGeorgian__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsGothic
extern __typeof (xmlUCSIsGothic) xmlUCSIsGothic __attribute((alias("xmlUCSIsGothic__internal_alias")));
#else
#ifndef xmlUCSIsGothic
extern __typeof (xmlUCSIsGothic) xmlUCSIsGothic__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsGothic xmlUCSIsGothic__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsGreek
extern __typeof (xmlUCSIsGreek) xmlUCSIsGreek __attribute((alias("xmlUCSIsGreek__internal_alias")));
#else
#ifndef xmlUCSIsGreek
extern __typeof (xmlUCSIsGreek) xmlUCSIsGreek__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsGreek xmlUCSIsGreek__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsGreekExtended
extern __typeof (xmlUCSIsGreekExtended) xmlUCSIsGreekExtended __attribute((alias("xmlUCSIsGreekExtended__internal_alias")));
#else
#ifndef xmlUCSIsGreekExtended
extern __typeof (xmlUCSIsGreekExtended) xmlUCSIsGreekExtended__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsGreekExtended xmlUCSIsGreekExtended__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsGreekandCoptic
extern __typeof (xmlUCSIsGreekandCoptic) xmlUCSIsGreekandCoptic __attribute((alias("xmlUCSIsGreekandCoptic__internal_alias")));
#else
#ifndef xmlUCSIsGreekandCoptic
extern __typeof (xmlUCSIsGreekandCoptic) xmlUCSIsGreekandCoptic__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsGreekandCoptic xmlUCSIsGreekandCoptic__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsGujarati
extern __typeof (xmlUCSIsGujarati) xmlUCSIsGujarati __attribute((alias("xmlUCSIsGujarati__internal_alias")));
#else
#ifndef xmlUCSIsGujarati
extern __typeof (xmlUCSIsGujarati) xmlUCSIsGujarati__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsGujarati xmlUCSIsGujarati__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsGurmukhi
extern __typeof (xmlUCSIsGurmukhi) xmlUCSIsGurmukhi __attribute((alias("xmlUCSIsGurmukhi__internal_alias")));
#else
#ifndef xmlUCSIsGurmukhi
extern __typeof (xmlUCSIsGurmukhi) xmlUCSIsGurmukhi__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsGurmukhi xmlUCSIsGurmukhi__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsHalfwidthandFullwidthForms
extern __typeof (xmlUCSIsHalfwidthandFullwidthForms) xmlUCSIsHalfwidthandFullwidthForms __attribute((alias("xmlUCSIsHalfwidthandFullwidthForms__internal_alias")));
#else
#ifndef xmlUCSIsHalfwidthandFullwidthForms
extern __typeof (xmlUCSIsHalfwidthandFullwidthForms) xmlUCSIsHalfwidthandFullwidthForms__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsHalfwidthandFullwidthForms xmlUCSIsHalfwidthandFullwidthForms__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsHangulCompatibilityJamo
extern __typeof (xmlUCSIsHangulCompatibilityJamo) xmlUCSIsHangulCompatibilityJamo __attribute((alias("xmlUCSIsHangulCompatibilityJamo__internal_alias")));
#else
#ifndef xmlUCSIsHangulCompatibilityJamo
extern __typeof (xmlUCSIsHangulCompatibilityJamo) xmlUCSIsHangulCompatibilityJamo__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsHangulCompatibilityJamo xmlUCSIsHangulCompatibilityJamo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsHangulJamo
extern __typeof (xmlUCSIsHangulJamo) xmlUCSIsHangulJamo __attribute((alias("xmlUCSIsHangulJamo__internal_alias")));
#else
#ifndef xmlUCSIsHangulJamo
extern __typeof (xmlUCSIsHangulJamo) xmlUCSIsHangulJamo__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsHangulJamo xmlUCSIsHangulJamo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsHangulSyllables
extern __typeof (xmlUCSIsHangulSyllables) xmlUCSIsHangulSyllables __attribute((alias("xmlUCSIsHangulSyllables__internal_alias")));
#else
#ifndef xmlUCSIsHangulSyllables
extern __typeof (xmlUCSIsHangulSyllables) xmlUCSIsHangulSyllables__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsHangulSyllables xmlUCSIsHangulSyllables__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsHanunoo
extern __typeof (xmlUCSIsHanunoo) xmlUCSIsHanunoo __attribute((alias("xmlUCSIsHanunoo__internal_alias")));
#else
#ifndef xmlUCSIsHanunoo
extern __typeof (xmlUCSIsHanunoo) xmlUCSIsHanunoo__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsHanunoo xmlUCSIsHanunoo__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsHebrew
extern __typeof (xmlUCSIsHebrew) xmlUCSIsHebrew __attribute((alias("xmlUCSIsHebrew__internal_alias")));
#else
#ifndef xmlUCSIsHebrew
extern __typeof (xmlUCSIsHebrew) xmlUCSIsHebrew__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsHebrew xmlUCSIsHebrew__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsHighPrivateUseSurrogates
extern __typeof (xmlUCSIsHighPrivateUseSurrogates) xmlUCSIsHighPrivateUseSurrogates __attribute((alias("xmlUCSIsHighPrivateUseSurrogates__internal_alias")));
#else
#ifndef xmlUCSIsHighPrivateUseSurrogates
extern __typeof (xmlUCSIsHighPrivateUseSurrogates) xmlUCSIsHighPrivateUseSurrogates__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsHighPrivateUseSurrogates xmlUCSIsHighPrivateUseSurrogates__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsHighSurrogates
extern __typeof (xmlUCSIsHighSurrogates) xmlUCSIsHighSurrogates __attribute((alias("xmlUCSIsHighSurrogates__internal_alias")));
#else
#ifndef xmlUCSIsHighSurrogates
extern __typeof (xmlUCSIsHighSurrogates) xmlUCSIsHighSurrogates__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsHighSurrogates xmlUCSIsHighSurrogates__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsHiragana
extern __typeof (xmlUCSIsHiragana) xmlUCSIsHiragana __attribute((alias("xmlUCSIsHiragana__internal_alias")));
#else
#ifndef xmlUCSIsHiragana
extern __typeof (xmlUCSIsHiragana) xmlUCSIsHiragana__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsHiragana xmlUCSIsHiragana__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsIPAExtensions
extern __typeof (xmlUCSIsIPAExtensions) xmlUCSIsIPAExtensions __attribute((alias("xmlUCSIsIPAExtensions__internal_alias")));
#else
#ifndef xmlUCSIsIPAExtensions
extern __typeof (xmlUCSIsIPAExtensions) xmlUCSIsIPAExtensions__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsIPAExtensions xmlUCSIsIPAExtensions__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsIdeographicDescriptionCharacters
extern __typeof (xmlUCSIsIdeographicDescriptionCharacters) xmlUCSIsIdeographicDescriptionCharacters __attribute((alias("xmlUCSIsIdeographicDescriptionCharacters__internal_alias")));
#else
#ifndef xmlUCSIsIdeographicDescriptionCharacters
extern __typeof (xmlUCSIsIdeographicDescriptionCharacters) xmlUCSIsIdeographicDescriptionCharacters__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsIdeographicDescriptionCharacters xmlUCSIsIdeographicDescriptionCharacters__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsKanbun
extern __typeof (xmlUCSIsKanbun) xmlUCSIsKanbun __attribute((alias("xmlUCSIsKanbun__internal_alias")));
#else
#ifndef xmlUCSIsKanbun
extern __typeof (xmlUCSIsKanbun) xmlUCSIsKanbun__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsKanbun xmlUCSIsKanbun__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsKangxiRadicals
extern __typeof (xmlUCSIsKangxiRadicals) xmlUCSIsKangxiRadicals __attribute((alias("xmlUCSIsKangxiRadicals__internal_alias")));
#else
#ifndef xmlUCSIsKangxiRadicals
extern __typeof (xmlUCSIsKangxiRadicals) xmlUCSIsKangxiRadicals__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsKangxiRadicals xmlUCSIsKangxiRadicals__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsKannada
extern __typeof (xmlUCSIsKannada) xmlUCSIsKannada __attribute((alias("xmlUCSIsKannada__internal_alias")));
#else
#ifndef xmlUCSIsKannada
extern __typeof (xmlUCSIsKannada) xmlUCSIsKannada__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsKannada xmlUCSIsKannada__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsKatakana
extern __typeof (xmlUCSIsKatakana) xmlUCSIsKatakana __attribute((alias("xmlUCSIsKatakana__internal_alias")));
#else
#ifndef xmlUCSIsKatakana
extern __typeof (xmlUCSIsKatakana) xmlUCSIsKatakana__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsKatakana xmlUCSIsKatakana__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsKatakanaPhoneticExtensions
extern __typeof (xmlUCSIsKatakanaPhoneticExtensions) xmlUCSIsKatakanaPhoneticExtensions __attribute((alias("xmlUCSIsKatakanaPhoneticExtensions__internal_alias")));
#else
#ifndef xmlUCSIsKatakanaPhoneticExtensions
extern __typeof (xmlUCSIsKatakanaPhoneticExtensions) xmlUCSIsKatakanaPhoneticExtensions__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsKatakanaPhoneticExtensions xmlUCSIsKatakanaPhoneticExtensions__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsKhmer
extern __typeof (xmlUCSIsKhmer) xmlUCSIsKhmer __attribute((alias("xmlUCSIsKhmer__internal_alias")));
#else
#ifndef xmlUCSIsKhmer
extern __typeof (xmlUCSIsKhmer) xmlUCSIsKhmer__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsKhmer xmlUCSIsKhmer__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsKhmerSymbols
extern __typeof (xmlUCSIsKhmerSymbols) xmlUCSIsKhmerSymbols __attribute((alias("xmlUCSIsKhmerSymbols__internal_alias")));
#else
#ifndef xmlUCSIsKhmerSymbols
extern __typeof (xmlUCSIsKhmerSymbols) xmlUCSIsKhmerSymbols__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsKhmerSymbols xmlUCSIsKhmerSymbols__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsLao
extern __typeof (xmlUCSIsLao) xmlUCSIsLao __attribute((alias("xmlUCSIsLao__internal_alias")));
#else
#ifndef xmlUCSIsLao
extern __typeof (xmlUCSIsLao) xmlUCSIsLao__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsLao xmlUCSIsLao__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsLatin1Supplement
extern __typeof (xmlUCSIsLatin1Supplement) xmlUCSIsLatin1Supplement __attribute((alias("xmlUCSIsLatin1Supplement__internal_alias")));
#else
#ifndef xmlUCSIsLatin1Supplement
extern __typeof (xmlUCSIsLatin1Supplement) xmlUCSIsLatin1Supplement__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsLatin1Supplement xmlUCSIsLatin1Supplement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsLatinExtendedA
extern __typeof (xmlUCSIsLatinExtendedA) xmlUCSIsLatinExtendedA __attribute((alias("xmlUCSIsLatinExtendedA__internal_alias")));
#else
#ifndef xmlUCSIsLatinExtendedA
extern __typeof (xmlUCSIsLatinExtendedA) xmlUCSIsLatinExtendedA__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsLatinExtendedA xmlUCSIsLatinExtendedA__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsLatinExtendedAdditional
extern __typeof (xmlUCSIsLatinExtendedAdditional) xmlUCSIsLatinExtendedAdditional __attribute((alias("xmlUCSIsLatinExtendedAdditional__internal_alias")));
#else
#ifndef xmlUCSIsLatinExtendedAdditional
extern __typeof (xmlUCSIsLatinExtendedAdditional) xmlUCSIsLatinExtendedAdditional__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsLatinExtendedAdditional xmlUCSIsLatinExtendedAdditional__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsLatinExtendedB
extern __typeof (xmlUCSIsLatinExtendedB) xmlUCSIsLatinExtendedB __attribute((alias("xmlUCSIsLatinExtendedB__internal_alias")));
#else
#ifndef xmlUCSIsLatinExtendedB
extern __typeof (xmlUCSIsLatinExtendedB) xmlUCSIsLatinExtendedB__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsLatinExtendedB xmlUCSIsLatinExtendedB__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsLetterlikeSymbols
extern __typeof (xmlUCSIsLetterlikeSymbols) xmlUCSIsLetterlikeSymbols __attribute((alias("xmlUCSIsLetterlikeSymbols__internal_alias")));
#else
#ifndef xmlUCSIsLetterlikeSymbols
extern __typeof (xmlUCSIsLetterlikeSymbols) xmlUCSIsLetterlikeSymbols__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsLetterlikeSymbols xmlUCSIsLetterlikeSymbols__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsLimbu
extern __typeof (xmlUCSIsLimbu) xmlUCSIsLimbu __attribute((alias("xmlUCSIsLimbu__internal_alias")));
#else
#ifndef xmlUCSIsLimbu
extern __typeof (xmlUCSIsLimbu) xmlUCSIsLimbu__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsLimbu xmlUCSIsLimbu__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsLinearBIdeograms
extern __typeof (xmlUCSIsLinearBIdeograms) xmlUCSIsLinearBIdeograms __attribute((alias("xmlUCSIsLinearBIdeograms__internal_alias")));
#else
#ifndef xmlUCSIsLinearBIdeograms
extern __typeof (xmlUCSIsLinearBIdeograms) xmlUCSIsLinearBIdeograms__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsLinearBIdeograms xmlUCSIsLinearBIdeograms__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsLinearBSyllabary
extern __typeof (xmlUCSIsLinearBSyllabary) xmlUCSIsLinearBSyllabary __attribute((alias("xmlUCSIsLinearBSyllabary__internal_alias")));
#else
#ifndef xmlUCSIsLinearBSyllabary
extern __typeof (xmlUCSIsLinearBSyllabary) xmlUCSIsLinearBSyllabary__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsLinearBSyllabary xmlUCSIsLinearBSyllabary__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsLowSurrogates
extern __typeof (xmlUCSIsLowSurrogates) xmlUCSIsLowSurrogates __attribute((alias("xmlUCSIsLowSurrogates__internal_alias")));
#else
#ifndef xmlUCSIsLowSurrogates
extern __typeof (xmlUCSIsLowSurrogates) xmlUCSIsLowSurrogates__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsLowSurrogates xmlUCSIsLowSurrogates__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsMalayalam
extern __typeof (xmlUCSIsMalayalam) xmlUCSIsMalayalam __attribute((alias("xmlUCSIsMalayalam__internal_alias")));
#else
#ifndef xmlUCSIsMalayalam
extern __typeof (xmlUCSIsMalayalam) xmlUCSIsMalayalam__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsMalayalam xmlUCSIsMalayalam__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsMathematicalAlphanumericSymbols
extern __typeof (xmlUCSIsMathematicalAlphanumericSymbols) xmlUCSIsMathematicalAlphanumericSymbols __attribute((alias("xmlUCSIsMathematicalAlphanumericSymbols__internal_alias")));
#else
#ifndef xmlUCSIsMathematicalAlphanumericSymbols
extern __typeof (xmlUCSIsMathematicalAlphanumericSymbols) xmlUCSIsMathematicalAlphanumericSymbols__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsMathematicalAlphanumericSymbols xmlUCSIsMathematicalAlphanumericSymbols__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsMathematicalOperators
extern __typeof (xmlUCSIsMathematicalOperators) xmlUCSIsMathematicalOperators __attribute((alias("xmlUCSIsMathematicalOperators__internal_alias")));
#else
#ifndef xmlUCSIsMathematicalOperators
extern __typeof (xmlUCSIsMathematicalOperators) xmlUCSIsMathematicalOperators__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsMathematicalOperators xmlUCSIsMathematicalOperators__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsMiscellaneousMathematicalSymbolsA
extern __typeof (xmlUCSIsMiscellaneousMathematicalSymbolsA) xmlUCSIsMiscellaneousMathematicalSymbolsA __attribute((alias("xmlUCSIsMiscellaneousMathematicalSymbolsA__internal_alias")));
#else
#ifndef xmlUCSIsMiscellaneousMathematicalSymbolsA
extern __typeof (xmlUCSIsMiscellaneousMathematicalSymbolsA) xmlUCSIsMiscellaneousMathematicalSymbolsA__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsMiscellaneousMathematicalSymbolsA xmlUCSIsMiscellaneousMathematicalSymbolsA__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsMiscellaneousMathematicalSymbolsB
extern __typeof (xmlUCSIsMiscellaneousMathematicalSymbolsB) xmlUCSIsMiscellaneousMathematicalSymbolsB __attribute((alias("xmlUCSIsMiscellaneousMathematicalSymbolsB__internal_alias")));
#else
#ifndef xmlUCSIsMiscellaneousMathematicalSymbolsB
extern __typeof (xmlUCSIsMiscellaneousMathematicalSymbolsB) xmlUCSIsMiscellaneousMathematicalSymbolsB__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsMiscellaneousMathematicalSymbolsB xmlUCSIsMiscellaneousMathematicalSymbolsB__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsMiscellaneousSymbols
extern __typeof (xmlUCSIsMiscellaneousSymbols) xmlUCSIsMiscellaneousSymbols __attribute((alias("xmlUCSIsMiscellaneousSymbols__internal_alias")));
#else
#ifndef xmlUCSIsMiscellaneousSymbols
extern __typeof (xmlUCSIsMiscellaneousSymbols) xmlUCSIsMiscellaneousSymbols__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsMiscellaneousSymbols xmlUCSIsMiscellaneousSymbols__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsMiscellaneousSymbolsandArrows
extern __typeof (xmlUCSIsMiscellaneousSymbolsandArrows) xmlUCSIsMiscellaneousSymbolsandArrows __attribute((alias("xmlUCSIsMiscellaneousSymbolsandArrows__internal_alias")));
#else
#ifndef xmlUCSIsMiscellaneousSymbolsandArrows
extern __typeof (xmlUCSIsMiscellaneousSymbolsandArrows) xmlUCSIsMiscellaneousSymbolsandArrows__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsMiscellaneousSymbolsandArrows xmlUCSIsMiscellaneousSymbolsandArrows__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsMiscellaneousTechnical
extern __typeof (xmlUCSIsMiscellaneousTechnical) xmlUCSIsMiscellaneousTechnical __attribute((alias("xmlUCSIsMiscellaneousTechnical__internal_alias")));
#else
#ifndef xmlUCSIsMiscellaneousTechnical
extern __typeof (xmlUCSIsMiscellaneousTechnical) xmlUCSIsMiscellaneousTechnical__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsMiscellaneousTechnical xmlUCSIsMiscellaneousTechnical__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsMongolian
extern __typeof (xmlUCSIsMongolian) xmlUCSIsMongolian __attribute((alias("xmlUCSIsMongolian__internal_alias")));
#else
#ifndef xmlUCSIsMongolian
extern __typeof (xmlUCSIsMongolian) xmlUCSIsMongolian__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsMongolian xmlUCSIsMongolian__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsMusicalSymbols
extern __typeof (xmlUCSIsMusicalSymbols) xmlUCSIsMusicalSymbols __attribute((alias("xmlUCSIsMusicalSymbols__internal_alias")));
#else
#ifndef xmlUCSIsMusicalSymbols
extern __typeof (xmlUCSIsMusicalSymbols) xmlUCSIsMusicalSymbols__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsMusicalSymbols xmlUCSIsMusicalSymbols__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsMyanmar
extern __typeof (xmlUCSIsMyanmar) xmlUCSIsMyanmar __attribute((alias("xmlUCSIsMyanmar__internal_alias")));
#else
#ifndef xmlUCSIsMyanmar
extern __typeof (xmlUCSIsMyanmar) xmlUCSIsMyanmar__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsMyanmar xmlUCSIsMyanmar__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsNumberForms
extern __typeof (xmlUCSIsNumberForms) xmlUCSIsNumberForms __attribute((alias("xmlUCSIsNumberForms__internal_alias")));
#else
#ifndef xmlUCSIsNumberForms
extern __typeof (xmlUCSIsNumberForms) xmlUCSIsNumberForms__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsNumberForms xmlUCSIsNumberForms__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsOgham
extern __typeof (xmlUCSIsOgham) xmlUCSIsOgham __attribute((alias("xmlUCSIsOgham__internal_alias")));
#else
#ifndef xmlUCSIsOgham
extern __typeof (xmlUCSIsOgham) xmlUCSIsOgham__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsOgham xmlUCSIsOgham__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsOldItalic
extern __typeof (xmlUCSIsOldItalic) xmlUCSIsOldItalic __attribute((alias("xmlUCSIsOldItalic__internal_alias")));
#else
#ifndef xmlUCSIsOldItalic
extern __typeof (xmlUCSIsOldItalic) xmlUCSIsOldItalic__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsOldItalic xmlUCSIsOldItalic__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsOpticalCharacterRecognition
extern __typeof (xmlUCSIsOpticalCharacterRecognition) xmlUCSIsOpticalCharacterRecognition __attribute((alias("xmlUCSIsOpticalCharacterRecognition__internal_alias")));
#else
#ifndef xmlUCSIsOpticalCharacterRecognition
extern __typeof (xmlUCSIsOpticalCharacterRecognition) xmlUCSIsOpticalCharacterRecognition__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsOpticalCharacterRecognition xmlUCSIsOpticalCharacterRecognition__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsOriya
extern __typeof (xmlUCSIsOriya) xmlUCSIsOriya __attribute((alias("xmlUCSIsOriya__internal_alias")));
#else
#ifndef xmlUCSIsOriya
extern __typeof (xmlUCSIsOriya) xmlUCSIsOriya__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsOriya xmlUCSIsOriya__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsOsmanya
extern __typeof (xmlUCSIsOsmanya) xmlUCSIsOsmanya __attribute((alias("xmlUCSIsOsmanya__internal_alias")));
#else
#ifndef xmlUCSIsOsmanya
extern __typeof (xmlUCSIsOsmanya) xmlUCSIsOsmanya__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsOsmanya xmlUCSIsOsmanya__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsPhoneticExtensions
extern __typeof (xmlUCSIsPhoneticExtensions) xmlUCSIsPhoneticExtensions __attribute((alias("xmlUCSIsPhoneticExtensions__internal_alias")));
#else
#ifndef xmlUCSIsPhoneticExtensions
extern __typeof (xmlUCSIsPhoneticExtensions) xmlUCSIsPhoneticExtensions__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsPhoneticExtensions xmlUCSIsPhoneticExtensions__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsPrivateUse
extern __typeof (xmlUCSIsPrivateUse) xmlUCSIsPrivateUse __attribute((alias("xmlUCSIsPrivateUse__internal_alias")));
#else
#ifndef xmlUCSIsPrivateUse
extern __typeof (xmlUCSIsPrivateUse) xmlUCSIsPrivateUse__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsPrivateUse xmlUCSIsPrivateUse__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsPrivateUseArea
extern __typeof (xmlUCSIsPrivateUseArea) xmlUCSIsPrivateUseArea __attribute((alias("xmlUCSIsPrivateUseArea__internal_alias")));
#else
#ifndef xmlUCSIsPrivateUseArea
extern __typeof (xmlUCSIsPrivateUseArea) xmlUCSIsPrivateUseArea__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsPrivateUseArea xmlUCSIsPrivateUseArea__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsRunic
extern __typeof (xmlUCSIsRunic) xmlUCSIsRunic __attribute((alias("xmlUCSIsRunic__internal_alias")));
#else
#ifndef xmlUCSIsRunic
extern __typeof (xmlUCSIsRunic) xmlUCSIsRunic__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsRunic xmlUCSIsRunic__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsShavian
extern __typeof (xmlUCSIsShavian) xmlUCSIsShavian __attribute((alias("xmlUCSIsShavian__internal_alias")));
#else
#ifndef xmlUCSIsShavian
extern __typeof (xmlUCSIsShavian) xmlUCSIsShavian__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsShavian xmlUCSIsShavian__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsSinhala
extern __typeof (xmlUCSIsSinhala) xmlUCSIsSinhala __attribute((alias("xmlUCSIsSinhala__internal_alias")));
#else
#ifndef xmlUCSIsSinhala
extern __typeof (xmlUCSIsSinhala) xmlUCSIsSinhala__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsSinhala xmlUCSIsSinhala__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsSmallFormVariants
extern __typeof (xmlUCSIsSmallFormVariants) xmlUCSIsSmallFormVariants __attribute((alias("xmlUCSIsSmallFormVariants__internal_alias")));
#else
#ifndef xmlUCSIsSmallFormVariants
extern __typeof (xmlUCSIsSmallFormVariants) xmlUCSIsSmallFormVariants__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsSmallFormVariants xmlUCSIsSmallFormVariants__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsSpacingModifierLetters
extern __typeof (xmlUCSIsSpacingModifierLetters) xmlUCSIsSpacingModifierLetters __attribute((alias("xmlUCSIsSpacingModifierLetters__internal_alias")));
#else
#ifndef xmlUCSIsSpacingModifierLetters
extern __typeof (xmlUCSIsSpacingModifierLetters) xmlUCSIsSpacingModifierLetters__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsSpacingModifierLetters xmlUCSIsSpacingModifierLetters__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsSpecials
extern __typeof (xmlUCSIsSpecials) xmlUCSIsSpecials __attribute((alias("xmlUCSIsSpecials__internal_alias")));
#else
#ifndef xmlUCSIsSpecials
extern __typeof (xmlUCSIsSpecials) xmlUCSIsSpecials__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsSpecials xmlUCSIsSpecials__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsSuperscriptsandSubscripts
extern __typeof (xmlUCSIsSuperscriptsandSubscripts) xmlUCSIsSuperscriptsandSubscripts __attribute((alias("xmlUCSIsSuperscriptsandSubscripts__internal_alias")));
#else
#ifndef xmlUCSIsSuperscriptsandSubscripts
extern __typeof (xmlUCSIsSuperscriptsandSubscripts) xmlUCSIsSuperscriptsandSubscripts__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsSuperscriptsandSubscripts xmlUCSIsSuperscriptsandSubscripts__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsSupplementalArrowsA
extern __typeof (xmlUCSIsSupplementalArrowsA) xmlUCSIsSupplementalArrowsA __attribute((alias("xmlUCSIsSupplementalArrowsA__internal_alias")));
#else
#ifndef xmlUCSIsSupplementalArrowsA
extern __typeof (xmlUCSIsSupplementalArrowsA) xmlUCSIsSupplementalArrowsA__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsSupplementalArrowsA xmlUCSIsSupplementalArrowsA__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsSupplementalArrowsB
extern __typeof (xmlUCSIsSupplementalArrowsB) xmlUCSIsSupplementalArrowsB __attribute((alias("xmlUCSIsSupplementalArrowsB__internal_alias")));
#else
#ifndef xmlUCSIsSupplementalArrowsB
extern __typeof (xmlUCSIsSupplementalArrowsB) xmlUCSIsSupplementalArrowsB__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsSupplementalArrowsB xmlUCSIsSupplementalArrowsB__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsSupplementalMathematicalOperators
extern __typeof (xmlUCSIsSupplementalMathematicalOperators) xmlUCSIsSupplementalMathematicalOperators __attribute((alias("xmlUCSIsSupplementalMathematicalOperators__internal_alias")));
#else
#ifndef xmlUCSIsSupplementalMathematicalOperators
extern __typeof (xmlUCSIsSupplementalMathematicalOperators) xmlUCSIsSupplementalMathematicalOperators__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsSupplementalMathematicalOperators xmlUCSIsSupplementalMathematicalOperators__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsSupplementaryPrivateUseAreaA
extern __typeof (xmlUCSIsSupplementaryPrivateUseAreaA) xmlUCSIsSupplementaryPrivateUseAreaA __attribute((alias("xmlUCSIsSupplementaryPrivateUseAreaA__internal_alias")));
#else
#ifndef xmlUCSIsSupplementaryPrivateUseAreaA
extern __typeof (xmlUCSIsSupplementaryPrivateUseAreaA) xmlUCSIsSupplementaryPrivateUseAreaA__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsSupplementaryPrivateUseAreaA xmlUCSIsSupplementaryPrivateUseAreaA__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsSupplementaryPrivateUseAreaB
extern __typeof (xmlUCSIsSupplementaryPrivateUseAreaB) xmlUCSIsSupplementaryPrivateUseAreaB __attribute((alias("xmlUCSIsSupplementaryPrivateUseAreaB__internal_alias")));
#else
#ifndef xmlUCSIsSupplementaryPrivateUseAreaB
extern __typeof (xmlUCSIsSupplementaryPrivateUseAreaB) xmlUCSIsSupplementaryPrivateUseAreaB__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsSupplementaryPrivateUseAreaB xmlUCSIsSupplementaryPrivateUseAreaB__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsSyriac
extern __typeof (xmlUCSIsSyriac) xmlUCSIsSyriac __attribute((alias("xmlUCSIsSyriac__internal_alias")));
#else
#ifndef xmlUCSIsSyriac
extern __typeof (xmlUCSIsSyriac) xmlUCSIsSyriac__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsSyriac xmlUCSIsSyriac__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsTagalog
extern __typeof (xmlUCSIsTagalog) xmlUCSIsTagalog __attribute((alias("xmlUCSIsTagalog__internal_alias")));
#else
#ifndef xmlUCSIsTagalog
extern __typeof (xmlUCSIsTagalog) xmlUCSIsTagalog__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsTagalog xmlUCSIsTagalog__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsTagbanwa
extern __typeof (xmlUCSIsTagbanwa) xmlUCSIsTagbanwa __attribute((alias("xmlUCSIsTagbanwa__internal_alias")));
#else
#ifndef xmlUCSIsTagbanwa
extern __typeof (xmlUCSIsTagbanwa) xmlUCSIsTagbanwa__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsTagbanwa xmlUCSIsTagbanwa__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsTags
extern __typeof (xmlUCSIsTags) xmlUCSIsTags __attribute((alias("xmlUCSIsTags__internal_alias")));
#else
#ifndef xmlUCSIsTags
extern __typeof (xmlUCSIsTags) xmlUCSIsTags__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsTags xmlUCSIsTags__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsTaiLe
extern __typeof (xmlUCSIsTaiLe) xmlUCSIsTaiLe __attribute((alias("xmlUCSIsTaiLe__internal_alias")));
#else
#ifndef xmlUCSIsTaiLe
extern __typeof (xmlUCSIsTaiLe) xmlUCSIsTaiLe__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsTaiLe xmlUCSIsTaiLe__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsTaiXuanJingSymbols
extern __typeof (xmlUCSIsTaiXuanJingSymbols) xmlUCSIsTaiXuanJingSymbols __attribute((alias("xmlUCSIsTaiXuanJingSymbols__internal_alias")));
#else
#ifndef xmlUCSIsTaiXuanJingSymbols
extern __typeof (xmlUCSIsTaiXuanJingSymbols) xmlUCSIsTaiXuanJingSymbols__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsTaiXuanJingSymbols xmlUCSIsTaiXuanJingSymbols__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsTamil
extern __typeof (xmlUCSIsTamil) xmlUCSIsTamil __attribute((alias("xmlUCSIsTamil__internal_alias")));
#else
#ifndef xmlUCSIsTamil
extern __typeof (xmlUCSIsTamil) xmlUCSIsTamil__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsTamil xmlUCSIsTamil__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsTelugu
extern __typeof (xmlUCSIsTelugu) xmlUCSIsTelugu __attribute((alias("xmlUCSIsTelugu__internal_alias")));
#else
#ifndef xmlUCSIsTelugu
extern __typeof (xmlUCSIsTelugu) xmlUCSIsTelugu__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsTelugu xmlUCSIsTelugu__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsThaana
extern __typeof (xmlUCSIsThaana) xmlUCSIsThaana __attribute((alias("xmlUCSIsThaana__internal_alias")));
#else
#ifndef xmlUCSIsThaana
extern __typeof (xmlUCSIsThaana) xmlUCSIsThaana__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsThaana xmlUCSIsThaana__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsThai
extern __typeof (xmlUCSIsThai) xmlUCSIsThai __attribute((alias("xmlUCSIsThai__internal_alias")));
#else
#ifndef xmlUCSIsThai
extern __typeof (xmlUCSIsThai) xmlUCSIsThai__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsThai xmlUCSIsThai__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsTibetan
extern __typeof (xmlUCSIsTibetan) xmlUCSIsTibetan __attribute((alias("xmlUCSIsTibetan__internal_alias")));
#else
#ifndef xmlUCSIsTibetan
extern __typeof (xmlUCSIsTibetan) xmlUCSIsTibetan__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsTibetan xmlUCSIsTibetan__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsUgaritic
extern __typeof (xmlUCSIsUgaritic) xmlUCSIsUgaritic __attribute((alias("xmlUCSIsUgaritic__internal_alias")));
#else
#ifndef xmlUCSIsUgaritic
extern __typeof (xmlUCSIsUgaritic) xmlUCSIsUgaritic__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsUgaritic xmlUCSIsUgaritic__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsUnifiedCanadianAboriginalSyllabics
extern __typeof (xmlUCSIsUnifiedCanadianAboriginalSyllabics) xmlUCSIsUnifiedCanadianAboriginalSyllabics __attribute((alias("xmlUCSIsUnifiedCanadianAboriginalSyllabics__internal_alias")));
#else
#ifndef xmlUCSIsUnifiedCanadianAboriginalSyllabics
extern __typeof (xmlUCSIsUnifiedCanadianAboriginalSyllabics) xmlUCSIsUnifiedCanadianAboriginalSyllabics__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsUnifiedCanadianAboriginalSyllabics xmlUCSIsUnifiedCanadianAboriginalSyllabics__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsVariationSelectors
extern __typeof (xmlUCSIsVariationSelectors) xmlUCSIsVariationSelectors __attribute((alias("xmlUCSIsVariationSelectors__internal_alias")));
#else
#ifndef xmlUCSIsVariationSelectors
extern __typeof (xmlUCSIsVariationSelectors) xmlUCSIsVariationSelectors__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsVariationSelectors xmlUCSIsVariationSelectors__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsVariationSelectorsSupplement
extern __typeof (xmlUCSIsVariationSelectorsSupplement) xmlUCSIsVariationSelectorsSupplement __attribute((alias("xmlUCSIsVariationSelectorsSupplement__internal_alias")));
#else
#ifndef xmlUCSIsVariationSelectorsSupplement
extern __typeof (xmlUCSIsVariationSelectorsSupplement) xmlUCSIsVariationSelectorsSupplement__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsVariationSelectorsSupplement xmlUCSIsVariationSelectorsSupplement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsYiRadicals
extern __typeof (xmlUCSIsYiRadicals) xmlUCSIsYiRadicals __attribute((alias("xmlUCSIsYiRadicals__internal_alias")));
#else
#ifndef xmlUCSIsYiRadicals
extern __typeof (xmlUCSIsYiRadicals) xmlUCSIsYiRadicals__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsYiRadicals xmlUCSIsYiRadicals__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsYiSyllables
extern __typeof (xmlUCSIsYiSyllables) xmlUCSIsYiSyllables __attribute((alias("xmlUCSIsYiSyllables__internal_alias")));
#else
#ifndef xmlUCSIsYiSyllables
extern __typeof (xmlUCSIsYiSyllables) xmlUCSIsYiSyllables__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsYiSyllables xmlUCSIsYiSyllables__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_UNICODE_ENABLED)
#ifdef bottom_xmlunicode
#undef xmlUCSIsYijingHexagramSymbols
extern __typeof (xmlUCSIsYijingHexagramSymbols) xmlUCSIsYijingHexagramSymbols __attribute((alias("xmlUCSIsYijingHexagramSymbols__internal_alias")));
#else
#ifndef xmlUCSIsYijingHexagramSymbols
extern __typeof (xmlUCSIsYijingHexagramSymbols) xmlUCSIsYijingHexagramSymbols__internal_alias __attribute((visibility("hidden")));
#define xmlUCSIsYijingHexagramSymbols xmlUCSIsYijingHexagramSymbols__internal_alias
#endif
#endif
#endif

#ifdef bottom_uri
#undef xmlURIEscape
extern __typeof (xmlURIEscape) xmlURIEscape __attribute((alias("xmlURIEscape__internal_alias")));
#else
#ifndef xmlURIEscape
extern __typeof (xmlURIEscape) xmlURIEscape__internal_alias __attribute((visibility("hidden")));
#define xmlURIEscape xmlURIEscape__internal_alias
#endif
#endif

#ifdef bottom_uri
#undef xmlURIEscapeStr
extern __typeof (xmlURIEscapeStr) xmlURIEscapeStr __attribute((alias("xmlURIEscapeStr__internal_alias")));
#else
#ifndef xmlURIEscapeStr
extern __typeof (xmlURIEscapeStr) xmlURIEscapeStr__internal_alias __attribute((visibility("hidden")));
#define xmlURIEscapeStr xmlURIEscapeStr__internal_alias
#endif
#endif

#ifdef bottom_uri
#undef xmlURIUnescapeString
extern __typeof (xmlURIUnescapeString) xmlURIUnescapeString __attribute((alias("xmlURIUnescapeString__internal_alias")));
#else
#ifndef xmlURIUnescapeString
extern __typeof (xmlURIUnescapeString) xmlURIUnescapeString__internal_alias __attribute((visibility("hidden")));
#define xmlURIUnescapeString xmlURIUnescapeString__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlUTF8Charcmp
extern __typeof (xmlUTF8Charcmp) xmlUTF8Charcmp __attribute((alias("xmlUTF8Charcmp__internal_alias")));
#else
#ifndef xmlUTF8Charcmp
extern __typeof (xmlUTF8Charcmp) xmlUTF8Charcmp__internal_alias __attribute((visibility("hidden")));
#define xmlUTF8Charcmp xmlUTF8Charcmp__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlUTF8Size
extern __typeof (xmlUTF8Size) xmlUTF8Size __attribute((alias("xmlUTF8Size__internal_alias")));
#else
#ifndef xmlUTF8Size
extern __typeof (xmlUTF8Size) xmlUTF8Size__internal_alias __attribute((visibility("hidden")));
#define xmlUTF8Size xmlUTF8Size__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlUTF8Strlen
extern __typeof (xmlUTF8Strlen) xmlUTF8Strlen __attribute((alias("xmlUTF8Strlen__internal_alias")));
#else
#ifndef xmlUTF8Strlen
extern __typeof (xmlUTF8Strlen) xmlUTF8Strlen__internal_alias __attribute((visibility("hidden")));
#define xmlUTF8Strlen xmlUTF8Strlen__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlUTF8Strloc
extern __typeof (xmlUTF8Strloc) xmlUTF8Strloc __attribute((alias("xmlUTF8Strloc__internal_alias")));
#else
#ifndef xmlUTF8Strloc
extern __typeof (xmlUTF8Strloc) xmlUTF8Strloc__internal_alias __attribute((visibility("hidden")));
#define xmlUTF8Strloc xmlUTF8Strloc__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlUTF8Strndup
extern __typeof (xmlUTF8Strndup) xmlUTF8Strndup __attribute((alias("xmlUTF8Strndup__internal_alias")));
#else
#ifndef xmlUTF8Strndup
extern __typeof (xmlUTF8Strndup) xmlUTF8Strndup__internal_alias __attribute((visibility("hidden")));
#define xmlUTF8Strndup xmlUTF8Strndup__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlUTF8Strpos
extern __typeof (xmlUTF8Strpos) xmlUTF8Strpos __attribute((alias("xmlUTF8Strpos__internal_alias")));
#else
#ifndef xmlUTF8Strpos
extern __typeof (xmlUTF8Strpos) xmlUTF8Strpos__internal_alias __attribute((visibility("hidden")));
#define xmlUTF8Strpos xmlUTF8Strpos__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlUTF8Strsize
extern __typeof (xmlUTF8Strsize) xmlUTF8Strsize __attribute((alias("xmlUTF8Strsize__internal_alias")));
#else
#ifndef xmlUTF8Strsize
extern __typeof (xmlUTF8Strsize) xmlUTF8Strsize__internal_alias __attribute((visibility("hidden")));
#define xmlUTF8Strsize xmlUTF8Strsize__internal_alias
#endif
#endif

#ifdef bottom_xmlstring
#undef xmlUTF8Strsub
extern __typeof (xmlUTF8Strsub) xmlUTF8Strsub __attribute((alias("xmlUTF8Strsub__internal_alias")));
#else
#ifndef xmlUTF8Strsub
extern __typeof (xmlUTF8Strsub) xmlUTF8Strsub__internal_alias __attribute((visibility("hidden")));
#define xmlUTF8Strsub xmlUTF8Strsub__internal_alias
#endif
#endif

#ifdef bottom_tree
#undef xmlUnlinkNode
extern __typeof (xmlUnlinkNode) xmlUnlinkNode __attribute((alias("xmlUnlinkNode__internal_alias")));
#else
#ifndef xmlUnlinkNode
extern __typeof (xmlUnlinkNode) xmlUnlinkNode__internal_alias __attribute((visibility("hidden")));
#define xmlUnlinkNode xmlUnlinkNode__internal_alias
#endif
#endif

#ifdef bottom_threads
#undef xmlUnlockLibrary
extern __typeof (xmlUnlockLibrary) xmlUnlockLibrary __attribute((alias("xmlUnlockLibrary__internal_alias")));
#else
#ifndef xmlUnlockLibrary
extern __typeof (xmlUnlockLibrary) xmlUnlockLibrary__internal_alias __attribute((visibility("hidden")));
#define xmlUnlockLibrary xmlUnlockLibrary__internal_alias
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_tree
#undef xmlUnsetNsProp
extern __typeof (xmlUnsetNsProp) xmlUnsetNsProp __attribute((alias("xmlUnsetNsProp__internal_alias")));
#else
#ifndef xmlUnsetNsProp
extern __typeof (xmlUnsetNsProp) xmlUnsetNsProp__internal_alias __attribute((visibility("hidden")));
#define xmlUnsetNsProp xmlUnsetNsProp__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_tree
#undef xmlUnsetProp
extern __typeof (xmlUnsetProp) xmlUnsetProp __attribute((alias("xmlUnsetProp__internal_alias")));
#else
#ifndef xmlUnsetProp
extern __typeof (xmlUnsetProp) xmlUnsetProp__internal_alias __attribute((visibility("hidden")));
#define xmlUnsetProp xmlUnsetProp__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED) && defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_valid
#undef xmlValidBuildContentModel
extern __typeof (xmlValidBuildContentModel) xmlValidBuildContentModel __attribute((alias("xmlValidBuildContentModel__internal_alias")));
#else
#ifndef xmlValidBuildContentModel
extern __typeof (xmlValidBuildContentModel) xmlValidBuildContentModel__internal_alias __attribute((visibility("hidden")));
#define xmlValidBuildContentModel xmlValidBuildContentModel__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidCtxtNormalizeAttributeValue
extern __typeof (xmlValidCtxtNormalizeAttributeValue) xmlValidCtxtNormalizeAttributeValue __attribute((alias("xmlValidCtxtNormalizeAttributeValue__internal_alias")));
#else
#ifndef xmlValidCtxtNormalizeAttributeValue
extern __typeof (xmlValidCtxtNormalizeAttributeValue) xmlValidCtxtNormalizeAttributeValue__internal_alias __attribute((visibility("hidden")));
#define xmlValidCtxtNormalizeAttributeValue xmlValidCtxtNormalizeAttributeValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidGetPotentialChildren
extern __typeof (xmlValidGetPotentialChildren) xmlValidGetPotentialChildren __attribute((alias("xmlValidGetPotentialChildren__internal_alias")));
#else
#ifndef xmlValidGetPotentialChildren
extern __typeof (xmlValidGetPotentialChildren) xmlValidGetPotentialChildren__internal_alias __attribute((visibility("hidden")));
#define xmlValidGetPotentialChildren xmlValidGetPotentialChildren__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidGetValidElements
extern __typeof (xmlValidGetValidElements) xmlValidGetValidElements __attribute((alias("xmlValidGetValidElements__internal_alias")));
#else
#ifndef xmlValidGetValidElements
extern __typeof (xmlValidGetValidElements) xmlValidGetValidElements__internal_alias __attribute((visibility("hidden")));
#define xmlValidGetValidElements xmlValidGetValidElements__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidNormalizeAttributeValue
extern __typeof (xmlValidNormalizeAttributeValue) xmlValidNormalizeAttributeValue __attribute((alias("xmlValidNormalizeAttributeValue__internal_alias")));
#else
#ifndef xmlValidNormalizeAttributeValue
extern __typeof (xmlValidNormalizeAttributeValue) xmlValidNormalizeAttributeValue__internal_alias __attribute((visibility("hidden")));
#define xmlValidNormalizeAttributeValue xmlValidNormalizeAttributeValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateAttributeDecl
extern __typeof (xmlValidateAttributeDecl) xmlValidateAttributeDecl __attribute((alias("xmlValidateAttributeDecl__internal_alias")));
#else
#ifndef xmlValidateAttributeDecl
extern __typeof (xmlValidateAttributeDecl) xmlValidateAttributeDecl__internal_alias __attribute((visibility("hidden")));
#define xmlValidateAttributeDecl xmlValidateAttributeDecl__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateAttributeValue
extern __typeof (xmlValidateAttributeValue) xmlValidateAttributeValue __attribute((alias("xmlValidateAttributeValue__internal_alias")));
#else
#ifndef xmlValidateAttributeValue
extern __typeof (xmlValidateAttributeValue) xmlValidateAttributeValue__internal_alias __attribute((visibility("hidden")));
#define xmlValidateAttributeValue xmlValidateAttributeValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateDocument
extern __typeof (xmlValidateDocument) xmlValidateDocument __attribute((alias("xmlValidateDocument__internal_alias")));
#else
#ifndef xmlValidateDocument
extern __typeof (xmlValidateDocument) xmlValidateDocument__internal_alias __attribute((visibility("hidden")));
#define xmlValidateDocument xmlValidateDocument__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateDocumentFinal
extern __typeof (xmlValidateDocumentFinal) xmlValidateDocumentFinal __attribute((alias("xmlValidateDocumentFinal__internal_alias")));
#else
#ifndef xmlValidateDocumentFinal
extern __typeof (xmlValidateDocumentFinal) xmlValidateDocumentFinal__internal_alias __attribute((visibility("hidden")));
#define xmlValidateDocumentFinal xmlValidateDocumentFinal__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateDtd
extern __typeof (xmlValidateDtd) xmlValidateDtd __attribute((alias("xmlValidateDtd__internal_alias")));
#else
#ifndef xmlValidateDtd
extern __typeof (xmlValidateDtd) xmlValidateDtd__internal_alias __attribute((visibility("hidden")));
#define xmlValidateDtd xmlValidateDtd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateDtdFinal
extern __typeof (xmlValidateDtdFinal) xmlValidateDtdFinal __attribute((alias("xmlValidateDtdFinal__internal_alias")));
#else
#ifndef xmlValidateDtdFinal
extern __typeof (xmlValidateDtdFinal) xmlValidateDtdFinal__internal_alias __attribute((visibility("hidden")));
#define xmlValidateDtdFinal xmlValidateDtdFinal__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateElement
extern __typeof (xmlValidateElement) xmlValidateElement __attribute((alias("xmlValidateElement__internal_alias")));
#else
#ifndef xmlValidateElement
extern __typeof (xmlValidateElement) xmlValidateElement__internal_alias __attribute((visibility("hidden")));
#define xmlValidateElement xmlValidateElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateElementDecl
extern __typeof (xmlValidateElementDecl) xmlValidateElementDecl __attribute((alias("xmlValidateElementDecl__internal_alias")));
#else
#ifndef xmlValidateElementDecl
extern __typeof (xmlValidateElementDecl) xmlValidateElementDecl__internal_alias __attribute((visibility("hidden")));
#define xmlValidateElementDecl xmlValidateElementDecl__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_XPATH_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED) || defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_tree
#undef xmlValidateNCName
extern __typeof (xmlValidateNCName) xmlValidateNCName __attribute((alias("xmlValidateNCName__internal_alias")));
#else
#ifndef xmlValidateNCName
extern __typeof (xmlValidateNCName) xmlValidateNCName__internal_alias __attribute((visibility("hidden")));
#define xmlValidateNCName xmlValidateNCName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_tree
#undef xmlValidateNMToken
extern __typeof (xmlValidateNMToken) xmlValidateNMToken __attribute((alias("xmlValidateNMToken__internal_alias")));
#else
#ifndef xmlValidateNMToken
extern __typeof (xmlValidateNMToken) xmlValidateNMToken__internal_alias __attribute((visibility("hidden")));
#define xmlValidateNMToken xmlValidateNMToken__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_tree
#undef xmlValidateName
extern __typeof (xmlValidateName) xmlValidateName __attribute((alias("xmlValidateName__internal_alias")));
#else
#ifndef xmlValidateName
extern __typeof (xmlValidateName) xmlValidateName__internal_alias __attribute((visibility("hidden")));
#define xmlValidateName xmlValidateName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateNameValue
extern __typeof (xmlValidateNameValue) xmlValidateNameValue __attribute((alias("xmlValidateNameValue__internal_alias")));
#else
#ifndef xmlValidateNameValue
extern __typeof (xmlValidateNameValue) xmlValidateNameValue__internal_alias __attribute((visibility("hidden")));
#define xmlValidateNameValue xmlValidateNameValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateNamesValue
extern __typeof (xmlValidateNamesValue) xmlValidateNamesValue __attribute((alias("xmlValidateNamesValue__internal_alias")));
#else
#ifndef xmlValidateNamesValue
extern __typeof (xmlValidateNamesValue) xmlValidateNamesValue__internal_alias __attribute((visibility("hidden")));
#define xmlValidateNamesValue xmlValidateNamesValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateNmtokenValue
extern __typeof (xmlValidateNmtokenValue) xmlValidateNmtokenValue __attribute((alias("xmlValidateNmtokenValue__internal_alias")));
#else
#ifndef xmlValidateNmtokenValue
extern __typeof (xmlValidateNmtokenValue) xmlValidateNmtokenValue__internal_alias __attribute((visibility("hidden")));
#define xmlValidateNmtokenValue xmlValidateNmtokenValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateNmtokensValue
extern __typeof (xmlValidateNmtokensValue) xmlValidateNmtokensValue __attribute((alias("xmlValidateNmtokensValue__internal_alias")));
#else
#ifndef xmlValidateNmtokensValue
extern __typeof (xmlValidateNmtokensValue) xmlValidateNmtokensValue__internal_alias __attribute((visibility("hidden")));
#define xmlValidateNmtokensValue xmlValidateNmtokensValue__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateNotationDecl
extern __typeof (xmlValidateNotationDecl) xmlValidateNotationDecl __attribute((alias("xmlValidateNotationDecl__internal_alias")));
#else
#ifndef xmlValidateNotationDecl
extern __typeof (xmlValidateNotationDecl) xmlValidateNotationDecl__internal_alias __attribute((visibility("hidden")));
#define xmlValidateNotationDecl xmlValidateNotationDecl__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_valid
#undef xmlValidateNotationUse
extern __typeof (xmlValidateNotationUse) xmlValidateNotationUse __attribute((alias("xmlValidateNotationUse__internal_alias")));
#else
#ifndef xmlValidateNotationUse
extern __typeof (xmlValidateNotationUse) xmlValidateNotationUse__internal_alias __attribute((visibility("hidden")));
#define xmlValidateNotationUse xmlValidateNotationUse__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateOneAttribute
extern __typeof (xmlValidateOneAttribute) xmlValidateOneAttribute __attribute((alias("xmlValidateOneAttribute__internal_alias")));
#else
#ifndef xmlValidateOneAttribute
extern __typeof (xmlValidateOneAttribute) xmlValidateOneAttribute__internal_alias __attribute((visibility("hidden")));
#define xmlValidateOneAttribute xmlValidateOneAttribute__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateOneElement
extern __typeof (xmlValidateOneElement) xmlValidateOneElement __attribute((alias("xmlValidateOneElement__internal_alias")));
#else
#ifndef xmlValidateOneElement
extern __typeof (xmlValidateOneElement) xmlValidateOneElement__internal_alias __attribute((visibility("hidden")));
#define xmlValidateOneElement xmlValidateOneElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateOneNamespace
extern __typeof (xmlValidateOneNamespace) xmlValidateOneNamespace __attribute((alias("xmlValidateOneNamespace__internal_alias")));
#else
#ifndef xmlValidateOneNamespace
extern __typeof (xmlValidateOneNamespace) xmlValidateOneNamespace__internal_alias __attribute((visibility("hidden")));
#define xmlValidateOneNamespace xmlValidateOneNamespace__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED) && defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_valid
#undef xmlValidatePopElement
extern __typeof (xmlValidatePopElement) xmlValidatePopElement __attribute((alias("xmlValidatePopElement__internal_alias")));
#else
#ifndef xmlValidatePopElement
extern __typeof (xmlValidatePopElement) xmlValidatePopElement__internal_alias __attribute((visibility("hidden")));
#define xmlValidatePopElement xmlValidatePopElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED) && defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_valid
#undef xmlValidatePushCData
extern __typeof (xmlValidatePushCData) xmlValidatePushCData __attribute((alias("xmlValidatePushCData__internal_alias")));
#else
#ifndef xmlValidatePushCData
extern __typeof (xmlValidatePushCData) xmlValidatePushCData__internal_alias __attribute((visibility("hidden")));
#define xmlValidatePushCData xmlValidatePushCData__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED) && defined(LIBXML_REGEXP_ENABLED)
#ifdef bottom_valid
#undef xmlValidatePushElement
extern __typeof (xmlValidatePushElement) xmlValidatePushElement __attribute((alias("xmlValidatePushElement__internal_alias")));
#else
#ifndef xmlValidatePushElement
extern __typeof (xmlValidatePushElement) xmlValidatePushElement__internal_alias __attribute((visibility("hidden")));
#define xmlValidatePushElement xmlValidatePushElement__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_TREE_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_tree
#undef xmlValidateQName
extern __typeof (xmlValidateQName) xmlValidateQName __attribute((alias("xmlValidateQName__internal_alias")));
#else
#ifndef xmlValidateQName
extern __typeof (xmlValidateQName) xmlValidateQName__internal_alias __attribute((visibility("hidden")));
#define xmlValidateQName xmlValidateQName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_VALID_ENABLED)
#ifdef bottom_valid
#undef xmlValidateRoot
extern __typeof (xmlValidateRoot) xmlValidateRoot __attribute((alias("xmlValidateRoot__internal_alias")));
#else
#ifndef xmlValidateRoot
extern __typeof (xmlValidateRoot) xmlValidateRoot__internal_alias __attribute((visibility("hidden")));
#define xmlValidateRoot xmlValidateRoot__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XINCLUDE_ENABLED)
#ifdef bottom_xinclude
#undef xmlXIncludeFreeContext
extern __typeof (xmlXIncludeFreeContext) xmlXIncludeFreeContext __attribute((alias("xmlXIncludeFreeContext__internal_alias")));
#else
#ifndef xmlXIncludeFreeContext
extern __typeof (xmlXIncludeFreeContext) xmlXIncludeFreeContext__internal_alias __attribute((visibility("hidden")));
#define xmlXIncludeFreeContext xmlXIncludeFreeContext__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XINCLUDE_ENABLED)
#ifdef bottom_xinclude
#undef xmlXIncludeNewContext
extern __typeof (xmlXIncludeNewContext) xmlXIncludeNewContext __attribute((alias("xmlXIncludeNewContext__internal_alias")));
#else
#ifndef xmlXIncludeNewContext
extern __typeof (xmlXIncludeNewContext) xmlXIncludeNewContext__internal_alias __attribute((visibility("hidden")));
#define xmlXIncludeNewContext xmlXIncludeNewContext__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XINCLUDE_ENABLED)
#ifdef bottom_xinclude
#undef xmlXIncludeProcess
extern __typeof (xmlXIncludeProcess) xmlXIncludeProcess __attribute((alias("xmlXIncludeProcess__internal_alias")));
#else
#ifndef xmlXIncludeProcess
extern __typeof (xmlXIncludeProcess) xmlXIncludeProcess__internal_alias __attribute((visibility("hidden")));
#define xmlXIncludeProcess xmlXIncludeProcess__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XINCLUDE_ENABLED)
#ifdef bottom_xinclude
#undef xmlXIncludeProcessFlags
extern __typeof (xmlXIncludeProcessFlags) xmlXIncludeProcessFlags __attribute((alias("xmlXIncludeProcessFlags__internal_alias")));
#else
#ifndef xmlXIncludeProcessFlags
extern __typeof (xmlXIncludeProcessFlags) xmlXIncludeProcessFlags__internal_alias __attribute((visibility("hidden")));
#define xmlXIncludeProcessFlags xmlXIncludeProcessFlags__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XINCLUDE_ENABLED)
#ifdef bottom_xinclude
#undef xmlXIncludeProcessNode
extern __typeof (xmlXIncludeProcessNode) xmlXIncludeProcessNode __attribute((alias("xmlXIncludeProcessNode__internal_alias")));
#else
#ifndef xmlXIncludeProcessNode
extern __typeof (xmlXIncludeProcessNode) xmlXIncludeProcessNode__internal_alias __attribute((visibility("hidden")));
#define xmlXIncludeProcessNode xmlXIncludeProcessNode__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XINCLUDE_ENABLED)
#ifdef bottom_xinclude
#undef xmlXIncludeProcessTree
extern __typeof (xmlXIncludeProcessTree) xmlXIncludeProcessTree __attribute((alias("xmlXIncludeProcessTree__internal_alias")));
#else
#ifndef xmlXIncludeProcessTree
extern __typeof (xmlXIncludeProcessTree) xmlXIncludeProcessTree__internal_alias __attribute((visibility("hidden")));
#define xmlXIncludeProcessTree xmlXIncludeProcessTree__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XINCLUDE_ENABLED)
#ifdef bottom_xinclude
#undef xmlXIncludeProcessTreeFlags
extern __typeof (xmlXIncludeProcessTreeFlags) xmlXIncludeProcessTreeFlags __attribute((alias("xmlXIncludeProcessTreeFlags__internal_alias")));
#else
#ifndef xmlXIncludeProcessTreeFlags
extern __typeof (xmlXIncludeProcessTreeFlags) xmlXIncludeProcessTreeFlags__internal_alias __attribute((visibility("hidden")));
#define xmlXIncludeProcessTreeFlags xmlXIncludeProcessTreeFlags__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XINCLUDE_ENABLED)
#ifdef bottom_xinclude
#undef xmlXIncludeSetFlags
extern __typeof (xmlXIncludeSetFlags) xmlXIncludeSetFlags __attribute((alias("xmlXIncludeSetFlags__internal_alias")));
#else
#ifndef xmlXIncludeSetFlags
extern __typeof (xmlXIncludeSetFlags) xmlXIncludeSetFlags__internal_alias __attribute((visibility("hidden")));
#define xmlXIncludeSetFlags xmlXIncludeSetFlags__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathAddValues
extern __typeof (xmlXPathAddValues) xmlXPathAddValues __attribute((alias("xmlXPathAddValues__internal_alias")));
#else
#ifndef xmlXPathAddValues
extern __typeof (xmlXPathAddValues) xmlXPathAddValues__internal_alias __attribute((visibility("hidden")));
#define xmlXPathAddValues xmlXPathAddValues__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathBooleanFunction
extern __typeof (xmlXPathBooleanFunction) xmlXPathBooleanFunction __attribute((alias("xmlXPathBooleanFunction__internal_alias")));
#else
#ifndef xmlXPathBooleanFunction
extern __typeof (xmlXPathBooleanFunction) xmlXPathBooleanFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathBooleanFunction xmlXPathBooleanFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastBooleanToNumber
extern __typeof (xmlXPathCastBooleanToNumber) xmlXPathCastBooleanToNumber __attribute((alias("xmlXPathCastBooleanToNumber__internal_alias")));
#else
#ifndef xmlXPathCastBooleanToNumber
extern __typeof (xmlXPathCastBooleanToNumber) xmlXPathCastBooleanToNumber__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastBooleanToNumber xmlXPathCastBooleanToNumber__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastBooleanToString
extern __typeof (xmlXPathCastBooleanToString) xmlXPathCastBooleanToString __attribute((alias("xmlXPathCastBooleanToString__internal_alias")));
#else
#ifndef xmlXPathCastBooleanToString
extern __typeof (xmlXPathCastBooleanToString) xmlXPathCastBooleanToString__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastBooleanToString xmlXPathCastBooleanToString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastNodeSetToBoolean
extern __typeof (xmlXPathCastNodeSetToBoolean) xmlXPathCastNodeSetToBoolean __attribute((alias("xmlXPathCastNodeSetToBoolean__internal_alias")));
#else
#ifndef xmlXPathCastNodeSetToBoolean
extern __typeof (xmlXPathCastNodeSetToBoolean) xmlXPathCastNodeSetToBoolean__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastNodeSetToBoolean xmlXPathCastNodeSetToBoolean__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastNodeSetToNumber
extern __typeof (xmlXPathCastNodeSetToNumber) xmlXPathCastNodeSetToNumber __attribute((alias("xmlXPathCastNodeSetToNumber__internal_alias")));
#else
#ifndef xmlXPathCastNodeSetToNumber
extern __typeof (xmlXPathCastNodeSetToNumber) xmlXPathCastNodeSetToNumber__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastNodeSetToNumber xmlXPathCastNodeSetToNumber__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastNodeSetToString
extern __typeof (xmlXPathCastNodeSetToString) xmlXPathCastNodeSetToString __attribute((alias("xmlXPathCastNodeSetToString__internal_alias")));
#else
#ifndef xmlXPathCastNodeSetToString
extern __typeof (xmlXPathCastNodeSetToString) xmlXPathCastNodeSetToString__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastNodeSetToString xmlXPathCastNodeSetToString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastNodeToNumber
extern __typeof (xmlXPathCastNodeToNumber) xmlXPathCastNodeToNumber __attribute((alias("xmlXPathCastNodeToNumber__internal_alias")));
#else
#ifndef xmlXPathCastNodeToNumber
extern __typeof (xmlXPathCastNodeToNumber) xmlXPathCastNodeToNumber__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastNodeToNumber xmlXPathCastNodeToNumber__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastNodeToString
extern __typeof (xmlXPathCastNodeToString) xmlXPathCastNodeToString __attribute((alias("xmlXPathCastNodeToString__internal_alias")));
#else
#ifndef xmlXPathCastNodeToString
extern __typeof (xmlXPathCastNodeToString) xmlXPathCastNodeToString__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastNodeToString xmlXPathCastNodeToString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastNumberToBoolean
extern __typeof (xmlXPathCastNumberToBoolean) xmlXPathCastNumberToBoolean __attribute((alias("xmlXPathCastNumberToBoolean__internal_alias")));
#else
#ifndef xmlXPathCastNumberToBoolean
extern __typeof (xmlXPathCastNumberToBoolean) xmlXPathCastNumberToBoolean__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastNumberToBoolean xmlXPathCastNumberToBoolean__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastNumberToString
extern __typeof (xmlXPathCastNumberToString) xmlXPathCastNumberToString __attribute((alias("xmlXPathCastNumberToString__internal_alias")));
#else
#ifndef xmlXPathCastNumberToString
extern __typeof (xmlXPathCastNumberToString) xmlXPathCastNumberToString__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastNumberToString xmlXPathCastNumberToString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastStringToBoolean
extern __typeof (xmlXPathCastStringToBoolean) xmlXPathCastStringToBoolean __attribute((alias("xmlXPathCastStringToBoolean__internal_alias")));
#else
#ifndef xmlXPathCastStringToBoolean
extern __typeof (xmlXPathCastStringToBoolean) xmlXPathCastStringToBoolean__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastStringToBoolean xmlXPathCastStringToBoolean__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastStringToNumber
extern __typeof (xmlXPathCastStringToNumber) xmlXPathCastStringToNumber __attribute((alias("xmlXPathCastStringToNumber__internal_alias")));
#else
#ifndef xmlXPathCastStringToNumber
extern __typeof (xmlXPathCastStringToNumber) xmlXPathCastStringToNumber__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastStringToNumber xmlXPathCastStringToNumber__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastToBoolean
extern __typeof (xmlXPathCastToBoolean) xmlXPathCastToBoolean __attribute((alias("xmlXPathCastToBoolean__internal_alias")));
#else
#ifndef xmlXPathCastToBoolean
extern __typeof (xmlXPathCastToBoolean) xmlXPathCastToBoolean__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastToBoolean xmlXPathCastToBoolean__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastToNumber
extern __typeof (xmlXPathCastToNumber) xmlXPathCastToNumber __attribute((alias("xmlXPathCastToNumber__internal_alias")));
#else
#ifndef xmlXPathCastToNumber
extern __typeof (xmlXPathCastToNumber) xmlXPathCastToNumber__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastToNumber xmlXPathCastToNumber__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCastToString
extern __typeof (xmlXPathCastToString) xmlXPathCastToString __attribute((alias("xmlXPathCastToString__internal_alias")));
#else
#ifndef xmlXPathCastToString
extern __typeof (xmlXPathCastToString) xmlXPathCastToString__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCastToString xmlXPathCastToString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCeilingFunction
extern __typeof (xmlXPathCeilingFunction) xmlXPathCeilingFunction __attribute((alias("xmlXPathCeilingFunction__internal_alias")));
#else
#ifndef xmlXPathCeilingFunction
extern __typeof (xmlXPathCeilingFunction) xmlXPathCeilingFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCeilingFunction xmlXPathCeilingFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCmpNodes
extern __typeof (xmlXPathCmpNodes) xmlXPathCmpNodes __attribute((alias("xmlXPathCmpNodes__internal_alias")));
#else
#ifndef xmlXPathCmpNodes
extern __typeof (xmlXPathCmpNodes) xmlXPathCmpNodes__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCmpNodes xmlXPathCmpNodes__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCompareValues
extern __typeof (xmlXPathCompareValues) xmlXPathCompareValues __attribute((alias("xmlXPathCompareValues__internal_alias")));
#else
#ifndef xmlXPathCompareValues
extern __typeof (xmlXPathCompareValues) xmlXPathCompareValues__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCompareValues xmlXPathCompareValues__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCompile
extern __typeof (xmlXPathCompile) xmlXPathCompile __attribute((alias("xmlXPathCompile__internal_alias")));
#else
#ifndef xmlXPathCompile
extern __typeof (xmlXPathCompile) xmlXPathCompile__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCompile xmlXPathCompile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCompiledEval
extern __typeof (xmlXPathCompiledEval) xmlXPathCompiledEval __attribute((alias("xmlXPathCompiledEval__internal_alias")));
#else
#ifndef xmlXPathCompiledEval
extern __typeof (xmlXPathCompiledEval) xmlXPathCompiledEval__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCompiledEval xmlXPathCompiledEval__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathConcatFunction
extern __typeof (xmlXPathConcatFunction) xmlXPathConcatFunction __attribute((alias("xmlXPathConcatFunction__internal_alias")));
#else
#ifndef xmlXPathConcatFunction
extern __typeof (xmlXPathConcatFunction) xmlXPathConcatFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathConcatFunction xmlXPathConcatFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathContainsFunction
extern __typeof (xmlXPathContainsFunction) xmlXPathContainsFunction __attribute((alias("xmlXPathContainsFunction__internal_alias")));
#else
#ifndef xmlXPathContainsFunction
extern __typeof (xmlXPathContainsFunction) xmlXPathContainsFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathContainsFunction xmlXPathContainsFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathConvertBoolean
extern __typeof (xmlXPathConvertBoolean) xmlXPathConvertBoolean __attribute((alias("xmlXPathConvertBoolean__internal_alias")));
#else
#ifndef xmlXPathConvertBoolean
extern __typeof (xmlXPathConvertBoolean) xmlXPathConvertBoolean__internal_alias __attribute((visibility("hidden")));
#define xmlXPathConvertBoolean xmlXPathConvertBoolean__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathConvertNumber
extern __typeof (xmlXPathConvertNumber) xmlXPathConvertNumber __attribute((alias("xmlXPathConvertNumber__internal_alias")));
#else
#ifndef xmlXPathConvertNumber
extern __typeof (xmlXPathConvertNumber) xmlXPathConvertNumber__internal_alias __attribute((visibility("hidden")));
#define xmlXPathConvertNumber xmlXPathConvertNumber__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathConvertString
extern __typeof (xmlXPathConvertString) xmlXPathConvertString __attribute((alias("xmlXPathConvertString__internal_alias")));
#else
#ifndef xmlXPathConvertString
extern __typeof (xmlXPathConvertString) xmlXPathConvertString__internal_alias __attribute((visibility("hidden")));
#define xmlXPathConvertString xmlXPathConvertString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCountFunction
extern __typeof (xmlXPathCountFunction) xmlXPathCountFunction __attribute((alias("xmlXPathCountFunction__internal_alias")));
#else
#ifndef xmlXPathCountFunction
extern __typeof (xmlXPathCountFunction) xmlXPathCountFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCountFunction xmlXPathCountFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathCtxtCompile
extern __typeof (xmlXPathCtxtCompile) xmlXPathCtxtCompile __attribute((alias("xmlXPathCtxtCompile__internal_alias")));
#else
#ifndef xmlXPathCtxtCompile
extern __typeof (xmlXPathCtxtCompile) xmlXPathCtxtCompile__internal_alias __attribute((visibility("hidden")));
#define xmlXPathCtxtCompile xmlXPathCtxtCompile__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathDebugDumpCompExpr
extern __typeof (xmlXPathDebugDumpCompExpr) xmlXPathDebugDumpCompExpr __attribute((alias("xmlXPathDebugDumpCompExpr__internal_alias")));
#else
#ifndef xmlXPathDebugDumpCompExpr
extern __typeof (xmlXPathDebugDumpCompExpr) xmlXPathDebugDumpCompExpr__internal_alias __attribute((visibility("hidden")));
#define xmlXPathDebugDumpCompExpr xmlXPathDebugDumpCompExpr__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_DEBUG_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathDebugDumpObject
extern __typeof (xmlXPathDebugDumpObject) xmlXPathDebugDumpObject __attribute((alias("xmlXPathDebugDumpObject__internal_alias")));
#else
#ifndef xmlXPathDebugDumpObject
extern __typeof (xmlXPathDebugDumpObject) xmlXPathDebugDumpObject__internal_alias __attribute((visibility("hidden")));
#define xmlXPathDebugDumpObject xmlXPathDebugDumpObject__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathDifference
extern __typeof (xmlXPathDifference) xmlXPathDifference __attribute((alias("xmlXPathDifference__internal_alias")));
#else
#ifndef xmlXPathDifference
extern __typeof (xmlXPathDifference) xmlXPathDifference__internal_alias __attribute((visibility("hidden")));
#define xmlXPathDifference xmlXPathDifference__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathDistinct
extern __typeof (xmlXPathDistinct) xmlXPathDistinct __attribute((alias("xmlXPathDistinct__internal_alias")));
#else
#ifndef xmlXPathDistinct
extern __typeof (xmlXPathDistinct) xmlXPathDistinct__internal_alias __attribute((visibility("hidden")));
#define xmlXPathDistinct xmlXPathDistinct__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathDistinctSorted
extern __typeof (xmlXPathDistinctSorted) xmlXPathDistinctSorted __attribute((alias("xmlXPathDistinctSorted__internal_alias")));
#else
#ifndef xmlXPathDistinctSorted
extern __typeof (xmlXPathDistinctSorted) xmlXPathDistinctSorted__internal_alias __attribute((visibility("hidden")));
#define xmlXPathDistinctSorted xmlXPathDistinctSorted__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathDivValues
extern __typeof (xmlXPathDivValues) xmlXPathDivValues __attribute((alias("xmlXPathDivValues__internal_alias")));
#else
#ifndef xmlXPathDivValues
extern __typeof (xmlXPathDivValues) xmlXPathDivValues__internal_alias __attribute((visibility("hidden")));
#define xmlXPathDivValues xmlXPathDivValues__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathEqualValues
extern __typeof (xmlXPathEqualValues) xmlXPathEqualValues __attribute((alias("xmlXPathEqualValues__internal_alias")));
#else
#ifndef xmlXPathEqualValues
extern __typeof (xmlXPathEqualValues) xmlXPathEqualValues__internal_alias __attribute((visibility("hidden")));
#define xmlXPathEqualValues xmlXPathEqualValues__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathErr
extern __typeof (xmlXPathErr) xmlXPathErr __attribute((alias("xmlXPathErr__internal_alias")));
#else
#ifndef xmlXPathErr
extern __typeof (xmlXPathErr) xmlXPathErr__internal_alias __attribute((visibility("hidden")));
#define xmlXPathErr xmlXPathErr__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathEval
extern __typeof (xmlXPathEval) xmlXPathEval __attribute((alias("xmlXPathEval__internal_alias")));
#else
#ifndef xmlXPathEval
extern __typeof (xmlXPathEval) xmlXPathEval__internal_alias __attribute((visibility("hidden")));
#define xmlXPathEval xmlXPathEval__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathEvalExpr
extern __typeof (xmlXPathEvalExpr) xmlXPathEvalExpr __attribute((alias("xmlXPathEvalExpr__internal_alias")));
#else
#ifndef xmlXPathEvalExpr
extern __typeof (xmlXPathEvalExpr) xmlXPathEvalExpr__internal_alias __attribute((visibility("hidden")));
#define xmlXPathEvalExpr xmlXPathEvalExpr__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathEvalExpression
extern __typeof (xmlXPathEvalExpression) xmlXPathEvalExpression __attribute((alias("xmlXPathEvalExpression__internal_alias")));
#else
#ifndef xmlXPathEvalExpression
extern __typeof (xmlXPathEvalExpression) xmlXPathEvalExpression__internal_alias __attribute((visibility("hidden")));
#define xmlXPathEvalExpression xmlXPathEvalExpression__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathEvalPredicate
extern __typeof (xmlXPathEvalPredicate) xmlXPathEvalPredicate __attribute((alias("xmlXPathEvalPredicate__internal_alias")));
#else
#ifndef xmlXPathEvalPredicate
extern __typeof (xmlXPathEvalPredicate) xmlXPathEvalPredicate__internal_alias __attribute((visibility("hidden")));
#define xmlXPathEvalPredicate xmlXPathEvalPredicate__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathEvaluatePredicateResult
extern __typeof (xmlXPathEvaluatePredicateResult) xmlXPathEvaluatePredicateResult __attribute((alias("xmlXPathEvaluatePredicateResult__internal_alias")));
#else
#ifndef xmlXPathEvaluatePredicateResult
extern __typeof (xmlXPathEvaluatePredicateResult) xmlXPathEvaluatePredicateResult__internal_alias __attribute((visibility("hidden")));
#define xmlXPathEvaluatePredicateResult xmlXPathEvaluatePredicateResult__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathFalseFunction
extern __typeof (xmlXPathFalseFunction) xmlXPathFalseFunction __attribute((alias("xmlXPathFalseFunction__internal_alias")));
#else
#ifndef xmlXPathFalseFunction
extern __typeof (xmlXPathFalseFunction) xmlXPathFalseFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathFalseFunction xmlXPathFalseFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathFloorFunction
extern __typeof (xmlXPathFloorFunction) xmlXPathFloorFunction __attribute((alias("xmlXPathFloorFunction__internal_alias")));
#else
#ifndef xmlXPathFloorFunction
extern __typeof (xmlXPathFloorFunction) xmlXPathFloorFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathFloorFunction xmlXPathFloorFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathFreeCompExpr
extern __typeof (xmlXPathFreeCompExpr) xmlXPathFreeCompExpr __attribute((alias("xmlXPathFreeCompExpr__internal_alias")));
#else
#ifndef xmlXPathFreeCompExpr
extern __typeof (xmlXPathFreeCompExpr) xmlXPathFreeCompExpr__internal_alias __attribute((visibility("hidden")));
#define xmlXPathFreeCompExpr xmlXPathFreeCompExpr__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathFreeContext
extern __typeof (xmlXPathFreeContext) xmlXPathFreeContext __attribute((alias("xmlXPathFreeContext__internal_alias")));
#else
#ifndef xmlXPathFreeContext
extern __typeof (xmlXPathFreeContext) xmlXPathFreeContext__internal_alias __attribute((visibility("hidden")));
#define xmlXPathFreeContext xmlXPathFreeContext__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathFreeNodeSet
extern __typeof (xmlXPathFreeNodeSet) xmlXPathFreeNodeSet __attribute((alias("xmlXPathFreeNodeSet__internal_alias")));
#else
#ifndef xmlXPathFreeNodeSet
extern __typeof (xmlXPathFreeNodeSet) xmlXPathFreeNodeSet__internal_alias __attribute((visibility("hidden")));
#define xmlXPathFreeNodeSet xmlXPathFreeNodeSet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathFreeNodeSetList
extern __typeof (xmlXPathFreeNodeSetList) xmlXPathFreeNodeSetList __attribute((alias("xmlXPathFreeNodeSetList__internal_alias")));
#else
#ifndef xmlXPathFreeNodeSetList
extern __typeof (xmlXPathFreeNodeSetList) xmlXPathFreeNodeSetList__internal_alias __attribute((visibility("hidden")));
#define xmlXPathFreeNodeSetList xmlXPathFreeNodeSetList__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathFreeObject
extern __typeof (xmlXPathFreeObject) xmlXPathFreeObject __attribute((alias("xmlXPathFreeObject__internal_alias")));
#else
#ifndef xmlXPathFreeObject
extern __typeof (xmlXPathFreeObject) xmlXPathFreeObject__internal_alias __attribute((visibility("hidden")));
#define xmlXPathFreeObject xmlXPathFreeObject__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathFreeParserContext
extern __typeof (xmlXPathFreeParserContext) xmlXPathFreeParserContext __attribute((alias("xmlXPathFreeParserContext__internal_alias")));
#else
#ifndef xmlXPathFreeParserContext
extern __typeof (xmlXPathFreeParserContext) xmlXPathFreeParserContext__internal_alias __attribute((visibility("hidden")));
#define xmlXPathFreeParserContext xmlXPathFreeParserContext__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathFunctionLookup
extern __typeof (xmlXPathFunctionLookup) xmlXPathFunctionLookup __attribute((alias("xmlXPathFunctionLookup__internal_alias")));
#else
#ifndef xmlXPathFunctionLookup
extern __typeof (xmlXPathFunctionLookup) xmlXPathFunctionLookup__internal_alias __attribute((visibility("hidden")));
#define xmlXPathFunctionLookup xmlXPathFunctionLookup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathFunctionLookupNS
extern __typeof (xmlXPathFunctionLookupNS) xmlXPathFunctionLookupNS __attribute((alias("xmlXPathFunctionLookupNS__internal_alias")));
#else
#ifndef xmlXPathFunctionLookupNS
extern __typeof (xmlXPathFunctionLookupNS) xmlXPathFunctionLookupNS__internal_alias __attribute((visibility("hidden")));
#define xmlXPathFunctionLookupNS xmlXPathFunctionLookupNS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathHasSameNodes
extern __typeof (xmlXPathHasSameNodes) xmlXPathHasSameNodes __attribute((alias("xmlXPathHasSameNodes__internal_alias")));
#else
#ifndef xmlXPathHasSameNodes
extern __typeof (xmlXPathHasSameNodes) xmlXPathHasSameNodes__internal_alias __attribute((visibility("hidden")));
#define xmlXPathHasSameNodes xmlXPathHasSameNodes__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathIdFunction
extern __typeof (xmlXPathIdFunction) xmlXPathIdFunction __attribute((alias("xmlXPathIdFunction__internal_alias")));
#else
#ifndef xmlXPathIdFunction
extern __typeof (xmlXPathIdFunction) xmlXPathIdFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathIdFunction xmlXPathIdFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathInit
extern __typeof (xmlXPathInit) xmlXPathInit __attribute((alias("xmlXPathInit__internal_alias")));
#else
#ifndef xmlXPathInit
extern __typeof (xmlXPathInit) xmlXPathInit__internal_alias __attribute((visibility("hidden")));
#define xmlXPathInit xmlXPathInit__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathIntersection
extern __typeof (xmlXPathIntersection) xmlXPathIntersection __attribute((alias("xmlXPathIntersection__internal_alias")));
#else
#ifndef xmlXPathIntersection
extern __typeof (xmlXPathIntersection) xmlXPathIntersection__internal_alias __attribute((visibility("hidden")));
#define xmlXPathIntersection xmlXPathIntersection__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathIsInf
extern __typeof (xmlXPathIsInf) xmlXPathIsInf __attribute((alias("xmlXPathIsInf__internal_alias")));
#else
#ifndef xmlXPathIsInf
extern __typeof (xmlXPathIsInf) xmlXPathIsInf__internal_alias __attribute((visibility("hidden")));
#define xmlXPathIsInf xmlXPathIsInf__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED) || defined(LIBXML_SCHEMAS_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathIsNaN
extern __typeof (xmlXPathIsNaN) xmlXPathIsNaN __attribute((alias("xmlXPathIsNaN__internal_alias")));
#else
#ifndef xmlXPathIsNaN
extern __typeof (xmlXPathIsNaN) xmlXPathIsNaN__internal_alias __attribute((visibility("hidden")));
#define xmlXPathIsNaN xmlXPathIsNaN__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathIsNodeType
extern __typeof (xmlXPathIsNodeType) xmlXPathIsNodeType __attribute((alias("xmlXPathIsNodeType__internal_alias")));
#else
#ifndef xmlXPathIsNodeType
extern __typeof (xmlXPathIsNodeType) xmlXPathIsNodeType__internal_alias __attribute((visibility("hidden")));
#define xmlXPathIsNodeType xmlXPathIsNodeType__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathLangFunction
extern __typeof (xmlXPathLangFunction) xmlXPathLangFunction __attribute((alias("xmlXPathLangFunction__internal_alias")));
#else
#ifndef xmlXPathLangFunction
extern __typeof (xmlXPathLangFunction) xmlXPathLangFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathLangFunction xmlXPathLangFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathLastFunction
extern __typeof (xmlXPathLastFunction) xmlXPathLastFunction __attribute((alias("xmlXPathLastFunction__internal_alias")));
#else
#ifndef xmlXPathLastFunction
extern __typeof (xmlXPathLastFunction) xmlXPathLastFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathLastFunction xmlXPathLastFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathLeading
extern __typeof (xmlXPathLeading) xmlXPathLeading __attribute((alias("xmlXPathLeading__internal_alias")));
#else
#ifndef xmlXPathLeading
extern __typeof (xmlXPathLeading) xmlXPathLeading__internal_alias __attribute((visibility("hidden")));
#define xmlXPathLeading xmlXPathLeading__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathLeadingSorted
extern __typeof (xmlXPathLeadingSorted) xmlXPathLeadingSorted __attribute((alias("xmlXPathLeadingSorted__internal_alias")));
#else
#ifndef xmlXPathLeadingSorted
extern __typeof (xmlXPathLeadingSorted) xmlXPathLeadingSorted__internal_alias __attribute((visibility("hidden")));
#define xmlXPathLeadingSorted xmlXPathLeadingSorted__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathLocalNameFunction
extern __typeof (xmlXPathLocalNameFunction) xmlXPathLocalNameFunction __attribute((alias("xmlXPathLocalNameFunction__internal_alias")));
#else
#ifndef xmlXPathLocalNameFunction
extern __typeof (xmlXPathLocalNameFunction) xmlXPathLocalNameFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathLocalNameFunction xmlXPathLocalNameFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathModValues
extern __typeof (xmlXPathModValues) xmlXPathModValues __attribute((alias("xmlXPathModValues__internal_alias")));
#else
#ifndef xmlXPathModValues
extern __typeof (xmlXPathModValues) xmlXPathModValues__internal_alias __attribute((visibility("hidden")));
#define xmlXPathModValues xmlXPathModValues__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathMultValues
extern __typeof (xmlXPathMultValues) xmlXPathMultValues __attribute((alias("xmlXPathMultValues__internal_alias")));
#else
#ifndef xmlXPathMultValues
extern __typeof (xmlXPathMultValues) xmlXPathMultValues__internal_alias __attribute((visibility("hidden")));
#define xmlXPathMultValues xmlXPathMultValues__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNamespaceURIFunction
extern __typeof (xmlXPathNamespaceURIFunction) xmlXPathNamespaceURIFunction __attribute((alias("xmlXPathNamespaceURIFunction__internal_alias")));
#else
#ifndef xmlXPathNamespaceURIFunction
extern __typeof (xmlXPathNamespaceURIFunction) xmlXPathNamespaceURIFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNamespaceURIFunction xmlXPathNamespaceURIFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNewBoolean
extern __typeof (xmlXPathNewBoolean) xmlXPathNewBoolean __attribute((alias("xmlXPathNewBoolean__internal_alias")));
#else
#ifndef xmlXPathNewBoolean
extern __typeof (xmlXPathNewBoolean) xmlXPathNewBoolean__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNewBoolean xmlXPathNewBoolean__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNewCString
extern __typeof (xmlXPathNewCString) xmlXPathNewCString __attribute((alias("xmlXPathNewCString__internal_alias")));
#else
#ifndef xmlXPathNewCString
extern __typeof (xmlXPathNewCString) xmlXPathNewCString__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNewCString xmlXPathNewCString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNewContext
extern __typeof (xmlXPathNewContext) xmlXPathNewContext __attribute((alias("xmlXPathNewContext__internal_alias")));
#else
#ifndef xmlXPathNewContext
extern __typeof (xmlXPathNewContext) xmlXPathNewContext__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNewContext xmlXPathNewContext__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNewFloat
extern __typeof (xmlXPathNewFloat) xmlXPathNewFloat __attribute((alias("xmlXPathNewFloat__internal_alias")));
#else
#ifndef xmlXPathNewFloat
extern __typeof (xmlXPathNewFloat) xmlXPathNewFloat__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNewFloat xmlXPathNewFloat__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNewNodeSet
extern __typeof (xmlXPathNewNodeSet) xmlXPathNewNodeSet __attribute((alias("xmlXPathNewNodeSet__internal_alias")));
#else
#ifndef xmlXPathNewNodeSet
extern __typeof (xmlXPathNewNodeSet) xmlXPathNewNodeSet__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNewNodeSet xmlXPathNewNodeSet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNewNodeSetList
extern __typeof (xmlXPathNewNodeSetList) xmlXPathNewNodeSetList __attribute((alias("xmlXPathNewNodeSetList__internal_alias")));
#else
#ifndef xmlXPathNewNodeSetList
extern __typeof (xmlXPathNewNodeSetList) xmlXPathNewNodeSetList__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNewNodeSetList xmlXPathNewNodeSetList__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNewParserContext
extern __typeof (xmlXPathNewParserContext) xmlXPathNewParserContext __attribute((alias("xmlXPathNewParserContext__internal_alias")));
#else
#ifndef xmlXPathNewParserContext
extern __typeof (xmlXPathNewParserContext) xmlXPathNewParserContext__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNewParserContext xmlXPathNewParserContext__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNewString
extern __typeof (xmlXPathNewString) xmlXPathNewString __attribute((alias("xmlXPathNewString__internal_alias")));
#else
#ifndef xmlXPathNewString
extern __typeof (xmlXPathNewString) xmlXPathNewString__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNewString xmlXPathNewString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNewValueTree
extern __typeof (xmlXPathNewValueTree) xmlXPathNewValueTree __attribute((alias("xmlXPathNewValueTree__internal_alias")));
#else
#ifndef xmlXPathNewValueTree
extern __typeof (xmlXPathNewValueTree) xmlXPathNewValueTree__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNewValueTree xmlXPathNewValueTree__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextAncestor
extern __typeof (xmlXPathNextAncestor) xmlXPathNextAncestor __attribute((alias("xmlXPathNextAncestor__internal_alias")));
#else
#ifndef xmlXPathNextAncestor
extern __typeof (xmlXPathNextAncestor) xmlXPathNextAncestor__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextAncestor xmlXPathNextAncestor__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextAncestorOrSelf
extern __typeof (xmlXPathNextAncestorOrSelf) xmlXPathNextAncestorOrSelf __attribute((alias("xmlXPathNextAncestorOrSelf__internal_alias")));
#else
#ifndef xmlXPathNextAncestorOrSelf
extern __typeof (xmlXPathNextAncestorOrSelf) xmlXPathNextAncestorOrSelf__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextAncestorOrSelf xmlXPathNextAncestorOrSelf__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextAttribute
extern __typeof (xmlXPathNextAttribute) xmlXPathNextAttribute __attribute((alias("xmlXPathNextAttribute__internal_alias")));
#else
#ifndef xmlXPathNextAttribute
extern __typeof (xmlXPathNextAttribute) xmlXPathNextAttribute__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextAttribute xmlXPathNextAttribute__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextChild
extern __typeof (xmlXPathNextChild) xmlXPathNextChild __attribute((alias("xmlXPathNextChild__internal_alias")));
#else
#ifndef xmlXPathNextChild
extern __typeof (xmlXPathNextChild) xmlXPathNextChild__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextChild xmlXPathNextChild__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextDescendant
extern __typeof (xmlXPathNextDescendant) xmlXPathNextDescendant __attribute((alias("xmlXPathNextDescendant__internal_alias")));
#else
#ifndef xmlXPathNextDescendant
extern __typeof (xmlXPathNextDescendant) xmlXPathNextDescendant__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextDescendant xmlXPathNextDescendant__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextDescendantOrSelf
extern __typeof (xmlXPathNextDescendantOrSelf) xmlXPathNextDescendantOrSelf __attribute((alias("xmlXPathNextDescendantOrSelf__internal_alias")));
#else
#ifndef xmlXPathNextDescendantOrSelf
extern __typeof (xmlXPathNextDescendantOrSelf) xmlXPathNextDescendantOrSelf__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextDescendantOrSelf xmlXPathNextDescendantOrSelf__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextFollowing
extern __typeof (xmlXPathNextFollowing) xmlXPathNextFollowing __attribute((alias("xmlXPathNextFollowing__internal_alias")));
#else
#ifndef xmlXPathNextFollowing
extern __typeof (xmlXPathNextFollowing) xmlXPathNextFollowing__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextFollowing xmlXPathNextFollowing__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextFollowingSibling
extern __typeof (xmlXPathNextFollowingSibling) xmlXPathNextFollowingSibling __attribute((alias("xmlXPathNextFollowingSibling__internal_alias")));
#else
#ifndef xmlXPathNextFollowingSibling
extern __typeof (xmlXPathNextFollowingSibling) xmlXPathNextFollowingSibling__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextFollowingSibling xmlXPathNextFollowingSibling__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextNamespace
extern __typeof (xmlXPathNextNamespace) xmlXPathNextNamespace __attribute((alias("xmlXPathNextNamespace__internal_alias")));
#else
#ifndef xmlXPathNextNamespace
extern __typeof (xmlXPathNextNamespace) xmlXPathNextNamespace__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextNamespace xmlXPathNextNamespace__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextParent
extern __typeof (xmlXPathNextParent) xmlXPathNextParent __attribute((alias("xmlXPathNextParent__internal_alias")));
#else
#ifndef xmlXPathNextParent
extern __typeof (xmlXPathNextParent) xmlXPathNextParent__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextParent xmlXPathNextParent__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextPreceding
extern __typeof (xmlXPathNextPreceding) xmlXPathNextPreceding __attribute((alias("xmlXPathNextPreceding__internal_alias")));
#else
#ifndef xmlXPathNextPreceding
extern __typeof (xmlXPathNextPreceding) xmlXPathNextPreceding__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextPreceding xmlXPathNextPreceding__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextPrecedingSibling
extern __typeof (xmlXPathNextPrecedingSibling) xmlXPathNextPrecedingSibling __attribute((alias("xmlXPathNextPrecedingSibling__internal_alias")));
#else
#ifndef xmlXPathNextPrecedingSibling
extern __typeof (xmlXPathNextPrecedingSibling) xmlXPathNextPrecedingSibling__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextPrecedingSibling xmlXPathNextPrecedingSibling__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNextSelf
extern __typeof (xmlXPathNextSelf) xmlXPathNextSelf __attribute((alias("xmlXPathNextSelf__internal_alias")));
#else
#ifndef xmlXPathNextSelf
extern __typeof (xmlXPathNextSelf) xmlXPathNextSelf__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNextSelf xmlXPathNextSelf__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeLeading
extern __typeof (xmlXPathNodeLeading) xmlXPathNodeLeading __attribute((alias("xmlXPathNodeLeading__internal_alias")));
#else
#ifndef xmlXPathNodeLeading
extern __typeof (xmlXPathNodeLeading) xmlXPathNodeLeading__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeLeading xmlXPathNodeLeading__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeLeadingSorted
extern __typeof (xmlXPathNodeLeadingSorted) xmlXPathNodeLeadingSorted __attribute((alias("xmlXPathNodeLeadingSorted__internal_alias")));
#else
#ifndef xmlXPathNodeLeadingSorted
extern __typeof (xmlXPathNodeLeadingSorted) xmlXPathNodeLeadingSorted__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeLeadingSorted xmlXPathNodeLeadingSorted__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeSetAdd
extern __typeof (xmlXPathNodeSetAdd) xmlXPathNodeSetAdd __attribute((alias("xmlXPathNodeSetAdd__internal_alias")));
#else
#ifndef xmlXPathNodeSetAdd
extern __typeof (xmlXPathNodeSetAdd) xmlXPathNodeSetAdd__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeSetAdd xmlXPathNodeSetAdd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeSetAddNs
extern __typeof (xmlXPathNodeSetAddNs) xmlXPathNodeSetAddNs __attribute((alias("xmlXPathNodeSetAddNs__internal_alias")));
#else
#ifndef xmlXPathNodeSetAddNs
extern __typeof (xmlXPathNodeSetAddNs) xmlXPathNodeSetAddNs__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeSetAddNs xmlXPathNodeSetAddNs__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeSetAddUnique
extern __typeof (xmlXPathNodeSetAddUnique) xmlXPathNodeSetAddUnique __attribute((alias("xmlXPathNodeSetAddUnique__internal_alias")));
#else
#ifndef xmlXPathNodeSetAddUnique
extern __typeof (xmlXPathNodeSetAddUnique) xmlXPathNodeSetAddUnique__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeSetAddUnique xmlXPathNodeSetAddUnique__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeSetContains
extern __typeof (xmlXPathNodeSetContains) xmlXPathNodeSetContains __attribute((alias("xmlXPathNodeSetContains__internal_alias")));
#else
#ifndef xmlXPathNodeSetContains
extern __typeof (xmlXPathNodeSetContains) xmlXPathNodeSetContains__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeSetContains xmlXPathNodeSetContains__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeSetCreate
extern __typeof (xmlXPathNodeSetCreate) xmlXPathNodeSetCreate __attribute((alias("xmlXPathNodeSetCreate__internal_alias")));
#else
#ifndef xmlXPathNodeSetCreate
extern __typeof (xmlXPathNodeSetCreate) xmlXPathNodeSetCreate__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeSetCreate xmlXPathNodeSetCreate__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeSetDel
extern __typeof (xmlXPathNodeSetDel) xmlXPathNodeSetDel __attribute((alias("xmlXPathNodeSetDel__internal_alias")));
#else
#ifndef xmlXPathNodeSetDel
extern __typeof (xmlXPathNodeSetDel) xmlXPathNodeSetDel__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeSetDel xmlXPathNodeSetDel__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeSetFreeNs
extern __typeof (xmlXPathNodeSetFreeNs) xmlXPathNodeSetFreeNs __attribute((alias("xmlXPathNodeSetFreeNs__internal_alias")));
#else
#ifndef xmlXPathNodeSetFreeNs
extern __typeof (xmlXPathNodeSetFreeNs) xmlXPathNodeSetFreeNs__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeSetFreeNs xmlXPathNodeSetFreeNs__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeSetMerge
extern __typeof (xmlXPathNodeSetMerge) xmlXPathNodeSetMerge __attribute((alias("xmlXPathNodeSetMerge__internal_alias")));
#else
#ifndef xmlXPathNodeSetMerge
extern __typeof (xmlXPathNodeSetMerge) xmlXPathNodeSetMerge__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeSetMerge xmlXPathNodeSetMerge__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeSetRemove
extern __typeof (xmlXPathNodeSetRemove) xmlXPathNodeSetRemove __attribute((alias("xmlXPathNodeSetRemove__internal_alias")));
#else
#ifndef xmlXPathNodeSetRemove
extern __typeof (xmlXPathNodeSetRemove) xmlXPathNodeSetRemove__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeSetRemove xmlXPathNodeSetRemove__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeSetSort
extern __typeof (xmlXPathNodeSetSort) xmlXPathNodeSetSort __attribute((alias("xmlXPathNodeSetSort__internal_alias")));
#else
#ifndef xmlXPathNodeSetSort
extern __typeof (xmlXPathNodeSetSort) xmlXPathNodeSetSort__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeSetSort xmlXPathNodeSetSort__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeTrailing
extern __typeof (xmlXPathNodeTrailing) xmlXPathNodeTrailing __attribute((alias("xmlXPathNodeTrailing__internal_alias")));
#else
#ifndef xmlXPathNodeTrailing
extern __typeof (xmlXPathNodeTrailing) xmlXPathNodeTrailing__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeTrailing xmlXPathNodeTrailing__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNodeTrailingSorted
extern __typeof (xmlXPathNodeTrailingSorted) xmlXPathNodeTrailingSorted __attribute((alias("xmlXPathNodeTrailingSorted__internal_alias")));
#else
#ifndef xmlXPathNodeTrailingSorted
extern __typeof (xmlXPathNodeTrailingSorted) xmlXPathNodeTrailingSorted__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNodeTrailingSorted xmlXPathNodeTrailingSorted__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNormalizeFunction
extern __typeof (xmlXPathNormalizeFunction) xmlXPathNormalizeFunction __attribute((alias("xmlXPathNormalizeFunction__internal_alias")));
#else
#ifndef xmlXPathNormalizeFunction
extern __typeof (xmlXPathNormalizeFunction) xmlXPathNormalizeFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNormalizeFunction xmlXPathNormalizeFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNotEqualValues
extern __typeof (xmlXPathNotEqualValues) xmlXPathNotEqualValues __attribute((alias("xmlXPathNotEqualValues__internal_alias")));
#else
#ifndef xmlXPathNotEqualValues
extern __typeof (xmlXPathNotEqualValues) xmlXPathNotEqualValues__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNotEqualValues xmlXPathNotEqualValues__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNotFunction
extern __typeof (xmlXPathNotFunction) xmlXPathNotFunction __attribute((alias("xmlXPathNotFunction__internal_alias")));
#else
#ifndef xmlXPathNotFunction
extern __typeof (xmlXPathNotFunction) xmlXPathNotFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNotFunction xmlXPathNotFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNsLookup
extern __typeof (xmlXPathNsLookup) xmlXPathNsLookup __attribute((alias("xmlXPathNsLookup__internal_alias")));
#else
#ifndef xmlXPathNsLookup
extern __typeof (xmlXPathNsLookup) xmlXPathNsLookup__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNsLookup xmlXPathNsLookup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathNumberFunction
extern __typeof (xmlXPathNumberFunction) xmlXPathNumberFunction __attribute((alias("xmlXPathNumberFunction__internal_alias")));
#else
#ifndef xmlXPathNumberFunction
extern __typeof (xmlXPathNumberFunction) xmlXPathNumberFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathNumberFunction xmlXPathNumberFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathObjectCopy
extern __typeof (xmlXPathObjectCopy) xmlXPathObjectCopy __attribute((alias("xmlXPathObjectCopy__internal_alias")));
#else
#ifndef xmlXPathObjectCopy
extern __typeof (xmlXPathObjectCopy) xmlXPathObjectCopy__internal_alias __attribute((visibility("hidden")));
#define xmlXPathObjectCopy xmlXPathObjectCopy__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathOrderDocElems
extern __typeof (xmlXPathOrderDocElems) xmlXPathOrderDocElems __attribute((alias("xmlXPathOrderDocElems__internal_alias")));
#else
#ifndef xmlXPathOrderDocElems
extern __typeof (xmlXPathOrderDocElems) xmlXPathOrderDocElems__internal_alias __attribute((visibility("hidden")));
#define xmlXPathOrderDocElems xmlXPathOrderDocElems__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathParseNCName
extern __typeof (xmlXPathParseNCName) xmlXPathParseNCName __attribute((alias("xmlXPathParseNCName__internal_alias")));
#else
#ifndef xmlXPathParseNCName
extern __typeof (xmlXPathParseNCName) xmlXPathParseNCName__internal_alias __attribute((visibility("hidden")));
#define xmlXPathParseNCName xmlXPathParseNCName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathParseName
extern __typeof (xmlXPathParseName) xmlXPathParseName __attribute((alias("xmlXPathParseName__internal_alias")));
#else
#ifndef xmlXPathParseName
extern __typeof (xmlXPathParseName) xmlXPathParseName__internal_alias __attribute((visibility("hidden")));
#define xmlXPathParseName xmlXPathParseName__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathPopBoolean
extern __typeof (xmlXPathPopBoolean) xmlXPathPopBoolean __attribute((alias("xmlXPathPopBoolean__internal_alias")));
#else
#ifndef xmlXPathPopBoolean
extern __typeof (xmlXPathPopBoolean) xmlXPathPopBoolean__internal_alias __attribute((visibility("hidden")));
#define xmlXPathPopBoolean xmlXPathPopBoolean__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathPopExternal
extern __typeof (xmlXPathPopExternal) xmlXPathPopExternal __attribute((alias("xmlXPathPopExternal__internal_alias")));
#else
#ifndef xmlXPathPopExternal
extern __typeof (xmlXPathPopExternal) xmlXPathPopExternal__internal_alias __attribute((visibility("hidden")));
#define xmlXPathPopExternal xmlXPathPopExternal__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathPopNodeSet
extern __typeof (xmlXPathPopNodeSet) xmlXPathPopNodeSet __attribute((alias("xmlXPathPopNodeSet__internal_alias")));
#else
#ifndef xmlXPathPopNodeSet
extern __typeof (xmlXPathPopNodeSet) xmlXPathPopNodeSet__internal_alias __attribute((visibility("hidden")));
#define xmlXPathPopNodeSet xmlXPathPopNodeSet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathPopNumber
extern __typeof (xmlXPathPopNumber) xmlXPathPopNumber __attribute((alias("xmlXPathPopNumber__internal_alias")));
#else
#ifndef xmlXPathPopNumber
extern __typeof (xmlXPathPopNumber) xmlXPathPopNumber__internal_alias __attribute((visibility("hidden")));
#define xmlXPathPopNumber xmlXPathPopNumber__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathPopString
extern __typeof (xmlXPathPopString) xmlXPathPopString __attribute((alias("xmlXPathPopString__internal_alias")));
#else
#ifndef xmlXPathPopString
extern __typeof (xmlXPathPopString) xmlXPathPopString__internal_alias __attribute((visibility("hidden")));
#define xmlXPathPopString xmlXPathPopString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathPositionFunction
extern __typeof (xmlXPathPositionFunction) xmlXPathPositionFunction __attribute((alias("xmlXPathPositionFunction__internal_alias")));
#else
#ifndef xmlXPathPositionFunction
extern __typeof (xmlXPathPositionFunction) xmlXPathPositionFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathPositionFunction xmlXPathPositionFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRegisterAllFunctions
extern __typeof (xmlXPathRegisterAllFunctions) xmlXPathRegisterAllFunctions __attribute((alias("xmlXPathRegisterAllFunctions__internal_alias")));
#else
#ifndef xmlXPathRegisterAllFunctions
extern __typeof (xmlXPathRegisterAllFunctions) xmlXPathRegisterAllFunctions__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRegisterAllFunctions xmlXPathRegisterAllFunctions__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRegisterFunc
extern __typeof (xmlXPathRegisterFunc) xmlXPathRegisterFunc __attribute((alias("xmlXPathRegisterFunc__internal_alias")));
#else
#ifndef xmlXPathRegisterFunc
extern __typeof (xmlXPathRegisterFunc) xmlXPathRegisterFunc__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRegisterFunc xmlXPathRegisterFunc__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRegisterFuncLookup
extern __typeof (xmlXPathRegisterFuncLookup) xmlXPathRegisterFuncLookup __attribute((alias("xmlXPathRegisterFuncLookup__internal_alias")));
#else
#ifndef xmlXPathRegisterFuncLookup
extern __typeof (xmlXPathRegisterFuncLookup) xmlXPathRegisterFuncLookup__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRegisterFuncLookup xmlXPathRegisterFuncLookup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRegisterFuncNS
extern __typeof (xmlXPathRegisterFuncNS) xmlXPathRegisterFuncNS __attribute((alias("xmlXPathRegisterFuncNS__internal_alias")));
#else
#ifndef xmlXPathRegisterFuncNS
extern __typeof (xmlXPathRegisterFuncNS) xmlXPathRegisterFuncNS__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRegisterFuncNS xmlXPathRegisterFuncNS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRegisterNs
extern __typeof (xmlXPathRegisterNs) xmlXPathRegisterNs __attribute((alias("xmlXPathRegisterNs__internal_alias")));
#else
#ifndef xmlXPathRegisterNs
extern __typeof (xmlXPathRegisterNs) xmlXPathRegisterNs__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRegisterNs xmlXPathRegisterNs__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRegisterVariable
extern __typeof (xmlXPathRegisterVariable) xmlXPathRegisterVariable __attribute((alias("xmlXPathRegisterVariable__internal_alias")));
#else
#ifndef xmlXPathRegisterVariable
extern __typeof (xmlXPathRegisterVariable) xmlXPathRegisterVariable__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRegisterVariable xmlXPathRegisterVariable__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRegisterVariableLookup
extern __typeof (xmlXPathRegisterVariableLookup) xmlXPathRegisterVariableLookup __attribute((alias("xmlXPathRegisterVariableLookup__internal_alias")));
#else
#ifndef xmlXPathRegisterVariableLookup
extern __typeof (xmlXPathRegisterVariableLookup) xmlXPathRegisterVariableLookup__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRegisterVariableLookup xmlXPathRegisterVariableLookup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRegisterVariableNS
extern __typeof (xmlXPathRegisterVariableNS) xmlXPathRegisterVariableNS __attribute((alias("xmlXPathRegisterVariableNS__internal_alias")));
#else
#ifndef xmlXPathRegisterVariableNS
extern __typeof (xmlXPathRegisterVariableNS) xmlXPathRegisterVariableNS__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRegisterVariableNS xmlXPathRegisterVariableNS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRegisteredFuncsCleanup
extern __typeof (xmlXPathRegisteredFuncsCleanup) xmlXPathRegisteredFuncsCleanup __attribute((alias("xmlXPathRegisteredFuncsCleanup__internal_alias")));
#else
#ifndef xmlXPathRegisteredFuncsCleanup
extern __typeof (xmlXPathRegisteredFuncsCleanup) xmlXPathRegisteredFuncsCleanup__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRegisteredFuncsCleanup xmlXPathRegisteredFuncsCleanup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRegisteredNsCleanup
extern __typeof (xmlXPathRegisteredNsCleanup) xmlXPathRegisteredNsCleanup __attribute((alias("xmlXPathRegisteredNsCleanup__internal_alias")));
#else
#ifndef xmlXPathRegisteredNsCleanup
extern __typeof (xmlXPathRegisteredNsCleanup) xmlXPathRegisteredNsCleanup__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRegisteredNsCleanup xmlXPathRegisteredNsCleanup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRegisteredVariablesCleanup
extern __typeof (xmlXPathRegisteredVariablesCleanup) xmlXPathRegisteredVariablesCleanup __attribute((alias("xmlXPathRegisteredVariablesCleanup__internal_alias")));
#else
#ifndef xmlXPathRegisteredVariablesCleanup
extern __typeof (xmlXPathRegisteredVariablesCleanup) xmlXPathRegisteredVariablesCleanup__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRegisteredVariablesCleanup xmlXPathRegisteredVariablesCleanup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRoot
extern __typeof (xmlXPathRoot) xmlXPathRoot __attribute((alias("xmlXPathRoot__internal_alias")));
#else
#ifndef xmlXPathRoot
extern __typeof (xmlXPathRoot) xmlXPathRoot__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRoot xmlXPathRoot__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathRoundFunction
extern __typeof (xmlXPathRoundFunction) xmlXPathRoundFunction __attribute((alias("xmlXPathRoundFunction__internal_alias")));
#else
#ifndef xmlXPathRoundFunction
extern __typeof (xmlXPathRoundFunction) xmlXPathRoundFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathRoundFunction xmlXPathRoundFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathStartsWithFunction
extern __typeof (xmlXPathStartsWithFunction) xmlXPathStartsWithFunction __attribute((alias("xmlXPathStartsWithFunction__internal_alias")));
#else
#ifndef xmlXPathStartsWithFunction
extern __typeof (xmlXPathStartsWithFunction) xmlXPathStartsWithFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathStartsWithFunction xmlXPathStartsWithFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathStringEvalNumber
extern __typeof (xmlXPathStringEvalNumber) xmlXPathStringEvalNumber __attribute((alias("xmlXPathStringEvalNumber__internal_alias")));
#else
#ifndef xmlXPathStringEvalNumber
extern __typeof (xmlXPathStringEvalNumber) xmlXPathStringEvalNumber__internal_alias __attribute((visibility("hidden")));
#define xmlXPathStringEvalNumber xmlXPathStringEvalNumber__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathStringFunction
extern __typeof (xmlXPathStringFunction) xmlXPathStringFunction __attribute((alias("xmlXPathStringFunction__internal_alias")));
#else
#ifndef xmlXPathStringFunction
extern __typeof (xmlXPathStringFunction) xmlXPathStringFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathStringFunction xmlXPathStringFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathStringLengthFunction
extern __typeof (xmlXPathStringLengthFunction) xmlXPathStringLengthFunction __attribute((alias("xmlXPathStringLengthFunction__internal_alias")));
#else
#ifndef xmlXPathStringLengthFunction
extern __typeof (xmlXPathStringLengthFunction) xmlXPathStringLengthFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathStringLengthFunction xmlXPathStringLengthFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathSubValues
extern __typeof (xmlXPathSubValues) xmlXPathSubValues __attribute((alias("xmlXPathSubValues__internal_alias")));
#else
#ifndef xmlXPathSubValues
extern __typeof (xmlXPathSubValues) xmlXPathSubValues__internal_alias __attribute((visibility("hidden")));
#define xmlXPathSubValues xmlXPathSubValues__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathSubstringAfterFunction
extern __typeof (xmlXPathSubstringAfterFunction) xmlXPathSubstringAfterFunction __attribute((alias("xmlXPathSubstringAfterFunction__internal_alias")));
#else
#ifndef xmlXPathSubstringAfterFunction
extern __typeof (xmlXPathSubstringAfterFunction) xmlXPathSubstringAfterFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathSubstringAfterFunction xmlXPathSubstringAfterFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathSubstringBeforeFunction
extern __typeof (xmlXPathSubstringBeforeFunction) xmlXPathSubstringBeforeFunction __attribute((alias("xmlXPathSubstringBeforeFunction__internal_alias")));
#else
#ifndef xmlXPathSubstringBeforeFunction
extern __typeof (xmlXPathSubstringBeforeFunction) xmlXPathSubstringBeforeFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathSubstringBeforeFunction xmlXPathSubstringBeforeFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathSubstringFunction
extern __typeof (xmlXPathSubstringFunction) xmlXPathSubstringFunction __attribute((alias("xmlXPathSubstringFunction__internal_alias")));
#else
#ifndef xmlXPathSubstringFunction
extern __typeof (xmlXPathSubstringFunction) xmlXPathSubstringFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathSubstringFunction xmlXPathSubstringFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathSumFunction
extern __typeof (xmlXPathSumFunction) xmlXPathSumFunction __attribute((alias("xmlXPathSumFunction__internal_alias")));
#else
#ifndef xmlXPathSumFunction
extern __typeof (xmlXPathSumFunction) xmlXPathSumFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathSumFunction xmlXPathSumFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathTrailing
extern __typeof (xmlXPathTrailing) xmlXPathTrailing __attribute((alias("xmlXPathTrailing__internal_alias")));
#else
#ifndef xmlXPathTrailing
extern __typeof (xmlXPathTrailing) xmlXPathTrailing__internal_alias __attribute((visibility("hidden")));
#define xmlXPathTrailing xmlXPathTrailing__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathTrailingSorted
extern __typeof (xmlXPathTrailingSorted) xmlXPathTrailingSorted __attribute((alias("xmlXPathTrailingSorted__internal_alias")));
#else
#ifndef xmlXPathTrailingSorted
extern __typeof (xmlXPathTrailingSorted) xmlXPathTrailingSorted__internal_alias __attribute((visibility("hidden")));
#define xmlXPathTrailingSorted xmlXPathTrailingSorted__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathTranslateFunction
extern __typeof (xmlXPathTranslateFunction) xmlXPathTranslateFunction __attribute((alias("xmlXPathTranslateFunction__internal_alias")));
#else
#ifndef xmlXPathTranslateFunction
extern __typeof (xmlXPathTranslateFunction) xmlXPathTranslateFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathTranslateFunction xmlXPathTranslateFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathTrueFunction
extern __typeof (xmlXPathTrueFunction) xmlXPathTrueFunction __attribute((alias("xmlXPathTrueFunction__internal_alias")));
#else
#ifndef xmlXPathTrueFunction
extern __typeof (xmlXPathTrueFunction) xmlXPathTrueFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPathTrueFunction xmlXPathTrueFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathValueFlipSign
extern __typeof (xmlXPathValueFlipSign) xmlXPathValueFlipSign __attribute((alias("xmlXPathValueFlipSign__internal_alias")));
#else
#ifndef xmlXPathValueFlipSign
extern __typeof (xmlXPathValueFlipSign) xmlXPathValueFlipSign__internal_alias __attribute((visibility("hidden")));
#define xmlXPathValueFlipSign xmlXPathValueFlipSign__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathVariableLookup
extern __typeof (xmlXPathVariableLookup) xmlXPathVariableLookup __attribute((alias("xmlXPathVariableLookup__internal_alias")));
#else
#ifndef xmlXPathVariableLookup
extern __typeof (xmlXPathVariableLookup) xmlXPathVariableLookup__internal_alias __attribute((visibility("hidden")));
#define xmlXPathVariableLookup xmlXPathVariableLookup__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathVariableLookupNS
extern __typeof (xmlXPathVariableLookupNS) xmlXPathVariableLookupNS __attribute((alias("xmlXPathVariableLookupNS__internal_alias")));
#else
#ifndef xmlXPathVariableLookupNS
extern __typeof (xmlXPathVariableLookupNS) xmlXPathVariableLookupNS__internal_alias __attribute((visibility("hidden")));
#define xmlXPathVariableLookupNS xmlXPathVariableLookupNS__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathWrapCString
extern __typeof (xmlXPathWrapCString) xmlXPathWrapCString __attribute((alias("xmlXPathWrapCString__internal_alias")));
#else
#ifndef xmlXPathWrapCString
extern __typeof (xmlXPathWrapCString) xmlXPathWrapCString__internal_alias __attribute((visibility("hidden")));
#define xmlXPathWrapCString xmlXPathWrapCString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathWrapExternal
extern __typeof (xmlXPathWrapExternal) xmlXPathWrapExternal __attribute((alias("xmlXPathWrapExternal__internal_alias")));
#else
#ifndef xmlXPathWrapExternal
extern __typeof (xmlXPathWrapExternal) xmlXPathWrapExternal__internal_alias __attribute((visibility("hidden")));
#define xmlXPathWrapExternal xmlXPathWrapExternal__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathWrapNodeSet
extern __typeof (xmlXPathWrapNodeSet) xmlXPathWrapNodeSet __attribute((alias("xmlXPathWrapNodeSet__internal_alias")));
#else
#ifndef xmlXPathWrapNodeSet
extern __typeof (xmlXPathWrapNodeSet) xmlXPathWrapNodeSet__internal_alias __attribute((visibility("hidden")));
#define xmlXPathWrapNodeSet xmlXPathWrapNodeSet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPathWrapString
extern __typeof (xmlXPathWrapString) xmlXPathWrapString __attribute((alias("xmlXPathWrapString__internal_alias")));
#else
#ifndef xmlXPathWrapString
extern __typeof (xmlXPathWrapString) xmlXPathWrapString__internal_alias __attribute((visibility("hidden")));
#define xmlXPathWrapString xmlXPathWrapString__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPATH_ENABLED)
#ifdef bottom_xpath
#undef xmlXPatherror
extern __typeof (xmlXPatherror) xmlXPatherror __attribute((alias("xmlXPatherror__internal_alias")));
#else
#ifndef xmlXPatherror
extern __typeof (xmlXPatherror) xmlXPatherror__internal_alias __attribute((visibility("hidden")));
#define xmlXPatherror xmlXPatherror__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrBuildNodeList
extern __typeof (xmlXPtrBuildNodeList) xmlXPtrBuildNodeList __attribute((alias("xmlXPtrBuildNodeList__internal_alias")));
#else
#ifndef xmlXPtrBuildNodeList
extern __typeof (xmlXPtrBuildNodeList) xmlXPtrBuildNodeList__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrBuildNodeList xmlXPtrBuildNodeList__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrEval
extern __typeof (xmlXPtrEval) xmlXPtrEval __attribute((alias("xmlXPtrEval__internal_alias")));
#else
#ifndef xmlXPtrEval
extern __typeof (xmlXPtrEval) xmlXPtrEval__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrEval xmlXPtrEval__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrEvalRangePredicate
extern __typeof (xmlXPtrEvalRangePredicate) xmlXPtrEvalRangePredicate __attribute((alias("xmlXPtrEvalRangePredicate__internal_alias")));
#else
#ifndef xmlXPtrEvalRangePredicate
extern __typeof (xmlXPtrEvalRangePredicate) xmlXPtrEvalRangePredicate__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrEvalRangePredicate xmlXPtrEvalRangePredicate__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrFreeLocationSet
extern __typeof (xmlXPtrFreeLocationSet) xmlXPtrFreeLocationSet __attribute((alias("xmlXPtrFreeLocationSet__internal_alias")));
#else
#ifndef xmlXPtrFreeLocationSet
extern __typeof (xmlXPtrFreeLocationSet) xmlXPtrFreeLocationSet__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrFreeLocationSet xmlXPtrFreeLocationSet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrLocationSetAdd
extern __typeof (xmlXPtrLocationSetAdd) xmlXPtrLocationSetAdd __attribute((alias("xmlXPtrLocationSetAdd__internal_alias")));
#else
#ifndef xmlXPtrLocationSetAdd
extern __typeof (xmlXPtrLocationSetAdd) xmlXPtrLocationSetAdd__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrLocationSetAdd xmlXPtrLocationSetAdd__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrLocationSetCreate
extern __typeof (xmlXPtrLocationSetCreate) xmlXPtrLocationSetCreate __attribute((alias("xmlXPtrLocationSetCreate__internal_alias")));
#else
#ifndef xmlXPtrLocationSetCreate
extern __typeof (xmlXPtrLocationSetCreate) xmlXPtrLocationSetCreate__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrLocationSetCreate xmlXPtrLocationSetCreate__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrLocationSetDel
extern __typeof (xmlXPtrLocationSetDel) xmlXPtrLocationSetDel __attribute((alias("xmlXPtrLocationSetDel__internal_alias")));
#else
#ifndef xmlXPtrLocationSetDel
extern __typeof (xmlXPtrLocationSetDel) xmlXPtrLocationSetDel__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrLocationSetDel xmlXPtrLocationSetDel__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrLocationSetMerge
extern __typeof (xmlXPtrLocationSetMerge) xmlXPtrLocationSetMerge __attribute((alias("xmlXPtrLocationSetMerge__internal_alias")));
#else
#ifndef xmlXPtrLocationSetMerge
extern __typeof (xmlXPtrLocationSetMerge) xmlXPtrLocationSetMerge__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrLocationSetMerge xmlXPtrLocationSetMerge__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrLocationSetRemove
extern __typeof (xmlXPtrLocationSetRemove) xmlXPtrLocationSetRemove __attribute((alias("xmlXPtrLocationSetRemove__internal_alias")));
#else
#ifndef xmlXPtrLocationSetRemove
extern __typeof (xmlXPtrLocationSetRemove) xmlXPtrLocationSetRemove__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrLocationSetRemove xmlXPtrLocationSetRemove__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrNewCollapsedRange
extern __typeof (xmlXPtrNewCollapsedRange) xmlXPtrNewCollapsedRange __attribute((alias("xmlXPtrNewCollapsedRange__internal_alias")));
#else
#ifndef xmlXPtrNewCollapsedRange
extern __typeof (xmlXPtrNewCollapsedRange) xmlXPtrNewCollapsedRange__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrNewCollapsedRange xmlXPtrNewCollapsedRange__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrNewContext
extern __typeof (xmlXPtrNewContext) xmlXPtrNewContext __attribute((alias("xmlXPtrNewContext__internal_alias")));
#else
#ifndef xmlXPtrNewContext
extern __typeof (xmlXPtrNewContext) xmlXPtrNewContext__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrNewContext xmlXPtrNewContext__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrNewLocationSetNodeSet
extern __typeof (xmlXPtrNewLocationSetNodeSet) xmlXPtrNewLocationSetNodeSet __attribute((alias("xmlXPtrNewLocationSetNodeSet__internal_alias")));
#else
#ifndef xmlXPtrNewLocationSetNodeSet
extern __typeof (xmlXPtrNewLocationSetNodeSet) xmlXPtrNewLocationSetNodeSet__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrNewLocationSetNodeSet xmlXPtrNewLocationSetNodeSet__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrNewLocationSetNodes
extern __typeof (xmlXPtrNewLocationSetNodes) xmlXPtrNewLocationSetNodes __attribute((alias("xmlXPtrNewLocationSetNodes__internal_alias")));
#else
#ifndef xmlXPtrNewLocationSetNodes
extern __typeof (xmlXPtrNewLocationSetNodes) xmlXPtrNewLocationSetNodes__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrNewLocationSetNodes xmlXPtrNewLocationSetNodes__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrNewRange
extern __typeof (xmlXPtrNewRange) xmlXPtrNewRange __attribute((alias("xmlXPtrNewRange__internal_alias")));
#else
#ifndef xmlXPtrNewRange
extern __typeof (xmlXPtrNewRange) xmlXPtrNewRange__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrNewRange xmlXPtrNewRange__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrNewRangeNodeObject
extern __typeof (xmlXPtrNewRangeNodeObject) xmlXPtrNewRangeNodeObject __attribute((alias("xmlXPtrNewRangeNodeObject__internal_alias")));
#else
#ifndef xmlXPtrNewRangeNodeObject
extern __typeof (xmlXPtrNewRangeNodeObject) xmlXPtrNewRangeNodeObject__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrNewRangeNodeObject xmlXPtrNewRangeNodeObject__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrNewRangeNodePoint
extern __typeof (xmlXPtrNewRangeNodePoint) xmlXPtrNewRangeNodePoint __attribute((alias("xmlXPtrNewRangeNodePoint__internal_alias")));
#else
#ifndef xmlXPtrNewRangeNodePoint
extern __typeof (xmlXPtrNewRangeNodePoint) xmlXPtrNewRangeNodePoint__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrNewRangeNodePoint xmlXPtrNewRangeNodePoint__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrNewRangeNodes
extern __typeof (xmlXPtrNewRangeNodes) xmlXPtrNewRangeNodes __attribute((alias("xmlXPtrNewRangeNodes__internal_alias")));
#else
#ifndef xmlXPtrNewRangeNodes
extern __typeof (xmlXPtrNewRangeNodes) xmlXPtrNewRangeNodes__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrNewRangeNodes xmlXPtrNewRangeNodes__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrNewRangePointNode
extern __typeof (xmlXPtrNewRangePointNode) xmlXPtrNewRangePointNode __attribute((alias("xmlXPtrNewRangePointNode__internal_alias")));
#else
#ifndef xmlXPtrNewRangePointNode
extern __typeof (xmlXPtrNewRangePointNode) xmlXPtrNewRangePointNode__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrNewRangePointNode xmlXPtrNewRangePointNode__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrNewRangePoints
extern __typeof (xmlXPtrNewRangePoints) xmlXPtrNewRangePoints __attribute((alias("xmlXPtrNewRangePoints__internal_alias")));
#else
#ifndef xmlXPtrNewRangePoints
extern __typeof (xmlXPtrNewRangePoints) xmlXPtrNewRangePoints__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrNewRangePoints xmlXPtrNewRangePoints__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrRangeToFunction
extern __typeof (xmlXPtrRangeToFunction) xmlXPtrRangeToFunction __attribute((alias("xmlXPtrRangeToFunction__internal_alias")));
#else
#ifndef xmlXPtrRangeToFunction
extern __typeof (xmlXPtrRangeToFunction) xmlXPtrRangeToFunction__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrRangeToFunction xmlXPtrRangeToFunction__internal_alias
#endif
#endif
#endif

#if defined(LIBXML_XPTR_ENABLED)
#ifdef bottom_xpointer
#undef xmlXPtrWrapLocationSet
extern __typeof (xmlXPtrWrapLocationSet) xmlXPtrWrapLocationSet __attribute((alias("xmlXPtrWrapLocationSet__internal_alias")));
#else
#ifndef xmlXPtrWrapLocationSet
extern __typeof (xmlXPtrWrapLocationSet) xmlXPtrWrapLocationSet__internal_alias __attribute((visibility("hidden")));
#define xmlXPtrWrapLocationSet xmlXPtrWrapLocationSet__internal_alias
#endif
#endif
#endif


#endif
#endif
#endif
#endif
#endif


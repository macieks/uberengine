#include "Base/ueBase.h"

// Misc.

ueBool ueRegexMatches(const char* name, const char* pattern)
{
	UE_NOT_IMPLEMENTED();
	return UE_FALSE;
}

ueBool ueWildcardMatches(const char* name, const char* pattern)
{
	if (!ueStrCmp(pattern, "*") || !ueStrCmp(pattern, "*.*")) return UE_TRUE;
	UE_NOT_IMPLEMENTED();
	return UE_FALSE;
}

// Memory

void ueMemCpy(void* dst, const void* src, ueSize size)
{
	memcpy(dst, src, size);
}

void ueMemSet(void* dst, u32 value, ueSize size)
{
	memset(dst, value, size);
}

#define ueMemSetS(dst, value) ueMemSet(&dst, sizeof(dst), value)

void ueMemZero(void* dst, ueSize size)
{
	if (!dst)
		UE_DEBUG_BREAK();
	memset(dst, 0, size);
}

s32 ueMemCmp(const void* a, const void* b, ueSize size)
{
	return memcmp(a, b, size);
}

void ueMemMove(void* dst, const void* src, ueSize size)
{
	memmove(dst, src, size);
}

void* ueMemDup(const void* src, ueSize size, ueAllocator* allocator)
{
	void* dst = allocator->Alloc(size);
	if (!dst)
		return NULL;
	ueMemCpy(dst, src, size);
	return dst;
}

u32 ueEnumGetValue(const ueEnumEntry* entries, const char* name)
{
	while (entries->m_name && ueStrCmp(entries->m_name, name))
		entries++;
	return entries->m_name ? entries->m_value : 0xFFFFFFFF;
}

const char* ueEnumGetName(const ueEnumEntry* entries, u32 value)
{
	while (entries->m_name && entries->m_value != value)
		entries++;
	return entries->m_name ? entries->m_name : NULL;
}

void ueGetEngineVersion(u32& majorVer, u32& minorVer)
{
	// Note: bump up engine version here with each major change

	majorVer = 1;
	minorVer = 0;
}

void uePlatform_FromString(uePlatform& platform, const char* s)
{
	if (!ueStrCmp(s, "win32")) platform = uePlatform_Win32;
	else if (!ueStrCmp(s, "win64")) platform = uePlatform_Win64;
	else if (!ueStrCmp(s, "x360")) platform = uePlatform_X360;
	else if (!ueStrCmp(s, "ps3")) platform = uePlatform_PS3;
	else if (!ueStrCmp(s, "wii")) platform = uePlatform_WII;
	else if (!ueStrCmp(s, "linux")) platform = uePlatform_Linux;
	else if (!ueStrCmp(s, "mac")) platform = uePlatform_Mac;
	else if (!ueStrCmp(s, "marmalade")) platform = uePlatform_Marmalade;
	else platform = uePlatform_Unknown;
}

const char* uePlatforms_AsString()
{
	return "win32, win64, marmalade, linux, mac, wii, x360, ps3";
}
#pragma once

#include "Base/ueBase.h"

class ueSampleApp;

//! Base sample class
class ueSample
{
public:
	virtual ~ueSample() {}
	virtual ueBool Init() = 0;
	virtual void Deinit() = 0;
	virtual void DoFrame(f32 dt) = 0;
};

// Sample initialization utils

typedef ueSample* (*ueSampleCreateFunc)(ueAllocator* allocator);

class ueSampleInitializer
{
public:
	ueSampleInitializer(ueSampleCreateFunc func, const char* name);
};

#define UE_DECLARE_SAMPLE(sampleClass, sampleName) \
	ueSample* ueSampleCreate_##sampleClass(ueAllocator* allocator) { return ueNew<sampleClass>(allocator, 16); } \
	static ueSampleInitializer s_sampleInitializer_##sampleClass(ueSampleCreate_##sampleClass, sampleName);

// Sample manager

void				ueSampleMgr_Register(ueSampleCreateFunc func, const char* name);
void				ueSampleMgr_Unregister(ueSampleCreateFunc func);

ueSampleCreateFunc	ueSampleMgr_GetByName(const char* name);
const char*			ueSampleMgr_GetName(ueSampleCreateFunc func);
ueSampleCreateFunc	ueSampleMgr_GetFirst();
ueSampleCreateFunc	ueSampleMgr_GetNext(ueSampleCreateFunc sample);
ueSampleCreateFunc	ueSampleMgr_GetPrev(ueSampleCreateFunc sample);
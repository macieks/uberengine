#include "Script/scLib.h"
#include "IO/ioFile.h"

ueBool scStack_GetBoolDef(scStack* stack, u32 index, ueBool defaultValue)
{
	ueBool value = defaultValue;
	scStack_GetBool(stack, index, value);
	return value;
}

s32 scStack_GetIntDef(scStack* stack, u32 index, s32 defaultValue)
{
	s32 value = defaultValue;
	scStack_GetInt(stack, index, value);
	return value;
}

f32 scStack_GetFloatDef(scStack* stack, u32 index, f32 defaultValue)
{
	f32 value = defaultValue;
	scStack_GetFloat(stack, index, value);
	return value;
}

const char* scStack_GetStringDef(scStack* stack, u32 index, const char* defaultValue)
{
	const char* value = defaultValue;
	scStack_GetString(stack, index, value);
	return value;
}

void* scStack_GetUserPtrDef(scStack* stack, u32 index, void* defaultValue)
{
	void* value = defaultValue;
	scStack_GetUserPtr(stack, index, value);
	return value;
}

ueBool scStack_GetBool(scStack* stack, u32 index)
{
	ueBool value;
	UE_ASSERT_FUNC(scStack_GetBool(stack, index, value));
	return value;
}

s32 scStack_GetInt(scStack* stack, u32 index)
{
	s32 value;
	UE_ASSERT_FUNC(scStack_GetInt(stack, index, value));
	return value;
}

f32 scStack_GetFloat(scStack* stack, u32 index)
{
	f32 value;
	UE_ASSERT_FUNC(scStack_GetFloat(stack, index, value));
	return value;
}

const char* scStack_GetString(scStack* stack, u32 index)
{
	const char* value;
	UE_ASSERT_FUNC(scStack_GetString(stack, index, value));
	return value;
}

void* scStack_GetUserPtr(scStack* stack, u32 index)
{
	void* value;
	UE_ASSERT_FUNC(scStack_GetUserPtr(stack, index, value));
	return value;
}

void* scStack_GetObj(scStack* stack, u32 index, scClassDesc* cd)
{
	void* value;
	UE_ASSERT_FUNC(scStack_GetObj(stack, index, cd, value));
	return value;
}

// scCtx

ueBool scCtx_DoFile(scCtx* ctx, const char* path)
{
	// Load file into single memory block

	ueAllocator* allocator = scLib_GetAllocator();
	ueSize dataSize = 0;
	char* data = NULL;
	if (!ioFile_Load(path, (void*&) data, dataSize, 0, allocator))
	{
		ueLogE("Failed to do file '%s', reason: failed to load file", path);
		return UE_FALSE;
	}

	// Execute string

	const ueBool result = scCtx_DoString(ctx, data, (u32) dataSize, path);
	allocator->Free(data);
	return result;
}
#include "Script/scLib.h"

// Squirrel headers

#include <assert.h>

#include "squirrel.h"
#include "sqstate.h"
#include "sqvm.h"

#include "sqstdblob.h"
#include "sqstdsystem.h"
#include "sqstdio.h"
#include "sqstdmath.h"
#include "sqstdstring.h"
#include "sqstdaux.h"

#undef assert

// Data

enum scStackState
{
	scStackState_PushingInputs = 0,
	scStackState_PoppingInputs,
	scStackState_PushingOutputs,
	scStackState_PoppingOutputs,

	scStackState_MAX
};

struct scStack
{
	scCtx* m_ctx;
	u32 m_numInputs;
	u32 m_numOutputs;
	scStackState m_state;
};

struct scCtx
{
	void* m_memory;
	u32 m_memorySize;

	u32 m_numStacks;
	u32 m_maxStacks;
	scStack* m_stacks;

	const u8* m_prog;
	u32 m_progSize;
	u32 m_progPtr;

	HSQUIRRELVM m_vm;
};

struct scLib
{
	ueAllocator* m_allocator;
	u32 m_numCtxs;

	scLib() :
		m_allocator(NULL),
		m_numCtxs(0)
	{}
};

static scLib s_mgr;

// Forward decls

ueBool scStack_PreGet(scStack* stack, u32 index, s32& stackIndex);
void scStack_PrePush(scStack* stack);
void scStack_PostPush(scStack* stack);

scStack* scCtx_AllocStack(scCtx* ctx);
void scCtx_FreeStack(scCtx* ctx, scStack* stack);
void scCtx_ThrowError(scCtx* ctx, const char* msg, ...);

// Squirrel memory functions

void* sq_vm_malloc(SQUnsignedInteger size)
{
	return s_mgr.m_allocator->Alloc(size);
}

void* sq_vm_realloc(void* p, SQUnsignedInteger oldsize, SQUnsignedInteger size)
{
	return s_mgr.m_allocator->Realloc(p, size);
}

void sq_vm_free(void* p, SQUnsignedInteger size)
{
	s_mgr.m_allocator->Free(p);
}

// Squirrel callbacks

void SQPrintCallback(HSQUIRRELVM vm, const SQChar* text, ...)
{
	va_list vl;
	va_start(vl, text);
	ueLogV((const char*) text, vl);
	va_end(vl);
}

void SQPrintErrorCallback(HSQUIRRELVM vm, const SQChar* text, ...)
{
	va_list vl;
	va_start(vl, text);
	ueLogV((const char*) text, vl);
	va_end(vl);
}

SQInteger SQLexReadCallback(SQUserPointer userdata)
{
	scCtx* ctx = (scCtx*) userdata;
	SQInteger value = *(ctx->m_prog + ctx->m_progPtr);
	ctx->m_progPtr++;
	return value;
}

SQInteger SQReadCallback(SQUserPointer userdata, SQUserPointer dest, SQInteger size)
{
	scCtx* ctx = (scCtx*) userdata;
	if (ctx->m_progSize - ctx->m_progPtr < (u32) size)
		return 0;
	ueMemCpy(dest, ctx->m_prog + ctx->m_progPtr, size);
	ctx->m_progPtr += size;
	return size;
}

SQInteger SQFuncCallback(HSQUIRRELVM vm)
{
	UE_ASSERT( sq_gettype(vm, -1) == OT_USERPOINTER );

	scFuncDesc* funcDesc = NULL;
	if (SQ_FAILED(sq_getuserpointer(vm, -1, (SQUserPointer*) &funcDesc)))
		return sq_throwerror(vm, "Failed to invoke native function, reason: failed to get user pointer (function description).");
	sq_poptop(vm);

	scStack stack;
	stack.m_ctx = (scCtx*) sq_getforeignptr(vm);
	stack.m_numInputs = sq_gettop(vm) - 1; // -1 ignores table (for global func) or class (for static class method)
	stack.m_numOutputs = 0;
	stack.m_state = scStackState_PoppingInputs;

	if (!funcDesc->m_func(&stack))
		return SQ_ERROR;
	UE_ASSERT(stack.m_numOutputs <= 1);
	return stack.m_numOutputs;
}

SQInteger SQMethodCallback(HSQUIRRELVM vm)
{
	UE_ASSERT( sq_gettype(vm, -1) == OT_USERPOINTER );

	scMethodDesc* methodDesc = NULL;
	if (SQ_FAILED(sq_getuserpointer(vm, -1, (SQUserPointer*) &methodDesc)))
		return sq_throwerror(vm, "Failed to invoke native class method, reason: failed to get user pointer (method description).");
	sq_poptop(vm);

	scStack stack;
	stack.m_ctx = (scCtx*) sq_getforeignptr(vm);
	stack.m_numInputs = sq_gettop(vm) - 1;
	stack.m_numOutputs = 0;
	stack.m_state = scStackState_PoppingInputs;

	void* obj = NULL;
	if (SQ_FAILED(sq_getinstanceup(stack.m_ctx->m_vm, 1, (SQUserPointer*) &obj, 0)))
		return sq_throwerror(vm, "Failed to pop this for method callback.");

	if (!methodDesc->m_method(obj, &stack))
		return SQ_ERROR;
	UE_ASSERT(stack.m_numOutputs <= 1);
	return stack.m_numOutputs;
}

SQInteger SQToStringCallback(HSQUIRRELVM vm)
{
	UE_ASSERT( sq_gettype(vm, -1) == OT_USERPOINTER );

	scClassDesc* cd = NULL;
	if (SQ_FAILED(sq_getuserpointer(vm, -1, (SQUserPointer*) &cd)))
		return sq_throwerror(vm, "Failed to invoke native class to string method, reason: failed to get user pointer (class description).");
	sq_poptop(vm);

	if (sq_gettop(vm) != 1)
		return sq_throwerror(vm, "Failed to invoke native class to string method, reason: invalid stack size (size != 1).");

	scStack stack;
	stack.m_ctx = (scCtx*) sq_getforeignptr(vm);
	stack.m_numInputs = 0;
	stack.m_numOutputs = 0;
	stack.m_state = scStackState_PoppingInputs;

	void* obj = NULL;
	if (SQ_FAILED(sq_getinstanceup(stack.m_ctx->m_vm, 1, (SQUserPointer*) &obj, 0)))
		return sq_throwerror(vm, "Failed to invoke native class to string method, reason: failed to get instance.");

	if (!cd->m_toString(obj, &stack))
		return SQ_ERROR;
	UE_ASSERT(stack.m_numOutputs == 1);
	return stack.m_numOutputs;
}

SQInteger SQConstructorCallback(HSQUIRRELVM vm)
{
	UE_ASSERT( sq_gettype(vm, -1) == OT_USERPOINTER );

	scClassDesc* cd = NULL;
	if (SQ_FAILED(sq_getuserpointer(vm, -1, (SQUserPointer*) &cd)))
		return sq_throwerror(vm, "Failed to invoke native class constructor, reason: failed to get user pointer (class description).");
	sq_poptop(vm);

	scStack stack;
	stack.m_ctx = (scCtx*) sq_getforeignptr(vm);
	stack.m_numInputs = sq_gettop(vm) - 1;
	stack.m_numOutputs = 0;
	stack.m_state = scStackState_PoppingInputs;

	void* obj = cd->m_constructor.m_constructor(&stack);
	if (!obj)
		return SQ_ERROR;

	if (SQ_FAILED(sq_setinstanceup(vm, 1, obj)))
		return sq_throwerror(vm, "Failed to invoke native class constructor, reason: failed to create instanece.");
	sq_setreleasehook(vm, 1, (SQRELEASEHOOK) (void*) cd->m_disposer); // Reinterpret cast for C function

	UE_ASSERT(stack.m_numOutputs == 0);
	return 0;
}

void SQCompileErrorCallback(HSQUIRRELVM vm, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column)
{
	ueLog("SQUIRREL COMPILATION ERROR in '%s', line: %d column: %d:\n%s\n", source, line, column, desc);
}

// Debugging

void scCtx_PrintArgs(scCtx* ctx)
{
	SQInteger numArgs = sq_gettop(ctx->m_vm); //number of arguments

	ueLog("args [%d]\n", numArgs);
	for (SQInteger n = 1; n <= numArgs; n++)
	{
		ueLog(" [%d]: ", n);
		switch (sq_gettype(ctx->m_vm, -n))
		{
		case OT_NULL:
			ueLog("null");
			break;
		case OT_BOOL:
		{
			SQBool value;
			sq_getbool(ctx->m_vm, -n, &value);
			ueLog("bool = %s", value ? "TRUE" : "FALSE");
			break;
		}
		case OT_INTEGER:
		{
			s32 value;
			sq_getinteger(ctx->m_vm, -n, &value);
			ueLog("int = %d", value);
			break;
		}
		case OT_FLOAT:
		{
			f32 value;
			sq_getfloat(ctx->m_vm, -n, &value);
			ueLog("float = %f", value);
			break;
		}
		case OT_STRING:
		{
			const SQChar* value = NULL;
			sq_getstring(ctx->m_vm, -n, &value);
			ueLog("string = '%s'", value);
			break;
		}
		case OT_TABLE:
			ueLog("table");
			break;
		case OT_ARRAY:
			ueLog("array");
			break;
		case OT_USERDATA:
			ueLog("userdata");
			break;
		case OT_CLOSURE:
		{
			SQUnsignedInteger nparams = 0;
			SQUnsignedInteger nfreevars = 0;
			UE_ASSERT_FUNC(SQ_SUCCEEDED(sq_getclosureinfo(ctx->m_vm, -n, &nparams, &nfreevars)));
			ueLog("closure(function) [params: %u, free vars: %u]", nparams, nfreevars);
			break;
		}
		case OT_NATIVECLOSURE:
		{
			ueLog("native closure(C function)");
			break;
		}
		case OT_GENERATOR:
			ueLog("generator");
			break;
		case OT_USERPOINTER:
			ueLog("userpointer");
			break;
		case OT_CLASS:
			ueLog("class");
			break;
		case OT_INSTANCE:
			ueLog("instance");
			break;
		case OT_WEAKREF:
			ueLog("weak reference");
			break;
		}
		ueLog("\n");
	}
}

// Helpers

scStack* scCtx_AllocStack(scCtx* ctx)
{
	if (ctx->m_numStacks == ctx->m_maxStacks)
		return NULL;
	return ctx->m_stacks + ctx->m_numStacks++;
}

void scCtx_FreeStack(scCtx* ctx, scStack* stack)
{
	UE_ASSERT(ctx->m_numStacks > 0);
	UE_ASSERT(stack == ctx->m_stacks + ctx->m_numStacks - 1);
	ctx->m_numStacks--;
}

void scLib_ToSQParamsAndTypeMask(s32& numParams, const char*& typeMask, char envChar)
{
	static char outTypeMask[64];

	outTypeMask[0] = envChar;
	ueStrCpy(&outTypeMask[1], UE_ARRAY_SIZE(outTypeMask) - 1, typeMask);
	typeMask = (char*) outTypeMask;

	if (numParams <= 0)
		numParams--;
	else
		numParams++;
}

// Stack

u32 scStack_GetNumInputs(scStack* stack)
{
	UE_ASSERT(stack->m_state == scStackState_PoppingInputs);
	return stack->m_numInputs;
}

u32 scStack_GetNumOutputs(scStack* stack)
{
	UE_ASSERT(stack->m_state == scStackState_PoppingOutputs);
	return stack->m_numOutputs;
}

ueBool scStack_PreGet(scStack* stack, u32 index, s32& stackIndex)
{
	switch (stack->m_state)
	{
		case scStackState_PoppingInputs:
			if (stack->m_numInputs <= index) return UE_FALSE;
			stackIndex = index - stack->m_numInputs;
			break;
		case scStackState_PoppingOutputs:
			if (stack->m_numOutputs <= index) return UE_FALSE;
			stackIndex = index - stack->m_numOutputs;
			break;
		default:
			UE_ASSERT(!"Can't get-type when pushing");
	}
	return UE_TRUE;
}

void scStack_PrePush(scStack* stack)
{
	switch (stack->m_state)
	{
		case scStackState_PushingInputs:
			break;
		case scStackState_PoppingInputs:
			stack->m_state = scStackState_PushingOutputs;
			sq_pop(stack->m_ctx->m_vm, stack->m_numInputs);
			break;
		case scStackState_PushingOutputs:
			UE_ASSERT_MSG(stack->m_numOutputs == 0, "Can't push more than 1 value as a function / method result.");
			break;
		case scStackState_PoppingOutputs:
			UE_ASSERT(!"Can't push in current state.");
			break;
	}
}

void scStack_PostPush(scStack* stack)
{
	switch (stack->m_state)
	{
		case scStackState_PushingInputs:
			stack->m_numInputs++;
			break;
		case scStackState_PushingOutputs:
			stack->m_numOutputs++;
			break;
	}
}

scType scStack_GetType(scStack* stack, u32 index, scClassDesc** cd)
{
	s32 stackIndex = 0;
	if (!scStack_PreGet(stack, index, stackIndex))
		return scType_Invalid;

	SQObjectType type = sq_gettype(stack->m_ctx->m_vm, stackIndex);
	switch (type)
	{
		case OT_BOOL:		return scType_Bool;
		case OT_INTEGER:	return scType_Int;
		case OT_FLOAT:		return scType_Float;
		case OT_STRING:		return scType_String;
		case OT_USERPOINTER:return scType_UserPtr;
		case OT_INSTANCE:
		{
			if (cd && (SQ_FAILED(sq_gettypetag(stack->m_ctx->m_vm, stackIndex, (SQUserPointer*) cd)) || !cd))
				return scType_Invalid;
			return scType_Obj;
		}
		default:			return scType_Invalid;
	}
}

ueBool scStack_GetBool(scStack* stack, u32 index, ueBool& value)
{
	s32 stackIndex = 0;
	if (!scStack_PreGet(stack, index, stackIndex))
		return UE_FALSE;

	SQBool sqBool;
	if (SQ_FAILED(sq_getbool(stack->m_ctx->m_vm, stackIndex, &sqBool)))
		return UE_FALSE;
	value = sqBool ? UE_TRUE : UE_FALSE;
	return UE_TRUE;
}

ueBool scStack_GetInt(scStack* stack, u32 index, s32& value)
{
	s32 stackIndex = 0;
	if (!scStack_PreGet(stack, index, stackIndex))
		return UE_FALSE;

	return sq_getinteger(stack->m_ctx->m_vm, stackIndex, &value) == SQ_OK;
}

ueBool scStack_GetFloat(scStack* stack, u32 index, f32& value)
{
	s32 stackIndex = 0;
	if (!scStack_PreGet(stack, index, stackIndex))
		return UE_FALSE;

	return sq_getfloat(stack->m_ctx->m_vm, stackIndex, &value) == SQ_OK;
}

ueBool scStack_GetString(scStack* stack, u32 index, const char*& buffer)
{
	s32 stackIndex = 0;
	if (!scStack_PreGet(stack, index, stackIndex))
		return UE_FALSE;

	return sq_getstring(stack->m_ctx->m_vm, stackIndex, (const SQChar**) &buffer) == SQ_OK;
}

ueBool scStack_GetUserPtr(scStack* stack, u32 index, void*& userPtr)
{
	s32 stackIndex = 0;
	if (!scStack_PreGet(stack, index, stackIndex))
		return UE_FALSE;

	return sq_getuserpointer(stack->m_ctx->m_vm, stackIndex, &userPtr) == SQ_OK;
}

ueBool scStack_GetObj(scStack* stack, u32 index, scClassDesc* cd, void*& objPtr)
{
	s32 stackIndex = 0;
	if (!scStack_PreGet(stack, index, stackIndex))
		return UE_FALSE;

	return sq_getinstanceup(stack->m_ctx->m_vm, stackIndex, (SQUserPointer*) &objPtr, cd) == SQ_OK;
}

ueBool scStack_PushBool(scStack* stack, ueBool value)
{
	scStack_PrePush(stack);
	sq_pushbool(stack->m_ctx->m_vm, value ? SQTrue : SQFalse);
	scStack_PostPush(stack);
	return UE_TRUE;
}

ueBool scStack_PushInt(scStack* stack, s32 value)
{
	scStack_PrePush(stack);
	sq_pushinteger(stack->m_ctx->m_vm, value);
	scStack_PostPush(stack);
	return UE_TRUE;
}

ueBool scStack_PushFloat(scStack* stack, f32 value)
{
	scStack_PrePush(stack);
	sq_pushfloat(stack->m_ctx->m_vm, value);
	scStack_PostPush(stack);
	return UE_TRUE;
}

ueBool scStack_PushString(scStack* stack, const char* string, u32 length)
{
	scStack_PrePush(stack);
	sq_pushstring(stack->m_ctx->m_vm, (const SQChar*) string, length == 0 ? -1 : length);
	scStack_PostPush(stack);
	return UE_TRUE;
}

ueBool scStack_PushUserPtr(scStack* stack, void* userPtr)
{
	scStack_PrePush(stack);
	sq_pushuserpointer(stack->m_ctx->m_vm, userPtr);
	scStack_PostPush(stack);
	return UE_TRUE;
}

ueBool scStack_PushObj(scStack* stack, scClassDesc* cd, void* obj)
{
	scStack_PrePush(stack);

	HSQUIRRELVM v = stack->m_ctx->m_vm;

	SQInteger oldtop = sq_gettop(v);

    sq_pushroottable(v);
	sq_pushstring(v, cd->m_name, -1);
    if (SQ_FAILED(sq_rawget(v, -2)))
	{
		sq_settop(v, oldtop);
		return scStack_ThrowError(stack, "Failed to get '%s' class from root table.", cd->m_name);
	}

	if (SQ_FAILED(sq_createinstance(v, -1)))
	{
		sq_settop(v, oldtop);
		return scStack_ThrowError(stack, "Failed to create instance of the '%s' class.", cd->m_name);
	}

    sq_remove(v, -3); //removes the root table
    sq_remove(v, -2); //removes the class
    if (SQ_FAILED(sq_setinstanceup(v, -1, obj)))
	{
		sq_settop(v, oldtop);
		return scStack_ThrowError(stack, "Failed to set up instance of the '%s' class.", cd->m_name);
	}
	sq_setreleasehook(v, -1, (SQRELEASEHOOK) (void*) cd->m_disposer);

	scStack_PostPush(stack);
	return UE_TRUE;
}

ueBool scStack_Call(scStack* stack)
{
	UE_ASSERT(stack->m_state == scStackState_PushingInputs);

	const SQInteger topBefore = sq_gettop(stack->m_ctx->m_vm);
	if (SQ_FAILED(sq_call(stack->m_ctx->m_vm, stack->m_numInputs + 1 /* root table */, SQTrue, SQTrue)))
		return UE_FALSE;
	const SQInteger topAfter = sq_gettop(stack->m_ctx->m_vm);

	// Determine number of outputs

	stack->m_numOutputs = topAfter - (topBefore - (stack->m_numInputs + 1));
	stack->m_state = scStackState_PoppingOutputs;
	return UE_TRUE;
}

void scStack_EndCall(scStack* stack)
{
	UE_ASSERT(stack->m_state == scStackState_PushingInputs ||
		stack->m_state == scStackState_PoppingOutputs);
	sq_pop(stack->m_ctx->m_vm, 2); // Pop function and root table
	scCtx_FreeStack(stack->m_ctx, stack);
}

scCtx* scStack_GetCtx(scStack* stack)
{
	return stack->m_ctx;
}

ueBool scStack_ThrowError(scStack* stack, const char* msg, ...)
{
	char buffer[1 << 10];

	va_list argList;
	va_start(argList, msg);
	ueStrFormatVArgs(buffer, UE_ARRAY_SIZE(buffer), msg, argList);
	va_end(argList);

	sq_throwerror(stack->m_ctx->m_vm, buffer);
	return UE_FALSE;
}

void scCtx_ThrowError(scCtx* ctx, const char* msg, ...)
{
	char buffer[1 << 10];

	va_list argList;
	va_start(argList, msg);
	ueStrFormatVArgs(buffer, UE_ARRAY_SIZE(buffer), msg, argList);
	va_end(argList);

	sq_throwerror(ctx->m_vm, buffer);
}

// Context

scCtx* scCtx_Create(u32 stackSize, u32 maxRegisteredFuncStacks)
{
	// Allocate single memory block for all data

	const u32 memorySize = sizeof(scCtx) + maxRegisteredFuncStacks * sizeof(scStack);
	u8* memory = (u8*) s_mgr.m_allocator->Alloc(memorySize);
	UE_ASSERT(memory);

	// Create context

	scCtx* ctx = new(memory) scCtx;
	ctx->m_memory = memory;
	ctx->m_memorySize = memorySize;
	memory += sizeof(scCtx);

	ctx->m_prog = NULL;

	ctx->m_numStacks = 0;
	ctx->m_maxStacks = maxRegisteredFuncStacks;
	ctx->m_stacks = (scStack*) memory;
	memory += maxRegisteredFuncStacks * sizeof(scStack);

	// Create squirrel VM

	ctx->m_vm = sq_open(stackSize);
	sq_setforeignptr(ctx->m_vm, ctx);

	// Set misc. callbacks

	sqstd_seterrorhandlers(ctx->m_vm);
	sq_setprintfunc(ctx->m_vm, SQPrintCallback, SQPrintErrorCallback);
	sq_setcompilererrorhandler(ctx->m_vm, SQCompileErrorCallback);

	// Push root table so stdlibs can register themselves to it

	sq_pushroottable(ctx->m_vm);

	// Register stdlibs

//	sqstd_register_iolib(ctx->m_vm); // FIXME: calls new
	UE_ASSERT_FUNC(SQ_SUCCEEDED(sqstd_register_mathlib(ctx->m_vm)));
	UE_ASSERT_FUNC(SQ_SUCCEEDED(sqstd_register_stringlib(ctx->m_vm)));
	UE_ASSERT_FUNC(SQ_SUCCEEDED(sqstd_register_systemlib(ctx->m_vm)));

	// Pop root table

	sq_poptop(ctx->m_vm);

	s_mgr.m_numCtxs++;
	return ctx;
}

void scCtx_Destroy(scCtx* ctx)
{
	sq_close(ctx->m_vm);
	s_mgr.m_allocator->Free(ctx->m_memory);
	s_mgr.m_numCtxs--;
}

void scCtx_GC(scCtx* ctx, ueBool full)
{
	sq_collectgarbage(ctx->m_vm);
}

ueBool scCtx_RegisterFunc(scCtx* ctx, const scFuncDesc* func)
{
	SQInteger oldtop = sq_gettop(ctx->m_vm);
	sq_pushroottable(ctx->m_vm);

	sq_pushstring(ctx->m_vm, (const SQChar*) func->m_name, -1);
	sq_pushuserpointer(ctx->m_vm, (SQUserPointer) func);
	sq_newclosure(ctx->m_vm, SQFuncCallback, 1);

	s32 numParams = func->m_numParams;
	const char* typeMask = func->m_typeMask;
	scLib_ToSQParamsAndTypeMask(numParams, typeMask, 't');
	if (SQ_FAILED(sq_setparamscheck(ctx->m_vm, numParams, typeMask)))
	{
		sq_settop(ctx->m_vm, oldtop);
		ueLogE("Failed to register function '%s', reason: failed to set params check (count & type mask)", func->m_name);
		return UE_FALSE;
	}

	if (SQ_FAILED(sq_createslot(ctx->m_vm, -3)))
	{
		sq_settop(ctx->m_vm, oldtop);
		ueLogE("Failed to register function '%s', reason: failed to create slot in root table", func->m_name);
		return UE_FALSE;
	}

	sq_poptop(ctx->m_vm);
	return UE_TRUE;
}

ueBool scCtx_RegisterLib(scCtx* ctx, const scFuncDesc* funcs)
{
	u32 i = 0;
	while (funcs[i].m_name)
	{
		if (!scCtx_RegisterFunc(ctx, funcs + i))
			return UE_FALSE;
		i++;
	}
	return UE_TRUE;
}

ueBool scCtx_RegisterClass(scCtx* ctx, const scClassDesc* cd)
{
	UE_ASSERT(cd->m_name && cd->m_constructor.m_constructor && cd->m_disposer);

	SQInteger oldtop = sq_gettop(ctx->m_vm);
	sq_pushroottable(ctx->m_vm);

	// Create baseless class

	sq_pushstring(ctx->m_vm, cd->m_name, -1);
	if (SQ_FAILED(sq_newclass(ctx->m_vm, SQFalse)))
	{
		sq_settop(ctx->m_vm, oldtop);
		ueLogE("Failed to register class '%s', reason: failed to new class", cd->m_name);
		return UE_FALSE;
	}
	if (SQ_FAILED(sq_settypetag(ctx->m_vm, -1, (SQUserPointer) cd)))
	{
		sq_settop(ctx->m_vm, oldtop);
		ueLogE("Failed to register class '%s', reason: failed to set type tag", cd->m_name);
		return UE_FALSE;
	}

	// Constructor

	sq_pushstring(ctx->m_vm, "constructor", -1);
	sq_pushuserpointer(ctx->m_vm, (SQUserPointer) cd);
	sq_newclosure(ctx->m_vm, SQConstructorCallback, 1);

	s32 numParams = cd->m_constructor.m_numParams;
	const char* typeMask = cd->m_constructor.m_typeMask;
	scLib_ToSQParamsAndTypeMask(numParams, typeMask, 'x');
	if (SQ_FAILED(sq_setparamscheck(ctx->m_vm, numParams, typeMask)))
	{
		sq_settop(ctx->m_vm, oldtop);
		ueLogE("Failed to register constructor for class '%s', reason: failed to set params check (count & type mask)", cd->m_name);
		return UE_FALSE;
	}
	if (SQ_FAILED(sq_createslot(ctx->m_vm, -3)))
	{
		sq_settop(ctx->m_vm, oldtop);
		ueLogE("Failed to register constructor for class '%s', reason: failed to create slot", cd->m_name);
		return UE_FALSE;
	}

	// ToString

	if (cd->m_toString)
	{
		sq_pushstring(ctx->m_vm, "tostring", -1);
		sq_pushuserpointer(ctx->m_vm, (SQUserPointer) cd);
		sq_newclosure(ctx->m_vm, SQToStringCallback, 1);

		UE_ASSERT_FUNC(SQ_SUCCEEDED(sq_setparamscheck(ctx->m_vm, 1, "x")));

		if (SQ_FAILED(sq_createslot(ctx->m_vm, -3)))
		{
			sq_settop(ctx->m_vm, oldtop);
			ueLogE("Failed to register tostring method for class '%s', reason: failed to create slot for constructor", cd->m_name);
			return UE_FALSE;
		}
	}

	// Methods

	u32 i = 0;
	while (cd->m_methods[i].m_name)
	{
		sq_pushstring(ctx->m_vm, cd->m_methods[i].m_name, -1);
		sq_pushuserpointer(ctx->m_vm, (SQUserPointer) &cd->m_methods[i]);
		sq_newclosure(ctx->m_vm, SQMethodCallback, 1);

		s32 numParams = cd->m_methods[i].m_numParams;
		const char* typeMask = cd->m_methods[i].m_typeMask;
		scLib_ToSQParamsAndTypeMask(numParams, typeMask, 'x');
		if (SQ_FAILED(sq_setparamscheck(ctx->m_vm, numParams, typeMask)))
		{
			sq_settop(ctx->m_vm, oldtop);
			ueLogE("Failed to register '%s' method for class '%s', reason: failed to set params check (count & type mask)", cd->m_methods[i].m_name, cd->m_name);
			return UE_FALSE;
		}

		if (SQ_FAILED(sq_createslot(ctx->m_vm, -3)))
		{
			sq_settop(ctx->m_vm, oldtop);
			ueLogE("Failed to register '%s' method for class '%s', reason: failed to create slot for constructor", cd->m_methods[i].m_name, cd->m_name);
			return UE_FALSE;
		}

		i++;
	}

	// Static methods

	i = 0;
	while (cd->m_staticMethods[i].m_name)
	{
		sq_pushstring(ctx->m_vm, cd->m_staticMethods[i].m_name, -1);
		sq_pushuserpointer(ctx->m_vm, (SQUserPointer) &cd->m_staticMethods[i]);
		sq_newclosure(ctx->m_vm, SQFuncCallback, 1);

		s32 numParams = cd->m_staticMethods[i].m_numParams;
		const char* typeMask = cd->m_staticMethods[i].m_typeMask;
		scLib_ToSQParamsAndTypeMask(numParams, typeMask, 'y');
		if (SQ_FAILED(sq_setparamscheck(ctx->m_vm, numParams, typeMask)))
		{
			sq_settop(ctx->m_vm, oldtop);
			ueLogE("Failed to register static '%s' method for class '%s', reason: failed to set params check (count & type mask)", cd->m_staticMethods[i].m_name, cd->m_name);
			return UE_FALSE;
		}

		if (SQ_FAILED(sq_createslot(ctx->m_vm, -3)))
		{
			sq_settop(ctx->m_vm, oldtop);
			ueLogE("Failed to register static '%s' method for class '%s', reason: failed to create slot for constructor", cd->m_staticMethods[i].m_name, cd->m_name);
			return UE_FALSE;
		}

		i++;
	}

	// Add class to table

	if (SQ_FAILED(sq_createslot(ctx->m_vm, -3)))
	{
		sq_settop(ctx->m_vm, oldtop);
		ueLogE("Failed to register class '%s', reason: failed to create slot in root table", cd->m_name);
		return UE_FALSE;
	}

	// Pop root table
	sq_poptop(ctx->m_vm);
	return UE_TRUE;
}

ueBool scCtx_DoString(scCtx* ctx, const char* s, u32 length, const char* scriptName)
{
	SQInteger oldtop = sq_gettop(ctx->m_vm);

	if (length == 0)
		length = ueStrLen(s);

	// Compile from string and push function onto the stack

	if (SQ_FAILED(sq_compilebuffer(
		ctx->m_vm,
		s,
		length,
		scriptName ? (const SQChar*) scriptName : "",
		SQTrue)))
	{
		sq_settop(ctx->m_vm, oldtop);
		ueLogE("Failed to do-string, reason: failed to compile from buffer");
		return UE_FALSE;
	}

	// Push 1st and only argument

	sq_pushroottable(ctx->m_vm);

	// Execute function

	if (SQ_FAILED(sq_call(ctx->m_vm, 1 /* root table */, SQFalse, SQTrue)))
	{
		sq_settop(ctx->m_vm, oldtop);
		ueLogE("Failed to do-string, reason: call failed");
		return UE_FALSE;
	}

	sq_settop(ctx->m_vm, oldtop);
	return UE_TRUE;
}

scStack* scCtx_BeginCall(scCtx* ctx, const char* funcName)
{
	SQInteger oldtop = sq_gettop(ctx->m_vm);

	// Retrieve function (also pushes root table, so we have 2 elements on the stack afterwards)

	sq_pushroottable(ctx->m_vm);
	sq_pushstring(ctx->m_vm, (const SQChar*) funcName, -1);
	if (SQ_FAILED(sq_get(ctx->m_vm, -2)))
	{
		sq_settop(ctx->m_vm, oldtop);
		ueLogE("Failed to begin call to '%s', reason: failed to find function in root table", funcName);
		return NULL;
	}

	// Verify the function exists

	const SQObjectType type = sq_gettype(ctx->m_vm, -1);
	if (type != OT_CLOSURE && type != OT_NATIVECLOSURE)
	{
		sq_settop(ctx->m_vm, oldtop);
		ueLogE("Failed to begin call to '%s', reason: '%s' is not a closure", funcName, funcName);
		return NULL;
	}

	// Push 1st argument - function environment

	sq_pushroottable(ctx->m_vm);

	// Create stack

	scStack* stack = scCtx_AllocStack(ctx);
	UE_ASSERT(stack);
	stack->m_ctx = ctx;
	stack->m_state = scStackState_PushingInputs;
	stack->m_numInputs = 0;
	stack->m_numOutputs = 0;
	return stack;
}

ueBool scCtx_PushBytecode(scCtx* ctx, const void* bytecode, u32 bytecodeSize)
{
	UE_ASSERT(((u8*) bytecode)[bytecodeSize - 1] == 0);

	ctx->m_prog = (u8*) bytecode;
	ctx->m_progPtr = 0;
	ctx->m_progSize = bytecodeSize;
	if (SQ_FAILED(sq_readclosure(ctx->m_vm, SQReadCallback, ctx)))
	{
		ctx->m_prog = NULL;
		return UE_FALSE;
	}

	ctx->m_prog = NULL;
	return UE_TRUE;
}

// Script manager

void scLib_Startup(ueAllocator* allocator)
{
	UE_ASSERT(!s_mgr.m_allocator);
	s_mgr.m_allocator = allocator;
}

void scLib_Shutdown()
{
	UE_ASSERT(s_mgr.m_allocator);
	UE_ASSERT(s_mgr.m_numCtxs == 0);
	s_mgr.m_allocator = NULL;
}

ueAllocator* scLib_GetAllocator()
{
	UE_ASSERT(s_mgr.m_allocator);
	return s_mgr.m_allocator;
}

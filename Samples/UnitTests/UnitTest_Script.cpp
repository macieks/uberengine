#include "Script/scLib.h"

extern void scInitLib_vec3(ueAllocator* allocator, u32 maxInstances);
extern void scDeinitLib_vec3();
extern ueBool scRegisterLib_vec3(scCtx* ctx);

ueBool Add3(scStack* stack)
{
	int a = 0, b = 0, c = 0;

	u32 numInputs = scStack_GetNumInputs(stack);
	if (numInputs != 3)
		return scStack_ThrowError(stack, "Invalid number of params for Add3 function");

	if (!scStack_GetInt(stack, 0, a)) return scStack_ThrowError(stack, "Failed to get 1st int param for Add3 function");
	if (!scStack_GetInt(stack, 1, b)) return scStack_ThrowError(stack, "Failed to get 2nd int param for Add3 function");
	if (!scStack_GetInt(stack, 2, c)) return scStack_ThrowError(stack, "Failed to get 3rd int param for Add3 function");

	return scStack_PushInt(stack, a + b + c);
}

void UnitTest_Script(ueAllocator* allocator)
{
	ueBool success;

	scLib_Startup(allocator);
	scInitLib_vec3(allocator, 10);

	// Create context

	scCtx* ctx = scCtx_Create(20, 1024);
	UE_ASSERT(ctx);

	// Register function

	scFuncDesc Add3Desc;
	Add3Desc.m_func = Add3;
	Add3Desc.m_name = "Add3";
	Add3Desc.m_numParams = 3;
	Add3Desc.m_typeMask = "iii";
	UE_ASSERT_FUNC(scCtx_RegisterFunc(ctx, &Add3Desc));

	// Register vec3 class

	UE_ASSERT_FUNC(scRegisterLib_vec3(ctx));

	// Do string (includes call to registered Add3 function)

	success = scCtx_DoFile(ctx, "ue_squirrel_test.txt");
	UE_ASSERT(success);

	// Invoke registered Add3 function from C++

	scStack* stack = scCtx_BeginCall(ctx, "Add3");
	UE_ASSERT(stack);

	scStack_PushInt(stack, 5);
	scStack_PushInt(stack, 4);
	scStack_PushInt(stack, 3);

	scCtx_PrintArgs(ctx);

	success = scStack_Call(stack);
	UE_ASSERT(success);

	u32 numOutputs = scStack_GetNumOutputs(stack);
	UE_ASSERT(numOutputs == 1);

	s32 result = 0;
	UE_ASSERT_FUNC(scStack_GetInt(stack, 0, result));
	UE_ASSERT(result == 5 + 4 + 3);

	scStack_EndCall(stack);

	// Shut down

	scCtx_Destroy(ctx);
	scDeinitLib_vec3();
	scLib_Shutdown();
}
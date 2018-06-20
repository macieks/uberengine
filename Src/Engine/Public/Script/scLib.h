#ifndef SC_LIB_H
#define SC_LIB_H

/**
 *	@defgroup sc Scripting
 *	@brief Scripting Library with support for 2-sided C/C++ binding of functions and classes.
 */

#include "Base/ueBase.h"

/**
 *	@addtogroup sc
 *	@{
 */

/**
 *	@struct scStack
 *	@brief Script stack
 */
struct scStack;

/**
 *	@struct scCtx
 *	@brief Script context
 */
struct scCtx;

//! Available script value types
enum scType
{
	scType_Bool = 0,	//!< Boolean
	scType_Int,			//!< Integer
	scType_Float,		//!< Float
	scType_String,		//!< String
	scType_UserPtr,		//!< Custom user pointer
	scType_Obj,			//!< Object

	scType_Invalid,		//!< Invalid

	scType_MAX
};

// Operator names to be used as function / method names

//! Plus operator name
#define SC_ADD_OP_NAME "_add"
//! Minus operator name
#define SC_SUB_OP_NAME "_sub"
//! Divide operator name
#define SC_DIV_OP_NAME "_div"
//! Multiply operator name
#define SC_MUL_OP_NAME "_mul"
//! Getter operator name
#define SC_GET_OP_NAME "_get"
//! Setter operator name
#define SC_SET_OP_NAME "_set"

// Handy macros used to reinterpret cast between function types

#define SC_CTOR(f)		(scCtor) (void*) (f)
#define SC_DISPOSER(f)	(scDisposer) (void*) (f)
#define SC_METHOD(f)	(scMethod) (void*) (f)
#define SC_FUNC(f)		(ueScriptFunc) (void*) (f)

// Script function

//! User registered function
typedef ueBool (*ueScriptFunc)(scStack* stack);

//! User registered function description
struct scFuncDesc
{
	const char* m_name;		//!< Function name
	ueScriptFunc m_func;	//!< Function
	s32 m_numParams;		//!< Number of function parameters
	const char* m_typeMask;	//!< Optional type mask for function parameters (helps verify parameters)
};

// Script class

//! User registered class constructor
typedef void* (*scCtor)(scStack* stack);
//! User registered class disposer (invoked on garbage collection)
typedef void (*scDisposer)(void* obj);
//! User registered class method
typedef ueBool (*scMethod)(void* obj, scStack* stack);

//! User registered class constructor description
struct scCtorDesc
{
	scCtor m_constructor;	//!< Constructor description
	s32 m_numParams;		//!< Number of parameters
	const char* m_typeMask;	//!< Optional parameters type mask (helps verify parameters)
};

//! User registered class method description
struct scMethodDesc
{
	const char* m_name;		//!< Method name
	scMethod m_method;		//!< Method function
	s32 m_numParams;		//!< Number of parameters
	const char* m_typeMask;	//!< Optional parameters type mask (helps verify parameters)
};

//! User registered class description
struct scClassDesc
{
	const char* m_name;					//!< Class name
	scCtorDesc m_constructor;			//!< Constructor
	scDisposer m_disposer;				//!< Disposer
	scMethod m_toString;				//!< Optional to-string function
	const scMethodDesc* m_methods;		//!< Class methods
	const scFuncDesc* m_staticMethods;	//!< Static class methods
};

// Script call stack
// -----------------------------------

//! Gets number of inputs
u32 scStack_GetNumInputs(scStack* stack);
//! Gets number of outputs
u32 scStack_GetNumOutputs(scStack* stack);

//! Gets type of paramter at given index; if it's an object and pointer to class description is privided, class description gets set
scType scStack_GetType(scStack* stack, u32 index, scClassDesc** cd = NULL);

//! Gets boolean parameter at given index; return true on success, UE_FALSE otherwise
ueBool scStack_GetBool(scStack* stack, u32 index, ueBool& value);
//! Gets integer parameter at given index; return true on success, UE_FALSE otherwise
ueBool scStack_GetInt(scStack* stack, u32 index, s32& value);
//! Gets float parameter at given index; return true on success, UE_FALSE otherwise
ueBool scStack_GetFloat(scStack* stack, u32 index, f32& value);
//! Gets string parameter at given index; return true on success, UE_FALSE otherwise
ueBool scStack_GetString(scStack* stack, u32 index, const char*& bufferPtr);
//! Gets user pointer parameter at given index; return true on success, UE_FALSE otherwise
ueBool scStack_GetUserPtr(scStack* stack, u32 index, void*& userPtr);
//! Gets object parameter at given index; return true on success, UE_FALSE otherwise
ueBool scStack_GetObj(scStack* stack, u32 index, scClassDesc* cd, void*& objPtr);

//! Gets boolean parameter at given index; asserts on failure
ueBool		scStack_GetBool(scStack* stack, u32 index);
//! Gets integer parameter at given index; asserts on failure
s32			scStack_GetInt(scStack* stack, u32 index);
//! Gets float parameter at given index; asserts on failure
f32			scStack_GetFloat(scStack* stack, u32 index);
//! Gets string parameter at given index; asserts on failure
const char*	scStack_GetString(scStack* stack, u32 index);
//! Gets user pointer parameter at given index; asserts on failure
void*		scStack_GetUserPtr(scStack* stack, u32 index);
//! Gets object parameter at given index; asserts on failure
void*		scStack_GetObj(scStack* stack, u32 index, scClassDesc* cd);

//! Gets boolean parameter at given index; on failure returns given default value
ueBool		scStack_GetBoolDef(scStack* stack, u32 index, ueBool defaultValue);
//! Gets integer parameter at given index; on failure returns given default value
s32			scStack_GetIntDef(scStack* stack, u32 index, s32 defaultValue);
//! Gets float parameter at given index; on failure returns given default value
f32			scStack_GetFloatDef(scStack* stack, u32 index, f32 defaultValue);
//! Gets string parameter at given index; on failure returns given default value
const char*	scStack_GetStringDef(scStack* stack, u32 index, const char* defaultValue);
//! Gets user pointer parameter at given index; on failure returns given default value
void*		scStack_GetUserPtrDef(scStack* stack, u32 index, void* defaultValue);

//! Pushes boolean value onto the stack
ueBool scStack_PushBool(scStack* stack, ueBool value);
//! Pushes integer value onto the stack
ueBool scStack_PushInt(scStack* stack, s32 value);
//! Pushes float value onto the stack
ueBool scStack_PushFloat(scStack* stack, f32 value);
//! Pushes string value onto the stack
ueBool scStack_PushString(scStack* stack, const char* string, u32 length = 0);
//! Pushes user pointer onto the stack
ueBool scStack_PushUserPtr(scStack* stack, void* userPtr);
//! Pushes object onto the stack
ueBool scStack_PushObj(scStack* stack, scClassDesc* cd, void* obj);

//! Starts script function call
ueBool scStack_Call(scStack* stack);
//! Ends script function call
void scStack_EndCall(scStack* stack);

//! Gets script context of this script stack
scCtx* scStack_GetCtx(scStack* stack);
//! Throws an error on failure and stops script execution
ueBool scStack_ThrowError(scStack* stack, const char* msg, ...);

// Script context
// ----------------------------

//! Creates script context
scCtx*			scCtx_Create(u32 stackSize = 8192, u32 maxRegisteredFuncStacks = 1);
//! Destroys the context
void			scCtx_Destroy(scCtx* ctx);
//! Performs garbage collection
void			scCtx_GC(scCtx* ctx, ueBool full);
//! Registers function
ueBool			scCtx_RegisterFunc(scCtx* ctx, const scFuncDesc* func);
//! Registers function library (takes function array as input; must be NULL terminated)
ueBool			scCtx_RegisterLib(scCtx* ctx, const scFuncDesc* funcs);
//! Registers class
ueBool			scCtx_RegisterClass(scCtx* ctx, const scClassDesc* classDesc);
//! Executes script from string
ueBool			scCtx_DoString(scCtx* ctx, const char* s, u32 length = 0, const char* scriptName = NULL);
//! Executed script from file
ueBool			scCtx_DoFile(scCtx* ctx, const char* path);
//! Begins call to given function; return stack on success, UE_FALSE otherwise
scStack*		scCtx_BeginCall(scCtx* ctx, const char* funcName);
//! Pushes bytecode onto the stack
ueBool			scCtx_PushBytecode(scCtx* ctx, const void* bytecode, u32 bytecodeSize);

//! Prints current arguments on the stack (to be used for debugging purposes)
void			scCtx_PrintArgs(scCtx* ctx);

// Script library
// ---------------------------

//! Starts up script library
void			scLib_Startup(ueAllocator* allocator);
//! Shuts down script library
void			scLib_Shutdown();
//! Gets script library allocator
ueAllocator*	scLib_GetAllocator();

// @}

#endif // SC_LIB_H
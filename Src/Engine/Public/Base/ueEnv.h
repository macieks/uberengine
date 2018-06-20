#ifndef UE_ENV_H
#define UE_ENV_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ut
 *	@{
 */

/**
 *	@struct ueVar
 *	@brief Environment variable
 */
struct ueVar;

//! Environment variable value step callback
typedef void (*ueVarStepCallback)(ueVar* var, ueBool increase);

//! Creates boolean variable
ueVar*		ueVar_CreateBool(const char* name, ueBool initialValue = UE_FALSE);
//! Creates integer variable
ueVar*		ueVar_CreateS32(const char* name, s32 initialValue = 0, s32 minValue = S32_MIN, s32 maxValue = S32_MAX, s32 step = 1);
//! Creates float variable
ueVar*		ueVar_CreateF32(const char* name, f32 initialValue = 0, f32 minValue = F32_MIN, f32 maxValue = F32_MAX, f32 step = 0.1f);
//! Creates strign variable
ueVar*		ueVar_CreateString(const char* name, u32 capacity = 64, const char* initialValue = NULL);
//! Destroys variable
void		ueVar_Destroy(ueVar* var);

//! Gets variable name
const char* ueVar_GetName(ueVar* var);

//! Gets boolean variable value
ueBool		ueVar_GetBool(ueVar* var);
//! Gets integer variable value
s32			ueVar_GetS32(ueVar* var);
//! Gets float variable value
f32			ueVar_GetF32(ueVar* var);
//! Gets string variable value
const char*	ueVar_GetString(ueVar* var);

//! Sets boolean variable value
void		ueVar_SetBool(ueVar* var, ueBool value);
//! Sets integer variable value
void		ueVar_SetS32(ueVar* var, s32 value);
//! Sets float variable value
void		ueVar_SetF32(ueVar* var, f32 value);
//! Sets string variable value
void		ueVar_SetString(ueVar* var, const char* value);

//! Sets variable from string
ueBool		ueVar_SetFromString(ueVar* var, const char* value);
//! Gets variable as string
void		ueVar_GetAsString(ueVar* var, char* buffer, u32 bufferSize);

//! Sets custom variable step callback
void		ueVar_SetStepCallback(ueVar* var, ueVarStepCallback callback);
//! Performs increase step on variable
void		ueVar_IncStep(ueVar* var);
//! Performs decrease step on variable
void		ueVar_DecStep(ueVar* var);

/**
 *	@struct ueFunc
 *	@brief Environment function
 */
struct ueFunc;
//! Environment function primitive
typedef ueBool (*ueFuncType)(ueFunc* func, u32 numArgs, char** args);

//! Creates environment function
ueFunc*		ueFunc_Create(const char* name, ueFuncType func, const char* typeMask = NULL, const char* description = NULL, const char* usage = NULL);
//! Destroys environment function
void		ueFunc_Destroy(ueFunc* func);

// Environment
// -------------------------

//! Starts up environment
void	ueEnv_Startup(ueAllocator* allocator, u32 maxVars = 512, u32 maxFuncs = 512);
//! Shuts down environment
void	ueEnv_Shutdown();

//! Executes string in environment
void	ueEnv_DoString(const char* s);
//! Executes text file in environment
ueBool	ueEnv_DoFile(const char* path);

//! Finds function of given name; returns NULL if not found
ueFunc*	ueEnv_FindFunc(const char* name);
//! Finds variable of given name; returns NULL if not found
ueVar*	ueEnv_FindVar(const char* name);

//! Performs auto-completion for a given buffer; forward parameter indicates whether to auto-complete to next or previous lexicographical keyword
void ueEnv_AutoComplete(char* buffer, u32 bufferSize, ueBool forward);

// @}

#endif // UE_ENV_H

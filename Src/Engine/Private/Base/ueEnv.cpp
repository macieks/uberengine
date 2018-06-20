#include "Base/ueSorting.h"
#include "Base/ueProperty.h"
#include "IO/ioFile.h"
#include "Base/Containers/ueHashSet.h"

// ueVar

struct ueVar
{
	const char* m_name;
	ueValue m_value;
	ueVarStepCallback m_stepCallback;

	union
	{
		struct
		{
			s32 m_min;
			s32 m_max;
			s32 m_step;
		} m_int;
		struct
		{
			f32 m_min;
			f32 m_max;
			f32 m_step;
		} m_float;
		struct
		{
			u32 m_capacity;
		} m_string;
	};

	struct HashPred
	{
		UE_INLINE u32 operator () (const ueVar& value) const
		{
			return ueCalcHash(value.m_name);
		}
	};

	struct CmpPred
	{
		UE_INLINE s32 operator () (const ueVar& a, const ueVar& b) const
		{
			return ueStrCmp(a.m_name, b.m_name);
		}
	};

	struct CmpPtrPred
	{
		UE_INLINE s32 operator () (const ueVar* a, const ueVar* b) const
		{
			return ueStrCmp(a->m_name, b->m_name);
		}
	};

	typedef ueHashSet<ueVar, HashPred, CmpPred> SetType;

	UE_INLINE ueVar() :
		m_stepCallback(NULL)
	{}
};

// ueFunc

struct ueFunc
{
	const char* m_name;
	ueFuncType m_func;
	const char* m_typeMask;
	const char* m_description;
	const char* m_usage;

	ueFunc() : m_isValid(UE_TRUE) {}

	ueBool m_isValid;

	struct HashPred
	{
		u32 operator () (const ueFunc& value) const
		{
			return ueCalcHash(value.m_name);
		}
	};

	struct CmpPred
	{
		s32 operator () (const ueFunc& a, const ueFunc& b) const
		{
			return ueStrCmp(a.m_name, b.m_name);
		}
	};

	struct CmpPtrPred
	{
		s32 operator () (const ueFunc* a, const ueFunc* b) const
		{
			return ueStrCmp(a->m_name, b->m_name);
		}
	};

	typedef ueHashSet<ueFunc, HashPred, CmpPred> SetType;
};

// ueEnvData

struct ueEnvData
{
	ueAllocator* m_allocator;

	ueVar::SetType m_vars;
	ueFunc::SetType m_funcs;

	ueEnvData() : m_allocator(NULL) {}
};

static ueEnvData* s_data = NULL;

// Helper functions

void ueEnv_PrintVar(u32 index, ueVar* var, ueBool includeType);
ueBool ueEnv_VerifyUniqueName(const char* name);

// ueVar

ueVar* ueVar_CreateBool(const char* name, ueBool initialValue)
{
	UE_ASSERT(ueEnv_VerifyUniqueName(name));

	ueVar var;

	var.m_name = name;
	var.m_value.m_type = ueValueType_Bool;
	var.m_value.m_bool = initialValue;

	return s_data->m_vars.Insert(var);
}

ueVar* ueVar_CreateS32(const char* name, s32 initialValue, s32 minValue, s32 maxValue, s32 step)
{
	UE_ASSERT(ueEnv_VerifyUniqueName(name));
	UE_ASSERT(minValue <= initialValue && initialValue <= maxValue);

	ueVar var;

	var.m_name = name;
	var.m_value.m_type = ueValueType_S32;
	var.m_value.m_s32 = initialValue;
	var.m_int.m_min = minValue;
	var.m_int.m_max = maxValue;
	var.m_int.m_step = step;

	return s_data->m_vars.Insert(var);
}

ueVar* ueVar_CreateF32(const char* name, f32 initialValue, f32 minValue, f32 maxValue, f32 step)
{
	UE_ASSERT(ueEnv_VerifyUniqueName(name));
	UE_ASSERT(minValue <= initialValue && initialValue <= maxValue);

	ueVar var;

	var.m_name = name;
	var.m_value.m_type = ueValueType_F32;
	var.m_value.m_f32 = initialValue;
	var.m_float.m_min = minValue;
	var.m_float.m_max = maxValue;
	var.m_float.m_step = step;

	return s_data->m_vars.Insert(var);
}

ueVar* ueVar_CreateString(const char* name, u32 capacity, const char* initialValue)
{
	UE_ASSERT(ueEnv_VerifyUniqueName(name));

	ueVar var;

	var.m_name = name;
	var.m_value.m_type = ueValueType_String;
	var.m_string.m_capacity = capacity;
	var.m_value.m_string = (char*) s_data->m_allocator->Alloc(capacity);
	UE_ASSERT(var.m_value.m_string);
	if (initialValue)
	{
		UE_ASSERT(ueStrLen(initialValue) < capacity);
		ueStrCpy(var.m_value.m_string, var.m_string.m_capacity, initialValue);
	}
	else
		var.m_value.m_string[0] = 0;

	return s_data->m_vars.Insert(var);
}

void ueVar_Destroy(ueVar* var)
{
	if (var->m_value.m_type == ueValueType_String)
	{
		s_data->m_allocator->Free(var->m_value.m_string);
		var->m_value.m_string = NULL;
	}

	s_data->m_vars.Remove(*var);
}

const char* ueVar_GetName(ueVar* var)
{
	return var->m_name;
}

ueBool ueVar_GetBool(ueVar* var)
{
	UE_ASSERT(var->m_value.m_type == ueValueType_Bool);
	return var->m_value.m_bool;
}

s32 ueVar_GetS32(ueVar* var)
{
	UE_ASSERT(var->m_value.m_type == ueValueType_S32);
	return var->m_value.m_s32;
}

f32 ueVar_GetF32(ueVar* var)
{
	UE_ASSERT(var->m_value.m_type == ueValueType_F32);
	return var->m_value.m_f32;
}

const char* ueVar_GetString(ueVar* var)
{
	UE_ASSERT(var->m_value.m_type == ueValueType_String);
	return var->m_value.m_string;
}

void ueVar_SetBool(ueVar* var, ueBool value)
{
	UE_ASSERT(var->m_value.m_type == ueValueType_Bool);
	var->m_value.m_bool = value;
}

void ueVar_SetS32(ueVar* var, s32 value)
{
	UE_ASSERT(var->m_value.m_type == ueValueType_S32);
	var->m_value.m_s32 = value;
}

void ueVar_SetF32(ueVar* var, f32 value)
{
	UE_ASSERT(var->m_value.m_type == ueValueType_F32);
	var->m_value.m_f32 = value;
}

void ueVar_SetString(ueVar* var, const char* value)
{
	UE_ASSERT(var->m_value.m_type == ueValueType_String);

	if (ueStrLen(value) + 1 >= var->m_string.m_capacity)
	{
		ueLogW("Failed to set var '%s' to '%s', reason: value exceeds max string length (%u).", var->m_name, value, var->m_string.m_capacity);
		return;
	}
	ueStrCpy(var->m_value.m_string, var->m_string.m_capacity, value);
}

ueBool ueVar_SetFromString(ueVar* var, const char* value)
{
	switch (var->m_value.m_type)
	{
		case ueValueType_Bool:
			return ueStrToBool(value, var->m_value.m_bool);
		case ueValueType_S32:
		{
			s32 intValue;
			if (!ueStrToS32(value, intValue)) return UE_FALSE;
			if (intValue < var->m_int.m_min || var->m_int.m_max < intValue) return UE_FALSE;
			var->m_value.m_s32 = intValue;
			break;
		}
		case ueValueType_F32:
		{
			f32 floatValue;
			if (!ueStrToF32(value, floatValue)) return UE_FALSE;
			if (floatValue < var->m_float.m_min || var->m_float.m_max < floatValue) return UE_FALSE;
			var->m_value.m_f32 = floatValue;
			break;
		}
		case ueValueType_String:
			if (ueStrLen(value) + 1 >= var->m_string.m_capacity) return UE_FALSE;
			ueStrCpy(var->m_value.m_string, var->m_string.m_capacity, value);
			break;
	}
	return UE_TRUE;
}

void ueVar_GetAsString(ueVar* var, char* buffer, u32 bufferSize)
{
	switch (var->m_value.m_type)
	{
		case ueValueType_Bool:
			ueStrFormat(buffer, bufferSize, ueStrFromBool(var->m_value.m_bool));
			break;
		case ueValueType_S32:
			ueStrFormat(buffer, bufferSize, "%d", var->m_value.m_s32);
			break;
		case ueValueType_F32:
			ueStrFormat(buffer, bufferSize, "%f", var->m_value.m_f32);
			break;
		case ueValueType_String:
			ueStrFormat(buffer, bufferSize, "\"%s\"", var->m_value.m_string);
			break;
	}
}

void ueVar_SetStepCallback(ueVar* var, ueVarStepCallback callback)
{
	var->m_stepCallback = callback;
}

void ueVar_IncStep(ueVar* var)
{
	if (var->m_stepCallback)
		var->m_stepCallback(var, UE_TRUE);
	else
		switch (var->m_value.m_type)
		{
			case ueValueType_Bool:
				var->m_value.m_bool = !var->m_value.m_bool;
				break;
			case ueValueType_S32:
				var->m_value.m_s32 = ueMin(var->m_int.m_max, var->m_value.m_s32 + var->m_int.m_step);
				break;
			case ueValueType_F32:
				var->m_value.m_f32 = ueMin(var->m_float.m_max, var->m_value.m_f32 + var->m_float.m_step);
				break;
		}
}

void ueVar_DecStep(ueVar* var)
{
	if (var->m_stepCallback)
		var->m_stepCallback(var, UE_FALSE);
	else
		switch (var->m_value.m_type)
		{
			case ueValueType_Bool:
				var->m_value.m_bool = !var->m_value.m_bool;
				break;
			case ueValueType_S32:
				var->m_value.m_s32 = ueMax(var->m_int.m_min, var->m_value.m_s32 - var->m_int.m_step);
				break;
			case ueValueType_F32:
				var->m_value.m_f32 = ueMax(var->m_float.m_min, var->m_value.m_f32 - var->m_float.m_step);
				break;
		}
}

// ueFunc

ueFunc* ueFunc_Create(const char* name, ueFuncType f, const char* typeMask, const char* description, const char* usage)
{
	UE_ASSERT(ueEnv_VerifyUniqueName(name));

	// Verify type mask
	if (typeMask)
		for (u32 i = 0; typeMask[i]; i++)
			switch (typeMask[i])
			{
				case 'n':
				case 'i':
				case 'f':
				case 's':
					break;
				default:
					UE_ASSERT_MSGP(0, "Unsupported function type mask character '%c'", typeMask[i]);
			}

	ueFunc func;

	func.m_name = name;
	func.m_func = f;
	func.m_typeMask = typeMask;
	func.m_description = description;
	func.m_usage = usage;

	return s_data->m_funcs.Insert(func);
}

void ueFunc_Destroy(ueFunc* func)
{
	s_data->m_funcs.Remove(*func);
}

// Built-in functions

ueBool ueEnv_set_func(ueFunc* func, u32 numArgs, char** args)
{
	UE_ASSERT(numArgs == 2);

	ueVar* var = ueEnv_FindVar(args[0]);
	if (!var)
	{
		ueLogE("Failed to set '%s' variable to '%s', reason: variable not found.", args[0], args[1]);
		return UE_TRUE;
	}

	if (!ueVar_SetFromString(var, args[1]))
	{
		ueLogE("Failed to set '%s' variable to '%s', reason: invalid value or range.", args[0], args[1]);
		return UE_TRUE;
	}
	return UE_TRUE;
}

ueBool ueEnv_list_vars_func(ueFunc* func, u32 numArgs, char** args)
{
	// Copy all vars to an array

	ueVar** varPtrs = (ueVar**) s_data->m_allocator->Alloc(sizeof(ueVar*) * s_data->m_vars.Size());
	if (!varPtrs)
		return UE_TRUE;

	u32 index = 0;
	ueVar::SetType::Iterator i(s_data->m_vars);
	while (ueVar* var = i.Next())
		varPtrs[index++] = var;

	// Sort vars by name

	ueVar::CmpPtrPred cmpPtrPred;
	ueSort(varPtrs, index, cmpPtrPred);

	// Print vars

	ueLogI("All vars [%d]:", index);
	for (u32 i = 0; i < index; i++)
		ueEnv_PrintVar(i, varPtrs[i], UE_TRUE);

	// Clean up

	s_data->m_allocator->Free(varPtrs);

	return UE_TRUE;
}

void ueEnv_PrintFunc(u32 index, ueFunc* func)
{
	ueLogI("  [%d] %s (params: %s) %s",
		index, func->m_name,
		func->m_typeMask ? (*func->m_typeMask ? func->m_typeMask : "<none>") : "<any>",
		func->m_description ? func->m_description : "");
}

ueBool ueEnv_list_funcs_func(ueFunc* func, u32 numArgs, char** args)
{
	// Copy all funcs to an array

	ueFunc** funcPtrs = (ueFunc**) s_data->m_allocator->Alloc(sizeof(ueFunc*) * s_data->m_funcs.Size());
	if (!funcPtrs)
		return UE_TRUE;

	u32 index = 0;
	ueFunc::SetType::Iterator i(s_data->m_funcs);
	while (ueFunc* func = i.Next())
		funcPtrs[index++] = func;

	// Sort funcs by name

	ueFunc::CmpPtrPred cmpPtrPred;
	ueSort(funcPtrs, index, cmpPtrPred);

	// Print funcs

	ueLogI("All funcs [%d]:", index);
	for (u32 i = 0; i < index; i++)
		ueEnv_PrintFunc(i, funcPtrs[i]);

	// Clean up

	s_data->m_allocator->Free(funcPtrs);

	return UE_TRUE;
}

ueBool ueEnv_help_func(ueFunc*, u32 numArgs, char** args)
{
	ueFunc* func = ueEnv_FindFunc(args[0]);
	if (!func)
	{
		ueLogW("Help for function '%s' failed, reason function not found.", args[0]);
		return UE_TRUE;
	}

	ueLogI("Help for '%s' (params: %s) function:",
		func->m_name,
		func->m_typeMask ? (*func->m_typeMask ? func->m_typeMask : "<none>") : "<any>");

	if (!func->m_description && !func->m_usage)
		ueLogI("  Help for '%s' function doesn't exist.");
	else
	{
		if (func->m_description)
			ueLogI("  Description: %s", func->m_description);
		if (func->m_usage)
			ueLogI("  Usage: %s", func->m_usage);
	}

	return UE_TRUE;
}

ueBool ueEnv_echo_func(ueFunc*, u32 numArgs, char** args)
{
	char buffer[1024];
	char* curr = (char*) buffer;

	for (u32 i = 0; i < numArgs; i++)
	{
		ueStrCat2(curr, args[i]);
		if (i + 1 < numArgs)
			ueStrCat2(curr, " ");
	}
	ueLogI(buffer);

	return UE_TRUE;
}

ueBool ueEnv_do_file_func(ueFunc*, u32 numArgs, char** args)
{
	ueEnv_DoFile(args[0]);
	return UE_TRUE;
}

ueBool ueEnv_save_vars_func(ueFunc*, u32 numArgs, char** args)
{
	ioFilePtr f;
	if (!f.Open(args[0], ioFileOpenFlags_Write | ioFileOpenFlags_Create))
	{
		ueLogW("Failed to save vars to '%s', reason: failed to open file for writing.", args[0]);
		return UE_TRUE;
	}

	static const char autoText[] = "// Automatically generated\n\n";
	static const u32 autoTextLen = ueStrLen(autoText);

	f.Write(autoText, autoTextLen);

	// Copy all vars to an array

	ueVar** varPtrs = (ueVar**) s_data->m_allocator->Alloc(sizeof(ueVar*) * s_data->m_vars.Size());
	if (!varPtrs)
		return UE_TRUE;

	u32 index = 0;
	ueVar::SetType::Iterator i(s_data->m_vars);
	while (ueVar* var = i.Next())
		varPtrs[index++] = var;

	// Sort vars by name

	ueVar::CmpPtrPred cmpPtrPred;
	ueSort(varPtrs, index, cmpPtrPred);

	// Write out vars

	for (u32 i = 0; i < s_data->m_vars.Size(); i++)
	{
		ueVar* var = varPtrs[i];

		f.Write("set ", 4);
		f.Write(var->m_name, ueStrLen(var->m_name));
		f.Write(" ", 1);
		switch (var->m_value.m_type)
		{
			case ueValueType_Bool:
			{
				char buffer[32];
				ueStrFromBool(buffer, UE_ARRAY_SIZE(buffer), var->m_value.m_bool);
				f.Write(buffer, ueStrLen(buffer));
				break;
			}
			case ueValueType_S32:
			{
				char buffer[32];
				ueStrFromS32(buffer, UE_ARRAY_SIZE(buffer), var->m_value.m_s32);
				f.Write(buffer, ueStrLen(buffer));
				break;
			}
			case ueValueType_F32:
			{
				char buffer[32];
				ueStrFromF32(buffer, UE_ARRAY_SIZE(buffer), var->m_value.m_f32);
				f.Write(buffer, ueStrLen(buffer));
				break;
			}
			case ueValueType_String:
			{
				f.Write("\"", 1);
				f.Write(var->m_value.m_string, ueStrLen(var->m_value.m_string));
				f.Write("\"", 1);
				break;
			}
			UE_INVALID_CASE(var->m_value.m_type);
		}

		f.Write("\n", 1);
	}

	f.Close();
	ueLogI("All vars succesfully saved to file '%s'.", args[0]);

	return UE_TRUE;
}

static ueFunc* ueEnv_set;
static ueFunc* ueEnv_list_vars;
static ueFunc* ueEnv_list_funcs;
static ueFunc* ueEnv_help;
static ueFunc* ueEnv_echo;
static ueFunc* ueEnv_do_file;
static ueFunc* ueEnv_save_vars;

// Env

void ueEnv_Startup(ueAllocator* allocator, u32 maxVars, u32 maxFuncs)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) ueEnvData();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;

	UE_ASSERT_FUNC(s_data->m_vars.Init(allocator, maxVars));
	UE_ASSERT_FUNC(s_data->m_funcs.Init(allocator, maxFuncs));

	ueEnv_set = ueFunc_Create("set", ueEnv_set_func, "ss", "Sets variable", "set <var-name> <value>");
	ueEnv_list_vars = ueFunc_Create("list_vars", ueEnv_list_vars_func, "", "Lists all variables");
	ueEnv_list_funcs = ueFunc_Create("list_funcs", ueEnv_list_funcs_func, "", "Lists all functions");
	ueEnv_help = ueFunc_Create("help", ueEnv_help_func, "s", "Prints help on function", "help <func-name>");
	ueEnv_echo = ueFunc_Create("echo", ueEnv_echo_func, NULL, "Echos string", "echo [<string_0>...<string_n>]");
	ueEnv_do_file = ueFunc_Create("do_file", ueEnv_do_file_func, "s", "Executes config file", "do_file <file-name>");
	ueEnv_save_vars = ueFunc_Create("save_vars", ueEnv_save_vars_func, "s", "Saves variables to file", "save_vars <file-name>");
}

void ueEnv_Shutdown()
{
	UE_ASSERT(s_data);

	ueFunc_Destroy(ueEnv_set);
	ueFunc_Destroy(ueEnv_list_vars);
	ueFunc_Destroy(ueEnv_list_funcs);
	ueFunc_Destroy(ueEnv_help);
	ueFunc_Destroy(ueEnv_echo);
	ueFunc_Destroy(ueEnv_do_file);
	ueFunc_Destroy(ueEnv_save_vars);

	s_data->m_funcs.Deinit();
	s_data->m_vars.Deinit();

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

ueBool ueEnv_VerifyUniqueName(const char* name)
{
	ueFunc func;
	func.m_name = name;
	if (s_data->m_funcs.Find(func))
		return UE_FALSE;

	ueVar var;
	var.m_name = name;
	if (s_data->m_vars.Find(var))
		return UE_FALSE;

	return UE_TRUE;
}

ueFunc* ueEnv_FindFunc(const char* name)
{
	ueFunc func;
	func.m_name = name;
	return s_data->m_funcs.Find(func);
}

ueVar* ueEnv_FindVar(const char* name)
{
	ueVar var;
	var.m_name = name;
	return s_data->m_vars.Find(var);
}

ueBool ueEnv_MatchFuncParams(ueFunc* func, u32 numParams, char** params)
{
	u32 i = 0;
	while (func->m_typeMask[i])
	{
		if (i == numParams)
			return UE_FALSE;

		switch (func->m_typeMask[i])
		{
		case 'i':
		{
			s32 intValue;
			if (!ueStrToS32(params[i], intValue)) return UE_FALSE;
			break;
		}
		case 'f':
		{
			f32 floatValue;
			if (!ueStrToF32(params[i], floatValue)) return UE_FALSE;
			break;
		}
		case 'n':
		{
			s32 intValue;
			f32 floatValue;
			if (!ueStrToS32(params[i], intValue) && !ueStrToF32(params[i], floatValue)) return UE_FALSE;
			break;
		}
		case 'b':
		{
			ueBool boolValue;
			if (!ueStrToBool(params[i], boolValue)) return UE_FALSE;
			break;
		}
		case 's':
			break;
		}
		i++;
	}


	return UE_TRUE;
}

void ueEnv_PrintVar(u32 index, ueVar* var, ueBool includeType = UE_FALSE)
{
	char indexBuffer[16];
	if (index != U32_MAX)
		ueStrFormatS(indexBuffer, "  [%d] ", index);
	else
		indexBuffer[0] = 0;

	switch (var->m_value.m_type)
	{
		case ueValueType_Bool: ueLogI("%s%s%s = %s", indexBuffer, var->m_name, includeType ? " : bool" : "", ueStrFromBool(var->m_value.m_bool)); break;
		case ueValueType_S32: ueLogI("%s%s%s = %d", indexBuffer, var->m_name, includeType ? " : int" : "", var->m_value.m_s32); break;
		case ueValueType_F32: ueLogI("%s%s%s = %f", indexBuffer, var->m_name, includeType ? " : float" : "", var->m_value.m_f32); break;
		case ueValueType_String: ueLogI("%s%s%s = %s", indexBuffer, var->m_name, includeType ? " : string" : "", var->m_value.m_string); break;
	}
}

void ueEnv_PrintFuncUsage(ueFunc* func)
{
	if (!func->m_typeMask && !func->m_usage)
		return;

	ueLogI("Usage for '%s' (params: %s) function:",
		func->m_name,
		func->m_typeMask ? (*func->m_typeMask ? func->m_typeMask : "<none>") : "<any>");

	if (func->m_usage)
		ueLogI("  %s", func->m_usage);
	else
		ueLogI("  Unknown");
}

ueBool ueEnv_DoFile(const char* path)
{
	char* buffer = NULL;
	ueSize bufferSize = 0;
	if (!ioFile_Load(path, (void*&) buffer, bufferSize, 0, s_data->m_allocator, UE_TRUE))
	{
		ueLogW("Failed to do file '%s', reason: failed to load file.", path);
		return UE_FALSE;
	}

	ueLogI("Executing file '%s'...", path);
	ueEnv_DoString(buffer);
	s_data->m_allocator->Free(buffer);
	return UE_TRUE;
}

void ueEnv_DoString(const char* s)
{
	UE_ASSERT(s);

	char buffer[8192];

	// Process all lines

	const char* curr = (char*) s;
	u32 line = 0;
	while (*curr)
	{
		// Parse tokens

		u32 numTokens = 0;
		char* tokens[64];
		u32 bufferUsedSize = 0;
		const char* lineStart = curr;

		ueBool isError = UE_FALSE;

		while (*curr)
		{
			tokens[numTokens] = &buffer[bufferUsedSize];

			curr = ueStrSkipSpaces(curr);
			if (ueStrIsEOL(*curr)) { curr = ueStrSkipLine(curr); break; }

			// Check for comment

			if (*curr == '/' && *(curr + 1) == '/')
			{
				curr += 2;
				curr = ueStrSkipLine(curr);
				break;
			}

			// Check for quotation mark

			if (*curr == '\"')
			{
				const char* tokenStart = ++curr; // Skip starting "
				curr = ueStrSkipToNextInLine('\"', curr);
				if (*curr != '\"')
				{
					curr = ueStrSkipLine(curr);
					isError = UE_TRUE;
					break;
				}
				const ueSize tokenLength = curr - tokenStart;
				ueMemCpy(tokens[numTokens], tokenStart, tokenLength);
				tokens[numTokens][tokenLength] = 0;
				numTokens++;
				bufferUsedSize += (u32) tokenLength + 1;
				curr++; // Skip ending "
			}
			else
			{
				const char* tokenStart = curr;
				curr = ueStrSkipToNextSpaceInLine(curr);
				if (curr == tokenStart)
				{
					curr = ueStrSkipLine(curr);
					break;
				}
				const ueSize tokenLength = curr - tokenStart;
				ueMemCpy(tokens[numTokens], tokenStart, tokenLength);
				tokens[numTokens][tokenLength] = 0;
				numTokens++;
				bufferUsedSize += (u32) tokenLength + 1;
			}
		}

		line++;

		// Handle parsed tokens

		if (isError)
		{
			// Copy the line into buffer

			u32 lineChar = 0;
			while (*lineStart && *lineStart != '\n')
				buffer[lineChar++] = *lineStart++;
			buffer[lineChar] = 0;

			ueLogW("Error executing command at line %d: '%s'", line, buffer);
		}
		else if (numTokens > 0)
		{
			// Try to invoke function

			ueFunc* func = ueEnv_FindFunc(tokens[0]);
			if (func)
			{
				if (func->m_typeMask && !ueEnv_MatchFuncParams(func, numTokens - 1, tokens + 1))
				{
					ueLogW("Failed to execute function '%s', reason: invalid parameters.", func->m_name);
					ueEnv_PrintFuncUsage(func);
					continue;
				}

				if (!func->m_func(func, numTokens - 1, tokens + 1))
				{
					ueLogW("Failed to execute function '%s'.", func->m_name);
					ueEnv_PrintFuncUsage(func);
					continue;
				}
				continue;
			}

			// Try to get variable name

			ueVar* var = ueEnv_FindVar(tokens[0]);
			if (var)
			{
				ueEnv_PrintVar(U32_MAX, var);
				continue;
			}

			ueLogW("Unknown command '%s'.", tokens[0]);
		}
	}
}

void ueEnv_AutoComplete(char* buffer, u32 bufferSize, ueBool forward)
{
	if (forward)
	{
		const char* firstName = NULL;
		const char* nextName = NULL;

		ueVar::SetType::Iterator varIter(s_data->m_vars);
		while (ueVar* var = varIter.Next())
		{
			if (!firstName || ueStrCmp(var->m_name, firstName) < 0)
				firstName = var->m_name;
			if (ueStrCmp(buffer, var->m_name) < 0 && (!nextName || ueStrCmp(var->m_name, nextName) < 0))
				nextName = var->m_name;
		}

		ueFunc::SetType::Iterator funcIter(s_data->m_funcs);
		while (ueFunc* func = funcIter.Next())
		{
			if (!firstName || ueStrCmp(func->m_name, firstName) < 0)
				firstName = func->m_name;
			if (ueStrCmp(buffer, func->m_name) < 0 && (!nextName || ueStrCmp(func->m_name, nextName) < 0))
				nextName = func->m_name;
		}

		if (nextName && ueStrCmp(buffer, nextName) < 0)
			ueStrCpy(buffer, bufferSize, nextName);
		else
			ueStrCpy(buffer, bufferSize, firstName);
	}
	else
	{
		const char* lastName = NULL;
		const char* prevName = NULL;

		ueVar::SetType::Iterator varIter(s_data->m_vars);
		while (ueVar* var = varIter.Next())
		{
			if (!lastName || ueStrCmp(var->m_name, lastName) > 0)
				lastName = var->m_name;
			if (ueStrCmp(buffer, var->m_name) > 0 && (!prevName || ueStrCmp(var->m_name, prevName) > 0))
				prevName = var->m_name;
		}

		ueFunc::SetType::Iterator funcIter(s_data->m_funcs);
		while (ueFunc* func = funcIter.Next())
		{
			if (!lastName || ueStrCmp(func->m_name, lastName) > 0)
				lastName = func->m_name;
			if (ueStrCmp(buffer, func->m_name) > 0 && (!prevName || ueStrCmp(func->m_name, prevName) > 0))
				prevName = func->m_name;
		}

		if (prevName && ueStrCmp(buffer, prevName) > 0)
			ueStrCpy(buffer, bufferSize, prevName);
		else
			ueStrCpy(buffer, bufferSize, lastName);
	}
}
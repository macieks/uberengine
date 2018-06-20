#include "Base/ueBase.h"

static ueVar* my_bool = NULL;
static ueVar* my_int = NULL;
static ueVar* my_float = NULL;
static ueVar* my_string = NULL;

ueBool my_add_func(ueFunc* func, u32 numArgs, char** args)
{
	const s32 a = atoi(args[0]);
	const s32 b = atoi(args[1]);
	printf("result of adding %d and %d is %d\n", a, b, a + b);
	return UE_TRUE;
}

static ueFunc* my_add = NULL;

#define TEST_FILE_NAME "ue_env_test.txt"

void UnitTest_Env(ueAllocator* allocator)
{
	ueEnv_Startup(allocator, 512, 512);

    my_bool = ueVar_CreateBool("my_bool", UE_TRUE);
	my_int = ueVar_CreateS32("my_int", 7, -15, 15, 1);
	my_float = ueVar_CreateF32("my_float", 4.6f, 0.0f, 8.0f, 0.5f);
	my_string = ueVar_CreateString("my_string", 32, "some funny string");
	my_add = ueFunc_Create("my_add", my_add_func, "ii", "Adds two integers");

	ueEnv_DoString(
		"save_vars "TEST_FILE_NAME"\n"
		"echo \"Hello world!!!!\"\n"
		"my_bool\n"
		"// This is some comment\n"
		"set my_bool false // Comment after command: setting my_bool to fale\n"
		"my_bool\n"
		"my_int\n"
		"set my_int  \"808\" \n"
		"set my_int  \"-11\" \n"
		"my_int\n"
		"my_float  \n"
		" my_string \n"
		"list_vars\n"
		"list_funcs\n"
		"set my_string  \"some stringggg\" \n"
		"set my_float  \"3.3337\" \n"
		"help help\n"
		"help set\n"
		"  \n"
		"crappy line\n"
		"my_add 7 \"19\"\n"
		"bla \" bla\n"
		"list_vars\n"
		"do_file "TEST_FILE_NAME"\n"
		"list_vars\n");

	UE_ASSERT_FUNC(ueEnv_DoFile(TEST_FILE_NAME));

	ueVar_Destroy(my_bool);
	ueVar_Destroy(my_int);
	ueVar_Destroy(my_float);
	ueVar_Destroy(my_string);
	ueFunc_Destroy(my_add);

	ueEnv_Shutdown();
}

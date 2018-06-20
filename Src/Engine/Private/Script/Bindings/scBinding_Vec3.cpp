#include "Script/scLib.h"
#include "Base/ueMath.h"
#include "Base/Containers/ueGenericPool.h"

struct vec3_Data
{
	ueBool m_isInitialized;
	ueGenericPool m_pool;
	vec3_Data() : m_isInitialized(UE_FALSE) {}
};

static vec3_Data s_data;

// Definition

ueVec3* vec3_constructor(scStack* stack);
void vec3_disposer(ueVec3* obj);
ueBool vec3_tostring(ueVec3* obj, scStack* stack);
ueBool vec3_len(ueVec3* obj, scStack* stack);
ueBool vec3_lensq(ueVec3* obj, scStack* stack);
ueBool vec3_norm(ueVec3* obj, scStack* stack);
ueBool vec3_add_op(ueVec3* obj, scStack* stack);
ueBool vec3_sub_op(ueVec3* obj, scStack* stack);
ueBool vec3_div_op(ueVec3* obj, scStack* stack);
ueBool vec3_mul_op(ueVec3* obj, scStack* stack);
ueBool vec3_get_op(ueVec3* obj, scStack* stack);
ueBool vec3_set_op(ueVec3* obj, scStack* stack);
ueBool vec3_dot(scStack* stack);
ueBool vec3_cross(scStack* stack);
ueBool vec3_lerp(scStack* stack);
ueBool vec3_dist(scStack* stack);

scMethodDesc s_vec3_methods[] =
{
	{"len", SC_METHOD(vec3_len), 0, NULL},
	{"lensq", SC_METHOD(vec3_lensq), 0, NULL},
	{"norm", SC_METHOD(vec3_norm), 0, NULL},
	{SC_ADD_OP_NAME, SC_METHOD(vec3_add_op), 1, "x|n"},
	{SC_SUB_OP_NAME, SC_METHOD(vec3_sub_op), 1, "x|n"},
	{SC_DIV_OP_NAME, SC_METHOD(vec3_div_op), 1, "n"},
	{SC_MUL_OP_NAME, SC_METHOD(vec3_mul_op), 1, "n"},
	{SC_GET_OP_NAME, SC_METHOD(vec3_get_op), 1, "s|i"},
	{SC_SET_OP_NAME, SC_METHOD(vec3_set_op), 2, "s|in"},
	{NULL, NULL, 0, NULL}
};

scFuncDesc s_vec3_static_methods[] =
{
	{"dot", SC_FUNC(vec3_dot), 2, "xx"},
	{"cross", SC_FUNC(vec3_cross), 2, "xx"},
	{"lerp", SC_FUNC(vec3_lerp), 3, "xxn"},
	{"dist", SC_FUNC(vec3_dist), 2, "xx"},
	{NULL, NULL, 0, NULL}
};

scClassDesc s_vec3_class =
{
	"vec3",
	{SC_CTOR(vec3_constructor), 0, "nnn"},
	SC_DISPOSER(vec3_disposer),
	SC_METHOD(vec3_tostring),
	s_vec3_methods,
	s_vec3_static_methods,
};

// Implementation

ueVec3* AllocVec3(f32 x, f32 y, f32 z)
{
	ueVec3* v = (ueVec3*) s_data.m_pool.Alloc();
	v->Set(x, y, z);
	return v;
}

ueVec3* vec3_constructor(scStack* stack)
{
	const f32 x = scStack_GetFloatDef(stack, 0, 0.0f);
	const f32 y = scStack_GetFloatDef(stack, 1, 0.0f);
	const f32 z = scStack_GetFloatDef(stack, 2, 0.0f);

	ueVec3* obj = AllocVec3(x, y, z);
	if (!obj)
	{
		scStack_ThrowError(stack, "Failed to allocate new instance of ueVec3 class");
		return NULL;
	}

	return obj;
}

void vec3_disposer(ueVec3* obj)
{
	s_data.m_pool.Free(obj);
}

ueBool vec3_tostring(ueVec3* obj, scStack* stack)
{
	char buffer[128];
	ueStrFormatS(buffer, "vec3(%f, %f, %f)", (*obj)[0], (*obj)[1], (*obj)[2]);
	return scStack_PushString(stack, buffer);
}

ueBool vec3_len(ueVec3* obj, scStack* stack)
{
	return scStack_PushFloat(stack, obj->Len());
}

ueBool vec3_lensq(ueVec3* obj, scStack* stack)
{
	return scStack_PushFloat(stack, obj->LenSq());
}

ueBool vec3_norm(ueVec3* obj, scStack* stack)
{
	obj->Normalize();
	return UE_TRUE;
}

ueBool vec3_add_op(ueVec3* obj, scStack* stack)
{
	ueVec3* result = (ueVec3*) s_data.m_pool.Alloc();
	if (!result)
		return scStack_ThrowError(stack, "Failed to allocate new ueVec3 instance.");

	*result = *obj;

	scClassDesc* cd = NULL;
	switch (scStack_GetType(stack, 0, &cd))
	{
		case scType_Int:
		case scType_Float:
		{
			const f32 other = scStack_GetFloat(stack, 0);
			*result += other;
			break;
		}
		case scType_Obj:
		{
			if (cd != &s_vec3_class)
				return scStack_ThrowError(stack, "Attempted to apply + operator between vec3 and %s classes", cd->m_name);

			ueVec3* other = (ueVec3*) scStack_GetObj(stack, 0, &s_vec3_class);
			*result += *other;
			break;
		}
		default:
		{
			s_data.m_pool.Free(result);
			return scStack_ThrowError(stack, "Attempted to apply + operator on vec3 using unsupported type");
		}
	}

	return scStack_PushObj(stack, &s_vec3_class, result);
}

ueBool vec3_sub_op(ueVec3* obj, scStack* stack)
{
	ueVec3* result = (ueVec3*) s_data.m_pool.Alloc();
	if (!result)
		return scStack_ThrowError(stack, "Failed to allocate new vec3 instance.");

	*result = *obj;

	scClassDesc* cd = NULL;
	switch (scStack_GetType(stack, 0, &cd))
	{
		case scType_Int:
		case scType_Float:
		{
			const f32 other = scStack_GetFloat(stack, 0);
			*result -= other;
			break;
		}
		case scType_Obj:
		{
			if (cd != &s_vec3_class)
				return scStack_ThrowError(stack, "Attempted to apply - operator between ueVec3 and %s classes", cd->m_name);

			ueVec3* other = (ueVec3*) scStack_GetObj(stack, 0, &s_vec3_class);
			*result -= *other;
			break;
		}
		default:
		{
			s_data.m_pool.Free(result);
			return scStack_ThrowError(stack, "Attempted to apply - operator on vec3 using unsupported type");
		}
	}

	return scStack_PushObj(stack, &s_vec3_class, result);
}

ueBool vec3_div_op(ueVec3* obj, scStack* stack)
{
	ueVec3* result = (ueVec3*) s_data.m_pool.Alloc();
	if (!result)
		return scStack_ThrowError(stack, "Failed to allocate new vec3 instance.");

	*result = *obj;

	switch (scStack_GetType(stack, 0))
	{
		case scType_Int:
		case scType_Float:
		{
			const f32 other = scStack_GetFloat(stack, 0);
			*result /= other;
			break;
		}
		default:
		{
			s_data.m_pool.Free(result);
			return scStack_ThrowError(stack, "Attempted to apply / operator on vec3 using unsupported type");
		}
	}

	return scStack_PushObj(stack, &s_vec3_class, result);
}

ueBool vec3_mul_op(ueVec3* obj, scStack* stack)
{
	ueVec3* result = (ueVec3*) s_data.m_pool.Alloc();
	if (!result)
		return scStack_ThrowError(stack, "Failed to allocate new vec3 instance.");

	*result = *obj;

	switch (scStack_GetType(stack, 0))
	{
		case scType_Int:
		case scType_Float:
		{
			const f32 other = scStack_GetFloat(stack, 0);
			*result *= other;
			break;
		}
		default:
		{
			s_data.m_pool.Free(result);
			return scStack_ThrowError(stack, "Attempted to apply * operator on vec3 using unsupported type");
		}
	}

	return scStack_PushObj(stack, &s_vec3_class, result);
}

ueBool vec3_get_accessor_index(scStack* stack, s32& index)
{
	const scType type = scStack_GetType(stack, 0);

	index = -1;
	switch (type)
	{
		case scType_String:
		{
			const char* buffer = scStack_GetString(stack, 0);
			switch (buffer[0])
			{
				case 'x': index = 0; break;
				case 'y': index = 1; break;
				case 'z': index = 2; break;
				default:
					return scStack_ThrowError(stack, "Attempted to get vec3 component at string index '%s'", buffer);
			}
			break;
		}
		case scType_Int:
			index = scStack_GetInt(stack, 0);
			if (index < 0 || 3 <= index)
				return scStack_ThrowError(stack, "Attempted to get vec3 component at int index %d", index);
			break;
		default:
			return scStack_ThrowError(stack, "Attempted to get vec3 component at index of unsupported type %d", type);
	}
	return UE_TRUE;
}

ueBool vec3_get_op(ueVec3* obj, scStack* stack)
{
	s32 index;
	if (!vec3_get_accessor_index(stack, index))
		return UE_FALSE;
	return scStack_PushFloat(stack, (*obj)[index]);
}

ueBool vec3_set_op(ueVec3* obj, scStack* stack)
{
	s32 index;
	if (!vec3_get_accessor_index(stack, index))
		return UE_FALSE;

	f32 value = 0;
	if (!scStack_GetFloat(stack, 1, value))
		return scStack_ThrowError(stack, "Failed to get float parameter for assignment operator");

	(*obj)[index] = value;
	return UE_TRUE;
}

ueBool vec3_dot(scStack* stack)
{
	ueVec3* a = NULL;
	ueVec3* b = NULL;
	if (!scStack_GetObj(stack, 0, &s_vec3_class, (void*&) a) || !scStack_GetObj(stack, 1, &s_vec3_class, (void*&) b))
		return scStack_ThrowError(stack, "Failed to get 2 vec3 parameters for dot function");
	return scStack_PushFloat(stack, ueVec3::Dot(*a, *b));
}

ueBool vec3_cross(scStack* stack)
{
	ueVec3* a = NULL;
	ueVec3* b = NULL;
	if (!scStack_GetObj(stack, 0, &s_vec3_class, (void*&) a) || !scStack_GetObj(stack, 1, &s_vec3_class, (void*&) b))
		return scStack_ThrowError(stack, "Failed to get 2 vec3 parameters for cross function");

	ueVec3* result = (ueVec3*) s_data.m_pool.Alloc();
	if (!result)
		return scStack_ThrowError(stack, "Failed to allocate new vec3 instance.");

	ueVec3::Cross(*result, *a, *b);
	return scStack_PushObj(stack, &s_vec3_class, result);
}

ueBool vec3_lerp(scStack* stack)
{
	ueVec3* a = NULL;
	ueVec3* b = NULL;
	if (!scStack_GetObj(stack, 0, &s_vec3_class, (void*&) a) || !scStack_GetObj(stack, 1, &s_vec3_class, (void*&) b))
		return scStack_ThrowError(stack, "Failed to get 2 vec3 parameters for lerp function");
	f32 scale;
	if (!scStack_GetFloat(stack, 2, scale))
		return scStack_ThrowError(stack, "Failed to get 3rd float param for vec3 lerp function");

	ueVec3* result = (ueVec3*) s_data.m_pool.Alloc();
	if (!result)
		return scStack_ThrowError(stack, "Failed to allocate new vec3 instance.");

	ueVec3::Lerp(*result, *a, *b, scale);
	return scStack_PushObj(stack, &s_vec3_class, result);
}

ueBool vec3_dist(scStack* stack)
{
	ueVec3* a = NULL;
	ueVec3* b = NULL;
	if (!scStack_GetObj(stack, 0, &s_vec3_class, (void*&) a) || !scStack_GetObj(stack, 1, &s_vec3_class, (void*&) b))
		return scStack_ThrowError(stack, "Failed to get 2 vec3 parameters for dist function");
	return scStack_PushFloat(stack, ueVec3::Dist(*a, *b));
}

// Initialization

void scInitLib_vec3(ueAllocator* allocator, u32 maxInstances)
{
	UE_ASSERT(!s_data.m_isInitialized);
	UE_ASSERT_FUNC(s_data.m_pool.Init(allocator, sizeof(ueVec3), maxInstances));
	s_data.m_isInitialized = UE_TRUE;
}

void scDeinitLib_vec3()
{
	UE_ASSERT(s_data.m_isInitialized);
	s_data.m_pool.Deinit();
	s_data.m_isInitialized = UE_FALSE;
}

ueBool scRegisterLib_vec3(scCtx* ctx)
{
	UE_ASSERT(s_data.m_isInitialized);
	return scCtx_RegisterClass(ctx, &s_vec3_class);
}

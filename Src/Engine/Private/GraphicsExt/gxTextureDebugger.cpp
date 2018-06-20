#include "Base/ueEnv.h"
#include "Input/inSys.h"
#include "GraphicsExt/gxDebugMenu.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "Graphics/glLib.h"
#include "Base/ueSorting.h"

struct gxTextureDebugger
{
	ueAllocator* m_allocator;

	ueVar* m_enable;
	ueVar* m_showR;
	ueVar* m_showG;
	ueVar* m_showB;
	ueVar* m_showA;
	ueVar* m_name;
	ueVar* m_index;

	gxTextureDebugger() :
		m_allocator(NULL),
		m_enable(NULL),
		m_showR(NULL),
		m_showG(NULL),
		m_showB(NULL),
		m_showA(NULL),
		m_name(NULL),
		m_index(NULL)
	{}
};

static gxTextureDebugger* s_data = NULL;

void gxTextureDebugger_Next(const char* actionName, void* userData);
void gxTextureDebugger_Previous(const char* actionName, void* userData);

void gxTextureDebugger_Startup(ueAllocator* allocator)
{
	UE_ASSERT(!s_data);
	s_data = new(allocator) gxTextureDebugger();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;

	s_data->m_enable = ueVar_CreateBool("debug.textures.enable", UE_FALSE);
	s_data->m_showR = ueVar_CreateBool("debug.textures.show_red", UE_TRUE);
	s_data->m_showG = ueVar_CreateBool("debug.textures.show_green", UE_TRUE);
	s_data->m_showB = ueVar_CreateBool("debug.textures.show_blue", UE_TRUE);
	s_data->m_showA = ueVar_CreateBool("debug.textures.show_alpha", UE_TRUE);
	s_data->m_name = ueVar_CreateString("debug.textures.name", GL_MAX_TEXTURE_BUFFER_DEBUG_NAME);
	s_data->m_index = ueVar_CreateString("debug.textures.index", 16);

	gxDebugMenuItem* debugMenu = gxDebugMenu_GetItem("Debug");
	gxDebugMenuItem* textureDebuggerMenu = gxDebugMenu_CreateMenu(debugMenu, "Textures");
	gxDebugMenu_CreateVar(textureDebuggerMenu, s_data->m_enable);
	gxDebugMenu_CreateVar(textureDebuggerMenu, s_data->m_showR);
	gxDebugMenu_CreateVar(textureDebuggerMenu, s_data->m_showG);
	gxDebugMenu_CreateVar(textureDebuggerMenu, s_data->m_showB);
	gxDebugMenu_CreateVar(textureDebuggerMenu, s_data->m_showA);
	gxDebugMenu_CreateVar(textureDebuggerMenu, s_data->m_name);
	gxDebugMenu_CreateVar(textureDebuggerMenu, s_data->m_index);
	gxDebugMenu_CreateUserAction(textureDebuggerMenu, "Next texture", gxTextureDebugger_Next);
	gxDebugMenu_CreateUserAction(textureDebuggerMenu, "Previous texture", gxTextureDebugger_Previous);
}

void gxTextureDebugger_Shutdown()
{
	UE_ASSERT(s_data);
	ueVar_Destroy(s_data->m_index);
	ueVar_Destroy(s_data->m_name);
	ueVar_Destroy(s_data->m_showA);
	ueVar_Destroy(s_data->m_showB);
	ueVar_Destroy(s_data->m_showG);
	ueVar_Destroy(s_data->m_showR);
	ueVar_Destroy(s_data->m_enable);
	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

void gxTextureDebugger_Update(f32 dt)
{
	UE_ASSERT(s_data);
	if (!ueVar_GetBool(s_data->m_enable))
		return;
}

void gxTextureDebugger_Draw()
{
	UE_ASSERT(s_data);

	if (!ueVar_GetBool(s_data->m_enable))
		return;

	// Find texture buffer by name

	u32 count;
	glTextureBuffer* array[1024];
	glDevice_GetTextureBuffers(array, UE_ARRAY_SIZE(array), glTextureBufferFlags_IsTexture, count);

	const char* currentTextureName = ueVar_GetString(s_data->m_name);

	glTextureBuffer* tb = NULL;
	for (u32 i = 0; i < count; i++)
		if (!ueStrCmp(glTextureBuffer_GetDebugName(array[i]), currentTextureName))
		{
			tb = array[i];
			break;
		}
	if (!tb)
		return;

	const glTextureBufferDesc* desc = glTextureBuffer_GetDesc(tb);

	// Set up debug rendering parameters

	const glDeviceStartupParams* devParams = glDevice_GetStartupParams();

	gxShapeDrawParams drawParams;
	drawParams.m_2DCanvas.Set(0, 0, (f32) devParams->m_width, (f32) devParams->m_height);
	gxShapeDraw_SetDrawParams(&drawParams);

	// Draw texture

	ueVec4 color(
		ueVar_GetBool(s_data->m_showR) ? 1.0f : 0.0f,
		ueVar_GetBool(s_data->m_showG) ? 1.0f : 0.0f,
		ueVar_GetBool(s_data->m_showB) ? 1.0f : 0.0f,
		ueVar_GetBool(s_data->m_showA) ? 1.0f : 0.0f);

	gxShape_TexturedRect rect;
	rect.m_colorMap = tb;
	rect.m_color = &color;
	rect.m_pos.Set(0, 0, (f32) desc->m_width, (f32) desc->m_height);

	gxShapeDraw_DrawTexturedRect(rect);
}

struct gxTextureBufferCmp
{
	UE_INLINE s32 operator () (const glTextureBuffer* a, const glTextureBuffer* b) const
	{
		s32 cmp = ueStrCmp(glTextureBuffer_GetDebugName(a), glTextureBuffer_GetDebugName(b));
		if (cmp)
			return cmp;
		return ueCmpPointers(a, b);
	}
};

void gxTextureDebugger_Scroll(ueBool next)
{
	// Get all textures

	u32 count;
	glTextureBuffer* array[1024];
	glDevice_GetTextureBuffers(array, UE_ARRAY_SIZE(array), glTextureBufferFlags_IsTexture, count);
	ueSort(array, count, gxTextureBufferCmp());

	// Get current texture name

	const char* currentTextureName = ueVar_GetString(s_data->m_name);

	// Switch to next or previous texture

	s32 index;
	if (next)
	{
		index = 0;
		while (index < (s32) count)
		{
			if (ueStrCmp(currentTextureName, glTextureBuffer_GetDebugName(array[index])) < 0)
				break;
			index++;
		}
		index = (index % count);
	}
	else
	{
		index = count - 1;
		while (index >= 0)
		{
			if (ueStrCmp(currentTextureName, glTextureBuffer_GetDebugName(array[index])) > 0)
				break;
			index--;
		}
		if (index == -1)
			index = count - 1;
	}

	// Store the resul by setting up vars appropriately

	ueVar_SetString(s_data->m_name, glTextureBuffer_GetDebugName(array[index]));

	char indexString[16];
	ueStrFormatS(indexString, "%u / %u", index + 1, count);
	ueVar_SetString(s_data->m_index, indexString);
}

void gxTextureDebugger_Next(const char* actionName, void* userData)
{
	gxTextureDebugger_Scroll(UE_TRUE);
}

void gxTextureDebugger_Previous(const char* actionName, void* userData)
{
	gxTextureDebugger_Scroll(UE_FALSE);
}
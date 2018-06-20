#include "UI/uiLib.h"
#include "Input/inSys.h"
#include "IO/ioFile.h"
#include "Base/Containers/ueGenericPool.h"
#include "Graphics/glVertexBufferFactory.h"
#include "Graphics/glIndexBufferFactory.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "GraphicsExt/gxShapeDraw_Vertex.h"
#include "GraphicsExt/gxTexture.h"

#include <Rocket/Core.h>
#include <Rocket/Controls.h>
#include <Rocket/Debugger.h>

struct uiLibData
{
	ueAllocator* m_allocator;
	uePath m_root;
	Rocket::Core::Context* m_ctx;
	ueMat44 m_proj;

	s32 m_mouseX;
	s32 m_mouseY;

	s32 m_width;
	s32 m_height;

	f32 m_elapsedTime;

	ueBool m_enableInput;

	u32 m_numDrawCalls;
	u32 m_numDrawTris;

	ueGenericPool m_eventListenersPool;
	uiEventListener m_eventListener;
	void* m_userData;

	uiLibData() : m_allocator(NULL) {}
};

static uiLibData s_data;

class uiLib_SystemInterface : public Rocket::Core::SystemInterface
{
public:
	virtual float GetElapsedTime()
	{
		return s_data.m_elapsedTime;
	}

	virtual int TranslateString(Rocket::Core::String& translated, const Rocket::Core::String& input)
	{
		if (input.Length() >= 2 && input[0] == '[' && input[input.Length() - 1] == ']')
		{
			translated = UE_LOC(input.CString());
			return 1;
		}

		translated = input;
		return 0;
	}

	virtual bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message)
	{
#if defined(UE_ENABLE_ASSERTION)
		if (Rocket::Core::Log::LT_ASSERT == type)
			UE_ASSERT_MSGP(0, "libRocket assert: %s", message.CString());
#endif

		ueLoggingLevel level = ueLoggingLevel_Info;
		switch (type)
		{
			case Rocket::Core::Log::LT_DEBUG:
				level = ueLoggingLevel_Debug;
				break;
			case Rocket::Core::Log::LT_WARNING:
				level = ueLoggingLevel_Warning;
				break;
			case Rocket::Core::Log::LT_ERROR:
			case Rocket::Core::Log::LT_ASSERT:
				level = ueLoggingLevel_Error;
				break;
		}

		ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, level).Log("libRocket: %s", message.CString());
		return true;
	}
};

class uiLib_RenderInterface : public Rocket::Core::RenderInterface
{
public:
	struct Texture
	{
		ueResourceHandle<gxTexture> m_tex; // Loaded from file
		glTextureBuffer* m_tb; // Dynamically generated

		Texture() : m_tb(NULL) {}
	};

	struct Geometry
	{
		glVertexBuffer* m_vb;
		u32 m_numVerts;
		glIndexBuffer* m_ib;
		u32 m_numIndices;
		glTextureBuffer* m_tb;
	};

	ueGenericPool m_texturesPool;
	ueGenericPool m_geometriesPool;

	glVertexBufferFactory m_VB;
	glIndexBufferFactory m_IB;

	glStreamFormat* m_SF;

	void Init(uiLibStartupParams* params)
	{
		m_texturesPool.Init(s_data.m_allocator, sizeof(Texture), params->m_maxTextures);
		m_geometriesPool.Init(s_data.m_allocator, sizeof(Geometry), params->m_maxGeometries);
		m_VB.Init(params->m_dynamicVBSize);
		m_IB.Init(params->m_numDynamicIBIndices, sizeof(u32));

		// Create stream format matching Rocket::Core::Vertex

		const glVertexElement sfElems[] =
		{
			{glSemantic_Position, 0, ueNumType_F32, 2, UE_FALSE, UE_OFFSET_OF(Rocket::Core::Vertex, position)},
			{glSemantic_Color, 0, ueNumType_U8, 4, UE_TRUE, UE_OFFSET_OF(Rocket::Core::Vertex, colour)},
			{glSemantic_TexCoord, 0, ueNumType_F32, 2, UE_FALSE, UE_OFFSET_OF(Rocket::Core::Vertex, tex_coord)}
		};
		glStreamFormatDesc sfDesc;
		sfDesc.m_stride = sizeof(Rocket::Core::Vertex);
		sfDesc.m_numElements = UE_ARRAY_SIZE(sfElems);
		sfDesc.m_elements = sfElems;
		m_SF = glStreamFormat_Create(&sfDesc);
		UE_ASSERT(m_SF);
	}

	void Deinit()
	{
		glStreamFormat_Destroy(m_SF);
		m_VB.Deinit();
		m_IB.Deinit();
		m_geometriesPool.Deinit();
		m_texturesPool.Deinit();
	}

	virtual float GetHorizontalTexelOffset()
	{
#if defined(GL_D3D9)
		return -0.5f;
#else
		return 0.0f;
#endif
	}

	virtual float GetVerticalTexelOffset()
	{
#if defined(GL_D3D9)
		return -0.5f;
#else
		return 0.0f;
#endif
	}

	virtual void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture_handle, const Rocket::Core::Vector2f& translation)
	{
		UE_PROF_SCOPE("uiLib::RenderGeometry");

		s_data.m_numDrawCalls++;
		s_data.m_numDrawTris += num_indices / 3;

		// Copy index and vertex data

		glVertexBufferFactory::LockedChunk vbChunk;
		UE_ASSERT_FUNC(m_VB.Allocate(sizeof(Rocket::Core::Vertex) * num_vertices, &vbChunk));
		ueMemCpy(vbChunk.m_data, vertices, sizeof(Rocket::Core::Vertex) * num_vertices);
		m_VB.Unlock(&vbChunk);
	
		glIndexBufferFactory::LockedChunk ibChunk;
		UE_ASSERT_FUNC(m_IB.Allocate(sizeof(u32) * num_indices, &ibChunk));
		ueMemCpy(ibChunk.m_data, indices, sizeof(u32) * num_indices);
		m_IB.Unlock(&ibChunk);

		// Draw

		if (texture_handle)
			glCtx_SetSamplerConstant(m_ctx, gxCommonConstants::ColorMap, ((Texture*) texture_handle)->m_tb);

		ueMat44 world, wvp;
		world.SetTranslation(translation.x, translation.y, 0.0f);
		ueMat44::Mul(wvp, world, s_data.m_proj);
		glCtx_SetFloat4x4Constant(m_ctx, gxCommonConstants::WorldViewProj, &wvp);

		glCtx_SetProgram(m_ctx, (texture_handle ? gxPosColTexVertex::m_program : gxPosColVertex::m_program).GetProgram());

		glCtx_SetIndices(m_ctx, ibChunk.m_IB);
		glCtx_SetStream(m_ctx, 0, vbChunk.m_VB, m_SF, vbChunk.m_offset);
		glCtx_DrawIndexed(m_ctx, glPrimitive_TriangleList, 0, 0, num_vertices, ibChunk.m_firstIndex, num_indices);
	}

	virtual Rocket::Core::CompiledGeometryHandle CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture_handle)
	{
		Geometry* geom = new(m_geometriesPool) Geometry;
		UE_ASSERT(geom);

		// Create index buffer

		geom->m_numIndices = num_indices;

		glIndexBufferDesc ibDesc;
		UE_ASSERT(num_vertices < (1 << 16));
		ibDesc.m_indexSize = sizeof(u16);
		ibDesc.m_numIndices = num_indices;
		geom->m_ib = glIndexBuffer_Create(&ibDesc);
		UE_ASSERT(geom->m_ib);
		u16* ibData = (u16*) glIndexBuffer_Lock(geom->m_ib);
		for (s32 i = 0; i < num_indices; i++)
			ibData[i] = indices[i];
		glIndexBuffer_Unlock(geom->m_ib);

		// Create vertex buffer

		geom->m_numVerts = num_vertices;

		glVertexBufferDesc vbDesc;
		vbDesc.m_size = sizeof(Rocket::Core::Vertex) * num_vertices;
		geom->m_vb = glVertexBuffer_Create(&vbDesc, vertices);
		UE_ASSERT(geom->m_vb);

		// Assign texture

		geom->m_tb = texture_handle ? ((Texture*) texture_handle)->m_tb : NULL;

		return geom;
	}

	virtual void RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry_handle, const Rocket::Core::Vector2f& translation)
	{
		UE_PROF_SCOPE("uiLib::RenderCompiledGeometry");

		Geometry* geometry = (Geometry*) geometry_handle;

		s_data.m_numDrawCalls++;
		s_data.m_numDrawTris += geometry->m_numIndices / 3;

		if (geometry->m_tb)
			glCtx_SetSamplerConstant(m_ctx, gxCommonConstants::ColorMap, geometry->m_tb);

		ueMat44 world, wvp;
		world.SetTranslation(translation.x, translation.y, 0.0f);
		ueMat44::Mul(wvp, world, s_data.m_proj);
		glCtx_SetFloat4x4Constant(m_ctx, gxCommonConstants::WorldViewProj, &wvp);

		glCtx_SetProgram(m_ctx, (geometry->m_tb ? gxPosColTexVertex::m_program : gxPosColVertex::m_program).GetProgram());

		glCtx_SetIndices(m_ctx, geometry->m_ib);
		glCtx_SetStream(m_ctx, 0, geometry->m_vb, m_SF);
		glCtx_DrawIndexed(m_ctx, glPrimitive_TriangleList, 0, 0, geometry->m_numVerts, 0, geometry->m_numIndices);
	}

	virtual void ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry_handle)
	{
		Geometry* geometry = (Geometry*) geometry_handle;
		glIndexBuffer_Destroy(geometry->m_ib);
		glVertexBuffer_Destroy(geometry->m_vb);
		m_geometriesPool.Free(geometry);
	}

	virtual void EnableScissorRegion(bool enable)
	{
		glCtx_SetScissorTest(m_ctx, enable ? UE_TRUE : UE_FALSE);
	}

	virtual void SetScissorRegion(int x, int y, int width, int height)
	{
		glCtx_SetScissorRectangle(m_ctx, x, y, width, height);
	}

	virtual bool LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions, const Rocket::Core::String& source)
	{
		uePath fullPath;
		ueStrFormatS(fullPath, "%s%s", s_data.m_root, source.CString());

		Texture* t = new(m_texturesPool) Texture();
		UE_ASSERT(t);
		if (!t->m_tex.SetByName(fullPath, UE_TRUE))
		{
			m_texturesPool.Free(t);
			return false;
		}
		t->m_tb = gxTexture_GetBuffer(*t->m_tex);

		const glTextureBufferDesc* desc = glTextureBuffer_GetDesc(t->m_tb);

		texture_handle = t;
		texture_dimensions.x = desc->m_width;
		texture_dimensions.y = desc->m_height;
		return true;
	}

	virtual bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source, const Rocket::Core::Vector2i& source_dimensions)
	{
		glTextureBufferDesc desc;
		desc.m_numLevels = 1;
		desc.m_format = glBufferFormat_Native_R8G8B8A8;
		desc.m_width = source_dimensions.x;
		desc.m_height = source_dimensions.y;

		if (glBufferFormat_Native_R8G8B8A8 != glBufferFormat_R8G8B8A8)
		{
			// TODO: Swap R with B into temp buffer
		}

		glTextureData data;
		data.m_data = (void*) source;
		data.m_rowPitch = desc.m_width * 4;

		glTextureInitData initData;
		initData.m_storage = glTexStorage_GPU;
		initData.m_dataArrayLength = 1;
		initData.m_dataArray = &data;

		glTextureBuffer* tb = glTextureBuffer_Create(&desc, &initData);
		if (!tb)
			return false;

		Texture* t = new(m_texturesPool) Texture();
		UE_ASSERT(t);
		t->m_tb = tb;

		texture_handle = t;
		return true;
	}

	virtual void ReleaseTexture(Rocket::Core::TextureHandle texture_handle)
	{
		Texture* t = (Texture*) texture_handle;
		t->m_tex.Release();
		if (t->m_tb)
			glTextureBuffer_Destroy(t->m_tb);
		m_texturesPool.Free(t);
	}

	glCtx* m_ctx;
};

class uiLib_FileInterface : public Rocket::Core::FileInterface
{
public:
	virtual Rocket::Core::FileHandle Open(const Rocket::Core::String& path)
	{
		uePath fullPath;
		ueStrFormatS(fullPath, "%s%s", s_data.m_root, path.CString());
		return ioFile_Open(fullPath, ioFileOpenFlags_Read);
	}

	virtual void Close(Rocket::Core::FileHandle file)
	{
		ioFile_Close((ioFile*) file);
	}

	virtual size_t Read(void* buffer, size_t size, Rocket::Core::FileHandle file)
	{
		return (size_t) ioFile_Read((ioFile*) file, buffer, 1, size);
	}

	virtual bool Seek(Rocket::Core::FileHandle file, long offset, int origin)
	{
		ioFileSeekType seekType;
		switch (origin)
		{
			case SEEK_SET: seekType = ioFileSeekType_Set; break;
			case SEEK_END: seekType = ioFileSeekType_End; break;
			case SEEK_CUR: seekType = ioFileSeekType_Offset; break;
		}
		return ioFile_Seek((ioFile*) file, seekType, offset);
	}

	virtual size_t Tell(Rocket::Core::FileHandle file)
	{
		return (size_t) ioFile_GetOffset((ioFile*) file);
	}
};

class uiLib_EventListener : public Rocket::Core::EventListener
{
public:
	uiLib_EventListener(const Rocket::Core::String& value) :
		m_value(value)
	{}

	virtual void ProcessEvent(Rocket::Core::Event& event)
	{
		if (!s_data.m_eventListener)
			return;

		uiEventType type;
		if (event == "click")
			type = uiEventType_OnClick;
		else
			return;

		s_data.m_eventListener(type, m_value.CString(), s_data.m_userData);
	}

private:
	Rocket::Core::String m_value;
};

class uiLib_EventInstancer : public Rocket::Core::EventListenerInstancer
{
public:
	virtual Rocket::Core::EventListener* InstanceEventListener(const Rocket::Core::String& value)
	{
		uiLib_EventListener* listener = new(s_data.m_eventListenersPool) uiLib_EventListener(value);
		UE_ASSERT(listener);
		return listener;
	}
	virtual void Release()
	{
		ueDelete(this, s_data.m_eventListenersPool);
	}
};

static uiLib_FileInterface s_fileInterface;
static uiLib_RenderInterface s_renderInterface;
static uiLib_SystemInterface s_systemInterface;
static uiLib_EventInstancer s_eventInstancer;

void uiLib_Startup(uiLibStartupParams* params)
{
	UE_ASSERT(!s_data.m_allocator);

	s_data.m_allocator = params->m_allocator;
	ueStrCpyS(s_data.m_root, params->m_rootAssetPath);

	s_renderInterface.Init(params);

	s_data.m_enableInput = UE_TRUE;
	s_data.m_mouseX = s_data.m_mouseY = 0;
	s_data.m_elapsedTime = 0.0f;

	s_data.m_eventListener = NULL;
	s_data.m_eventListenersPool.Init(params->m_allocator, sizeof(uiLib_EventListener), params->m_maxEvents);

	// Rocket initialisation

	Rocket::Core::SetRenderInterface(&s_renderInterface);
	Rocket::Core::SetSystemInterface(&s_systemInterface);
	Rocket::Core::SetFileInterface(&s_fileInterface);

	Rocket::Core::Initialise();
	Rocket::Controls::Initialise();

	const glDeviceStartupParams* devParams = glDevice_GetStartupParams();
	s_data.m_width = devParams->m_width;
	s_data.m_height = devParams->m_height;

	s_data.m_ctx = Rocket::Core::CreateContext("main", Rocket::Core::Vector2i(s_data.m_width, s_data.m_height));
	UE_ASSERT(s_data.m_ctx);

	if (params->m_enableDebugger)
		Rocket::Debugger::Initialise(s_data.m_ctx);

	Rocket::Core::Factory::RegisterEventListenerInstancer(&s_eventInstancer);
}

void uiLib_Shutdown()
{
	UE_ASSERT(s_data.m_allocator);

	s_data.m_ctx->RemoveReference();
	Rocket::Core::Shutdown();

	s_renderInterface.Deinit();
	s_data.m_eventListenersPool.Deinit();
	s_data.m_allocator = NULL;
}

void uiLib_SetDimensions(u32 width, u32 height)
{
	UE_ASSERT(s_data.m_allocator);

	s_data.m_width = width;
	s_data.m_height = height;

	s_data.m_ctx->SetDimensions(Rocket::Core::Vector2i(width, height));
}

void uiLib_EnableInput(ueBool enable)
{
	UE_ASSERT(s_data.m_allocator);
	s_data.m_enableInput = enable;
}

void uiLib_SetEventListener(uiEventListener listener, void* userData)
{
	s_data.m_eventListener = listener;
	s_data.m_userData = userData;
}

ueBool uiLib_LoadCursor(const char* name)
{
	uePath pathRml;
	ueStrFormatS(pathRml, "%s.rml", name);
	Rocket::Core::ElementDocument* cursor = s_data.m_ctx->LoadMouseCursor(pathRml);
	if (cursor)
		cursor->RemoveReference();
	return cursor != NULL;
}

void uiLib_Update(f32 dt)
{
	UE_ASSERT(s_data.m_allocator);

	s_data.m_elapsedTime += dt;

	if (s_data.m_enableInput)
	{
		const u32 userIndex = 0;

		// Determine modifiers state

		int modifierState = 0;
		if (inKeyboard_IsDown(userIndex, inKey_Capital)) modifierState |= Rocket::Core::Input::KM_CAPSLOCK;
		if (inKeyboard_IsDown(userIndex, inKey_Alt)) modifierState |= Rocket::Core::Input::KM_ALT;
		if (inKeyboard_IsDown(userIndex, inKey_Control)) modifierState |= Rocket::Core::Input::KM_CTRL;
		if (inKeyboard_IsDown(userIndex, inKey_Shift)) modifierState |= Rocket::Core::Input::KM_SHIFT;

		// Process keyboard events

	#define UI_HANDLE_KEY(engineName, uiName) \
		if (inKeyboard_WasPressed(userIndex, inKey_##engineName))		s_data.m_ctx->ProcessKeyDown(Rocket::Core::Input::KI_##uiName, modifierState); \
		else if (inKeyboard_WasReleased(userIndex, inKey_##engineName)) s_data.m_ctx->ProcessKeyUp(Rocket::Core::Input::KI_##uiName, modifierState);

		UI_HANDLE_KEY(Space, SPACE);
		UI_HANDLE_KEY(0, 0);
		UI_HANDLE_KEY(1, 1);
		UI_HANDLE_KEY(2, 2);
		UI_HANDLE_KEY(3, 3);
		UI_HANDLE_KEY(4, 4);
		UI_HANDLE_KEY(5, 5);
		UI_HANDLE_KEY(6, 6);
		UI_HANDLE_KEY(7, 7);
		UI_HANDLE_KEY(8, 8);
		UI_HANDLE_KEY(9, 9);
		UI_HANDLE_KEY(A, A);
		UI_HANDLE_KEY(B, B);
		UI_HANDLE_KEY(C, C);
		UI_HANDLE_KEY(D, D);
		UI_HANDLE_KEY(E, E);
		UI_HANDLE_KEY(F, F);
		UI_HANDLE_KEY(G, G);
		UI_HANDLE_KEY(H, H);
		UI_HANDLE_KEY(I, I);
		UI_HANDLE_KEY(J, J);
		UI_HANDLE_KEY(K, K);
		UI_HANDLE_KEY(L, L);
		UI_HANDLE_KEY(M, M);
		UI_HANDLE_KEY(N, N);
		UI_HANDLE_KEY(O, O);
		UI_HANDLE_KEY(P, P);
		UI_HANDLE_KEY(Q, Q);
		UI_HANDLE_KEY(R, R);
		UI_HANDLE_KEY(S, S);
		UI_HANDLE_KEY(T, T);
		UI_HANDLE_KEY(U, U);
		UI_HANDLE_KEY(V, V);
		UI_HANDLE_KEY(W, W);
		UI_HANDLE_KEY(X, X);
		UI_HANDLE_KEY(Y, Y);
		UI_HANDLE_KEY(Z, Z);
		UI_HANDLE_KEY(Semicolon, OEM_1);
		UI_HANDLE_KEY(Equal, OEM_PLUS);
		UI_HANDLE_KEY(Comma, OEM_COMMA);
		UI_HANDLE_KEY(Minus, OEM_MINUS);
		UI_HANDLE_KEY(Period, OEM_PERIOD);
		UI_HANDLE_KEY(Slash, OEM_2);
		UI_HANDLE_KEY(Tilde, OEM_3);
		UI_HANDLE_KEY(Numpad0, NUMPAD0);
		UI_HANDLE_KEY(Numpad1, NUMPAD1);
		UI_HANDLE_KEY(Numpad2, NUMPAD2);
		UI_HANDLE_KEY(Numpad3, NUMPAD3);
		UI_HANDLE_KEY(Numpad4, NUMPAD4);
		UI_HANDLE_KEY(Numpad5, NUMPAD5);
		UI_HANDLE_KEY(Numpad6, NUMPAD6);
		UI_HANDLE_KEY(Numpad7, NUMPAD7);
		UI_HANDLE_KEY(Numpad8, NUMPAD8);
		UI_HANDLE_KEY(Numpad9, NUMPAD9);
		UI_HANDLE_KEY(Back, BACK);
		UI_HANDLE_KEY(Tab, TAB);
		UI_HANDLE_KEY(Return, RETURN);
		UI_HANDLE_KEY(Pause, PAUSE);
		UI_HANDLE_KEY(Capital, CAPITAL);
		UI_HANDLE_KEY(Escape, ESCAPE);
		UI_HANDLE_KEY(PageUp, PRIOR);
		UI_HANDLE_KEY(PageDown, NEXT);
		UI_HANDLE_KEY(End, END);
		UI_HANDLE_KEY(Home, HOME);
		UI_HANDLE_KEY(Left, LEFT);
		UI_HANDLE_KEY(Right, RIGHT);
		UI_HANDLE_KEY(Down, DOWN);
		UI_HANDLE_KEY(Insert, INSERT);
		UI_HANDLE_KEY(Delete, DELETE);
		UI_HANDLE_KEY(LeftWindows, LWIN);
		UI_HANDLE_KEY(RightWindows, RWIN);
		UI_HANDLE_KEY(AppMenu, APPS);
		UI_HANDLE_KEY(F1, F1);
		UI_HANDLE_KEY(F2, F2);
		UI_HANDLE_KEY(F3, F3);
		UI_HANDLE_KEY(F4, F4);
		UI_HANDLE_KEY(F5, F5);
		UI_HANDLE_KEY(F6, F6);
		UI_HANDLE_KEY(F7, F7);
		UI_HANDLE_KEY(F8, F8);
		UI_HANDLE_KEY(F9, F9);
		UI_HANDLE_KEY(F10, F10);
		UI_HANDLE_KEY(F11, F11);
		UI_HANDLE_KEY(F12, F12);

		// Process mouse events

		s32 x, y;
		inMouse_GetVector(x, y);
		if (x != 0 || y != 0)
		{
			s_data.m_mouseX += x;
			s_data.m_mouseY += y;

			s_data.m_mouseX = ueClamp(s_data.m_mouseX, 0, s_data.m_width - 1);
			s_data.m_mouseY = ueClamp(s_data.m_mouseY, 0, s_data.m_height - 1);

			s_data.m_ctx->ProcessMouseMove(s_data.m_mouseX, s_data.m_mouseY, 0);
		}

		s32 wheelDelta;
		inMouse_GetWheelDelta(wheelDelta);
		if (wheelDelta != 0)
			s_data.m_ctx->ProcessMouseWheel(wheelDelta, modifierState);

		if (inMouse_WasPressed(inMouseButton_Left))			
			s_data.m_ctx->ProcessMouseButtonDown(0, modifierState);
		else if (inMouse_WasReleased(inMouseButton_Left))	s_data.m_ctx->ProcessMouseButtonUp(0, modifierState);

		if (inMouse_WasPressed(inMouseButton_Middle))		s_data.m_ctx->ProcessMouseButtonDown(1, modifierState);
		else if (inMouse_WasReleased(inMouseButton_Middle))	s_data.m_ctx->ProcessMouseButtonUp(1, modifierState);

		if (inMouse_WasPressed(inMouseButton_Right))		s_data.m_ctx->ProcessMouseButtonDown(2, modifierState);
		else if (inMouse_WasReleased(inMouseButton_Right))	s_data.m_ctx->ProcessMouseButtonUp(2, modifierState);
	}

	// Update

	s_data.m_ctx->Update();
}

void uiLib_Draw(glCtx* ctx)
{
	UE_ASSERT(s_data.m_allocator);

	s_renderInterface.m_ctx = ctx;

	// Set common render states

	glCtx_SetDepthTest(ctx, UE_FALSE);
	glCtx_SetDepthWrite(ctx, UE_FALSE);

	glCtx_SetBlending(ctx, UE_TRUE);
	glCtx_SetBlendFunc(ctx, glBlendingFunc_SrcAlpha, glBlendingFunc_InvSrcAlpha);

	s_data.m_proj.SetOrthoOffCenter(0, (f32) s_data.m_width, 0, (f32) s_data.m_height, 0, 1, glCtx_IsRenderingToTexture(ctx));

	// Draw

	s_data.m_numDrawCalls = 0;
	s_data.m_numDrawTris = 0;
	s_data.m_ctx->Render();

	// Revert default render states

	glCtx_SetDepthTest(ctx, UE_TRUE);
	glCtx_SetDepthWrite(ctx, UE_TRUE);
}

ueBool uiLib_LoadFont(const char* name)
{
	return Rocket::Core::FontDatabase::LoadFontFace(name);
}

void uiLib_ShowDebugger(ueBool show)
{
	Rocket::Debugger::SetVisible(show);
}

ueBool uiLib_IsDebuggerVisible()
{
	return Rocket::Debugger::IsVisible();
}

uiScene* uiScene_Create(const char* name)
{
	uePath pathRml;
	ueStrFormatS(pathRml, "%s.rml", name);
	return (uiScene*) s_data.m_ctx->LoadDocument(pathRml);
}

void uiScene_Destroy(uiScene* scene)
{
	((Rocket::Core::ElementDocument*) scene)->RemoveReference();
}

void uiScene_SetVisible(uiScene* scene, ueBool visible)
{
	if (visible)
		((Rocket::Core::ElementDocument*) scene)->Show();
	else
		((Rocket::Core::ElementDocument*) scene)->Hide();
}
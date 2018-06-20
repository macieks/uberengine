#ifndef GX_TEXTURE_DEBUGGER
#define GX_TEXTURE_DEBUGGER

void gxTextureDebugger_Startup(ueAllocator* allocator);
void gxTextureDebugger_Shutdown();

void gxTextureDebugger_Update(f32 dt);
void gxTextureDebugger_Draw();

#endif // GX_TEXTURE_DEBUGGER
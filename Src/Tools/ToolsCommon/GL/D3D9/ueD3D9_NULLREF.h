#pragma once

#include "Base/ueToolsBase.h"
#include <d3dx9.h>

void ueD3D9_NULLREF_Startup();
void ueD3D9_NULLREF_Shutdown();
IDirect3DDevice9* ueD3D9_NULLREF_GetHandle();

#define D3DEV_NULLREF ueD3D9_NULLREF_GetHandle()
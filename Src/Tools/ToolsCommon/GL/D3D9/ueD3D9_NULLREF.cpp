#include "GL/D3D9/ueD3D9_NULLREF.h"

static IDirect3D9* s_d3d9 = NULL;
static IDirect3DDevice9* s_d3dev = NULL;
static u32 s_refCount = 0;

void ueD3D9_NULLREF_Startup()
{
	s_refCount++;
	if (s_refCount > 1)
		return;

	// Create D3D9

	s_d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (!s_d3d9)
	{
		ueLogE("Direct3DCreate9 failed.");
		ueExit(-1);
	}

	// Create D3D9 device

	D3DPRESENT_PARAMETERS pp;
	memset(&pp, 0, sizeof(pp));
	pp.Windowed = TRUE;
	pp.hDeviceWindow = GetConsoleWindow();
	pp.BackBufferWidth = 1;
	pp.BackBufferHeight = 1;
	pp.BackBufferCount = 1;
	pp.BackBufferFormat = D3DFMT_UNKNOWN;//D3DFMT_A8B8G8R8;
	pp.EnableAutoDepthStencil = FALSE;
	pp.AutoDepthStencilFormat = D3DFMT_D24S8;
	pp.MultiSampleType = D3DMULTISAMPLE_NONE;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.Flags = 0;
	pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	pp.FullScreen_RefreshRateInHz = 0;

	if (!SUCCEEDED(s_d3d9->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_NULLREF,
			NULL,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&pp,
			&s_d3dev)))
	{
		ueLogE("CreateDevice(...D3DDEVTYPE_NULLREF...) failed.");
		ueExit(-1);
	}
}

void ueD3D9_NULLREF_Shutdown()
{
	s_refCount--;
	if (s_refCount != 0)
		return;

	s_d3dev->Release();
	s_d3dev = NULL;
	s_d3d9->Release();
	s_d3d9 = NULL;
}

IDirect3DDevice9* ueD3D9_NULLREF_GetHandle()
{
	return s_d3dev;
}
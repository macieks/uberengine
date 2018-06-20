#include "Base/ueBase.h"

#if defined(GL_D3D9)

#include "Graphics/glLib_Private.h"

u32 glUtils_GetNumAdapters()
{
	UE_ASSERT_MSG(D3D, "Need to pre-startup render device - call glDevice::PreStartup()");
	return D3D->GetAdapterCount();
}

ueBool glUtils_GetAdapterInfo(u32 adapterIndex, glAdapterInfo* info)
{
	UE_ASSERT_MSG(D3D, "Need to pre-startup render device - call glDevice::PreStartup()");
	
	D3DADAPTER_IDENTIFIER9 d3dAdapter;
	if (FAILED(D3D->GetAdapterIdentifier(adapterIndex, 0, &d3dAdapter)))
		return UE_FALSE;
	
	info->m_flags = adapterIndex == D3DADAPTER_DEFAULT ? glAdapterInfo::Flags_IsPrimary : 0;
	info->m_numModes = D3D->GetAdapterModeCount(adapterIndex, D3DFMT_X8R8G8B8);
	ueStrCpy(info->m_driver, UE_ARRAY_SIZE(info->m_driver), d3dAdapter.Driver);
	ueStrCpy(info->m_description, UE_ARRAY_SIZE(info->m_description), d3dAdapter.Description);
	ueStrCpy(info->m_deviceName, UE_ARRAY_SIZE(info->m_deviceName), d3dAdapter.DeviceName);

	// Retrieve monitor info

	HMONITOR monitor = D3D->GetAdapterMonitor(adapterIndex);
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	if (!GetMonitorInfo(monitor, &monitorInfo))
	{
		info->m_monitorWidth = 0;
		info->m_monitorHeight = 0;
		return UE_FALSE;
	}

	info->m_flags |= glAdapterInfo::Flags_HasMonitor;
	info->m_flags |= (monitorInfo.dwFlags & MONITORINFOF_PRIMARY) != 0 ? glAdapterInfo::Flags_HasPrimaryMonitor : 0;
	info->m_monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	info->m_monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
	info->m_aspectRatio = (f32) info->m_monitorWidth / (f32) info->m_monitorHeight;
	return UE_TRUE;
}

ueBool glUtils_GetOutputMode(u32 adapterIndex, u32 modeIndex, glOutputMode* mode)
{
	UE_ASSERT_MSG(D3D, "Need to pre-startup render device - call glDevice::PreStartup()");

	D3DDISPLAYMODE d3dMode;
	if (FAILED(D3D->EnumAdapterModes(adapterIndex, D3DFMT_X8R8G8B8, modeIndex, &d3dMode)))
		return UE_FALSE;

	mode->m_width = d3dMode.Width;
	mode->m_height = d3dMode.Height;
	mode->m_frequency = d3dMode.RefreshRate;
	mode->m_bitsPerPixel = 32;
	return UE_TRUE;
}

#else // defined(GL_D3D9)
	UE_NO_EMPTY_FILE
#endif
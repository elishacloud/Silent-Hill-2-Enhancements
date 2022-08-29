/**
* Copyright (C) 2022 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "d3d8wrapper.h"

#define ATI_VENDOR_ID		0x1002	/* ATI Technologies Inc.			*/
#define NVIDIA_VENDOR_ID	0x10DE	/* NVIDIA Corporation				*/
#define MATROX_VENDOR_ID	0x102B	/* Matrox Electronic Systems Ltd.	*/
#define _3DFX_VENDOR_ID		0x121A	/* 3dfx Interactive Inc.			*/
#define S3_VENDOR_ID		0x5333	/* S3 Graphics Co., Ltd.			*/
#define INTEL_VENDOR_ID		0x8086	/* Intel Corporation				*/

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

WNDPROC OriginalWndProc = nullptr;
HWND DeviceWindow = nullptr;
LONG BufferWidth = 0, BufferHeight = 0;
DWORD VendorID = 0;
bool CopyRenderTarget = false;
bool SetSSAA = false;
bool TakeScreenShot = false;
D3DMULTISAMPLE_TYPE DeviceMultiSampleType = D3DMULTISAMPLE_NONE;
m_IDirect3DDevice8* IDirect3DDevice8Ptr = nullptr;

HRESULT m_IDirect3D8::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	if ((riid == IID_IDirect3D8 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG m_IDirect3D8::AddRef()
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3D8::Release()
{
	Logging::LogDebug() << __FUNCTION__;

	ULONG count = ProxyInterface->Release();

	if (count == 0)
	{
		delete this;
	}

	return count;
}

HRESULT m_IDirect3D8::EnumAdapterModes(THIS_ UINT Adapter, UINT Mode, D3DDISPLAYMODE* pMode)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->EnumAdapterModes(Adapter, Mode, pMode);
}

UINT m_IDirect3D8::GetAdapterCount()
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterCount();
}

HRESULT m_IDirect3D8::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT m_IDirect3D8::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 *pIdentifier)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT m_IDirect3D8::GetAdapterModeCount(THIS_ UINT Adapter)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterModeCount(Adapter);
}

HMONITOR m_IDirect3D8::GetAdapterMonitor(UINT Adapter)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterMonitor(Adapter);
}

HRESULT m_IDirect3D8::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8 *pCaps)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HRESULT m_IDirect3D8::RegisterSoftwareDevice(void *pInitializeFunction)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->RegisterSoftwareDevice(pInitializeFunction);
}

HRESULT m_IDirect3D8::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT m_IDirect3D8::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT m_IDirect3D8::CheckDeviceMultiSampleType(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType)
{
	Logging::LogDebug() << __FUNCTION__;

	if (ScreenMode != 3)
	{
		Windowed = true;
	}

	return ProxyInterface->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType);
}

HRESULT m_IDirect3D8::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{
	Logging::LogDebug() << __FUNCTION__;

	if (ScreenMode != 3)
	{
		Windowed = true;
	}

	return ProxyInterface->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}

DWORD GetBitCount(D3DFORMAT Format)
{
	switch (Format)
	{
	case D3DFMT_UNKNOWN:
		break;
	case D3DFMT_R3G3B2:
	case D3DFMT_A8:
	case D3DFMT_P8:
	case D3DFMT_L8:
	case D3DFMT_A4L4:
		return 8;
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_A4R4G4B4:
	case D3DFMT_A8R3G3B2:
	case D3DFMT_X4R4G4B4:
	case D3DFMT_A8P8:
	case D3DFMT_A8L8:
	case D3DFMT_V8U8:
	case D3DFMT_L6V5U5:
	case D3DFMT_D16_LOCKABLE:
	case D3DFMT_D15S1:
	case D3DFMT_D16:
	case D3DFMT_UYVY:
	case D3DFMT_YUY2:
		return 16;
	case D3DFMT_R8G8B8:
		return 24;
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A2B10G10R10:
	case D3DFMT_G16R16:
	case D3DFMT_X8L8V8U8:
	case D3DFMT_Q8W8V8U8:
	case D3DFMT_V16U16:
	case D3DFMT_A2W10V10U10:
	case D3DFMT_D32:
	case D3DFMT_D24S8:
	case D3DFMT_D24X8:
	case D3DFMT_D24X4S4:
		return 32;
	case D3DFMT_DXT1:
	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
	case D3DFMT_DXT4:
		break;
	}

	LOG_LIMIT(100, __FUNCTION__ << " Display format not Implemented: " << Format);
	return 0;
}

HRESULT m_IDirect3D8::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice8 **ppReturnedDeviceInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	// Get device information
	D3DADAPTER_IDENTIFIER8 dai;
	if (SUCCEEDED(ProxyInterface->GetAdapterIdentifier(Adapter, 0, &dai)))
	{
		VendorID = dai.VendorId;
		Logging::Log() << "|---------- VIDEO CARD ----------";
		Logging::Log() << "| Using video card: " << dai.Description << " (" << Logging::hex(dai.VendorId) << ")";
		Logging::Log() << "|--------------------------------";
	}

	// Check if render target needs to be replaced
	if (DeviceMultiSampleType || FixGPUAntiAliasing)
	{
		CopyRenderTarget = true;
	}

	// Check for SSAA
	if ((DeviceMultiSampleType || FixGPUAntiAliasing) &&
		(ProxyInterface->CheckDeviceFormat(Adapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A')) == S_OK || VendorID == NVIDIA_VENDOR_ID))
	{
		SetSSAA = true;
	}

	// Update presentation parameters
	UpdatePresentParameter(pPresentationParameters, hFocusWindow, true);

	// Set Silent Hill 2 window to forground
	SetForegroundWindow(DeviceWindow);

	HRESULT hr = D3DERR_INVALIDCALL;

	// Get multisample quality level
	if (AntiAliasing)
	{
		D3DPRESENT_PARAMETERS d3dpp;

		CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
		d3dpp.BackBufferCount = (d3dpp.BackBufferCount) ? d3dpp.BackBufferCount : 1;

		// Check AntiAliasing quality
		for (int x = min((AntiAliasing == 1 ? 16 : AntiAliasing), 16); x > 0; x--)
		{
			if (SUCCEEDED(ProxyInterface->CheckDeviceMultiSampleType(Adapter,
				DeviceType, (d3dpp.BackBufferFormat) ? d3dpp.BackBufferFormat : D3DFMT_A8R8G8B8, d3dpp.Windowed,
				(D3DMULTISAMPLE_TYPE)x)))
			{
				// Update Present Parameters for Multisample
				UpdatePresentParameterForMultisample(&d3dpp, (D3DMULTISAMPLE_TYPE)x);

				Logging::LogDebug() << __FUNCTION__ << " Trying MultiSample " << d3dpp.MultiSampleType;

				// Create Device
				hr = ProxyInterface->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, &d3dpp, ppReturnedDeviceInterface);

				// Check if device was created successfully
				if (SUCCEEDED(hr))
				{
					Logging::Log() << "Setting AntiAliasing MultiSample at " << d3dpp.MultiSampleType;
					DeviceMultiSampleType = d3dpp.MultiSampleType;
					break;
				}
			}
		}
		if (FAILED(hr))
		{
			Logging::Log() << "Failed to enable AntiAliasing!";
		}
	}

	if (FAILED(hr))
	{
		hr = ProxyInterface->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
	}

	if (SUCCEEDED(hr) && ppReturnedDeviceInterface)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Created device!");

		*ppReturnedDeviceInterface = new m_IDirect3DDevice8(*ppReturnedDeviceInterface, this);
		IDirect3DDevice8Ptr = (m_IDirect3DDevice8*)*ppReturnedDeviceInterface;
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Failed! Error: " << (D3DERR)hr;
	}

	// Create thread to save screenshot file
	RUNCODEONCE(CreateThread(nullptr, 0, SaveScreenshotFile, nullptr, 0, nullptr));

	// Get WndProc
	if (DeviceWindow && (ScreenMode != 3 || EnableScreenshots || DynamicResolution) && !OriginalWndProc)
	{
		OriginalWndProc = (WNDPROC)GetWindowLongA(DeviceWindow, GWL_WNDPROC);
		if (OriginalWndProc)
		{
			SetWindowLongA(DeviceWindow, GWL_WNDPROC, (LONG)WndProc);
		}
	}

	return hr;
}

// Set Presentation Parameters
void UpdatePresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters, HWND hFocusWindow, bool SetWindow)
{
	if (!pPresentationParameters)
	{
		return;
	}

	LONG OldBufferWidth = BufferWidth, OldBufferHeight = BufferHeight;

	BufferWidth = (pPresentationParameters->BackBufferWidth) ? pPresentationParameters->BackBufferWidth : BufferWidth;
	BufferHeight = (pPresentationParameters->BackBufferHeight) ? pPresentationParameters->BackBufferHeight : BufferHeight;

	DeviceWindow = (pPresentationParameters->hDeviceWindow) ? pPresentationParameters->hDeviceWindow :
		(hFocusWindow) ? hFocusWindow : DeviceWindow;

	// Check if resolution changed
	if (OldBufferWidth != BufferWidth || OldBufferHeight != BufferHeight)
	{
		Logging::Log() << "Setting resolution: " << BufferWidth << "x" << BufferHeight;

		// Set correct resolution for Room 312
		if (PauseScreenFix)
		{
			SetRoom312Resolution(&BufferWidth);
		}

		if (AutoScaleImages || AutoScaleVideos)
		{
			SetDymanicScale((float)BufferWidth / (float)BufferHeight);
		}

		// Set fullscreen image resolution
		if (FullscreenImages)
		{
			SetFullscreenImagesRes(BufferWidth, BufferHeight);
		}

		// Set fullscreen video resolution
		if (FullscreenVideos)
		{
			SetFullscreenVideoRes(BufferWidth, BufferHeight);
		}
	}

	// Check if window is minimized and restore it
	if (IsWindow(DeviceWindow) && IsIconic(DeviceWindow))
	{
		ShowWindow(DeviceWindow, SW_RESTORE);
	}

	// Set window size if window mode is enabled
	if (ScreenMode != 3 && (pPresentationParameters->hDeviceWindow || DeviceWindow || hFocusWindow))
	{
		pPresentationParameters->Windowed = true;
		pPresentationParameters->FullScreen_RefreshRateInHz = 0;

		if (SetWindow)
		{
			static int LastScreenMode = 0;
			static LONG LastBufferWidth = 0, LastBufferHeight = 0;
			bool AnyChange = (ScreenMode != LastScreenMode || BufferWidth != LastBufferWidth || BufferHeight != LastBufferHeight);

			// Reset display size
			if ((ScreenMode == 1 && ScreenMode != LastScreenMode) || (ScreenMode == 2 && AnyChange))
			{
				ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, CDS_RESET, nullptr);
			}

			// Update Silent Hill 2 window
			if (AnyChange)
			{
				AdjustWindow(DeviceWindow, BufferWidth, BufferHeight);
			}

			// Set new display size 
			if (ScreenMode == 2 && AnyChange)
			{
				// Get monitor info
				MONITORINFOEX infoex = {};
				infoex.cbSize = sizeof(MONITORINFOEX);
				BOOL bRet = GetMonitorInfo(GetMonitorHandle(), &infoex);

				// Get resolution list for specified monitor
				DEVMODE newSettings = {};
				newSettings.dmSize = sizeof(newSettings);
				if (EnumDisplaySettings(bRet ? infoex.szDevice : nullptr, ENUM_CURRENT_SETTINGS, &newSettings) != 0)
				{
					newSettings.dmPelsWidth = BufferWidth;
					newSettings.dmPelsHeight = BufferHeight;
					newSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
					ChangeDisplaySettingsEx(bRet ? infoex.szDevice : nullptr, &newSettings, nullptr, CDS_FULLSCREEN, nullptr);
				}
			}

			// Reset variables
			LastScreenMode = ScreenMode;
			LastBufferWidth = BufferWidth;
			LastBufferHeight = BufferHeight;
		}
	}
}

void UpdatePresentParameterForMultisample(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DMULTISAMPLE_TYPE MultiSampleType)
{
	if (!pPresentationParameters)
	{
		return;
	}

	pPresentationParameters->MultiSampleType = MultiSampleType;

	pPresentationParameters->Flags &= ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	pPresentationParameters->SwapEffect = D3DSWAPEFFECT_DISCARD;

	if (!pPresentationParameters->EnableAutoDepthStencil)
	{
		pPresentationParameters->EnableAutoDepthStencil = true;
		pPresentationParameters->AutoDepthStencilFormat = D3DFMT_D24S8;
	}
}

// Adjusting the window position for WindowMode
void AdjustWindow(HWND MainhWnd, LONG displayWidth, LONG displayHeight)
{
	// Set default window background color to black
	if (IsWindow(MainhWnd))
	{
		HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
		SetClassLongPtr(MainhWnd, GCLP_HBRBACKGROUND, (LONG_PTR)brush);
	}

	if (!MainhWnd || !displayWidth || !displayHeight)
	{
		Logging::Log() << __FUNCTION__ << " Error: could not set window size, nullptr.";
		return;
	}

	if (ScreenMode == 3)
	{
		// Don't adjust window or set border when in exclusive fullscreen mode
		return;
	}

	// Handle Windows themes
	if (WndModeBorder)
	{
		SetWindowTheme(MainhWnd);
	}

	// Get screen width and height
	LONG screenWidth = 0, screenHeight = 0;
	GetDesktopRes(screenWidth, screenHeight);
	RECT screenRect = {};
	GetDesktopRect(screenRect);

	// Set window active and focus
	HWND hCurWnd = GetForegroundWindow();
	DWORD dwMyID = GetCurrentThreadId();
	DWORD dwCurID = GetWindowThreadProcessId(hCurWnd, nullptr);
	AttachThreadInput(dwCurID, dwMyID, TRUE);
	SetWindowPos(MainhWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(MainhWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
	SetForegroundWindow(MainhWnd);
	SetFocus(MainhWnd);
	SetActiveWindow(MainhWnd);

	// Get window border
	LONG lStyle = GetWindowLong(MainhWnd, GWL_STYLE) | WS_VISIBLE;
	if (ScreenMode == 1 && WndModeBorder && screenWidth > displayWidth + (GetSystemMetrics(SM_CXSIZEFRAME) * 2) &&
		screenHeight > displayHeight + (GetSystemMetrics(SM_CYSIZEFRAME) * 2) + GetSystemMetrics(SM_CYCAPTION))
	{
		lStyle = (lStyle | WS_OVERLAPPEDWINDOW) & ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
	}
	else
	{
		lStyle &= ~WS_OVERLAPPEDWINDOW;
	}

	// Set window border
	SetWindowLong(MainhWnd, GWL_STYLE, lStyle);
	SetWindowPos(MainhWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	// Set window size
	SetWindowPos(MainhWnd, nullptr, 0, 0, displayWidth, displayHeight, SWP_NOMOVE | SWP_NOZORDER);

	// Adjust for window decoration to ensure client area matches display size
	RECT tempRect;
	GetClientRect(MainhWnd, &tempRect);
	LONG newDisplayWidth = (displayWidth - tempRect.right) + displayWidth;
	LONG newDisplayHeight = (displayHeight - tempRect.bottom) + displayHeight;

	// Move window to center and adjust size
	LONG xLoc = 0;
	LONG yLoc = 0;
	if (ScreenMode == 1 && screenWidth >= newDisplayWidth && screenHeight >= newDisplayHeight)
	{
		xLoc = screenRect.left + (screenWidth - newDisplayWidth) / 2;
		yLoc = screenRect.top + (screenHeight - newDisplayHeight) / 2;
	}
	SetWindowPos(MainhWnd, nullptr, xLoc, yLoc, newDisplayWidth, newDisplayHeight, SWP_NOZORDER);

	// Detach thread input
	AttachThreadInput(dwCurID, dwMyID, FALSE);
}

// Get keyboard press
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Logging::LogDebug() << __FUNCTION__ << " " << Logging::hex(wParam);

	static HMONITOR LastMonitorHandle = nullptr;

	switch (uMsg)
	{
	case  WM_WININICHANGE:
		if (lParam && WndModeBorder && ScreenMode != 3 && !_stricmp((char*)lParam, "ImmersiveColorSet"))
		{
			SetWindowTheme(DeviceWindow);
		}
	case WM_SYSKEYDOWN:
		if (wParam == VK_RETURN && DynamicResolution && ScreenMode != 3)
		{
			DeviceLost = true;
			ScreenMode = (ScreenMode == 1) ? 2 : 1;
		}
		break;
	case WM_KEYUP:
		if (wParam == VK_SNAPSHOT && EnableScreenshots)
		{
			TakeScreenShot = true;
		}
		break;
	case WM_MOVE:
	case WM_WINDOWPOSCHANGED:
	{
		HMONITOR MonitorHandle = GetMonitorHandle();
		if (LastMonitorHandle && LastMonitorHandle != MonitorHandle)
		{
			DeviceLost = true;
			SetResolutionList(BufferWidth, BufferHeight);
		}
		LastMonitorHandle = MonitorHandle;
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case 0x44: // Letter D
		{
			if (GetAsyncKeyState(VK_CONTROL)) 
			{
				if (IDirect3DDevice8Ptr && EnableDebugOverlay)
				{
					IDirect3DDevice8Ptr->ToggleShowDebugOverlay();
				}
			}
			break;
		}

		}
		break;
	}

	}

	if (!OriginalWndProc)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: no WndProc specified " << Logging::hex(uMsg));
		return NULL;
	}

	return OriginalWndProc(hWnd, uMsg, wParam, lParam);
}

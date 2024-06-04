/**
* Copyright (C) 2024 Elisha Riedlinger
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
#include "Patches\InputTweaks.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

WNDPROC OriginalWndProc = nullptr;
HWND DeviceWindow = nullptr;
LONG BufferWidth = 0, BufferHeight = 0;
LONG DefaultWidth = 0, DefaultHeight = 0;
DWORD VendorID = 0;
bool WindowInChange = false;
bool UsingWindowBorder = true;
bool CopyRenderTarget = false;
bool SetSSAA = false;
bool SetATOC = false;
bool IsWindowShrunk = false;
bool TakeScreenShot = false;
D3DMULTISAMPLE_TYPE DeviceMultiSampleType = D3DMULTISAMPLE_NONE;

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

	if (ScreenMode != EXCLUSIVE_FULLSCREEN)
	{
		Windowed = true;
	}

	return ProxyInterface->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType);
}

HRESULT m_IDirect3D8::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{
	Logging::LogDebug() << __FUNCTION__;

	if (ScreenMode != EXCLUSIVE_FULLSCREEN)
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

	// Get default resolution
	RUNCODEONCE(GetDesktopRes(DefaultWidth, DefaultHeight));

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
	CopyRenderTarget = (DeviceMultiSampleType || FixGPUAntiAliasing);

	// Check Transparency Antialiasing
	if (DeviceMultiSampleType || FixGPUAntiAliasing)
	{
		SetSSAA = (ProxyInterface->CheckDeviceFormat(Adapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, FOURCC_SSAA) == D3D_OK);
		SetATOC = (ProxyInterface->CheckDeviceFormat(Adapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, FOURCC_ATOC) == D3D_OK);
		Logging::Log() << "Transparency Antialiasing features. SSAA: " << SetSSAA << " ATOC: " << SetATOC;
	}

	// Update presentation parameters
	UpdatePresentParameter(pPresentationParameters, hFocusWindow);

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

		RestorePresentParameter(pPresentationParameters);

		// Handle display modes
		SetScreenAndWindowSize();
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Failed! Error: " << (D3DERR)hr;
	}

	// Create thread to save screenshot file
	if (EnableScreenshots)
	{
		RUNCODEONCE(CreateThread(nullptr, 0, SaveScreenshotFile, nullptr, 0, nullptr));
	}

	// Get WndProc
	if (HookWndProc && DeviceWindow && !OriginalWndProc)
	{
		OriginalWndProc = (WNDPROC)GetWindowLongA(DeviceWindow, GWL_WNDPROC);
		if (OriginalWndProc)
		{
			SetWindowLongA(DeviceWindow, GWL_WNDPROC, (LONG)WndProc);
		}
	}

	GameWindowHandle = DeviceWindow;

	return hr;
}

// Restore Presentation Parameters
void RestorePresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	if (IsScaledResolutionsEnabled() && pPresentationParameters)
	{
		pPresentationParameters->BackBufferWidth = BufferWidth;
		pPresentationParameters->BackBufferHeight = BufferHeight;

		pPresentationParameters->EnableAutoDepthStencil = TRUE;
		pPresentationParameters->AutoDepthStencilFormat = D3DFMT_D24S8;
	}
}

// Set Presentation Parameters
void UpdatePresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters, HWND hFocusWindow)
{
	if (!pPresentationParameters)
	{
		return;
	}

	// Get updated width and height
	BufferWidth = (pPresentationParameters->BackBufferWidth) ? pPresentationParameters->BackBufferWidth : BufferWidth;
	BufferHeight = (pPresentationParameters->BackBufferHeight) ? pPresentationParameters->BackBufferHeight : BufferHeight;

	// Check if window size is larger than screen resolution
	IsWindowShrunk = (ScreenMode == WINDOWED && (DefaultWidth < BufferWidth || DefaultHeight < BufferHeight));

	// Set scaled width and height
	if (IsScaledResolutionsEnabled())
	{
		if (pPresentationParameters->EnableAutoDepthStencil)
		{
			pPresentationParameters->EnableAutoDepthStencil = FALSE;
			pPresentationParameters->AutoDepthStencilFormat = D3DFMT_UNKNOWN;
		}

		GetNonScaledResolution(pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight);
	}

	DeviceWindow = (pPresentationParameters->hDeviceWindow) ? pPresentationParameters->hDeviceWindow :
		(hFocusWindow) ? hFocusWindow : DeviceWindow;

	pPresentationParameters->Windowed = (ScreenMode == WINDOWED || ScreenMode == WINDOWED_FULLSCREEN);
	pPresentationParameters->FullScreen_RefreshRateInHz = (pPresentationParameters->Windowed) ? 0 : pPresentationParameters->FullScreen_RefreshRateInHz;

	// Update patches for resolution change
	UpdateResolutionPatches(BufferWidth, BufferHeight);
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

	if (!pPresentationParameters->EnableAutoDepthStencil && !IsScaledResolutionsEnabled())
	{
		pPresentationParameters->EnableAutoDepthStencil = TRUE;
		pPresentationParameters->AutoDepthStencilFormat = D3DFMT_D24S8;
	}
}

bool IsScaledResolutionsEnabled()
{
	return (UsingScaledResolutions || IsWindowShrunk);
}

void SetScreenAndWindowSize()
{
	WindowInChange = true;

	static int LastScreenMode = -1;
	if (ScreenMode != LastScreenMode)
	{
		Logging::Log() << __FUNCTION__ << " Setting display mode: " <<
			(ScreenMode == WINDOWED ? "Windowed" : ScreenMode == WINDOWED_FULLSCREEN ? "Windowed Fullscreen" : "Exclusive Fullscreen");
	}
	LastScreenMode = ScreenMode;

	// Check for iconic window
	if (IsWindow(DeviceWindow))
	{
		// Check if window is minimized and restore it
		if (IsIconic(DeviceWindow))
		{
			ShowWindow(DeviceWindow, SW_RESTORE);
		}

		// Set default window background color to black
		if (WhiteShaderFix)
		{
			HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
			SetClassLongPtr(DeviceWindow, GCLP_HBRBACKGROUND, (LONG_PTR)brush);
		}

		// Handle Windows themes
		if (WndModeBorder)
		{
			SetWindowTheme(DeviceWindow);
		}

		// Set window size if window mode is enabled
		if (ScreenMode != EXCLUSIVE_FULLSCREEN)
		{
			LONG Width = BufferWidth;
			LONG Height = BufferHeight;

			// Reset resolution scaling
			if (IsScaledResolutionsEnabled())
			{
				GetNonScaledResolution(Width, Height);
			}

			// Get current resolution
			LONG CurrentWidth = 0, CurrentHeight = 0;
			GetDesktopRes(CurrentWidth, CurrentHeight);

			// Reset display
			if (ScreenMode == WINDOWED && (CurrentWidth != DefaultWidth || CurrentHeight != DefaultHeight))
			{
				LONG result = ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, CDS_RESET, nullptr);

				while (int x = 0 && x < 20 && result == DISP_CHANGE_SUCCESSFUL && GetDesktopRes(CurrentWidth, CurrentHeight) &&
					(CurrentWidth != DefaultWidth || CurrentHeight != DefaultHeight))
				{
					x++;
					Sleep(100);
				}
			}

			// Set new display size
			if (ScreenMode == WINDOWED_FULLSCREEN && (CurrentWidth != Width || CurrentHeight != Height))
			{
				BOOL ret = SetDesktopRes(Width, Height);

				while (int x = 0 && x < 20 && ret && GetDesktopRes(CurrentWidth, CurrentHeight) &&
					CurrentWidth != Width && CurrentHeight != Height)
				{
					x++;
					Sleep(100);
				}
			}

			// Update Silent Hill 2 window
			AdjustWindow(DeviceWindow, Width, Height);
		}
	}

	ClearGDISurface(DeviceWindow, RGB(0, 0, 0));

	WindowInChange = false;
}

// Adjusting the window position for WindowMode
void AdjustWindow(HWND MainhWnd, LONG displayWidth, LONG displayHeight)
{
	if (!IsWindow(MainhWnd) || !displayWidth || !displayHeight)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not set window size, nullptr.");
		return;
	}

	// Remember first run
	static bool FirstRun = true;

	// Set window active and focus
	SetWindowPos(MainhWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	if (!ForceTopMost)
	{
		SetWindowPos(MainhWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
	}
	SetForegroundWindow(MainhWnd);
	SetFocus(MainhWnd);
	SetActiveWindow(MainhWnd);

	// Get screen width and height
	LONG screenWidth = 0, screenHeight = 0;
	GetDesktopRes(screenWidth, screenHeight);
	RECT screenRect = {};
	GetDesktopRect(screenRect);

	// Get update width and height
	if (ScreenMode == WINDOWED && (screenWidth < displayWidth || screenHeight < displayHeight))
	{
		float Ratio = max((float)displayWidth / screenWidth, (float)displayHeight / screenHeight);
		displayWidth /= Ratio;
		displayHeight /= Ratio;
	}

	// Get window style
	LONG lStyle = GetWindowLong(MainhWnd, GWL_STYLE) | WS_VISIBLE;
	LONG lExStyle = GetWindowLong(MainhWnd, GWL_EXSTYLE);

	// Get new style
	LONG lNewStyle = (lStyle | WS_OVERLAPPEDWINDOW) & ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
	RECT Rect = { 0, 0, displayWidth, displayHeight };
	AdjustWindowRectEx(&Rect, lNewStyle, GetMenu(MainhWnd) != NULL, lExStyle);
	if (ScreenMode == WINDOWED && WndModeBorder && screenWidth > Rect.right - Rect.left && screenHeight > Rect.bottom - Rect.top)
	{
		UsingWindowBorder = true;
		lStyle = lNewStyle;
	}
	else
	{
		UsingWindowBorder = false;
		lStyle &= ~WS_OVERLAPPEDWINDOW;
	}

	// Set window style
	SetWindowLong(MainhWnd, GWL_STYLE, lStyle);
	SetWindowPos(MainhWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	// Get new window rect
	Rect = { 0, 0, displayWidth, displayHeight };
	AdjustWindowRectEx(&Rect, lStyle, GetMenu(MainhWnd) != NULL, lExStyle);
	Rect = { 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top };
	LONG xLoc = 0, yLoc = 0;

	// Load window placement
	bool UseWindowPlacement = false;
	WINDOWPLACEMENT wndpl = {};
	if (UsingWindowBorder && ReadRegistryStruct(L"Konami\\Silent Hill 2\\sh2e", L"GameWindowPlacement", &wndpl, sizeof(WINDOWPLACEMENT)))
	{
		xLoc = wndpl.rcNormalPosition.left;
		yLoc = wndpl.rcNormalPosition.top;
		if (wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left == Rect.right &&
			wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top == Rect.bottom)
		{
			wndpl.length = sizeof(WINDOWPLACEMENT);
			UseWindowPlacement = true;
		}
	}

	// Adjust window location/size and center if needed
	if (ScreenMode == WINDOWED && screenWidth >= Rect.right && screenHeight >= Rect.bottom)
	{
		// Center window on load or if not using window border
		if (FirstRun || !UsingWindowBorder)
		{
			xLoc = (screenWidth - Rect.right) / 2;
			yLoc = (screenHeight - Rect.bottom) / 2;
		}
		else
		{
			// Keep current location if not using window placement location
			if (!xLoc && !yLoc)
			{
				RECT wRect = {};
				GetWindowRect(MainhWnd, &wRect);
				xLoc = wRect.left;
				yLoc = wRect.top;
			}
			// Check if window is being pushed off the screen
			if (xLoc + Rect.right > screenWidth && screenWidth >= Rect.right)
			{
				xLoc = screenWidth - Rect.right;
			}
			if (yLoc + Rect.bottom > screenHeight && screenHeight >= Rect.bottom)
			{
				yLoc = screenHeight - Rect.bottom;
			}
		}
	}
	SetWindowPos(MainhWnd, HWND_TOP, xLoc, yLoc, Rect.right, Rect.bottom, SWP_SHOWWINDOW | SWP_NOZORDER);

	// Wait for window position change
	int x = 0;
	RECT tempRect = {};
	while (++x < 20 && GetWindowRect(MainhWnd, &tempRect) &&
		tempRect.left != xLoc && tempRect.top != yLoc &&
		tempRect.right != xLoc + Rect.right && tempRect.bottom != yLoc + Rect.bottom)
	{
		Sleep(100);
	}

	// Set window placement
	if (UseWindowPlacement)
	{
		SetWindowPlacement(MainhWnd, &wndpl);
	}

	// Unset frist run
	FirstRun = false;
}

void SaveWindowPlacement()
{
	// Save window placement if using window border
	if (IsWindow(DeviceWindow) && UsingWindowBorder)
	{
		WINDOWPLACEMENT wndpl;
		wndpl.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(DeviceWindow, &wndpl);

		WriteRegistryStruct(L"Konami\\Silent Hill 2\\sh2e", L"GameWindowPlacement", REG_BINARY, &wndpl, sizeof(WINDOWPLACEMENT));
	}
}

// Get keyboard press
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Logging::LogDebug() << __FUNCTION__ << " " << Logging::hex(uMsg);

	static HMONITOR LastMonitorHandle = nullptr;

	switch (uMsg)
	{
	case WM_WININICHANGE:
		if (lParam && WndModeBorder && !_stricmp((char*)lParam, "ImmersiveColorSet"))
		{
			SetWindowTheme(DeviceWindow);
		}
		break;
	case WM_SYSKEYDOWN:
		if (wParam == VK_RETURN && DynamicResolution)
		{
			DeviceLost = true;
			ScreenMode = (ScreenMode == WINDOWED) ? WINDOWED_FULLSCREEN : WINDOWED;
			SetNewDisplayModeSetting();
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
		if (!WindowInChange)
		{
			HMONITOR MonitorHandle = GetMonitorHandle();
			if (LastMonitorHandle && LastMonitorHandle != MonitorHandle)
			{
				GetDesktopRes(DefaultWidth, DefaultHeight);
				SetResolutionList(BufferWidth, BufferHeight);
				DeviceLost = true;
			}
			LastMonitorHandle = MonitorHandle;
			if (hWnd == DeviceWindow && ScreenMode == WINDOWED)
			{
				SaveWindowPlacement();
			}
		}
		break;
	case WM_SETFOCUS:
		InputTweaksRef.ClearMouseInputs();
		break;
	}

	if (!OriginalWndProc)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: no WndProc specified " << Logging::hex(uMsg));
		return NULL;
	}

	return OriginalWndProc(hWnd, uMsg, wParam, lParam);
}

/**
* Copyright (C) 2018 Elisha Riedlinger
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

void AdjustWindow(HWND MainhWnd, LONG displayWidth, LONG displayHeight);

HRESULT m_IDirect3D8::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
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
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3D8::Release()
{
	ULONG count = ProxyInterface->Release();

	if (count == 0)
	{
		delete this;
	}

	return count;
}

HRESULT m_IDirect3D8::EnumAdapterModes(THIS_ UINT Adapter, UINT Mode, D3DDISPLAYMODE* pMode)
{
	return ProxyInterface->EnumAdapterModes(Adapter, Mode, pMode);
}

UINT m_IDirect3D8::GetAdapterCount()
{
	return ProxyInterface->GetAdapterCount();
}

HRESULT m_IDirect3D8::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
	return ProxyInterface->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT m_IDirect3D8::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 *pIdentifier)
{
	return ProxyInterface->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT m_IDirect3D8::GetAdapterModeCount(THIS_ UINT Adapter)
{
	return ProxyInterface->GetAdapterModeCount(Adapter);
}

HMONITOR m_IDirect3D8::GetAdapterMonitor(UINT Adapter)
{
	return ProxyInterface->GetAdapterMonitor(Adapter);
}

HRESULT m_IDirect3D8::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8 *pCaps)
{
	return ProxyInterface->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HRESULT m_IDirect3D8::RegisterSoftwareDevice(void *pInitializeFunction)
{
	return ProxyInterface->RegisterSoftwareDevice(pInitializeFunction);
}

HRESULT m_IDirect3D8::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	return ProxyInterface->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT m_IDirect3D8::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return ProxyInterface->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT m_IDirect3D8::CheckDeviceMultiSampleType(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType)
{
	return ProxyInterface->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType);
}

HRESULT m_IDirect3D8::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{
	return ProxyInterface->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}

HRESULT m_IDirect3D8::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice8 **ppReturnedDeviceInterface)
{
	if (EnableWndMode)
	{
		pPresentationParameters->Windowed = true;
		AdjustWindow((hFocusWindow) ? hFocusWindow : pPresentationParameters->hDeviceWindow, pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight);
	}

	HRESULT hr = ProxyInterface->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	if (SUCCEEDED(hr))
	{
		*ppReturnedDeviceInterface = new m_IDirect3DDevice8(*ppReturnedDeviceInterface, this);
	}
	return hr;
}

// Adjusting the window position for SetDisplayMode()
void AdjustWindow(HWND MainhWnd, LONG displayWidth, LONG displayHeight)
{
	if (MainhWnd)
	{
		// Get screen width
		LONG screenWidth = GetSystemMetrics(SM_CXSCREEN);
		LONG screenHeight = GetSystemMetrics(SM_CYSCREEN);
		// Update window border
		if (WndModeBorder && screenHeight > displayHeight + 32)
		{
			LONG lStyle = GetWindowLong(MainhWnd, GWL_STYLE);
			lStyle |= (WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
			SetWindowLong(MainhWnd, GWL_STYLE, lStyle);
			LONG lExStyle = GetWindowLong(MainhWnd, GWL_EXSTYLE);
			lExStyle |= (WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
			SetWindowLong(MainhWnd, GWL_EXSTYLE, lExStyle);
		}
		else
		{
			LONG lStyle = GetWindowLong(MainhWnd, GWL_STYLE);
			lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
			SetWindowLong(MainhWnd, GWL_STYLE, lStyle);
			LONG lExStyle = GetWindowLong(MainhWnd, GWL_EXSTYLE);
			lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
			SetWindowLong(MainhWnd, GWL_EXSTYLE, lExStyle);
		}
		// Set window border
		SetWindowPos(MainhWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		// Set window size
		SetWindowPos(MainhWnd, nullptr, 0, 0, displayWidth, displayHeight, SWP_NOMOVE | SWP_NOZORDER);
		// Adjust for window decoration to ensure client area matches display size
		RECT tempRect;
		GetClientRect(MainhWnd, &tempRect);
		tempRect.right = (displayWidth - tempRect.right) + displayWidth;
		tempRect.bottom = (displayHeight - tempRect.bottom) + displayHeight;
		// Move window to center and adjust size
		LONG xLoc = 0;
		LONG yLoc = 0;
		if (screenWidth >= displayWidth && screenHeight >= displayHeight)
		{
			xLoc = (screenWidth - displayWidth) / 2;
			yLoc = (screenHeight - displayHeight) / 2;
		}
		SetWindowPos(MainhWnd, nullptr, xLoc, yLoc, tempRect.right, tempRect.bottom, SWP_NOZORDER);
	}
}

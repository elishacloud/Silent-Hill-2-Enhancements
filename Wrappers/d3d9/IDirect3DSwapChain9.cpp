/**
* Copyright (C) 2014 Patrick Mours. All rights reserved.
* License: https://github.com/crosire/reshade#license
*
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

#include "d3d9wrapper.h"

HRESULT m_IDirect3DSwapChain9::QueryInterface(REFIID riid, void** ppvObj)
{
	if ((riid == IID_IDirect3DSwapChain9 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG m_IDirect3DSwapChain9::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DSwapChain9::Release()
{
	const ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		_runtime->on_reset();
		_runtime.reset();

		const auto it = std::find(D3Device->_additional_swapchains.begin(), D3Device->_additional_swapchains.end(), this);
		if (it != D3Device->_additional_swapchains.end())
		{
			D3Device->_additional_swapchains.erase(it);
			D3Device->Release(); // Remove the reference that was added in 'm_IDirect3DDevice9::CreateAdditionalSwapChain'
		}

		delete this;
	}

	return ref;
}

HRESULT m_IDirect3DSwapChain9::Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion, DWORD dwFlags)
{
	// Only call into runtime if the entire surface is presented, to avoid partial updates messing up effects and the GUI
	if (is_presenting_entire_surface(pSourceRect, hDestWindowOverride))
	{
		_runtime->on_present();
	}
	D3Device->_buffer_detection.reset(false);

	return ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

HRESULT m_IDirect3DSwapChain9::GetFrontBufferData(IDirect3DSurface9 *pDestSurface)
{
	return ProxyInterface->GetFrontBufferData(pDestSurface);
}

HRESULT m_IDirect3DSwapChain9::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9 **ppBackBuffer)
{
	return ProxyInterface->GetBackBuffer(iBackBuffer, Type, ppBackBuffer);
}

HRESULT m_IDirect3DSwapChain9::GetRasterStatus(D3DRASTER_STATUS *pRasterStatus)
{
	return ProxyInterface->GetRasterStatus(pRasterStatus);
}

HRESULT m_IDirect3DSwapChain9::GetDisplayMode(D3DDISPLAYMODE *pMode)
{
	return ProxyInterface->GetDisplayMode(pMode);
}

HRESULT m_IDirect3DSwapChain9::GetDevice(IDirect3DDevice9 **ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3Device->AddRef();
	*ppDevice = D3Device;

	return D3D_OK;
}

HRESULT m_IDirect3DSwapChain9::GetPresentParameters(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	return ProxyInterface->GetPresentParameters(pPresentationParameters);
}

bool m_IDirect3DSwapChain9::is_presenting_entire_surface(const RECT *source_rect, HWND hwnd)
{
	if (source_rect == nullptr)
	{
		return true;
	}

	if (hwnd != nullptr)
	{
		RECT window_rect = {};
		GetClientRect(hwnd, &window_rect);
		if (source_rect->left == window_rect.left && source_rect->top == window_rect.top &&
			source_rect->right == window_rect.right && source_rect->bottom == window_rect.bottom)
		{
			return true;
		}
	}

	return false;
}

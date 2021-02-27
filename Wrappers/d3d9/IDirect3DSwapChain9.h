/**
* Copyright (C) 2014 Patrick Mours. All rights reserved.
* License: https://github.com/crosire/reshade#license
*
* Copyright (C) 2021 Elisha Riedlinger
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

#pragma once

#include "d3d9wrapper.h"
#include <memory>

namespace reshade::d3d9 { class runtime_d3d9; }

class m_IDirect3DSwapChain9 : public IDirect3DSwapChain9
{
private:
	LPDIRECT3DSWAPCHAIN9 ProxyInterface;
	m_IDirect3DDevice9 *const D3Device;

public:
	std::shared_ptr<reshade::d3d9::runtime_d3d9> _runtime;

public:
	m_IDirect3DSwapChain9(m_IDirect3DDevice9 *device, IDirect3DSwapChain9 *original, const std::shared_ptr<reshade::d3d9::runtime_d3d9> &runtime) :
		ProxyInterface(original),
		D3Device(device),
		_runtime(runtime)
	{
		Logging::LogDebug() << "Creating device " << __FUNCTION__ << "(" << this << ")";

		assert(ProxyInterface != nullptr && D3Device != nullptr && _runtime != nullptr);
	}
	~m_IDirect3DSwapChain9()
	{
		Logging::LogDebug() << __FUNCTION__ << "(" << this << ")" << " deleting device!";
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DSwapChain9 methods ***/
	STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
	STDMETHOD(GetFrontBufferData)(THIS_ IDirect3DSurface9* pDestSurface);
	STDMETHOD(GetBackBuffer)(THIS_ UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
	STDMETHOD(GetRasterStatus)(THIS_ D3DRASTER_STATUS* pRasterStatus);
	STDMETHOD(GetDisplayMode)(THIS_ D3DDISPLAYMODE* pMode);
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	STDMETHOD(GetPresentParameters)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);

	static bool is_presenting_entire_surface(const RECT *source_rect, HWND hwnd);
};

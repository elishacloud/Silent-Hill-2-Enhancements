/**
* Copyright (C) 2014 Patrick Mours. All rights reserved.
* License: https://github.com/crosire/reshade#license
*
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

#include "d3d9wrapper.h"

HRESULT m_IDirect3D9::QueryInterface(REFIID riid, void** ppvObj)
{
	if ((riid == IID_IDirect3D9 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG m_IDirect3D9::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3D9::Release()
{
	ULONG count = ProxyInterface->Release();

	if (count == 0)
	{
		delete this;
	}

	return count;
}

HRESULT m_IDirect3D9::EnumAdapterModes(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	return ProxyInterface->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

UINT m_IDirect3D9::GetAdapterCount()
{
	return ProxyInterface->GetAdapterCount();
}

HRESULT m_IDirect3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
	return ProxyInterface->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT m_IDirect3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
	return ProxyInterface->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT m_IDirect3D9::GetAdapterModeCount(THIS_ UINT Adapter, D3DFORMAT Format)
{
	return ProxyInterface->GetAdapterModeCount(Adapter, Format);
}

HMONITOR m_IDirect3D9::GetAdapterMonitor(UINT Adapter)
{
	return ProxyInterface->GetAdapterMonitor(Adapter);
}

HRESULT m_IDirect3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps)
{
	return ProxyInterface->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HRESULT m_IDirect3D9::RegisterSoftwareDevice(void *pInitializeFunction)
{
	return ProxyInterface->RegisterSoftwareDevice(pInitializeFunction);
}

HRESULT m_IDirect3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	return ProxyInterface->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT m_IDirect3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return ProxyInterface->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT m_IDirect3D9::CheckDeviceMultiSampleType(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
	return ProxyInterface->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT m_IDirect3D9::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{
	return ProxyInterface->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}

HRESULT m_IDirect3D9::CheckDeviceFormatConversion(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	return ProxyInterface->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT m_IDirect3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface)
{
	Logging::LogDebug() << "Redirecting " << "IDirect3D9::CreateDevice" << '('
		<< "this = " << this
		<< ", Adapter = " << Adapter
		<< ", DeviceType = " << DeviceType
		<< ", hFocusWindow = " << hFocusWindow
		<< ", BehaviorFlags = " << Logging::Hex(BehaviorFlags)
		<< ", pPresentationParameters = " << pPresentationParameters
		<< ", ppReturnedDeviceInterface = " << ppReturnedDeviceInterface
		<< ')' << " ...";

	if (pPresentationParameters == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	if ((BehaviorFlags & D3DCREATE_ADAPTERGROUP_DEVICE) != 0)
	{
		Logging::Log() << "Adapter group devices are unsupported.";
		return D3DERR_NOTAVAILABLE;
	}

	D3DPRESENT_PARAMETERS pp = *pPresentationParameters;

	const bool use_software_rendering = (BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) != 0;
	if (use_software_rendering)
	{
		Logging::Log() << "> Replacing 'D3DCREATE_SOFTWARE_VERTEXPROCESSING' flag with 'D3DCREATE_MIXED_VERTEXPROCESSING' to allow for hardware rendering ...";
		BehaviorFlags = (BehaviorFlags & ~D3DCREATE_SOFTWARE_VERTEXPROCESSING) | D3DCREATE_MIXED_VERTEXPROCESSING;
	}

	const HRESULT hr = ProxyInterface->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, &pp, ppReturnedDeviceInterface);
	// Update output values (see https://docs.microsoft.com/windows/win32/api/d3d9/nf-d3d9-idirect3d9-createdevice)
	pPresentationParameters->BackBufferWidth = pp.BackBufferWidth;
	pPresentationParameters->BackBufferHeight = pp.BackBufferHeight;
	pPresentationParameters->BackBufferFormat = pp.BackBufferFormat;
	pPresentationParameters->BackBufferCount = pp.BackBufferCount;

	if (FAILED(hr))
	{
		Logging::Log() << "IDirect3D9::CreateDevice" << " failed with error code " << hr << '!';
		return hr;
	}

	init_runtime_d3d(*ppReturnedDeviceInterface, DeviceType, pp, use_software_rendering);

	return hr;
}

template <typename T>
static void m_IDirect3D9::init_runtime_d3d(T *&device, D3DDEVTYPE device_type, D3DPRESENT_PARAMETERS pp, bool use_software_rendering)
{
	// Enable software vertex processing if the application requested a software device
	if (use_software_rendering)
	{
		device->SetSoftwareVertexProcessing(TRUE);
	}

	if (device_type == D3DDEVTYPE_NULLREF)
	{
		Logging::Log() << "Skipping device because the device type is 'D3DDEVTYPE_NULLREF'.";
		return;
	}

	IDirect3DSwapChain9 *swapchain = nullptr;
	device->GetSwapChain(0, &swapchain);
	assert(swapchain != nullptr);

	// Retrieve present parameters here again, to get correct values for 'BackBufferWidth' and 'BackBufferHeight'
	// They may otherwise still be set to zero (which is valid for creation)
	swapchain->GetPresentParameters(&pp);

	const auto device_proxy = new m_IDirect3DDevice9(device, use_software_rendering);

	const auto runtime = std::make_shared<reshade::d3d9::runtime_d3d9>(device, swapchain, &device_proxy->_buffer_detection);
	if (!runtime->on_init(pp))
	{
		Logging::Log() << "Failed to initialize Direct3D 9 runtime environment on runtime " << runtime.get() << '.';
	}

	device_proxy->_implicit_swapchain = new m_IDirect3DSwapChain9(device_proxy, swapchain, runtime);

	// Get and set depth-stencil surface so that the depth detection callbacks are called with the auto depth-stencil surface
	if (pp.EnableAutoDepthStencil)
	{
		device->GetDepthStencilSurface(&device_proxy->_auto_depthstencil);
		device_proxy->SetDepthStencilSurface(device_proxy->_auto_depthstencil.get());
	}

	// Overwrite returned device with hooked one
	device = device_proxy;

	Logging::LogDebug() << "Returning IDirect3DDevice9 object " << device << '.';
}

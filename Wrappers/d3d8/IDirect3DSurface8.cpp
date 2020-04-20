/**
* Copyright (C) 2020 Elisha Riedlinger
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

HRESULT m_IDirect3DSurface8::QueryInterface(THIS_ REFIID riid, void** ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	// Get proxy interface
	if (riid == IID_GetProxyInterface && ppvObj)
	{
		*ppvObj = ProxyInterface;
		return S_OK;
	}

	// Get proxy interface
	if (riid == IID_SetDefaultRenderTarget)
	{
		IsDefaultRenderTarget = true;
		return S_OK;
	}

	// Get render target interface
	if (riid == IID_GetRenderTarget && ppvObj)
	{
		if (CopyRenderTarget && !IsDefaultRenderTarget && !RenderInterface && !ReplacedInterface && m_pDevice)
		{
			D3DSURFACE_DESC Desc;
			if (SUCCEEDED(ProxyInterface->GetDesc(&Desc)) && !Desc.MultiSampleType &&
				SUCCEEDED(m_pDevice->CreateRenderTarget(Desc.Width, Desc.Height, Desc.Format, DeviceMultiSampleType, FALSE, &RenderInterface)) && RenderInterface)
			{
				m_pDevice->AddSurfaceToVector(this, RenderInterface);
				IDirect3DSurface8* pSurface = this;
				RenderInterface->QueryInterface(IID_SetReplacedInterface, (void**)&pSurface);
			}
		}

		if (RenderInterface)
		{
			*ppvObj = RenderInterface;
			return S_OK;
		}
	}

	// Clear render target interface
	if (riid == IID_ClearRenderTarget)
	{
		RenderInterface = nullptr;
		return S_OK;
	}

	// Get replaced interface
	if (riid == IID_GetReplacedInterface && ppvObj && ReplacedInterface)
	{
		*ppvObj = ReplacedInterface;
		return S_OK;
	}

	// Set replaced interface
	if (riid == IID_SetReplacedInterface && ppvObj && *ppvObj)
	{
		ReplacedInterface = (IDirect3DSurface8*)*ppvObj;
		return S_OK;
	}

	if ((riid == IID_IDirect3DSurface8 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj, m_pDevice);
	}

	return hr;
}

ULONG m_IDirect3DSurface8::AddRef(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DSurface8::Release(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->Release();
}

HRESULT m_IDirect3DSurface8::GetDevice(THIS_ IDirect3DDevice8** ppDevice)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ppDevice)
	{
		return D3DERR_INVALIDCALL;
	}

	m_pDevice->AddRef();

	*ppDevice = m_pDevice;

	return D3D_OK;
}

HRESULT m_IDirect3DSurface8::SetPrivateData(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

HRESULT m_IDirect3DSurface8::GetPrivateData(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
}

HRESULT m_IDirect3DSurface8::FreePrivateData(THIS_ REFGUID refguid)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->FreePrivateData(refguid);
}

HRESULT m_IDirect3DSurface8::GetContainer(THIS_ REFIID riid, void** ppContainer)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->GetContainer(riid, ppContainer);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppContainer, m_pDevice);
	}

	return hr;
}

HRESULT m_IDirect3DSurface8::GetDesc(THIS_ D3DSURFACE_DESC *pDesc)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetDesc(pDesc);
}

HRESULT m_IDirect3DSurface8::LockRect(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->LockRect(pLockedRect, pRect, Flags);

	if (FAILED(hr) && DeviceMultiSampleType != D3DMULTISAMPLE_NONE)
	{
		D3DSURFACE_DESC Desc;
		if (SUCCEEDED(GetDesc(&Desc)))
		{
			DWORD bits = GetBitCount(Desc.Format) / 8;
			DWORD size = bits * Desc.Width * Desc.Height;
			if (!bits || (IsLocked && surfaceArray.size() < size))
			{
				return hr;
			}
			else if (surfaceArray.size() < size)
			{
				surfaceArray.resize(size);
			}

			LOG_LIMIT(100, __FUNCTION__ << " Emulating the surface lock. Data may be lost here!");

			DWORD start = (pRect) ? ((pRect->top * Desc.Width * bits) + (pRect->left * bits)) : 0;

			// ToDo: copy surface data to memory
			pLockedRect->pBits = &surfaceArray[start];
			pLockedRect->Pitch = Desc.Width * bits;

			IsLocked = true;

			return D3D_OK;
		}
	}

	return hr;
}

HRESULT m_IDirect3DSurface8::UnlockRect(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->UnlockRect();

	if (SUCCEEDED(hr))
	{
		// ToDo: copy data back from emulated surface
		IsLocked = false;
	}

	return hr;
}

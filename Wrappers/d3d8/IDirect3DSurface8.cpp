/**
* Copyright (C) 2019 Elisha Riedlinger
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

	return ProxyInterface->LockRect(pLockedRect, pRect, Flags);
}

HRESULT m_IDirect3DSurface8::UnlockRect(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->UnlockRect();
}

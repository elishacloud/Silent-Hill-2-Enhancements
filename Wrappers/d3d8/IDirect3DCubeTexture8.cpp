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

HRESULT m_IDirect3DCubeTexture8::QueryInterface(THIS_ REFIID riid, void** ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	if ((riid == IID_IDirect3DCubeTexture8 || riid == IID_IUnknown || riid == IID_IDirect3DResource8 || riid == IID_IDirect3DBaseTexture8) && ppvObj)
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

ULONG m_IDirect3DCubeTexture8::AddRef(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DCubeTexture8::Release(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->Release();
}

HRESULT m_IDirect3DCubeTexture8::GetDevice(THIS_ IDirect3DDevice8** ppDevice)
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

HRESULT m_IDirect3DCubeTexture8::SetPrivateData(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

HRESULT m_IDirect3DCubeTexture8::GetPrivateData(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
}

HRESULT m_IDirect3DCubeTexture8::FreePrivateData(THIS_ REFGUID refguid)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->FreePrivateData(refguid);
}

DWORD m_IDirect3DCubeTexture8::SetPriority(THIS_ DWORD PriorityNew)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetPriority(PriorityNew);
}

DWORD m_IDirect3DCubeTexture8::GetPriority(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetPriority();
}

void m_IDirect3DCubeTexture8::PreLoad(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	ProxyInterface->PreLoad();
}

D3DRESOURCETYPE m_IDirect3DCubeTexture8::GetType(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetType();
}

DWORD m_IDirect3DCubeTexture8::SetLOD(THIS_ DWORD LODNew)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetLOD(LODNew);
}

DWORD m_IDirect3DCubeTexture8::GetLOD(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetLOD();
}

DWORD m_IDirect3DCubeTexture8::GetLevelCount(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetLevelCount();
}

HRESULT m_IDirect3DCubeTexture8::GetLevelDesc(THIS_ UINT Level, D3DSURFACE_DESC *pDesc)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetLevelDesc(Level, pDesc);
}

HRESULT m_IDirect3DCubeTexture8::GetCubeMapSurface(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface8** ppCubeMapSurface)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->GetCubeMapSurface(FaceType, Level, ppCubeMapSurface);

	if (SUCCEEDED(hr) && ppCubeMapSurface)
	{
		*ppCubeMapSurface = m_pDevice->ProxyAddressLookupTableD3d8->FindAddress<m_IDirect3DSurface8>(*ppCubeMapSurface);
	}

	return hr;
}

HRESULT m_IDirect3DCubeTexture8::LockRect(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->LockRect(FaceType, Level, pLockedRect, pRect, Flags);
}

HRESULT m_IDirect3DCubeTexture8::UnlockRect(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->UnlockRect(FaceType, Level);
}

HRESULT m_IDirect3DCubeTexture8::AddDirtyRect(THIS_ D3DCUBEMAP_FACES FaceType, CONST RECT* pDirtyRect)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->AddDirtyRect(FaceType, pDirtyRect);
}

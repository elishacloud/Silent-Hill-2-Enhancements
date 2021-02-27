/**
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

#include "d3d8wrapper.h"

HRESULT m_IDirect3DVolumeTexture8::QueryInterface(THIS_ REFIID riid, void** ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	if ((riid == IID_IDirect3DVolumeTexture8 || riid == IID_IUnknown || riid == IID_IDirect3DResource8 || riid == IID_IDirect3DBaseTexture8) && ppvObj)
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

ULONG m_IDirect3DVolumeTexture8::AddRef(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DVolumeTexture8::Release(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->Release();
}

HRESULT m_IDirect3DVolumeTexture8::GetDevice(THIS_ IDirect3DDevice8** ppDevice)
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

HRESULT m_IDirect3DVolumeTexture8::SetPrivateData(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

HRESULT m_IDirect3DVolumeTexture8::GetPrivateData(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
}

HRESULT m_IDirect3DVolumeTexture8::FreePrivateData(THIS_ REFGUID refguid)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->FreePrivateData(refguid);
}

DWORD m_IDirect3DVolumeTexture8::SetPriority(THIS_ DWORD PriorityNew)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetPriority(PriorityNew);
}

DWORD m_IDirect3DVolumeTexture8::GetPriority(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetPriority();
}

void m_IDirect3DVolumeTexture8::PreLoad(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->PreLoad();
}

D3DRESOURCETYPE m_IDirect3DVolumeTexture8::GetType(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetType();
}

DWORD m_IDirect3DVolumeTexture8::SetLOD(THIS_ DWORD LODNew)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetLOD(LODNew);
}

DWORD m_IDirect3DVolumeTexture8::GetLOD(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetLOD();
}

DWORD m_IDirect3DVolumeTexture8::GetLevelCount(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetLevelCount();
}

HRESULT m_IDirect3DVolumeTexture8::GetLevelDesc(THIS_ UINT Level, D3DVOLUME_DESC *pDesc)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetLevelDesc(Level, pDesc);
}

HRESULT m_IDirect3DVolumeTexture8::GetVolumeLevel(THIS_ UINT Level, IDirect3DVolume8** ppVolumeLevel)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->GetVolumeLevel(Level, ppVolumeLevel);

	if (SUCCEEDED(hr) && ppVolumeLevel)
	{
		*ppVolumeLevel = m_pDevice->ProxyAddressLookupTableD3d8->FindAddress<m_IDirect3DVolume8>(*ppVolumeLevel);
	}

	return hr;
}

HRESULT m_IDirect3DVolumeTexture8::LockBox(THIS_ UINT Level, D3DLOCKED_BOX* pLockedVolume, CONST D3DBOX* pBox, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->LockBox(Level, pLockedVolume, pBox, Flags);
}

HRESULT m_IDirect3DVolumeTexture8::UnlockBox(THIS_ UINT Level)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->UnlockBox(Level);
}

HRESULT m_IDirect3DVolumeTexture8::AddDirtyBox(THIS_ CONST D3DBOX* pDirtyBox)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->AddDirtyBox(pDirtyBox);
}

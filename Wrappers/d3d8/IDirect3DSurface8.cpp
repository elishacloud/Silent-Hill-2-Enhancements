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

HRESULT m_IDirect3DSurface8::QueryInterface(THIS_ REFIID riid, void** ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	// Get proxy interface
	if (riid == IID_GetProxyInterface && ppvObj)
	{
		*ppvObj = ProxyInterface;
		return S_OK;
	}

	// Set texture interface
	if (riid == IID_SetTextureRenderTarget)
	{
		IsTextureRenderTarget = true;
		return S_OK;
	}

	// Set as a surface of a texture
	if (riid == IID_SetSurfaceOfTexture)
	{
		IsTextureOfSurface = true;
		return S_OK;
	}

	// Get render target interface
	if (riid == IID_GetRenderTarget && ppvObj)
	{
		if (CopyRenderTarget && IsTextureRenderTarget && !RenderInterface && !ReplacedInterface && m_pDevice)
		{
			D3DSURFACE_DESC Desc;
			if (SUCCEEDED(ProxyInterface->GetDesc(&Desc)) &&
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
	if (riid == IID_ClearCachedSurfaces)
	{
		RenderInterface = nullptr;
		ReplacedInterface = nullptr;
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

	ULONG ref = ProxyInterface->Release();

	if (IsTextureOfSurface)
	{
		ref = min(ref, ref - 1);
	}

	if (ref == 0 && pEmuSurface)
	{
		pEmuSurface->UnlockRect();
		pEmuSurface->Release();
		pEmuSurface = nullptr;
	}

	return ref;
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

	if (hr == D3DERR_INVALIDCALL && !IsLocked && pLockedRect && !pEmuSurface && (DeviceMultiSampleType != D3DMULTISAMPLE_NONE || UsingScaledResolutions))
	{
		D3DSURFACE_DESC Desc;
		if (SUCCEEDED(GetDesc(&Desc)))
		{
			DWORD bytes = GetBitCount(Desc.Format) / 8;
			if (!bytes)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Surface Lock error: " << (D3DERR)hr);
				return hr;
			}

			// Copy surface data to memory
			if (SUCCEEDED(m_pDevice->CreateImageSurface(Desc.Width, Desc.Height, Desc.Format, &pEmuSurface)) && pEmuSurface)
			{
				EmuReadOnly = (Flags & D3DLOCK_READONLY);
				EmuRect.left = 0;
				EmuRect.top = 0;
				EmuRect.right = (LONG)Desc.Width;
				EmuRect.bottom = (LONG)Desc.Height;
				if (pRect) { memcpy(&EmuRect, pRect, sizeof(RECT)); }
				POINT Point = { EmuRect.left, EmuRect.top };

				if (SUCCEEDED(m_pDevice->CopyRects(this, &EmuRect, 1, pEmuSurface, &Point)))
				{
					D3DLOCKED_RECT LockedRect = { NULL };
					if (SUCCEEDED(pEmuSurface->LockRect(&LockedRect, &EmuRect, Flags)) && LockedRect.pBits)
					{
						pLockedRect->pBits = LockedRect.pBits;
						pLockedRect->Pitch = LockedRect.Pitch;
						return D3D_OK;
					}
					else
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: locking emulated surface!");
					}
				}
				else
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: copying emulated surface!");
				}
				pEmuSurface->Release();
				pEmuSurface = nullptr;
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: creating emulated surface!");
			}

			LOG_LIMIT(100, __FUNCTION__ << " Error: Surface Lock error: " << (D3DERR)hr);

			return hr;
		}
	}
	else if (SUCCEEDED(hr))
	{
		IsLocked = true;
	}

	return hr;
}

HRESULT m_IDirect3DSurface8::UnlockRect(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = D3D_OK;

	// Copy data back from emulated surface
	if (pEmuSurface)
	{
		hr = pEmuSurface->UnlockRect();
		POINT Point = { EmuRect.left, EmuRect.top };
		if (!EmuReadOnly && FAILED(m_pDevice->CopyRects(pEmuSurface, &EmuRect, 1, this, &Point)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: copying surface!");
		}
		pEmuSurface->Release();
		pEmuSurface = nullptr;
	}
	else
	{
		hr = ProxyInterface->UnlockRect();

		if (SUCCEEDED(hr))
		{
			IsLocked = false;
		}
	}

	return hr;
}

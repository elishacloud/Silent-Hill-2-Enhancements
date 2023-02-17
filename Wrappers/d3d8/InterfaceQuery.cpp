/**
* Copyright (C) 2023 Elisha Riedlinger
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

void genericQueryInterface(REFIID riid, LPVOID *ppvObj, m_IDirect3DDevice8* m_pDevice)
{
	if (!ppvObj || !*ppvObj || !m_pDevice)
	{
		return;
	}

	if (riid == IID_IDirect3D8)
	{
		IDirect3D8 *pD3D8 = nullptr;
		if (SUCCEEDED(m_pDevice->GetDirect3D(&pD3D8)) && pD3D8)
		{
			*ppvObj = pD3D8;
			pD3D8->Release();
			return;
		}
	}

	if (riid == IID_IDirect3DDevice8)
	{
		*ppvObj = m_pDevice;
		return;
	}

#define QUERYINTERFACE(x) \
	if (riid == IID_ ## x) \
		{ \
			*ppvObj = m_pDevice->ProxyAddressLookupTableD3d8->FindAddress<m_ ## x>(*ppvObj); \
			return; \
		}

	QUERYINTERFACE(IDirect3DTexture8);
	QUERYINTERFACE(IDirect3DCubeTexture8);
	QUERYINTERFACE(IDirect3DVolumeTexture8);
	QUERYINTERFACE(IDirect3DVertexBuffer8);
	QUERYINTERFACE(IDirect3DIndexBuffer8);
	QUERYINTERFACE(IDirect3DSurface8);
	QUERYINTERFACE(IDirect3DVolume8);
	QUERYINTERFACE(IDirect3DSwapChain8);
}

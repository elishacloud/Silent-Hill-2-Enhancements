/**
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

#include "Resource.h"
#include "d3d9wrapper.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "External\Hooking\Hook.h"

Direct3DCreate9Proc m_pDirect3DCreate9 = nullptr;

IDirect3D9 *WINAPI Direct3DCreate9Wrapper(UINT SDKVersion)
{
	if (!m_pDirect3DCreate9)
	{
		Logging::Log() << __FUNCTION__ << " Error finding 'Direct3DCreate9'";
		return nullptr;
	}

	LOG_ONCE("Initializing crosire's ReShade version '" RESHADE_STRING_FILE "' (32-bit) built on '" RESHADE_DATE " " RESHADE_TIME "' loaded ...");

	IDirect3D9 *pD3D9 = m_pDirect3DCreate9(SDKVersion);

	RunDelayedOneTimeItems();

	if (pD3D9)
	{
		return new m_IDirect3D9((IDirect3D9Ex*)pD3D9);
	}

	return nullptr;
}

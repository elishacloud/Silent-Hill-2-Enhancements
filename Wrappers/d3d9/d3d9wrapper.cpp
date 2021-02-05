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

#include "Resources\sh2-enhce.h"
#include "d3d9wrapper.h"
#include "Common\Settings.h"
#include "External\Hooking\Hook.h"

Direct3DCreate9Proc m_pDirect3DCreate9 = nullptr;

HMODULE d3d9dll = nullptr;

void Initd3d9()
{
	static bool RunOnce = true;
	if (!RunOnce)
	{
		return;
	}
	RunOnce = false;

	// Load dll
	char path[MAX_PATH];
	strcpy_s(path, "d3d9.dll");
	Logging::Log() << "Loading " << path;
	d3d9dll = LoadLibraryA(path);
	if (!d3d9dll)
	{
		Logging::Log() << __FUNCTION__ << " Error: Cannnot open d3d9.dll!";
		return;
	}

	// Get function addresses
	m_pDirect3DCreate9 = (Direct3DCreate9Proc)GetProcAddress(d3d9dll, "Direct3DCreate9");
}

IDirect3D9 *WINAPI rs_Direct3DCreate9(UINT SDKVersion)
{
	Initd3d9();
	if (!m_pDirect3DCreate9)
	{
		Logging::Log() << __FUNCTION__ << " Error finding 'Direct3DCreate9'";
		return nullptr;
	}

	LOG_ONCE("Initializing crosire's ReShade version '" RESHADE_STRING_FILE "' (32-bit) built on '" RESHADE_DATE " " RESHADE_TIME "' loaded ...");

	IDirect3D9 *pD3D9 = m_pDirect3DCreate9(SDKVersion);

	if (pD3D9)
	{
		return new m_IDirect3D9((IDirect3D9Ex*)pD3D9);
	}

	return nullptr;
}

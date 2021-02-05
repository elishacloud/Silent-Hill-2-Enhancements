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
*
* ValidatePixelShader and ValidateVertexShader created from source code found in Wine
* https://github.com/alexhenrie/wine/tree/master/dlls/d3d8
*/

#define WIN32_LEAN_AND_MEAN
#include "d3d9\d3d9wrapper.h"
#include <Shlwapi.h>
#include "External\d3d8to9\source\d3d8to9.hpp"
#include "External\d3d8to9\source\d3dx9.hpp"
#include "wrapper.h"
#include "d3d8to9.h"
#include "External\Hooking\Hook.h"
#include "Common\Utils.h"
#include "Common\LoadModules.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"
#include "BuildNo.rc"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define APP_VERSION TOSTRING(FILEVERSION)

Direct3DCreate9Proc p_Direct3DCreate9 = nullptr;

PFN_D3DXAssembleShader D3DXAssembleShader = nullptr;
PFN_D3DXDisassembleShader D3DXDisassembleShader = nullptr;
PFN_D3DXLoadSurfaceFromSurface D3DXLoadSurfaceFromSurface = nullptr;

// Redirects or hooks 'Direct3DCreate8' to go to d3d8to9
void EnableD3d8to9()
{
	d3d8::ValidatePixelShader_var = (FARPROC)*d8_ValidatePixelShader;
	d3d8::ValidateVertexShader_var = (FARPROC)*d8_ValidateVertexShader;
	d3d8::Direct3DCreate8_var = (FARPROC)*Direct3DCreate8to9;
	m_pDirect3DCreate8 = (Direct3DCreate8Proc)*Direct3DCreate8to9;
}

// Initializes d3d9 and Direct3DCreate9 for d3d8to9
void Initd3d8to9()
{
	if (!EnableCustomShaders)
	{
		Initd3d9();
		p_Direct3DCreate9 = m_pDirect3DCreate9;
	}
	else
	{
		p_Direct3DCreate9 = rs_Direct3DCreate9;
	}
}

HRESULT WINAPI d8_ValidatePixelShader(DWORD* pixelshader, DWORD* reserved1, BOOL flag, DWORD* toto)
{
	UNREFERENCED_PARAMETER(flag);
	UNREFERENCED_PARAMETER(toto);

	LOG_LIMIT(1, __FUNCTION__);

	if (!pixelshader)
	{
		return D3DERR_INVALIDCALL;
	}
	if (reserved1)
	{
		return D3DERR_INVALIDCALL;
	}
	switch (*pixelshader)
	{
	case 0xFFFF0100:
	case 0xFFFF0101:
	case 0xFFFF0102:
	case 0xFFFF0103:
	case 0xFFFF0104:
		return D3D_OK;
		break;
	default:
		return D3DERR_INVALIDCALL;
	}
}

HRESULT WINAPI d8_ValidateVertexShader(DWORD* vertexshader, DWORD* reserved1, DWORD* reserved2, BOOL flag, DWORD* toto)
{
	UNREFERENCED_PARAMETER(flag);
	UNREFERENCED_PARAMETER(toto);

	LOG_LIMIT(1, __FUNCTION__);

	if (!vertexshader)
	{
		return D3DERR_INVALIDCALL;
	}
	if (reserved1 || reserved2)
	{
		return D3DERR_INVALIDCALL;
	}
	switch (*vertexshader)
	{
	case 0xFFFE0100:
	case 0xFFFE0101:
		return D3D_OK;
		break;
	default:
		return D3DERR_INVALIDCALL;
	}
}

// Handles calls to 'Direct3DCreate8' for d3d8to9
Direct3D8 *WINAPI Direct3DCreate8to9(UINT SDKVersion)
{
	UNREFERENCED_PARAMETER(SDKVersion);

	Initd3d8to9();
	if (!p_Direct3DCreate9)
	{
		Logging::Log() << __FUNCTION__ << " Error finding 'Direct3DCreate9'";
		return nullptr;
	}

	LOG_ONCE("Starting D3d8to9 v" << APP_VERSION);

	Logging::Log() << "Redirecting 'Direct3DCreate8' to --> 'Direct3DCreate9' (" << SDKVersion << ")";

	IDirect3D9 *const d3d = p_Direct3DCreate9(D3D_SDK_VERSION);

	if (d3d == nullptr)
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create 'Direct3DCreate9'!";
		return nullptr;
	}

	// Load D3DX
	if (!D3DXAssembleShader || !D3DXDisassembleShader || !D3DXLoadSurfaceFromSurface)
	{
		// Declare module vars
		static HMODULE d3dx9Module = nullptr;

		// Declare d3dx9_xx.dll name
		wchar_t d3dx9name[MAX_PATH] = { 0 };

		// Declare d3dx9_xx.dll version
		for (int x = 99; x > 9 && !d3dx9Module; x--)
		{
			// Get dll name
			swprintf_s(d3dx9name, L"d3dx9_%d.dll", x);
			// Load dll
			d3dx9Module = LoadLibrary(d3dx9name);
		}

		// Extract d3dx9 tools from binary
		if (!d3dx9Module)
		{
			ExtractD3DX9Tools();
			d3dx9Module = LoadLibrary(L"d3dx9_43.dll");
		}

		if (d3dx9Module)
		{
			Logging::Log() << "Loaded " << d3dx9name;
			D3DXAssembleShader = reinterpret_cast<PFN_D3DXAssembleShader>(GetProcAddress(d3dx9Module, "D3DXAssembleShader"));
			D3DXDisassembleShader = reinterpret_cast<PFN_D3DXDisassembleShader>(GetProcAddress(d3dx9Module, "D3DXDisassembleShader"));
			D3DXLoadSurfaceFromSurface = reinterpret_cast<PFN_D3DXLoadSurfaceFromSurface>(GetProcAddress(d3dx9Module, "D3DXLoadSurfaceFromSurface"));
		}
		else
		{
			// Should never get here
			Logging::Log() << __FUNCTION__ << " Error: Failed to load d3dx9_xx.dll! Some features will not work correctly.";
		}
	}

	// Check if WineD3D is loaded
	if (GetModuleHandle(L"libwine.dll") || GetModuleHandle(L"wined3d.dll"))
	{
		Logging::Log() << __FUNCTION__ << " Warning: WineD3D detected!  It is not recommended to use WineD3D with Silent Hill 2 Enhancements.";
	}

	return new Direct3D8(d3d);
}

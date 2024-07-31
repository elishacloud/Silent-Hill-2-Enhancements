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
*
* ValidatePixelShader and ValidateVertexShader created from source code found in Wine
* https://github.com/alexhenrie/wine/tree/master/dlls/d3d8
*/

#define WIN32_LEAN_AND_MEAN
#include "d3d9\d3d9wrapper.h"
#include "External\d3d8to9\source\d3d8to9.hpp"
#include "External\d3d8to9\source\d3dx9.hpp"
#include "wrapper.h"
#include "d3d8to9.h"
#include "External\Hooking\Hook.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"
#include "BuildNo.rc"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define APP_VERSION TOSTRING(FILEVERSION)

extern FARPROC f_D3DXAssembleShader;
extern FARPROC f_D3DXDisassembleShader;
extern FARPROC f_D3DXLoadSurfaceFromSurface;

PFN_D3DXAssembleShader D3DXAssembleShader = (PFN_D3DXAssembleShader)f_D3DXAssembleShader;
PFN_D3DXDisassembleShader D3DXDisassembleShader = (PFN_D3DXDisassembleShader)f_D3DXDisassembleShader;
PFN_D3DXLoadSurfaceFromSurface D3DXLoadSurfaceFromSurface = (PFN_D3DXLoadSurfaceFromSurface)f_D3DXLoadSurfaceFromSurface;

extern Direct3DCreate8Proc m_pDirect3DCreate8_d3d8to9;

// Redirects 'Direct3DCreate8' to go to d3d8to9
void EnableD3d8to9()
{
	d3d8::ValidatePixelShader_var = (FARPROC)*d8_ValidatePixelShader;
	d3d8::ValidateVertexShader_var = (FARPROC)*d8_ValidateVertexShader;
	d3d8::Direct3DCreate8_var = (FARPROC)*Direct3DCreate8to9;
	m_pDirect3DCreate8_d3d8to9 = (Direct3DCreate8Proc)*Direct3DCreate8to9;
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

	LOG_ONCE("Starting D3d8to9 v" << APP_VERSION);

	Logging::Log() << "Redirecting 'Direct3DCreate8' to --> 'Direct3DCreate9' (" << SDKVersion << ")";

	IDirect3D9 *const d3d = Direct3DCreate9Wrapper(D3D_SDK_VERSION);

	if (!d3d)
	{
		return nullptr;
	}

	IsUsingD3d8to9 = true;

	return new Direct3D8(d3d);
}

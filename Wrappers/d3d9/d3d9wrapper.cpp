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

#include "Resource.h"
#include "d3d9wrapper.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "External\Hooking\Hook.h"

Direct3DCreate9Proc m_pDirect3DCreate9 = nullptr;

HMODULE GetSystemD3d9()
{
	static HMODULE h_d3d9 = nullptr;

	// Get System d3d9.dll
	if (!h_d3d9)
	{
		char Path[MAX_PATH] = {};
		GetSystemDirectoryA(Path, MAX_PATH);
		strcat_s(Path, "\\d3d9.dll");
		GetModuleHandleExA(NULL, Path, &h_d3d9);
	}

	return h_d3d9;
}

FARPROC GetD3d9UnnamedOrdinal(WORD Ordinal)
{
	FARPROC proc = nullptr;

	HMODULE dll = GetSystemD3d9();
	if (!dll)
	{
		Logging::Log() << __FUNCTION__ << " System32 d3d9.dll is not loaded!";
		return nullptr;
	}

	proc = GetProcAddress(dll, reinterpret_cast<LPCSTR>(Ordinal));

	bool FuncNameExists = Hook::CheckExportAddress(dll, proc);

	if (!proc || FuncNameExists)
	{
		Logging::Log() << __FUNCTION__ << " cannot find unnamed ordinal '" << Ordinal << "' in System32 d3d9.dll!";
		return nullptr;
	}

	return proc;
}

void WINAPI Direct3D9ForceHybridEnumeration(UINT Mode)
{
	const WORD Ordinal = 16;

	static FARPROC proc = nullptr;
	if (!proc)
	{
		proc = GetD3d9UnnamedOrdinal(Ordinal);

		if (!proc)
		{
			return;
		}
	}

	Logging::Log() << __FUNCTION__ << " Calling 'Direct3D9ForceHybridEnumeration' ... " << Mode;

	reinterpret_cast<decltype(&Direct3D9ForceHybridEnumeration)>(proc)(Mode);
}

void WINAPI Direct3D9SetSwapEffectUpgradeShim(int Unknown)
{
	const WORD Ordinal = 18;

	static FARPROC proc = nullptr;
	if (!proc)
	{
		proc = GetD3d9UnnamedOrdinal(Ordinal);

		if (!proc)
		{
			return;
		}
	}

	Logging::Log() << __FUNCTION__ << " Calling 'Direct3D9SetSwapEffectUpgradeShim' ... " << Unknown;

	reinterpret_cast<decltype(&Direct3D9SetSwapEffectUpgradeShim)>(proc)(Unknown);
}

bool Direct3D9DisableMaximizedWindowedMode()
{
	static Direct3D9EnableMaximizedWindowedModeShimProc Direct3D9EnableMaximizedWindowedModeShim = nullptr;

	if (!Direct3D9EnableMaximizedWindowedModeShim)
	{
		// Load d3d9.dll from System32
		HMODULE dll = GetSystemD3d9();

		if (!dll)
		{
			Logging::Log() << __FUNCTION__ << " d3d9.dll is not loaded!";
			return false;
		}

		// Get function address
		BYTE* addr = (BYTE*)GetProcAddress(dll, "Direct3D9EnableMaximizedWindowedModeShim");
		if (!addr)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to get `Direct3D9EnableMaximizedWindowedModeShim` address!";
			return false;
		}

		// Check memory address
		if (*(BYTE*)(addr + 6) != 1)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to vaidate memory address!";
			return false;
		}

		// Update function to disable Maximized Windowed Mode
		DWORD Protect;
		BOOL ret = VirtualProtect((LPVOID)(addr + 6), 1, PAGE_EXECUTE_READWRITE, &Protect);
		if (ret == 0)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to VirtualProtect memory!";
			return false;
		}
		*(BYTE*)(addr + 6) = 0;
		VirtualProtect((LPVOID)(addr + 6), 1, Protect, &Protect);

		// Set function address
		Direct3D9EnableMaximizedWindowedModeShim = (Direct3D9EnableMaximizedWindowedModeShimProc)addr;
	}

	// Launch function to disable Maximized Windowed Mode
	Logging::Log() << __FUNCTION__ << " Disabling MaximizedWindowedMode for Direct3D9! Ret = " << (void*)Direct3D9EnableMaximizedWindowedModeShim(0);

	return true;
}

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

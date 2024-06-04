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
* Code for 'Direct3D9SetSwapEffectUpgradeShim' taken from here:  https://github.com/crosire/reshade/commit/3fe0b050706fb9f3510ed48d619cad71f7cb28f2
*/

#include "Resource.h"
#include "d3d9wrapper.h"
#include "Patches\Patches.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "External\Hooking\Hook.h"

Direct3DCreate9Proc m_pDirect3DCreate9 = nullptr;
Direct3DCreate9Proc m_pDirect3DCreate9_local = nullptr;
Direct3DCreate9On12Proc m_pDirect3DCreate9On12 = nullptr;

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

// Get 'Direct3DCreate9On12' for d3d9.dll
bool GetDirect3DCreate9On12()
{
	// Only allow function to run once
	static bool AlreadyRun = false;
	if (AlreadyRun)
	{
		return (m_pDirect3DCreate9On12 != nullptr);
	}
	AlreadyRun = true;

	// Load d3d9.dll
	HMODULE h_d3d9 = GetSystemD3d9();
	if (h_d3d9)
	{
		m_pDirect3DCreate9On12 = (Direct3DCreate9On12Proc)GetProcAddress(h_d3d9, "Direct3DCreate9On12");
		if (m_pDirect3DCreate9On12)
		{
			return true;
		}
	}
	Logging::Log() << __FUNCTION__ << " Warning: Failed to get `Direct3DCreate9On12` proc!";
	return false;
}

// Get 'Direct3DCreate9' for d3d9.dll
bool GetDirect3DCreate9()
{
	// Only allow function to run once
	static bool AlreadyRun = false;
	if (AlreadyRun)
	{
		return (m_pDirect3DCreate9 != nullptr);
	}
	AlreadyRun = true;

	HMODULE h_d3d9 = LoadLibrary(L"d3d9.dll");
	if (h_d3d9)
	{
		m_pDirect3DCreate9 = (Direct3DCreate9Proc)GetProcAddress(h_d3d9, "Direct3DCreate9");
		if (m_pDirect3DCreate9)
		{
			return true;
		}
	}
	Logging::Log() << __FUNCTION__ << " Warning: Failed to get `Direct3DCreate9` proc!";
	return false;
}

// Get 'Direct3DCreate9' for local d3d9.dll
bool GetLocalDirect3DCreate9()
{
	// Only allow function to run once
	static bool AlreadyRun = false;
	if (AlreadyRun)
	{
		return (m_pDirect3DCreate9_local != nullptr);
	}
	AlreadyRun = true;

	// Get proc address
	HMODULE h_d3d9 = nullptr;
	char Path[MAX_PATH] = {};
	GetModulePath(Path, MAX_PATH);
	strcat_s(Path, "\\d3d9.dll");
	GetModuleHandleExA(NULL, Path, &h_d3d9);
	if (h_d3d9)
	{
		m_pDirect3DCreate9_local = (Direct3DCreate9Proc)GetProcAddress(h_d3d9, "Direct3DCreate9");
		if (m_pDirect3DCreate9_local)
		{
			return true;
		}
	}
	return false;
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
		FARPROC addr = GetD3d9UnnamedOrdinal(Ordinal);

		if (!addr)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to get address!";
			return;
		}

		if (memcmp(addr, "\x8B\xFF\x55\x8B\xEC\x8B\x45\x08\xA3", 9) != S_OK)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to vaidate memory address!";
			return;
		}

		proc = addr;
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
		FARPROC addr = GetD3d9UnnamedOrdinal(Ordinal);

		if (!addr)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to get address!";
			return;
		}

		if (memcmp(addr, "\x8B\xFF\x55\x8B\xEC\x8B\x0D", 7) != S_OK)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to vaidate memory address!";
			return;
		}

		proc = addr;
	}

	Logging::Log() << __FUNCTION__ << " Calling 'Direct3D9SetSwapEffectUpgradeShim' ... " << Unknown;

	reinterpret_cast<decltype(&Direct3D9SetSwapEffectUpgradeShim)>(proc)(Unknown);
}

void WINAPI Direct3D9DisableMaximizedWindowedMode()
{
	static FARPROC proc = nullptr;

	if (!proc)
	{
		HMODULE dll = GetSystemD3d9();
		if (!dll)
		{
			Logging::Log() << __FUNCTION__ << " System32 d3d9.dll is not loaded!";
			return;
		}

		FARPROC addr = GetProcAddress(dll, "Direct3D9EnableMaximizedWindowedModeShim");
		if (!addr)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to get address!";
			return;
		}

		if (memcmp(addr, "\x8B\xFF\x55\x8B\xEC\x6A\x01\xFF\x75\x08\xE8", 11) != S_OK)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to vaidate memory address!";
			return;
		}

		DWORD Protect;
		if (VirtualProtect((LPVOID)((BYTE*)addr + 6), 1, PAGE_EXECUTE_READWRITE, &Protect) == FALSE)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to VirtualProtect memory!";
			return;
		}
		*(BYTE*)((BYTE*)addr + 6) = 0;
		VirtualProtect((LPVOID)((BYTE*)addr + 6), 1, Protect, &Protect);

		proc = addr;
	}

	Logging::Log() << __FUNCTION__ << " Calling 'Direct3D9EnableMaximizedWindowedModeShim' to disable MaximizedWindowedMode ...";

	reinterpret_cast<decltype(&Direct3D9DisableMaximizedWindowedMode)>(proc)();

	return;
}

IDirect3D9* WINAPI Direct3DCreate9Wrapper(UINT SDKVersion)
{
	if (ForceHybridEnumeration)
	{
		Direct3D9ForceHybridEnumeration(1);
	}

	if (SetSwapEffectUpgradeShim)
	{
		Direct3D9SetSwapEffectUpgradeShim(0);
	}

	if (DisableMaximizedWindowedMode)
	{
		Direct3D9DisableMaximizedWindowedMode();
	}

	IDirect3D9* pD3D9 = nullptr;

	// Try loading 9On12 version of dll
	if (Direct3DCreate9On12 && GetDirect3DCreate9On12())
	{
		Logging::Log() << __FUNCTION__ << " Attempting to load 'Direct3DCreate9On12'...";

		// Setup arguments
		D3D9ON12_ARGS args;
		memset(&args, 0, sizeof(args));
		args.Enable9On12 = TRUE;

		// Call function
		pD3D9 = m_pDirect3DCreate9On12(SDKVersion, &args, 1);
	}

	// Try loading local version of dll
	if (!pD3D9 && GetLocalDirect3DCreate9())
	{
		pD3D9 = m_pDirect3DCreate9_local(SDKVersion);
	}

	// Try loading base version of dll
	if (!pD3D9 && GetDirect3DCreate9())
	{
		pD3D9 = m_pDirect3DCreate9(SDKVersion);
	}

	// Check device creation
	if (!pD3D9)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to create Direct3D9 device!";
		return nullptr;
	}

	// Use Direct3D9 wrapper if shaders are enabled
	if (EnableCustomShaders)
	{
		LOG_ONCE("Initializing crosire's ReShade version '" RESHADE_STRING_FILE "' (32-bit) built on '" RESHADE_DATE " " RESHADE_TIME "' loaded ...");

		return new m_IDirect3D9((IDirect3D9Ex*)pD3D9);
	}

	return pD3D9;
}

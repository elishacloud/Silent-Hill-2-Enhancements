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
#include "Wrappers\wrapper.h"
#include "Common\Utils.h"

Direct3DCreate8Proc m_pDirect3DCreate8 = nullptr;
Direct3DCreate8Proc m_pDirect3DCreate8_d3d8to9 = nullptr;
Direct3DCreate8Proc m_pDirect3DCreate8_script = nullptr;
Direct3DCreate8Proc m_pDirect3DCreate8_local = nullptr;

HMODULE GetD3d8ScriptDll()
{
	// Load wrapper d3d8 from scripts or plugins folder
	HMODULE script_d3d8_dll = nullptr;

	// Get script paths
	wchar_t scriptpath[MAX_PATH];
	bool ret = GetSH2FolderPath(scriptpath, MAX_PATH);
	wchar_t* pdest = wcsrchr(scriptpath, '\\');
	if (ret && pdest)
	{
		*(pdest + 1) = '\0';
	}
	std::wstring script_path(scriptpath + std::wstring(L"scripts"));
	std::wstring script_path_dll(script_path + L"\\d3d8.dll");
	std::wstring plugin_path(scriptpath + std::wstring(L"plugins"));
	std::wstring plugin_path_dll(plugin_path + L"\\d3d8.dll");

	// Store the current folder
	wchar_t currentDir[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, currentDir);

	// Load d3d8.dll from 'scripts' folder
	SetCurrentDirectory(script_path.c_str());
	script_d3d8_dll = LoadLibrary(script_path_dll.c_str());
	if (script_d3d8_dll)
	{
		Logging::Log() << "Loaded d3d8.dll from: " << script_path_dll.c_str();
		PinModule(script_d3d8_dll);
	}

	// Load d3d8.dll from 'plugins' folder
	if (!script_d3d8_dll)
	{
		SetCurrentDirectory(plugin_path.c_str());
		script_d3d8_dll = LoadLibrary(plugin_path_dll.c_str());
		if (script_d3d8_dll)
		{
			Logging::Log() << "Loaded d3d8.dll from: " << plugin_path_dll.c_str();
			PinModule(script_d3d8_dll);
		}
	}

	// Set current folder back
	SetCurrentDirectory(currentDir);

	return script_d3d8_dll;
}

// Hook d3d8 API
void HookDirect3DCreate8(HMODULE ScriptDll)
{
	// Get Direct3DCreate8 address
	constexpr BYTE SearchBytes[]{ 0x84, 0xC0, 0x74, 0x06, 0xB8, 0x03, 0x00, 0x00, 0x00, 0xC3, 0x68, 0xDC, 0x00, 0x00, 0x00, 0xE8 };
	DWORD Address = SearchAndGetAddresses(0x004F6315, 0x004F65C5, 0x004F5E85, SearchBytes, sizeof(SearchBytes), 0x0F, __FUNCTION__);

	// Checking address pointer
	if (!Address)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get function address
	m_pDirect3DCreate8 = (Direct3DCreate8Proc)(Address + 5 + *(DWORD*)(Address + 1));

	// Get defined d3d8 script wrapper
	if (ScriptDll)
	{
		m_pDirect3DCreate8_script = (Direct3DCreate8Proc)GetProcAddress(ScriptDll, "Direct3DCreate8");
	}

	// Write to memory
	WriteCalltoMemory((BYTE*)Address, *Direct3DCreate8Wrapper, 5);
}

// Get 'Direct3DCreate8' for local d3d8.dll
bool GetLocalDirect3DCreate8()
{
	// Only allow function to run once
	static bool AlreadyRun = false;
	if (AlreadyRun)
	{
		return (m_pDirect3DCreate8_local != nullptr);
	}
	AlreadyRun = true;

	// Get proc address
	char Path[MAX_PATH] = {};
	GetModulePath(Path, MAX_PATH);
	strcat_s(Path, "\\d3d8.dll");
	HMODULE h_d3d8 = LoadLibraryA(Path);
	if (h_d3d8)
	{
		m_pDirect3DCreate8_local = (Direct3DCreate8Proc)GetProcAddress(h_d3d8, "Direct3DCreate8");
		if (m_pDirect3DCreate8_local)
		{
			PinModule(h_d3d8);
			return true;
		}
	}
	return false;
}

IDirect3D8 *WINAPI Direct3DCreate8Wrapper(UINT SDKVersion)
{
	LOG_LIMIT(3, "Redirecting 'Direct3DCreate8' ...");

	LPDIRECT3D8 pD3D8 = nullptr;

	// Try loading d3d8to9
	if (m_pDirect3DCreate8_d3d8to9)
	{
		pD3D8 = m_pDirect3DCreate8_d3d8to9(SDKVersion);
	}

	// Try loading script version of dll
	if (!pD3D8 && m_pDirect3DCreate8_script)
	{
		pD3D8 = m_pDirect3DCreate8_script(SDKVersion);
	}

	// Try loading local version of dll
	if (!pD3D8 && GetLocalDirect3DCreate8())
	{
		pD3D8 = m_pDirect3DCreate8_local(SDKVersion);
	}

	// Try loading base version of dll
	if (!pD3D8 && m_pDirect3DCreate8)
	{
		pD3D8 = m_pDirect3DCreate8(SDKVersion);
	}

	// Check device creation
	if (!pD3D8)
	{
		Logging::Log() << __FUNCTION__ << " Error: 'Direct3DCreate8' Failed!";
		return nullptr;
	}

	// Check if WineD3D is loaded
	if (GetModuleHandle(L"libwine.dll") || GetModuleHandle(L"wined3d.dll"))
	{
		Logging::Log() << __FUNCTION__ << " Warning: WineD3D detected!  It is not recommended to use WineD3D with Silent Hill 2 Enhancements.";
	}

	RunDelayedOneTimeItems();

	return new m_IDirect3D8(pD3D8);
}

void RunPresentCode(IDirect3DDevice8* ProxyInterface, LONG Width, LONG Height)
{
	UNREFERENCED_PARAMETER(ProxyInterface);
	UNREFERENCED_PARAMETER(Width);
	UNREFERENCED_PARAMETER(Height);
	// Blank function
}

void RunResetCode(IDirect3DDevice8* ProxyInterface)
{
	UNREFERENCED_PARAMETER(ProxyInterface);
	// Blank function
}
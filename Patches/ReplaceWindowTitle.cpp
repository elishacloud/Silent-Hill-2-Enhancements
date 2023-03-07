/**
* Copyright (C) 2022 Bruno Russi
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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "resource.h"
#include "Common\Settings.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "External\Hooking.Patterns\Hooking.Patterns.h"

// Replace the vanilla "SH2PC Title" with ours
void PatchWindowTitle()
{
	struct ClassHandler
	{
		static HWND WINAPI CreateWindowExAHandler(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
		{
			lpWindowName = "Silent Hill 2: Enhanced Edition";
			X = 0;
			Y = 0;

			return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		}
	};

	auto pattern = hook::pattern("FF 15 ? ? ? ? 85 C0 A3 ? ? ? ? 74 ? 8B 0D ? ? ? ? 6A");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	Logging::Log() << "Patching window title...";

	WriteCalltoMemory((BYTE*)pattern.count(1).get(0).get<uint32_t*>(0), ClassHandler::CreateWindowExAHandler, 6);
}

// Add icon to the Windows taskbar
void PatchWindowIcon()
{
	struct ClassHandler
	{
		static ATOM WINAPI RegisterClassExAHandler(WNDCLASSEXA* lpwcx)
		{
			lpwcx->hIcon = LoadIconA(m_hModule, MAKEINTRESOURCEA(OIC_SH2_ICON));
			lpwcx->hIconSm = LoadIconA(m_hModule, MAKEINTRESOURCEA(OIC_SH2_ICON));
			lpwcx->hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
			if (!lpwcx->hIcon || !lpwcx->hIconSm)
			{
				Logging::Log() << __FUNCTION__ " Error: failed to create icon!";
			}

			return RegisterClassExA(lpwcx);
		}
	};

	// Get RegisterClass address
	constexpr BYTE SearchBytesRegisterClass[]{ 0x8D, 0x4C, 0x24, 0x04, 0x51, 0x89, 0x44, 0x24, 0x24, 0xC7, 0x44, 0x24, 0x28, 0x06, 0x00, 0x00, 0x00, 0x89, 0x74, 0x24, 0x2C, 0xC7, 0x44, 0x24, 0x30 };
	DWORD RegisterClassAddr = SearchAndGetAddresses(0x0040699E, 0x0040699E, 0x004069AE, SearchBytesRegisterClass, sizeof(SearchBytesRegisterClass), 0x21, __FUNCTION__);

	if (!RegisterClassAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	Logging::Log() << "Patching window icon...";

	WriteCalltoMemory((BYTE*)RegisterClassAddr, ClassHandler::RegisterClassExAHandler, 6);
}

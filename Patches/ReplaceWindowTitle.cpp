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
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "External/Hooking.Patterns/Hooking.Patterns.h"

HWND __stdcall CreateWindowExA_Hook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	UNREFERENCED_PARAMETER(lpWindowName);

	return CreateWindowExA(dwExStyle, lpClassName, "Silent Hill 2: Enhanced Edition" , dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

// Replace the vanilla "SH2PC Title" with ours
void PatchWindowTitle()
{
	Logging::Log() << "Patching window title...";

	auto pattern = hook::pattern("FF 15 ? ? ? ? 85 C0 A3 ? ? ? ? 74 ? 8B 0D ? ? ? ? 6A");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	WriteCalltoMemory((BYTE*)pattern.count(1).get(0).get<uint32_t*>(0), CreateWindowExA_Hook, 6);
}

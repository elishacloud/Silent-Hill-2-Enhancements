/**
* Copyright (C) 2026 Elisha Riedlinger
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
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

namespace {
	BYTE* QuickLoadOverwriteAddr = (BYTE*)0x0040250F;
	const void* QuickLoad1VarAddr = (void*)0x00402511;
	const void* QuickLoad2VarAddr = (void*)0x00402517;

	// Variables for ASM
	BYTE DisableQuickLoadFlag = FALSE;
	void* QuickLoad1Addr = nullptr;
	void* QuickLoad2Addr = nullptr;
}

static __declspec(naked) void __stdcall QuickLoadConditionsASM()
{
	__asm
	{
		pushfd
		cmp byte ptr[DisableQuickLoadFlag], 1
		je Return

	; EnableQuickLoad
		push eax
		mov eax, dword ptr [QuickLoad1Addr]
		mov dword ptr [eax], ebx
		mov eax, dword ptr[QuickLoad2Addr]
		mov dword ptr[eax], ebx
		pop eax

	Return:
		popfd
		ret
	}
}

void RunGameQuickLoadCheck()
{
	static BYTE* InGameVoiceEvent = GetInGameVoiceEvent();

	if (InGameVoiceEvent)
	{
		DisableQuickLoadFlag = (*InGameVoiceEvent != 0);
	}
}

void PatchGameQuickLoad()
{
	// Get address for quick load
	constexpr BYTE SearchBytes[]{ 0x56, 0x89, 0x1D };
	DWORD QuickLoadAddrCheck = SearchAndGetAddresses(0x0040250E, 0x0040250E, 0x0040250E, SearchBytes, sizeof(SearchBytes), 0, __FUNCTION__);
	if (!QuickLoadAddrCheck)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Get variable addresses
	memcpy(&QuickLoad1Addr, QuickLoad1VarAddr, sizeof(void*));
	memcpy(&QuickLoad2Addr, QuickLoad2VarAddr, sizeof(void*));

	// Update SH2 code
	Logging::Log() << "Patching Quick Load conditions...";
	WriteCalltoMemory(QuickLoadOverwriteAddr, *QuickLoadConditionsASM, 12);
}

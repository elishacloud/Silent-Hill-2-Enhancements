#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

void* jmp_return;
void* jmp_to_loop;

DWORD* muteSound;
DWORD EventIndex;
BYTE MenuEventIndex;

// BGM Fading out instructions
#pragma warning(suppress: 4740)
__declspec(naked) void __stdcall FixInventoryBGMBugASM()
{
	EventIndex = GetEventIndex();
	MenuEventIndex = GetMenuEvent();
	// Works only if this conditions are met, if you want to mute sound on specific area you can add a new line or simply you can create
	// new comparison and send them through jmp_return address
	if (MenuEventIndex == 0xd || MenuEventIndex == 0x11 && EventIndex == 0x0)
	{
		if (EventIndex == 0xb)
		{
			*muteSound = 0xF;
			__asm
			{
				jmp jmp_return
			}
		}

		if (EventIndex > 3 && EventIndex < 10 || EventIndex == 0x10 || MenuEventIndex == 0x11)
		{
			__asm
			{
				jmp jmp_return
			}
		}
	}
	__asm
	{
		jmp jmp_to_loop
	}
}

void PatchInventoryBGMBug()
{
	constexpr BYTE BuggyBGMBytes[] = { 0x83, 0xf8, 0x04, 0x75, 0x0d, 0x68 };
	const DWORD BuggyBGMAddr = SearchAndGetAddresses(0x005166E8, 0x00516A18, 0x00516338, BuggyBGMBytes, sizeof(BuggyBGMBytes), -0x1f);

	// Check errors
	if (!BuggyBGMAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	memcpy(&muteSound, (DWORD*)(BuggyBGMAddr - 4), sizeof(DWORD));
	jmp_return = reinterpret_cast<void*>(BuggyBGMAddr + 0x24);
	jmp_to_loop = reinterpret_cast<void*>(BuggyBGMAddr + 0x31);

	// Update SH2 code
	Logging::Log() << "Fixing Inventory BGM...";
	WriteJMPtoMemory(reinterpret_cast<BYTE*>(BuggyBGMAddr), *FixInventoryBGMBugASM, 0x24);
}
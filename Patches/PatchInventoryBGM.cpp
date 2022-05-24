#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

void* jmp_return;
void* jmp_to_loop;

__declspec(naked) void __stdcall FixInventoryBGMBugASM()
{
	__asm
	{
		cmp eax, 06
			jne code
		jmp jmp_return

		code:
			cmp eax, 04
				jne jmpPoint
			jmp jmp_return

		jmpPoint:
			jmp jmp_to_loop
	}
}

void PatchInventoryBGMBug()
{
	constexpr BYTE BuggyBGMBytes[] = { 0x83, 0xf8, 0x04, 0x75, 0x0d, 0x68 };
	const DWORD BuggyBGMAddr = SearchAndGetAddresses(0x05166E8, 0x0516A18, 0x0516338, BuggyBGMBytes, sizeof(BuggyBGMBytes), 0);

	if (!BuggyBGMAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	jmp_return = reinterpret_cast<void*>(BuggyBGMAddr + 0x5);
	jmp_to_loop = reinterpret_cast<void*>(BuggyBGMAddr + 0x12);

	WriteJMPtoMemory(reinterpret_cast<BYTE*>(BuggyBGMAddr), *FixInventoryBGMBugASM);
}
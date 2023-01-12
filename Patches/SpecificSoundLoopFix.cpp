#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Patches.h"
#include "Common/Utils.h"
#include "Logging/Logging.h"
#include "External/injector/include/injector/injector.hpp"

void MothSFXLoopingFix();
void ChainsawSFXLoopingFix();

// Function instances 
typedef uintptr_t* (__cdecl* Stop_Moth_Sfx)(void);
Stop_Moth_Sfx stop_moth_sfx;

// I really dont know what this does. This function was in the pause menu instruction and there was only 1 beautiful spot for injecting.
typedef uintptr_t* (__cdecl* dunno)(uint32_t);
// This address is universal
dunno Dunno = (dunno)0x00401670;

// ASM function to stop moth sfx
#pragma warning(suppress: 4740)
void __stdcall StopMothSfxOnPauseMenu()
{
	// It checks room id, if the id is points to final boss it runs the stop function
	// Note: This code also a fix for the moths on west side apartments. stop_moth_sfx also kill's the moth but when we clean bytes of reset instuctions they won't coming back.
	// So we have to be sure this code will work only on final boss.
	if (GetRoomID() == 0xBB)
	{
		stop_moth_sfx();
	}

	__asm
	{
		call Dunno
	}

	return;
}

void PatchSpecificSoundLoopFix()
{
	MothSFXLoopingFix();
	ChainsawSFXLoopingFix();
}

void MothSFXLoopingFix()
{
	// Call play moth sfx function after leaving inventory
	constexpr BYTE SearchMothSound[]{ 0x8a, 0x46, 0x02, 0x83, 0xc4, 0x0c, 0x3c, 0x03, 0x88, 0x5e, 0x03 };
	const auto MothCallAddr = (DWORD)CheckMultiMemoryAddress((void*)0x004ac5d4, (void*)0x004ac884, (void*)0x004ac144, (void*)SearchMothSound, sizeof(SearchMothSound));

	// Points to enWaitAllInsect (based on ps2 demo version).
	constexpr BYTE CutSoundByte[]{ 0x80, 0x3e, 0x0f, 0x75, 0x05, 0x38, 0x4e, 0x02, 0x7c, 0x0e };
	const auto CutSoundF = (DWORD)CheckMultiMemoryAddress((void*)0x004ab6b0, (void*)0x004ab960, (void*)0x004ab220, (void*)CutSoundByte, sizeof(CutSoundByte));

	// Sets the stop_moth_sfx function instances address.
	const auto StopSfxAddr = (uintptr_t * (__cdecl*)(void))(CutSoundF - 0x20);

	// Checking address pointer
	if (!MothCallAddr || !StopSfxAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Update SH2 code
	// I NOP'ed in here MOV instruction because of when mov instruction sets to 0 ,spawns moths again and calls play sfx function twice.

	// These are resets moth pos after leaving inventory, we don't need these because game already stores these value's in different memory and these mem's are just useless.
	injector::MakeNOP(MothCallAddr - 0x53, 0x37);
	// This memory summon moths again but again we don't need because when game call's this it's also resets moth timer and pos. Also it calls the play_moth_sfx because there is already one below
	// of the byte.
	injector::MakeNOP(MothCallAddr + 0x8, 0x3);
	// Ahh i'm tired to comment everything out, you can find more info in here: https://github.com/elishacloud/Silent-Hill-2-Enhancements/issues/424
	// Simply it just NOP's the useless enDeleteEnemy(0x004ab73d) call when timer is out, it instantly destroy moth's but we like the moth's goes away and destroy's theirself. This thing does it already 0x004ac29a;  
	injector::MakeNOP(CutSoundF + 0x8D, 0x5);

	// This address is universal
	BYTE* PauseMenuAddr = (BYTE*)0x00402366;

	stop_moth_sfx = (Stop_Moth_Sfx)StopSfxAddr;

	// injects same inventories stop_moth_sfx function to pause menu
	WriteCalltoMemory(PauseMenuAddr, *StopMothSfxOnPauseMenu);

	Logging::Log() << "Fixing Moth Sound Looping...";
}

void ChainsawSFXLoopingFix()
{
	// Points to stop sfx chainsaw sound logical branch.
	constexpr BYTE SearchChainsawSound[]{ 0x68, 0x24, 0x2b, 0x00, 0x00, 0xe8, 0xf6, 0xa3, 0xfc, 0xff, 0x68, 0x25, 0x2b, 0x00, 0x00, 0xe8 };
	BYTE* ChainsawSoundStopAddr = (BYTE*)SearchAndGetAddresses(0x0054a970, 0x0054aca0, 0x0054a5c0, SearchChainsawSound, sizeof(SearchChainsawSound), -0xE);

	// Checking address pointer
	if (!ChainsawSoundStopAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Fixes logical problem on "stop chainsaw sound" statement.
	UpdateMemoryAddress((void*)ChainsawSoundStopAddr, "\x77", 1);

	Logging::Log() << "Fixing Chainsaw Sound Looping...";
}

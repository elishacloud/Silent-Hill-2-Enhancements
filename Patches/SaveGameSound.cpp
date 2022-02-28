#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>

#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

void* GameSoundReturnAddress;

// Function instance created
typedef uintptr_t* (__cdecl* Play_Sound)(uint32_t some_value, uint32_t volume_maybe, uint32_t sound_id);
Play_Sound play_sound;

__declspec(naked) void __stdcall SaveGameSoundASM()
{
	__asm
	{
		push eax
		mov eax, PauseMenuButtonIndexAddr
		cmp [eax], 0x1
		jne real_code
		push esi
		push 0x3F800000
		push 0x0002743
		call play_sound
		pop eax
		jmp GameSoundReturnAddress
		real_code :
		push esi
		push 0x3F800000
		push 0x0002712
		call play_sound
		pop eax
		jmp GameSoundReturnAddress
	}
}

void PatchSaveGameSound()
{
	// Play Sound Effect function address
	constexpr BYTE SearchPlayMusicBytes[]{ 0x6A, 0x01, 0xE8, 0x19, 0x90, 0xFE, 0xFF, 0x83,0xC4,0x04 };
	const auto PlayMusicAddress = (uintptr_t * (__cdecl*)(uint32_t, uint32_t, uint32_t))(CheckMultiMemoryAddress((void*)0x00515580, (void*)0x005158B0, (void*)0x005151D0, (void*)SearchPlayMusicBytes, sizeof(SearchPlayMusicBytes)));

	//Assigned Sound Effect function address to function instance
	play_sound = static_cast<Play_Sound>(PlayMusicAddress);

	// calls this function for the getting PauseMenuButtonIndexAddr
	GetPauseMenuButtonIndex();

	// Get Play Music Call Function
	constexpr BYTE SearchSoundEffectCallBytes[]{ 0x68, 0x00, 0x00, 0x80, 0x3F, 0x68, 0x12, 0x27, 0x00, 0x00 };
	DWORD SoundEffectCallAddr = SearchAndGetAddresses(0x00402824, 0x00402824, 0x00402824, SearchSoundEffectCallBytes, sizeof(SearchSoundEffectCallBytes), -1);

	// Checking address pointer
	if (!SoundEffectCallAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	GameSoundReturnAddress = reinterpret_cast<void*>(SoundEffectCallAddr + 16);
	
	// Update SH2 code
	WriteJMPtoMemory(reinterpret_cast<BYTE*>(SoundEffectCallAddr), *SaveGameSoundASM, 16);

	Logging::Log() << "Fixing \"Save Game\" Sound...";
}

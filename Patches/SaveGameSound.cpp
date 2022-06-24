/**
* Copyright (C) 2022
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
	real_code:
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

	// Sound effect function address, same address for all known versions of the game
	DWORD SoundEffectCallAddr = 0x00402823;

	// Checking address pointer
	if (!SoundEffectCallAddr || !PlayMusicAddress || !PauseMenuButtonIndexAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	GameSoundReturnAddress = reinterpret_cast<void*>(SoundEffectCallAddr + 16);
	
	// Update SH2 code
	Logging::Log() << "Fixing \"Save Game\" Sound...";
	WriteJMPtoMemory(reinterpret_cast<BYTE*>(SoundEffectCallAddr), *SaveGameSoundASM, 16);
}

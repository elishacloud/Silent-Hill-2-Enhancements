/**
* Copyright (C) 2022 Elisha Riedlinger
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

extern bool ClearFontBeforePrint;

typedef void(__cdecl* LoadMariaProc)(int a1);
typedef void(__cdecl* MariaFunctionProc)();

LoadMariaProc oLoadMaria = nullptr;

// Variables for ASM
BYTE AllowQuickSaveFlag = TRUE;
void *QuickSaveCmpAddr;
void *jmpSoftLockAddr;
void *TimerMemoryAddr;
void *FlashFixEAXAddr;
void *jmpFlashFixAddr;
void *TextMemoryAddr;
void *callSaveTimerAddr;
void *jmpSaveTimerAddr;
void *callTextOverlapAddr;
void *jmpTextOverlapAddr;
void *jmpQuickSaveAddr;
bool IsInGameResults = false;
BYTE *SaveIndexAddr = nullptr;
BYTE *SaveArrayAddr = nullptr;
DWORD *InGameAddr = nullptr;
void *jmpIndexCheckAddr = nullptr;
void *jmpMariaFunctionAddr = nullptr;
DWORD FilesLoadedAddr;
DWORD CutsceneValueAddr;

// ASM function for Quick Save Soft-Lock Fix
__declspec(naked) void __stdcall SoftLockASM()
{
	__asm
	{
		cmp dword ptr ds : [esi + 0x04], 0x00004214 // locked with flashlight on
		je near SoftLockFix1 // jumps to soft-lock fix if locked
		cmp dword ptr ds : [esi + 0x04], 0x00004014 // locked with flashlight off
		je near SoftLockFix2 // jumps to soft-lock fix if locked
		mov ecx, dword ptr ds : [esi + 0x04]
		mov dword ptr ds : [edi - 0x04], ecx
		jmp jmpSoftLockAddr

	SoftLockFix1:
		mov ecx, 0x0000214 // unlocks
		mov dword ptr ds : [edi - 0x04], ecx
		jmp jmpSoftLockAddr
			
	SoftLockFix2:
		mov ecx, 0x0000014 // unlocks
		mov dword ptr ds : [edi - 0x04], ecx
		jmp jmpSoftLockAddr
	}
}

// ASM function for Quick Save Timer Fix
__declspec(naked) void __stdcall SaveTimerASM()
{
	__asm
	{
		push eax
		mov eax, dword ptr ds : [TimerMemoryAddr]
		mov dword ptr ds : [eax], esi
		mov eax, dword ptr ds : [FullscreenImageEventAddr]
		cmp dword ptr ds : [eax], 0x02
		pop eax
		je near Exit // jumps to code exit if interactive text is displayed
		call callSaveTimerAddr

	Exit:
		jmp jmpSaveTimerAddr
	}
}

// ASM function for Quick Save Flash Fix
__declspec(naked) void __stdcall FlashFixASM()
{
	__asm
	{
		push ecx
		mov ecx, dword ptr ds : [EventIndexAddr]
		cmp byte ptr ds : [ecx], 0x00
		je near Exit
		mov ecx, dword ptr ds : [FlashFixEAXAddr]
		mov dword ptr ds : [ecx], eax

	Exit:
		pop ecx
		jmp jmpFlashFixAddr
	}
}

// ASM function for Quick Save Text Overlap Fix
__declspec(naked) void __stdcall TextOverlapASM()
{
	__asm
	{
		push eax
		mov eax, dword ptr ds : [TextMemoryAddr]
		mov dword ptr ds : [eax], esi
		mov eax, dword ptr ds : [FullscreenImageEventAddr]
		cmp dword ptr ds : [eax], 0x02
		pop eax
		je near Exit // jumps to code exit if interactive text is displayed
		call callTextOverlapAddr

	Exit:
		jmp jmpTextOverlapAddr
	}
}

// ASM function to disable quick saves
__declspec(naked) void __stdcall QuickSaveASM()
{
	__asm
	{
		cmp byte ptr ds : [AllowQuickSaveFlag], TRUE
		je near AllowQuickSave
	//DisallowQuickSave:
		jmp near Exit
	AllowQuickSave:
		mov eax, dword ptr ds : [QuickSaveCmpAddr]
		cmp dword ptr ds : [eax], esi
	Exit:
		jmp jmpQuickSaveAddr
	}
}

// Check if loading a Game Result save
void GameResultSave()
{
	BYTE Value = *(BYTE*)(((DWORD)*SaveIndexAddr * 24) + (DWORD)SaveArrayAddr + 2);
	if (Value == 0x50 || Value == 0xFF)
	{
		*InGameAddr = 0x00;
		if (GetRoomID())
		{
			IsInGameResults = true;
		}
	}
}

// ASM function to Fix Game Result Saves
__declspec(naked) void __stdcall GameResultSaveASM()
{
	__asm
	{
		push eax
		push ecx
		push edx
		mov eax, dword ptr ds : [SaveIndexAddr]
		mov dword ptr ds : [eax], esi
		call GameResultSave
		pop edx
		pop ecx
		pop eax
		jmp jmpIndexCheckAddr
	}
}

// ASM function to disable quick saves
__declspec(naked) void __stdcall MariaFunctionASM()
{
	__asm
	{
		sub esp, 0x208
		jmp jmpMariaFunctionAddr
	}
}
MariaFunctionProc oMariaFunction = (MariaFunctionProc)*MariaFunctionASM;

void __cdecl NewMariaFunction()
{
	BYTE bFilesLoaded[4] = { 0 };
	SIZE_T BytesRead = 0;

	ReadProcessMemory(GetCurrentProcess(), (LPVOID)FilesLoadedAddr, bFilesLoaded, sizeof(bFilesLoaded), &BytesRead);

	BYTE CutsceneValue[4] = { 0 };
	SIZE_T BytesReadCutscene = 0;

	ReadProcessMemory(GetCurrentProcess(), (LPVOID)CutsceneValueAddr, CutsceneValue, sizeof(CutsceneValue), &BytesReadCutscene);

	if (bFilesLoaded[0] == 0)
	{
		if (CutsceneValue[3] > 0 && CutsceneValue[3] < 8)
		{
			oLoadMaria(1);
		}
	}
	return oMariaFunction();
}

void PatchGameLoad()
{
	// Get in-game check address
	constexpr BYTE InGameCheckSearchBytes[]{ 0x80, 0x3A, 0xD8, 0x74, 0x11, 0x41, 0x83, 0xC2, 0x18, 0x3B, 0xCF, 0x7C, 0xF3, 0x5F, 0x5E, 0xB8, 0x02, 0x00, 0x00, 0x00, 0x5B, 0xC3, 0x8B, 0xD6, 0x6B, 0xD2, 0x64, 0x03, 0xCA, 0x8D, 0x0C, 0x49 };
	BYTE *InGameCheckAddr = (BYTE*)SearchAndGetAddresses(0x00454BF5, 0x00454E55, 0x00454E55, InGameCheckSearchBytes, sizeof(InGameCheckSearchBytes), 0x1AF);

	// Get save Index address
	constexpr BYTE IndexSearchBytes[]{ 0x33, 0xC9, 0x0F, 0xBE, 0xF3, 0x0F, 0xB6, 0x10, 0x3B, 0xD6, 0x75, 0x0D, 0x0F, 0xB6, 0x50, 0x01, 0x0F, 0xBE, 0x7C, 0x24, 0x0C, 0x3B, 0xD7, 0x74, 0x0B, 0x41, 0x83, 0xC0, 0x18, 0x83, 0xF9, 0x64, 0x7C, 0xE3, 0xEB, 0x05 };
	BYTE *IndexCheckAddr = (BYTE*)SearchAndGetAddresses(0x0044CF0F, 0x0044D16F, 0x0044D16F, IndexSearchBytes, sizeof(IndexSearchBytes), 0x91);

	// Checking address pointer
	if (!InGameCheckAddr || !IndexCheckAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpIndexCheckAddr = IndexCheckAddr + 6;

	InGameAddr = (DWORD*)*(DWORD*)(InGameCheckAddr + 1);	// 0x00932034;
	SaveIndexAddr = (BYTE*)*(DWORD*)(IndexCheckAddr + 2);	// 0x009335E4;
	SaveArrayAddr = SaveIndexAddr + 0x44;					// 0x00933628;

	// Update SH2 code
	Logging::Log() << "Fixing Game Results loading crash...";
	WriteJMPtoMemory((BYTE*)IndexCheckAddr, *GameResultSaveASM, 6);
}

DWORD space = 0x0402437;
DWORD quickSaveToggle;
void* DontPauseGame = (void*)0x040243d;

__declspec(naked) void __stdcall LockESCWhileRenderedText()
{
	if (*(DWORD*)quickSaveToggle == 1 || *(DWORD*)0x093203C == 1 || *(float*)0x093268C > 0)
	{
	}
	else
	{
		*EventIndexAddr = 0x10;
	}
	__asm
	{
		jmp DontPauseGame
	}
}

void SetGameLoad()
{
	// Get elevator room save address
	constexpr BYTE GameLoadSearchBytes[]{ 0x83, 0xC4, 0x10, 0xF7, 0xC1, 0x00, 0x00, 0x00, 0x04, 0x5E, 0x74, 0x0F, 0xC7, 0x05 };
	DWORD GameLoadAddr = SearchAndGetAddresses(0x0058312C, 0x005839DC, 0x005832FC, GameLoadSearchBytes, sizeof(GameLoadSearchBytes), 0x94);
	if (!GameLoadAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Fix momentarily "flash" when save file is loaded
	constexpr BYTE FlashFixSearchBytes[]{ 0x5F, 0x5E, 0x5D, 0x33, 0xC0, 0x5B, 0xC3, 0x90, 0x90, 0x33, 0xC0, 0xA3 };
	DWORD FlashFixAddr = SearchAndGetAddresses(0x004EEA37, 0x004EECE7, 0x004EE5A7, FlashFixSearchBytes, sizeof(FlashFixSearchBytes), 0x0B);
	EventIndexAddr = GetEventIndexPointer();
	if (!FlashFixAddr || !EventIndexAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	FlashFixEAXAddr = (void*)*(DWORD*)(FlashFixAddr + 1);
	jmpFlashFixAddr = (void*)(FlashFixAddr + 5);

	// Disable Quick Save Reset
	constexpr BYTE QuickSaveResetSearchBytes[]{ 0x57, 0x8D, 0x7D, 0x30, 0xEB, 0x03, 0x8D, 0x49, 0x00, 0x0F, 0xBE, 0x46, 0x11, 0x85, 0xC0, 0x0F, 0x8E };
	DWORD QuickSaveResetFunction = SearchAndGetAddresses(0x0053AE37, 0x0053B167, 0x0053AA87, QuickSaveResetSearchBytes, sizeof(QuickSaveResetSearchBytes), 0x21);
	if (!QuickSaveResetFunction)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Quick Save Soft-Lock Fix
	DWORD SoftLockFunction = QuickSaveResetFunction + 5;
	jmpSoftLockAddr = (void*)(SoftLockFunction + 6);

	// Quick Save Timer Fix
	constexpr BYTE SaveTimerSearchBytes[]{ 0x83, 0xC4, 0x04, 0x85, 0xC0, 0x74, 0x4C, 0x39, 0x35 };
	DWORD SaveTimerFunction = SearchAndGetAddresses(0x00402495, 0x00402495, 0x00402495, SaveTimerSearchBytes, sizeof(SaveTimerSearchBytes), -0x15);
	FullscreenImageEventAddr = GetFullscreenImageEventPointer();
	if (!SaveTimerFunction || !FullscreenImageEventAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	TimerMemoryAddr = (void*)*(DWORD*)(SaveTimerFunction - 0x1B);
	callSaveTimerAddr = (void*)(*(DWORD*)(SaveTimerFunction + 7) + SaveTimerFunction + 0x0B);
	jmpSaveTimerAddr = (void*)(SaveTimerFunction + 0x0B);

	// Quick Save Text Overlap Fix
	DWORD TextOverlapFunction = SaveTimerFunction + 0xB4;
	TextMemoryAddr = (void*)((DWORD)TimerMemoryAddr - 0x08);
	callTextOverlapAddr = (void*)(*(DWORD*)(TextOverlapFunction + 7) + TextOverlapFunction + 0x0B);
	jmpTextOverlapAddr = (void*)(TextOverlapFunction + 0x0B);

	// Location for to disabling quick saves
	constexpr BYTE QuickSaveSearchBytes[]{ 0x83, 0xC4, 0x04, 0x85, 0xC0, 0x74, 0x4C, 0x39, 0x35 };
	DWORD QuickSaveFunction = SearchAndGetAddresses(0x00402495, 0x00402495, 0x00402495, QuickSaveSearchBytes, sizeof(QuickSaveSearchBytes), 0x07);
	if (!QuickSaveFunction)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpQuickSaveAddr = (void*)(QuickSaveFunction + 0x06);
	QuickSaveCmpAddr = (void*)*(DWORD*)(QuickSaveFunction + 0x02);

	constexpr BYTE LoadMariaSearchBytes[]{ 0x8B, 0x44, 0x24, 0x04, 0x85, 0xC0, 0x75, 0x5E };
	oLoadMaria = (LoadMariaProc)SearchAndGetAddresses(0x00593FC5, 0x00594875, 0x00594195, LoadMariaSearchBytes, sizeof(LoadMariaSearchBytes), -0x05);
	constexpr BYTE MariaFunctionSearchBytes[]{ 0x81, 0xEC, 0x08, 0x02, 0x00, 0x00, 0x53, 0x55 };
	DWORD MariaFunctionAddr = SearchAndGetAddresses(0x00594C40, 0x005954F0, 0x00594E10, MariaFunctionSearchBytes, sizeof(MariaFunctionSearchBytes), 0x00);
	if (!(DWORD)*oLoadMaria || !MariaFunctionAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpMariaFunctionAddr = (void*)(MariaFunctionAddr + 6);
	UpdateMemoryAddress((void*)&FilesLoadedAddr, (void*)(MariaFunctionAddr + 0x6D), sizeof(DWORD));
	UpdateMemoryAddress((void*)&CutsceneValueAddr, (void*)(MariaFunctionAddr + 0x53), sizeof(DWORD));

	// Update SH2 code
	Logging::Log() << "Enabling Load Game Fix...";
	DWORD Value = 0x00;
	UpdateMemoryAddress((void*)GameLoadAddr, &Value, sizeof(DWORD));
	UpdateMemoryAddress((void*)QuickSaveResetFunction, "\x90\x90\x90\x90\x90", 5);
	WriteJMPtoMemory((BYTE*)SoftLockFunction, *SoftLockASM, 6);
	WriteJMPtoMemory((BYTE*)FlashFixAddr, *FlashFixASM, 5);
	WriteJMPtoMemory((BYTE*)SaveTimerFunction, *SaveTimerASM, 6);
	WriteJMPtoMemory((BYTE*)TextOverlapFunction, *TextOverlapASM, 6);
	WriteJMPtoMemory((BYTE*)QuickSaveFunction, *QuickSaveASM, 6);
	WriteJMPtoMemory((BYTE*)MariaFunctionAddr, *NewMariaFunction, 6);

	memcpy(&quickSaveToggle, (DWORD*)(0x04024bd),sizeof(DWORD));

	ClearFontBeforePrint = true;

	WriteJMPtoMemory((BYTE*)space, *LockESCWhileRenderedText, 6); 
}

void RunGameLoad()
{
	// Update save code elevator room
	RUNCODEONCE(SetGameLoad());

	// Get game save address
	static BYTE *SaveGameAddress = nullptr;
	if (!SaveGameAddress)
	{
		RUNONCE();

		// Get address for game save
		constexpr BYTE SearchBytes[]{ 0x3C, 0x1B, 0x74, 0x27, 0x3C, 0x25, 0x74, 0x23, 0x3C, 0x30, 0x74, 0x1F, 0x3C, 0x31, 0x74, 0x1B, 0x3C, 0x32, 0x74, 0x17, 0x3C, 0x33, 0x74, 0x13, 0x3C, 0x34, 0x74, 0x0F };
		SaveGameAddress = (BYTE*)ReadSearchedAddresses(0x0044C648, 0x0044C7E8, 0x0044C7E8, SearchBytes, sizeof(SearchBytes), -0x0D);
		if (!SaveGameAddress)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
	}

	// Get elevator running address
	static BYTE *ElevatorRunning = nullptr;
	if (!ElevatorRunning)
	{
		RUNONCE();

		// Get address for game save
		constexpr BYTE SearchBytes[]{ 0xF7, 0xC6, 0x00, 0x0C, 0x00, 0x00, 0x0F, 0x95, 0xC0, 0x49, 0x74, 0x0A, 0x49, 0x75, 0x25, 0x84, 0xC0 };
		ElevatorRunning = (BYTE*)ReadSearchedAddresses(0x0052EA81, 0x0052EDB1, 0x0052E6D1, SearchBytes, sizeof(SearchBytes), -0x0E);
		if (!ElevatorRunning)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
	}

	// Get in-game voice event address
	static BYTE *InGameVoiceEvent = nullptr;
	if (!InGameVoiceEvent)
	{
		RUNONCE();

		// Get address for in game voice event
		constexpr BYTE SearchBytes[]{ 0x85, 0xC0, 0x75, 0x07, 0xD9, 0x05 };
		InGameVoiceEvent = (BYTE*)ReadSearchedAddresses(0x004A0305, 0x004A05B5, 0x0049FE75, SearchBytes, sizeof(SearchBytes), -0x04);
		if (!InGameVoiceEvent)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
	}

	// Get Pause Menu Button address
	GetPauseMenuButtonIndex();
	if (!PauseMenuButtonIndexAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Set static variables
	static bool ValueSet = false;
	static bool ValueUnSet = false;

	bool DisableQuickSave = false;

	// Enable game saves for specific rooms
	if (GetRoomID() == 0x29)
	{
		*SaveGameAddress = 1;
		ValueSet = true;
	}
	// Disable game saves for specific rooms
	else if (GetRoomID() == 0x09 || GetRoomID() == 0x0A || GetRoomID() == 0x0B || GetRoomID() == 0x13 || GetRoomID() == 0x17 || GetRoomID() == 0x2A || GetRoomID() == 0x46 || GetRoomID() == 0xAA || GetRoomID() == 0xC7 ||
		(GetRoomID() == 0x04 && GetJamesPosZ() > 49000.0f) ||
		(GetRoomID() == 0x78 && GetJamesPosX() < -18600.0f) ||
		(GetRoomID() == 0x9D && GetJamesPosX() < 60650.0f) ||
		(GetRoomID() == 0xB8 && GetJamesPosX() > -15800.0f))
	{
		*SaveGameAddress = 0;
		ValueUnSet = true;
	}
	// Reset static variables
	else
	{
		if (ValueSet)
		{
			ValueSet = false;
		}
		if (ValueUnSet)
		{
			*SaveGameAddress = 1;
			ValueUnSet = false;
		}
	}

	// Reset the Pause Button Menu Index
	static bool PauseValueUnSet = true;
	if (GetEventIndex() != 16 && PauseValueUnSet)
	{
		*PauseMenuButtonIndexAddr = 0;
		PauseValueUnSet = false;
	}
	else
	{
		PauseValueUnSet = true;
	}

	// Clear InGameVoiceEvent when Room ID changes and upon end of cutscene ID 0x4E (Laura scares James with the piano)
	static DWORD LastRoomID = 0, LastCutsceneID = 0;
	DWORD CurrentRoomID = GetRoomID(), CurrentCutsceneID = GetCutsceneID();
	if (LastRoomID != CurrentRoomID || (LastCutsceneID == 0x4E && CurrentCutsceneID != 0x4E))
	{
		*InGameVoiceEvent = 0;
	}
	LastRoomID = CurrentRoomID;
	LastCutsceneID = CurrentCutsceneID;

	// Disable quick save during certain in-game voice events and during fullscreen image events
	if (*InGameVoiceEvent != 0 || GetFullscreenImageEvent() == 2)
	{
		DisableQuickSave = true;
		AllowQuickSaveFlag = FALSE;
	}

	// Reset quick save when needed
	if (!DisableQuickSave && !AllowQuickSaveFlag)
	{
		AllowQuickSaveFlag = TRUE;
	}
}

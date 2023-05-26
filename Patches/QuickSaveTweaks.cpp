/**
* Copyright (C) 2023 The Machine Ambassador, mercury501, Steam006
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
#include "InputTweaks.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include <d3d9types.h>
#include <sstream>
#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\utility.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"

DWORD GameSavedTextColorAddr;
DWORD CantSavedTextColorAddr;
DWORD NoQuickSaveTextColorAddr;
DWORD printGameSavedFunction;
DWORD printCantSavedFunction;
DWORD printNoQuickSavedFunction;

DWORD tmpReg;
void* PosGameSavedRetAddr;
void* PosCantSaveRetAddr;
void* PosNoQuickSaveRetAddr;

bool ClearFontBeforePrint = false;

// Render Game Saved Text Function Call
typedef void(__stdcall* printQuickSaveString)(void);
printQuickSaveString PrintSave;

// Render Can't Save Text Function Call
typedef void(__stdcall* printCantSaveString)(void);
printCantSaveString PrintCantSave;

// Render No Save Text (Unused maybe ?) Function Call
typedef void(__stdcall* printNoSaveString)(void);
printNoSaveString PrintNoSave;

// Clear Font function to use clear rendered save texts before rendering
typedef void(__cdecl* fontClear)(void);
fontClear clearFont;
//0047f180

// X pos of save text
int TextPosVecX = 0x14;	// Use game's default

// Use asm to handle larger than byte values
__declspec(naked) void __stdcall PosGameSavedASM()
{
	__asm
	{
		push	0x1BD
		mov		tmpReg, eax
		mov		eax, TextPosVecX
		push	eax
		mov		eax, tmpReg
		jmp		PosGameSavedRetAddr
	}
}

__declspec(naked) void __stdcall PosCantSaveASM()
{
	__asm
	{
		push	0x1BD
		mov		tmpReg, eax
		mov		eax, TextPosVecX
		push	eax
		mov		eax, tmpReg
		jmp		PosCantSaveRetAddr
	}
}

__declspec(naked) void __stdcall PosNoQuickSaveASM()
{
	__asm
	{
		push	0x1BD
		mov		tmpReg, eax
		mov		eax, TextPosVecX
		push	eax
		mov		eax, tmpReg
		jmp		PosNoQuickSaveRetAddr
	}
}

// I don't think game uses RGB or something like that but anyway it's just works :)
struct RGBA
{
	uint32_t R, G, B, A;
};

// Window width and height for the aspect ratio calc
extern LONG BufferWidth, BufferHeight;

void ChangeSaveColor(void* Addr,RGBA savedColorValue)
{
	UpdateMemoryAddress(((BYTE*)Addr + 0x00), &savedColorValue.A, sizeof(savedColorValue.A));
	UpdateMemoryAddress(((BYTE*)Addr + 0x05), &savedColorValue.B, sizeof(savedColorValue.B));

	if (GameVersion == SH2V_10)
	{
		UpdateMemoryAddress(((BYTE*)Addr + 0x0B), &savedColorValue.G, sizeof(savedColorValue.G));
		UpdateMemoryAddress(((BYTE*)Addr + 0x10), &savedColorValue.R, sizeof(savedColorValue.R));
	}
	else
	{
		UpdateMemoryAddress(((BYTE*)Addr + 0x0A), &savedColorValue.G, sizeof(savedColorValue.G));
		UpdateMemoryAddress(((BYTE*)Addr + 0x0F), &savedColorValue.R, sizeof(savedColorValue.R));
	}
}

// This aspect ratio calc is made by @Polymega https://github.com/elishacloud/Silent-Hill-2-Enhancements/issues/564#issue-1301079704
void __stdcall SetAspectRatio()
{
	TextPosVecX = (int)(((((BufferHeight * 1.333333333) - BufferWidth) * (static_cast<double>(BufferWidth) / BufferHeight)) / BufferWidth) * 195 + 20);
}

void __stdcall print_quick_saved_string(void)
{
	SetAspectRatio();
	if (ClearFontBeforePrint)
	{
		clearFont();
	}
	PrintSave();
}

void __stdcall print_cant_save_string(void)
{
	SetAspectRatio();
	if (ClearFontBeforePrint)
	{
		clearFont();
	}
	PrintCantSave();
}

void __stdcall print_no_quick_save_string(void)
{
	SetAspectRatio();
	if (ClearFontBeforePrint)
	{
		clearFont();
	}
	PrintNoSave();
}

injector::hook_back<void(__cdecl*)(void)> ClearTextFun;
void __cdecl ClearTextHook()
{
	RunQuickSaveTweaks();

	return ClearTextFun.fun();
}

void RunQuickSaveTweaks()
{
	LOG_ONCE("Patching Quick Save Text Fading Too Quickly...");

	//Currently in-game && a text prompt is NOT currently on-screen
	if (GetClearTextPointer() && *GetClearTextPointer() == 0x05 && GetEventIndex() == EVENT_IN_GAME && !IsInFullScreenImageEvent())
	{
		UpdateMemoryAddress(GetClearTextPointer(), "\x00", 1);
	}
}

void PatchQuickSaveTweaks()
{
	Logging::Log() << "Patching Quick Save Text Position...";

	constexpr BYTE textPosLockBypassBytes[] = { 0x7C, 0x18, 0x85, 0xC9, 0x7C, 0x14, 0xb8, 0x01, 0x00, 0x00, 0x00 };
	DWORD textPosLockBypassAddr = SearchAndGetAddresses(0x0048051F, 0x004807BF, 0x004809CF, textPosLockBypassBytes, sizeof(textPosLockBypassBytes), 0x00, __FUNCTION__);

	injector::MakeNOP((BYTE*)textPosLockBypassAddr, 6);

	constexpr BYTE quick_saved_addr_bytes[] = { 0x68, 0x80, 0x00, 0x00,0x00 , 0x68, 0x96,0x00,0x00,0x00 };
	GameSavedTextColorAddr = SearchAndGetAddresses(   (DWORD)0x0044C688,   (DWORD)0x0044C856, (DWORD)0x0044C856, (BYTE*)quick_saved_addr_bytes,  sizeof(quick_saved_addr_bytes),   0x01, __FUNCTION__);
	CantSavedTextColorAddr = SearchAndGetAddresses(   (DWORD)0x0044C6D8,   (DWORD)0x0044C8E6, (DWORD)0x0044C8E6, (BYTE*)quick_saved_addr_bytes,  sizeof(quick_saved_addr_bytes),   0x01, __FUNCTION__);
	NoQuickSaveTextColorAddr = SearchAndGetAddresses( (DWORD)0x0044C728,   (DWORD)0x0044C976, (DWORD)0x0044C976, (BYTE*)quick_saved_addr_bytes,  sizeof(quick_saved_addr_bytes),   0x01, __FUNCTION__);

	BYTE newPosXBytes[] = { 0x68, 0xBD, 0x01, 0x00, 0x00, 0x6A, 0x14 };

	DWORD GameSavedPosAddr = SearchAndGetAddresses((DWORD)0x0044C6A9, (DWORD)0x0044C872, (DWORD)0x0044C872, newPosXBytes, sizeof(newPosXBytes), 0x00, __FUNCTION__);
	DWORD CantSavedPosAddr = SearchAndGetAddresses((DWORD)0x0044C6F9, (DWORD)0x0044C902, (DWORD)0x0044C902, newPosXBytes, sizeof(newPosXBytes), 0x00, __FUNCTION__);
	DWORD NoQuickSavePosAddr = SearchAndGetAddresses((DWORD)0x0044C749, (DWORD)0x0044C992, (DWORD)0x0044C992, newPosXBytes, sizeof(newPosXBytes), 0x00, __FUNCTION__);

	PosGameSavedRetAddr = (void*)(GameSavedPosAddr + 0x7);
	PosCantSaveRetAddr = (void*)(CantSavedPosAddr + 0x7);
	PosNoQuickSaveRetAddr = (void*)(NoQuickSavePosAddr + 0x7);

	WriteJMPtoMemory((BYTE*)GameSavedPosAddr, *PosGameSavedASM, 7);
	WriteJMPtoMemory((BYTE*)CantSavedPosAddr, *PosCantSaveASM, 7);
	WriteJMPtoMemory((BYTE*)NoQuickSavePosAddr, *PosNoQuickSaveASM, 7);

	// Points to enWaitAllInsect (based on ps2 demo version).
	constexpr BYTE FontClearBytes[]{ 0x56, 0x33, 0xf6, 0x56, 0x66, 0x89, 0x35 ,0xC6 };
	const auto FontClearAddr = (DWORD)CheckMultiMemoryAddress((void*)0x0047EEE0, (void*)0x0047F180, (void*)0x0047F390, (void*)FontClearBytes, sizeof(FontClearBytes), __FUNCTION__);

	// Sets the stop_moth_sfx function instances address.
	const auto fontClearAddr = (uintptr_t * (__cdecl*)(void))(FontClearAddr);
	clearFont = (fontClear)fontClearAddr;

	switch (GameVersion)
	{
	case SH2V_10:
		printGameSavedFunction = (DWORD)0x0044C680;
		printCantSavedFunction = (DWORD)0x0044C6d0;
		printNoQuickSavedFunction = (DWORD)0x0044C720;
		break;
	case SH2V_11:
	case SH2V_DC:
		printGameSavedFunction = (DWORD)0x0044C820;
		printCantSavedFunction = (DWORD)0x0044C8b0;
		printNoQuickSavedFunction = (DWORD)0x0044C940;
		break;
	default:
		break;
	}

	RGBA gameSavedColorValue{ 0x60,0x60,0x7F,0xFF };
	RGBA CantSavedColorValue{ 0x7F,0x30,0x30,0xFF };

	ChangeSaveColor((void*)GameSavedTextColorAddr, gameSavedColorValue);
	ChangeSaveColor((void*)CantSavedTextColorAddr, CantSavedColorValue);
	ChangeSaveColor((void*)NoQuickSaveTextColorAddr, CantSavedColorValue);

	if (GameVersion == SH2V_11 || GameVersion == SH2V_DC)
	{
		GameSavedTextColorAddr = GameSavedTextColorAddr - 0x05;
		CantSavedTextColorAddr = CantSavedTextColorAddr - 0x05;
		NoQuickSaveTextColorAddr = NoQuickSaveTextColorAddr - 0x05;
	}

	if (printGameSavedFunction == NULL || printCantSavedFunction == NULL || printNoQuickSavedFunction == NULL )
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// I had to overwrite on to render text functions because of adding some fixes.
	WriteCalltoMemory((BYTE*)0x040253A, *print_quick_saved_string);
	WriteCalltoMemory((BYTE*)0x04024D2, *print_cant_save_string);
	WriteCalltoMemory((BYTE*)0x0402568, *print_no_quick_save_string);

	const auto GameSavedFunction = (void* (__cdecl*)(void))(printGameSavedFunction);
	PrintSave = (printQuickSaveString)GameSavedFunction;

	const auto CantSavedFunction = (void* (__cdecl*)(void))(printCantSavedFunction);
	PrintCantSave = (printCantSaveString)CantSavedFunction;

	const auto NoAutoSavedFunction = (void* (__cdecl*)(void))(printNoQuickSavedFunction);
	PrintNoSave = (printNoSaveString)NoAutoSavedFunction;
}

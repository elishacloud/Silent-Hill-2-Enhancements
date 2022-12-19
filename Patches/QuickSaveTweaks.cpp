#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include <d3d9types.h>
#include <sstream>
#include "External\injector\include\injector\injector.hpp"

DWORD GameSavedTextColorAddr;
DWORD CantSavedTextColorAddr;
DWORD NoQuickSaveTextColorAddr;
DWORD printGameSavedFunction;
DWORD printCantSavedFunction;
DWORD printNoQuickSavedFunction;

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
int8_t TextPosVecX;

// I don't think game uses RGB or something like that but anyway it's just works :)
struct RGBA
{
	uint32_t R, G, B, A;
};

// Window width and height for the aspect ratio calc
extern LONG BufferWidth, BufferHeight;

void ChangeSaveColor(void* Addr,RGBA savedColorValue)
{
	UpdateMemoryAddress(((BYTE*)Addr + 0x0), &savedColorValue.A, sizeof(savedColorValue.A));
	UpdateMemoryAddress(((BYTE*)Addr + 0x5), &savedColorValue.B, sizeof(savedColorValue.B));

	if (GameVersion == SH2V_10)
	{
		UpdateMemoryAddress(((BYTE*)Addr + 0xB), &savedColorValue.G, sizeof(savedColorValue.G));
		UpdateMemoryAddress(((BYTE*)Addr + 0x10), &savedColorValue.R, sizeof(savedColorValue.R));
	}
	else
	{
		UpdateMemoryAddress(((BYTE*)Addr + 0xA), &savedColorValue.G, sizeof(savedColorValue.G));
		UpdateMemoryAddress(((BYTE*)Addr + 0xF), &savedColorValue.R, sizeof(savedColorValue.R));
	}
}

// This aspect ratio calc is made by @Polymega https://github.com/elishacloud/Silent-Hill-2-Enhancements/issues/564#issue-1301079704
void __stdcall SetAspectRatio()
{
	TextPosVecX = ((((BufferHeight * 1.333333333) - BufferWidth) * (static_cast<double>(BufferWidth) / BufferHeight)) / BufferWidth) * 195 + 20;
}

void __stdcall print_quick_saved_string(void)
{
	SetAspectRatio();
	if(ClearFontBeforePrint)
		clearFont();
	UpdateMemoryAddress((void*)(GameSavedTextColorAddr + 0x26), &TextPosVecX, sizeof(TextPosVecX));

	PrintSave();
}

void __stdcall print_cant_save_string(void)
{
	SetAspectRatio();
	if (ClearFontBeforePrint)
		clearFont();
	UpdateMemoryAddress((void*)(CantSavedTextColorAddr + 0x26), &TextPosVecX, sizeof(TextPosVecX));

	PrintCantSave();
}

void __stdcall print_no_quick_save_string(void)
{
	SetAspectRatio();
	if (ClearFontBeforePrint)
		clearFont();
	UpdateMemoryAddress((void*)(NoQuickSaveTextColorAddr + 0x26), &TextPosVecX, sizeof(TextPosVecX));

	PrintNoSave();
}

void PatchQuickSavePos()
{
	constexpr BYTE textPosLockBypassBytes[] = { 0x7c, 0x18, 0x85, 0xc9, 0x7c, 0x14, 0xb8, 0x01, 0x00, 0x00, 0x00 };
	DWORD textPosLockBypassAddr = SearchAndGetAddresses(0x0048051f, 0x004807bf, 0x004809cf, textPosLockBypassBytes, sizeof(textPosLockBypassBytes), 0x0);

	injector::MakeNOP((BYTE*)textPosLockBypassAddr, 6);

	constexpr BYTE quick_saved_addr_bytes[] = { 0x68, 0x80, 0x00, 0x00,0x00 , 0x68, 0x96,0x00,0x00,0x00 };
	GameSavedTextColorAddr = SearchAndGetAddresses(   (DWORD)0x0044c688,   (DWORD)0x0044c856, (DWORD)0x0044c856, (BYTE*)quick_saved_addr_bytes,  sizeof(quick_saved_addr_bytes),   0x1);
	CantSavedTextColorAddr = SearchAndGetAddresses(   (DWORD)0x0044c6d8,   (DWORD)0x0044c8e6, (DWORD)0x0044c8e6, (BYTE*)quick_saved_addr_bytes,  sizeof(quick_saved_addr_bytes),   0x1);
	NoQuickSaveTextColorAddr = SearchAndGetAddresses( (DWORD)0x0044c728,   (DWORD)0x0044c976, (DWORD)0x0044c976, (BYTE*)quick_saved_addr_bytes,  sizeof(quick_saved_addr_bytes),   0x1);
	

	// Points to enWaitAllInsect (based on ps2 demo version).
	constexpr BYTE FontClearBytes[]{ 0x56, 0x33, 0xf6, 0x56, 0x66, 0x89, 0x35 ,0xC6 };
	const auto FontClearAddr = (DWORD)CheckMultiMemoryAddress((void*)0x0047eee0, (void*)0x0047f180, (void*)0x0047f390, (void*)FontClearBytes, sizeof(FontClearBytes));


	// Sets the stop_moth_sfx function instances address.
	const auto fontClearAddr = (uintptr_t * (__cdecl*)(void))(FontClearAddr);
	clearFont = (fontClear)fontClearAddr;

	switch (GameVersion)
	{
	case SH2V_10:
		printGameSavedFunction = (DWORD)0x0044c680;
		printCantSavedFunction = (DWORD)0x0044c6d0;
		printNoQuickSavedFunction = (DWORD)0x0044c720;
		break;
	case SH2V_11:
	case SH2V_DC:
		printGameSavedFunction = (DWORD)0x0044c820;
		printCantSavedFunction = (DWORD)0x0044c8b0;
		printNoQuickSavedFunction = (DWORD)0x0044c940;
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
		GameSavedTextColorAddr = GameSavedTextColorAddr - 5;
		CantSavedTextColorAddr = CantSavedTextColorAddr - 5;
		NoQuickSaveTextColorAddr = NoQuickSaveTextColorAddr - 5;
	}

	if (printGameSavedFunction == NULL || printCantSavedFunction == NULL || printNoQuickSavedFunction == NULL )
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// I had to overwrite on to render text functions because of adding some fixes.
	WriteCalltoMemory((BYTE*)0x040253a, *print_quick_saved_string);
	WriteCalltoMemory((BYTE*)0x04024d2, *print_cant_save_string);
	WriteCalltoMemory((BYTE*)0x0402568, *print_no_quick_save_string);

	const auto GameSavedFunction = (void* (__cdecl*)(void))(printGameSavedFunction);
	PrintSave = (printQuickSaveString)GameSavedFunction;

	const auto CantSavedFunction = (void* (__cdecl*)(void))(printCantSavedFunction);
	PrintCantSave = (printCantSaveString)CantSavedFunction;

	const auto NoAutoSavedFunction = (void* (__cdecl*)(void))(printNoQuickSavedFunction);
	PrintNoSave = (printNoSaveString)NoAutoSavedFunction;
}
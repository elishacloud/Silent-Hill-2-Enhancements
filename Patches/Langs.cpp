/**
* Copyright (C) 2018 Elisha Riedlinger
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
#include "Common\Settings.h"
#include <string>

using namespace std;

constexpr BYTE LangSearchBytesA[] = { 0xDB, 0x01, 0x48, 0x83, 0xF8, 0x04, 0x77, 0x41, 0xFF, 0x24, 0x85 };
constexpr BYTE LangSearchBytesB[] = { 0x56, 0x41, 0x85, 0xC0, 0xBE, 0x01, 0x00, 0x00, 0x00, 0x57, 0x8B, 0xFE, 0x89, 0x0D, 0xA4 };
constexpr BYTE LangSearchBytesC[] = { 0xFF, 0x15, 0x70, 0x6A, 0x4A, 0x02, 0x83, 0xC4, 0x08, 0xC3, 0x8B, 0x4C, 0x24, 0x08, 0x40, 0x50 };
constexpr BYTE LangSearchBytesD[] = { 0x83, 0xC4, 0x08, 0xC3, 0x8B, 0x4C, 0x24, 0x0C, 0x49, 0x83, 0xF9, 0x04, 0x77, 0x74, 0xFF, 0x24 };
constexpr BYTE LangSearchBytesE[] = { 0x68, 0x80, 0x00, 0x00, 0x00, 0x68, 0x96, 0x00, 0x00, 0x00, 0x48, 0x68, 0x96, 0x00, 0x00, 0x00, 0x68, 0x96, 0x00, 0x00, 0x00, 0x89, 0x44, 0x24 };
constexpr BYTE LangSearchBytesF[] = { 0x48, 0x83, 0xF8, 0x04, 0x56, 0x77, 0x23, 0xFF, 0x24, 0x85, 0x90, 0xC8, 0x44, 0x00, 0xBE, 0x74 };
constexpr BYTE LangSearchBytesG[] = { 0x6A, 0x04, 0xE8, 0xB2, 0x28, 0xFE, 0xFF, 0x83, 0xC4, 0x04, 0x85, 0xC0, 0x75, 0x05, 0x83, 0xFE, 0x01, 0x75, 0x6F };
constexpr BYTE LangSearchBytesH[] = { 0xA3, 0x00, 0x8B, 0x51, 0x08, 0x83, 0xC0, 0x08, 0x3B, 0xC2, 0x75, 0xEA, 0x5E, 0xC3, 0x90, 0x90 };
constexpr BYTE LangSearchBytesI[] = { 0x01, 0x00, 0x00, 0x6B, 0xC0, 0x1B, 0x2D, 0x8C, 0x00, 0x00, 0x00, 0x50, 0x6A, 0xFB, 0xE8, 0xCD, 0xCC, 0xFF, 0xFF, 0x0F };
constexpr BYTE LangSearchBytesJ[] = { 0x03, 0x00, 0x00, 0x6B, 0xC0, 0x1B, 0x2D, 0x8C, 0x00, 0x00, 0x00, 0x50, 0x6A, 0xFB, 0xE8, 0x37, 0xCE, 0xFF, 0xFF, 0x0F };

#define STR_PER_LANG 25

BYTE langMin = 1;
char *str_ptr[STR_PER_LANG * 6];
BYTE *gLangID;

void *LangsPauseRetAddr;
void *LangsPauseStrPtr;
BYTE LangsPauseLangID = 255;

void LangsPauseHelper()
{
	if (LangsPauseLangID != *gLangID) {
		LangsPauseStrPtr = (void *)&str_ptr[STR_PER_LANG * (int)*gLangID];
		UpdateMemoryAddress((void *)((BYTE*)LangsPauseRetAddr + 6), (void *)&LangsPauseStrPtr, 4);
		LangsPauseLangID = *gLangID;
	}
}

__declspec(naked) void __stdcall LangsPauseASM()
{
	__asm
	{
		push	eax
		call	LangsPauseHelper
		pop		eax
		jmp		LangsPauseRetAddr
	}
}

void *LangsErrorsRetAddr;
BYTE LangsErrorsLangID = 255;
bool isMultiLang;

void LangsErrorsHelper()
{
	if (LangsErrorsLangID != *gLangID) {
		if (isMultiLang) {
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01CF), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 9], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01F7), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 9], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x022F), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 9], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x017B), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 10], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0185), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 10], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x018F), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 10], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0239), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 11], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0249), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 12], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0201), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 13], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01D9), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 14], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01B1), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 15], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01A7), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 16], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0211), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 17], 4);
		} else {
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x00D5), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 9], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x00F6), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 9], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0124), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 9], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0086), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 10], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0090), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 10], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x009A), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 10], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x012B), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 11], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0138), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 12], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x00FD), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 13], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x00DC), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 14], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x00BB), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 15], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x00B1), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 16], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x010A), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 17], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0069), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 18], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01B5), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 19], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01E5), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 19], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0207), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 19], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0193), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 20], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0237), (void *)&str_ptr[STR_PER_LANG * (int)*gLangID + 20], 4);
		}
		LangsErrorsLangID = *gLangID;
	}
}

__declspec(naked) void __stdcall LangsErrorsASM()
{
	__asm
	{
		push	eax
		push	esi
		push	ecx
		call	LangsErrorsHelper
		pop		ecx
		pop		esi
		pop		eax
		push	esi
		inc		ecx
		test	eax, eax
		mov		esi, 1
		jmp		LangsErrorsRetAddr
	}
}

void *LangsButtonRetAddr;

__declspec(naked) void __stdcall LangsButtonASM()
{
	__asm
	{
		mov		edx, gLangID
		movzx	eax, [edx]
		imul	eax, STR_PER_LANG
		mov		edx, [str_ptr + 21 * 4 + eax * 4]
		push	edx
		jmp		LangsButtonRetAddr
	}
}

void *LangsGameSavedRetAddr;

__declspec(naked) void __stdcall LangsGameSavedASM()
{
	__asm
	{
		push	eax
		push	ecx
		mov		edx, gLangID
		movzx	eax, [edx]
		imul	eax, STR_PER_LANG
		mov		edx, [str_ptr + 22 * 4 + eax * 4]
		pop		ecx
		pop		eax
		push	edx
		jmp		LangsGameSavedRetAddr
	}
}

void *LangsCantSaveRetAddr;

__declspec(naked) void __stdcall LangsCantSaveASM()
{
	__asm
	{
		push	eax
		push	ecx
		mov		edx, gLangID
		movzx	eax, [edx]
		imul	eax, STR_PER_LANG
		mov		edx, [str_ptr + 23 * 4 + eax * 4]
		pop		ecx
		pop		eax
		push	edx
		jmp		LangsCantSaveRetAddr
	}
}

void *LangsNoQuickSaveRetAddr;

__declspec(naked) void __stdcall LangsNoQuickSaveASM()
{
	__asm
	{
		push	eax
		push	ecx
		mov		edx, gLangID
		movzx	eax, [edx]
		imul	eax, STR_PER_LANG
		mov		edx, [str_ptr + 24 * 4 + eax * 4]
		pop		ecx
		pop		eax
		push	edx
		jmp		LangsNoQuickSaveRetAddr
	}
}

void *LangsGameSavedRetAddr2;

__declspec(naked) void __stdcall LangsGameSavedASM2()
{
	__asm
	{
		push	eax
		mov		edx, gLangID
		movzx	eax, [edx]
		imul	eax, STR_PER_LANG
		mov		esi, [str_ptr + 22 * 4 + eax * 4]
		pop		eax
		jmp		LangsGameSavedRetAddr2
	}
}

void *LangsCantSaveRetAddr2;

__declspec(naked) void __stdcall LangsCantSaveASM2()
{
	__asm
	{
		push	eax
		mov		edx, gLangID
		movzx	eax, [edx]
		imul	eax, STR_PER_LANG
		mov		esi, [str_ptr + 23 * 4 + eax * 4]
		pop		eax
		jmp		LangsCantSaveRetAddr2
	}
}

void *LangsNoQuickSaveRetAddr2;

__declspec(naked) void __stdcall LangsNoQuickSaveASM2()
{
	__asm
	{
		push	eax
		mov		edx, gLangID
		movzx	eax, [edx]
		imul	eax, STR_PER_LANG
		mov		esi, [str_ptr + 24 * 4 + eax * 4]
		pop		eax
		jmp		LangsNoQuickSaveRetAddr2
	}
}

DWORD *gLangID_S;
WORD *SomeWord;
BYTE *SomeByte;
DWORD *SomeDataA;
DWORD *SomeDataB;

DWORD (*FunctionA)(int a, int b, int c);
DWORD (*FunctionB)(int a);
DWORD (*FunctionC)(int a);
DWORD (*FunctionD)(int a, int b);

void *LabelA;
void *LabelB;

__declspec(naked) void __stdcall LangsSelectorASM()
{
	__asm
	{
		mov		eax, SomeWord
		movzx	eax, [eax]
		cmp     ax, bx
		jnz     jmplabA
		mov		eax, SomeByte
		mov     al, [eax]
		test    al, al
		jnz     jmplabA
		push    ebx
		mov		eax, SomeDataA
		push    eax
		push    ebx
		call    FunctionA
		add     esp, 0Ch
		test    eax, eax
		jnz     localC
		push    8
		call    FunctionB
		add     esp, 4
		test    eax, eax
		jnz     localC
		cmp     ebp, 2
		jz      localC
	localA:
		push    ebx
		mov		eax, SomeDataA
		push    eax
		push    ebx
		call    FunctionA
		add     esp, 0Ch
		test    eax, eax
		jnz     localF
		push    8
		call    FunctionB
		add     esp, 4
		test    eax, eax
		jnz     localF
		cmp     ebp, 2
		jz      localF
		push    ebx
		mov		eax, SomeDataB
		push    eax
		push    ebx
		call    FunctionA
		add     esp, 0Ch
		test    eax, eax
		jnz     localB
		push    4
		call    FunctionB
		add     esp, 4
		test    eax, eax
		jnz     localB
		cmp     ebp, 1
		jnz     localD
	localB:
		mov		eax, gLangID
		movzx	eax, [eax]
		cmp     al, [langMin]
		jnz     localD
		push    2
		mov		eax, gLangID
		mov     [eax], 5
		call    FunctionC
		push    -1
		push    ebx
		call    FunctionD
		mov		eax, gLangID
		movzx	eax, [eax]
		mov		ecx, gLangID_S
		mov     [ecx], eax
		add     esp, 0Ch
		jmp     jmplabB
	localC:
		mov		eax, gLangID
		movzx	eax, [eax]
		cmp     eax, 5
		jnz     localA
		push    2
		mov		eax, gLangID
		mov		cl, [langMin]
		mov		[eax], cl
		call    FunctionC
		push    -1
		push    ebx
		mov		eax, gLangID
		movzx	eax, [eax]
		mov		ecx, gLangID_S
		mov		[ecx], eax
		call    FunctionD
		add     esp, 0Ch
		jmp     jmplabB
	localD:
		push    ebx
		mov		eax, SomeDataB
		push    eax
		push    ebx
		call    FunctionA
		add     esp, 0Ch
		test    eax, eax
		jnz     localE
		push    4
		call    FunctionB
		add     esp, 4
		test    eax, eax
		jnz     localE
		cmp     ebp, 1
		jnz     jmplabB
	localE:
		mov		eax, gLangID
		movzx	eax, [eax]
		mov     cl, al
		dec     cl
		push    2
		mov		eax, gLangID
		mov     [eax], cl
		call    FunctionC
		push    -1
		push    ebx
		call    FunctionD
		mov		eax, gLangID
		movzx	eax, [eax]
		mov		ecx, gLangID_S
		mov		[ecx], eax
		add     esp, 0Ch
		jmp     jmplabB
	localF:
		mov		eax, gLangID
		movzx	eax, [eax]
		mov     cl, al
		inc     cl
		push    2
		mov		eax, gLangID
		mov		[eax], cl
		call    FunctionC
		push    -1
		push    ebx
		call    FunctionD
		mov		eax, gLangID
		movzx	eax, [eax]
		mov		ecx, gLangID_S
		mov		[ecx], eax
		add     esp, 0Ch
		jmp     jmplabB
	jmplabA:
		jmp		LabelA
	jmplabB:
		jmp		LabelB
	}
}

void UpdateCustomExeStr()
{
	void *DSpecAddrA = CheckMultiMemoryAddress((void*)0x0040730A, (void*)0x0040730A, (void*)0x0040731A, (void*)LangSearchBytesA, sizeof(LangSearchBytesA));
	
	// Search for address
	if (!DSpecAddrA) {
		Logging::Log() << __FUNCTION__ << " searching for memory address!";
		DSpecAddrA = GetAddressOfData(LangSearchBytesA, sizeof(LangSearchBytesA), 1, 0x00407000, 1800);
	}

	// Checking address pointer
	if (!DSpecAddrA) {
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	langMin = (UnlockJapLang == false);
	gLangID = (BYTE *)*(DWORD *)((BYTE*)DSpecAddrA - 2);

	ifstream file("sh2e/text/exe_str.txt");

	if (file.is_open()) {
		string line;
		int i = 0;
		while (getline(file, line)) {
			if (!line.empty() && !((*(char *)line.c_str() == '/') && (*(char *)(line.c_str() + 1) == '/'))) {
				str_ptr[i] = (char *)malloc(line.length() + 10);
				if (str_ptr[i])
					strcpy(str_ptr[i], line.c_str());
				i++;
			}
		}
		file.close();

		// Pause menu
		LangsPauseRetAddr = (void *)((BYTE*)DSpecAddrA + 0x0F);
		WriteJMPtoMemory((BYTE*)DSpecAddrA + 6, *LangsPauseASM, 9);

		// Errors
		void *DSpecAddrB = CheckMultiMemoryAddress((void*)0x00407629, (void*)0x00407629, (void*)0x00407639, (void*)LangSearchBytesB, sizeof(LangSearchBytesB));
		if (!DSpecAddrB) {
			DSpecAddrB = GetAddressOfData(LangSearchBytesB, sizeof(LangSearchBytesB), 4, 0x00508000, 1800);
		}
		if (DSpecAddrB) {
			LangsErrorsRetAddr = (void *)((BYTE*)DSpecAddrB + 9);
			isMultiLang = *(WORD *)((BYTE*)LangsErrorsRetAddr + 0x70) == 0xF685;
			WriteJMPtoMemory((BYTE*)DSpecAddrB, *LangsErrorsASM, 9);
		}

		// Button
		BYTE codeA[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
		void *DSpecAddrC = GetAddressOfData(LangSearchBytesC, sizeof(LangSearchBytesC), 1, 0x005AEE90, 1800);
		if (DSpecAddrC) {
			WriteJMPtoMemory((BYTE*)DSpecAddrC + 0x10, *LangsButtonASM, 5);
			LangsButtonRetAddr = (void *)((BYTE*)DSpecAddrC + 0x15);
		} else {
			void *DSpecAddrD = GetAddressOfData(LangSearchBytesD, sizeof(LangSearchBytesD), 1, 0x005AF000, 1800);
			if (DSpecAddrD) {
				UpdateMemoryAddress((void *)((BYTE*)DSpecAddrD + 0x0C), (void *)codeA, 9);
				WriteJMPtoMemory((BYTE*)DSpecAddrD + 0x1B, *LangsButtonASM, 5);
				LangsButtonRetAddr = (void *)((BYTE*)DSpecAddrD + 0x20);
			}
		}

		// Quick save
		void *DSpecAddrE = GetAddressOfData(LangSearchBytesE, sizeof(LangSearchBytesE), 1, 0x0044C680, 1800);
		if (DSpecAddrE) {
			WriteJMPtoMemory((BYTE*)DSpecAddrE + 0x28, *LangsGameSavedASM, 5);
			LangsGameSavedRetAddr = (void *)((BYTE*)DSpecAddrE + 0x2D);
			DSpecAddrE = GetAddressOfData(LangSearchBytesE, sizeof(LangSearchBytesE), 1, (DWORD)DSpecAddrE, 1800);
			if (DSpecAddrE) {
				WriteJMPtoMemory((BYTE*)DSpecAddrE + 0x28, *LangsCantSaveASM, 5);
				LangsCantSaveRetAddr = (void *)((BYTE*)DSpecAddrE + 0x2D);
				DSpecAddrE = GetAddressOfData(LangSearchBytesE, sizeof(LangSearchBytesE), 1, (DWORD)DSpecAddrE, 1800);
				if (DSpecAddrE) {
					WriteJMPtoMemory((BYTE*)DSpecAddrE + 0x28, *LangsNoQuickSaveASM, 5);
					LangsNoQuickSaveRetAddr = (void *)((BYTE*)DSpecAddrE + 0x2D);
				}
			}
		} else {
			void *DSpecAddrF = GetAddressOfData(LangSearchBytesF, sizeof(LangSearchBytesF), 1, 0x0044C800, 1800);
			if (DSpecAddrF) {
				UpdateMemoryAddress((void *)((BYTE*)DSpecAddrF + 5), (void *)codeA, 9);
				WriteJMPtoMemory((BYTE*)DSpecAddrF + 0x0E, *LangsGameSavedASM2, 5);
				LangsGameSavedRetAddr2 = (void *)((BYTE*)DSpecAddrF + 0x13);
				DSpecAddrF = (void *)((BYTE*)DSpecAddrF + 0x90);
				UpdateMemoryAddress((void *)((BYTE*)DSpecAddrF + 5), (void *)codeA, 9);
				WriteJMPtoMemory((BYTE*)DSpecAddrF + 0x0E, *LangsCantSaveASM2, 5);
				LangsCantSaveRetAddr2 = (void *)((BYTE*)DSpecAddrF + 0x13);
				DSpecAddrF = (void *)((BYTE*)DSpecAddrF + 0x90);
				UpdateMemoryAddress((void *)((BYTE*)DSpecAddrF + 5), (void *)codeA, 9);
				WriteJMPtoMemory((BYTE*)DSpecAddrF + 0x0E, *LangsNoQuickSaveASM2, 5);
				LangsNoQuickSaveRetAddr2 = (void *)((BYTE*)DSpecAddrF + 0x13);
			}
		}

		// Unlock language selector
		void *DSpecAddrG = GetAddressOfData(LangSearchBytesG, sizeof(LangSearchBytesG), 1, 0x00463F70, 1800);
		if (DSpecAddrG) {
			UpdateMemoryAddress((void *)((BYTE*)DSpecAddrG + 0x19), (void *)&langMin, 1);
			UpdateMemoryAddress((void *)((BYTE*)DSpecAddrG + 0x5F), (void *)&langMin, 1);
		} else {
			void *DSpecAddrH = GetAddressOfData(LangSearchBytesH, sizeof(LangSearchBytesH), 1, 0x004F7100, 1800);
			if (DSpecAddrH) {
				gLangID_S = (DWORD *)*(DWORD *)((BYTE*)DSpecAddrH + 0x32);
				void *DSpecAddrI = GetAddressOfData(LangSearchBytesI, sizeof(LangSearchBytesI), 1, 0x00463D00, 1800);  //00463D20
				if (DSpecAddrI) {
					SomeWord = (WORD *)*(DWORD *)((BYTE*)DSpecAddrI + 0xBF);
					SomeByte = (BYTE *)*(DWORD *)((BYTE*)DSpecAddrI + 0xCB);
					SomeDataA = (DWORD *)*(DWORD *)((BYTE*)DSpecAddrI + 0xFC);
					SomeDataB = (DWORD *)*(DWORD *)((BYTE*)DSpecAddrI + 0xD7);
					FunctionA = (DWORD(*)(int a, int b, int c))(((BYTE*)DSpecAddrI + 0xE1) + *(int *)((BYTE*)DSpecAddrI + 0xDD));
					FunctionB = (DWORD(*)(int a))(((BYTE*)DSpecAddrI + 0xEF) + *(int *)((BYTE*)DSpecAddrI + 0xEB));
					FunctionC = (DWORD(*)(int a))(((BYTE*)DSpecAddrI - 0x0548) + *(int *)((BYTE*)DSpecAddrI - 0x054C));
					FunctionD = (DWORD(*)(int a, int b))(((BYTE*)DSpecAddrI - 0x540) + *(int *)((BYTE*)DSpecAddrI - 0x0544));
					LabelA = (DWORD *)(((BYTE*)DSpecAddrI + 0xD5) + *(int *)((BYTE*)DSpecAddrI + 0xD1));
					LabelB = (DWORD *)(((BYTE*)DSpecAddrI + 0x03) + *(int *)((BYTE*)DSpecAddrI - 0x01));
					WriteJMPtoMemory((BYTE*)DSpecAddrI - 0xE8, *LangsSelectorASM, 5);
				} else {
					void *DSpecAddrJ = GetAddressOfData(LangSearchBytesJ, sizeof(LangSearchBytesJ), 1, 0x00463E00, 1800); //00463E16
					if (DSpecAddrJ) {
						SomeWord = (WORD *)*(DWORD *)((BYTE*)DSpecAddrJ + 0x010D);
						SomeByte = (BYTE *)*(DWORD *)((BYTE*)DSpecAddrJ + 0x0119);
						SomeDataA = (DWORD *)*(DWORD *)((BYTE*)DSpecAddrJ + 0x0138);
						SomeDataB = (DWORD *)*(DWORD *)((BYTE*)DSpecAddrJ + 0x0125);
						FunctionA = (DWORD(*)(int a, int b, int c))(((BYTE*)DSpecAddrJ + 0x012F) + *(int *)((BYTE*)DSpecAddrJ + 0x012B));
						FunctionB = (DWORD(*)(int a))(((BYTE*)DSpecAddrJ + 0x0150) + *(int *)((BYTE*)DSpecAddrJ + 0x014C));
						FunctionC = (DWORD(*)(int a))(((BYTE*)DSpecAddrJ - 0x03CE) + *(int *)((BYTE*)DSpecAddrJ - 0x03D2));
						FunctionD = (DWORD(*)(int a, int b))(((BYTE*)DSpecAddrJ - 0x3C6) + *(int *)((BYTE*)DSpecAddrJ - 0x03CA));
						LabelA = (DWORD *)(((BYTE*)DSpecAddrJ + 0x9A) + *(int *)((BYTE*)DSpecAddrJ + 0x96));
						LabelB = (DWORD *)(((BYTE*)DSpecAddrJ + 0x0186) + *(int *)((BYTE*)DSpecAddrJ + 0x0182));
						WriteJMPtoMemory((BYTE*)DSpecAddrJ + 0x95, *LangsSelectorASM, 5);
					}
				}
			}
		}
	} else {
		Logging::Log() << __FUNCTION__ << " Error: Could not find text file";
	}
}

/**
* Copyright (C) 2023 Elisha Riedlinger
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
#include <shlwapi.h>
#include "Resource.h"
#include "Common\FileSystemHooks.h"
#include "Common\Utils.h"
#include "Common\FileSystemHooks.h"
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
constexpr BYTE LangSearchBytesK[] = { 0x75, 0x0E, 0x68, 0xD0, 0x00, 0x00, 0x00, 0x68, 0x0E, 0x01, 0x00, 0x00, 0x6A, 0x46, 0xEB, 0x42, 0x3C };

struct LANGSTRUCT
{
	char* Name;
	DWORD RcData;
};

// Order of languages in array is important
constexpr LANGSTRUCT LangList[] = {
	{ "r_menu_j.res", IDR_LANG_RES_JA },
	{ "r_menu_e.res", IDR_LANG_RES_EN },
	{ "r_menu_f.res", IDR_LANG_RES_FR },
	{ "r_menu_g.res", IDR_LANG_RES_DE },
	{ "r_menu_i.res", IDR_LANG_RES_IT },
	{ "r_menu_s.res", IDR_LANG_RES_ES },
};

#define STR_PER_LANG 29

BYTE langMin = 1;
char *exeStrPtr[STR_PER_LANG * 6];
BYTE *gLangID;

void *LangsPauseRetAddr;
void *LangsPauseStrPtr;
BYTE LangsPauseLangID = 255;

void LangsPauseHelper()
{
	if (LangsPauseLangID != *gLangID)
	{
		LangsPauseStrPtr = (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID];
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
	if (LangsErrorsLangID != *gLangID)
	{
		if (isMultiLang)
		{
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01CF), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 9], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01F7), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 9], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x022F), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 9], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x017B), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 10], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0185), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 10], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x018F), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 10], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0239), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 11], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0249), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 12], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0201), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 13], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01D9), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 14], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01B1), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 15], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01A7), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 16], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0211), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 17], 4);
		}
		else
		{
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x00D5), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 9], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x00F6), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 9], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0124), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 9], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0086), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 10], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0090), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 10], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x009A), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 10], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x012B), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 11], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0138), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 12], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x00FD), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 13], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x00DC), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 14], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x00BB), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 15], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x00B1), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 16], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x010A), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 17], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0069), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 18], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01B5), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 19], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x01E5), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 19], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0207), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 19], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0193), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 20], 4);
			UpdateMemoryAddress((void *)((BYTE*)LangsErrorsRetAddr + 0x0237), (void *)&exeStrPtr[STR_PER_LANG * (int)*gLangID + 20], 4);
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
		mov		edx, [exeStrPtr + 21 * 4 + eax * 4]
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
		mov		edx, [exeStrPtr + 22 * 4 + eax * 4]
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
		mov		edx, [exeStrPtr + 23 * 4 + eax * 4]
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
		mov		edx, [exeStrPtr + 24 * 4 + eax * 4]
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
		mov		esi, [exeStrPtr + 22 * 4 + eax * 4]
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
		mov		esi, [exeStrPtr + 23 * 4 + eax * 4]
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
		mov		esi, [exeStrPtr + 24 * 4 + eax * 4]
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

void *BloodArrowFixRetAddr;
int *PosByLang;

__declspec(naked) void __stdcall BloodArrowFixASM()
{
	__asm
	{
		mov		eax, PosByLang
		mov		eax, [eax]
		add     eax, 0FFFFFFFBh
		push    0FFFFFFE0h
		push	eax
		jmp BloodArrowFixRetAddr
	}
}

void *BloodArrowFixRetAddr2;

__declspec(naked) void __stdcall BloodArrowFixASM2()
{
	__asm
	{
		push    ecx
		add     edx, 1Eh
		mov		ecx, PosByLang
		mov		ecx, [ecx]
		add     edx, ecx
		push	edx
		jmp BloodArrowFixRetAddr2
	}
}

BYTE *MFSomeByte;
BYTE *MFMenuYPos;
void *MouseFixRetAddr;

DWORD (*RetMouseB)(int a);
DWORD (*RetMouseX)(int a);
DWORD (*RetMouseY)(int a);

__declspec(naked) void __stdcall MouseFixASM()
{
	__asm
	{
		mov		ecx, MFSomeByte
		mov		al, [ecx]
		cmp		al, 0
		jz		MFExit
		mov     ebp, 0
		push	1
		call	RetMouseB
		add		esp, 4
		test	eax, eax
		jz		MFNext
		call	RetMouseX
		mov		ecx, PosByLang
		mov		ecx, [ecx]
		add		ecx, 0F0h
		cmp     eax, ecx
		jle     MFNext
		call	RetMouseX
		mov		ecx, PosByLang
		mov		ecx, [ecx]
		add		ecx, 10Bh
		cmp     eax, ecx
		jge     MFNext
		call    RetMouseY
		mov		ecx, MFMenuYPos
		movsx   ecx, [ecx]
		imul    ecx, 1Bh
		add     ecx, 53h
		cmp     eax, ecx
		jle     MFNext
		call    RetMouseY
		mov		edx, MFMenuYPos
		movsx   edx, [edx]
		imul    edx, 1Bh
		add     edx, 6Eh
		cmp     eax, edx
		jge     MFNext
		mov     ebp, 1
		jmp     MFExit
	MFNext:
		push    1
		call    RetMouseB
		add     esp, 4
		test    eax, eax
		jz      MFExit
		call    RetMouseX
		mov		edx, MFMenuYPos
		movsx   edx, [edx]
		mov		ecx, PosByLang
		mov		ecx, [ecx]
		add     ecx, [esp + edx * 4 + 3Ch]
		cmp     eax, ecx
		jle     MFExit
		call    RetMouseX
		mov		ecx, MFMenuYPos
		movsx   ecx, [ecx]
		mov     edx, [esp + ecx * 4 + 3Ch]
		add     edx, 1Bh
		mov		ecx, PosByLang
		mov		ecx, [ecx]
		add     edx, ecx
		cmp     eax, edx
		jge     MFExit
		call    RetMouseY
		mov		ecx, MFMenuYPos
		movsx   ecx, [ecx]
		imul    ecx, 1Bh
		add     ecx, 53h
		cmp     eax, ecx
		jle     MFExit
		call    RetMouseY
		mov		edx, MFMenuYPos
		movsx   edx, [edx]
		imul    edx, 1Bh
		add     edx, 6Eh
		cmp     eax, edx
		jge     MFExit
		mov     ebp, 2
	MFExit:
		mov		ecx, MFSomeByte
		mov		al, [ecx]
		jmp		MouseFixRetAddr
	}
}

__declspec(naked) void __stdcall MouseFixASM2()
{
	__asm
	{
		mov		ecx, MFSomeByte
		mov		al, [ecx]
		cmp		al, 0
		jz		MFExit
		mov     esi, 0
		push	1
		call	RetMouseB
		add		esp, 4
		test	eax, eax
		jz		MFNext
		call	RetMouseX
		mov		ecx, PosByLang
		mov		ecx, [ecx]
		add		ecx, 0F0h
		cmp     eax, ecx
		jle     MFNext
		call	RetMouseX
		mov		ecx, PosByLang
		mov		ecx, [ecx]
		add		ecx, 10Bh
		cmp     eax, ecx
		jge     MFNext
		call    RetMouseY
		mov		ecx, MFMenuYPos
		movsx   ecx, [ecx]
		imul    ecx, 1Bh
		add     ecx, 53h
		cmp     eax, ecx
		jle     MFNext
		call    RetMouseY
		mov		edx, MFMenuYPos
		movsx   edx, [edx]
		imul    edx, 1Bh
		add     edx, 6Eh
		cmp     eax, edx
		jge     MFNext
		mov     esi, 1
		jmp     MFExit
	MFNext:
		push    1
		call    RetMouseB
		add     esp, 4
		test    eax, eax
		jz      MFExit
		call    RetMouseX
		mov		edx, MFMenuYPos
		movsx   edx, [edx]
		mov		ecx, PosByLang
		mov		ecx, [ecx]
		add     ecx, [esp + edx * 4 + 3Ch]
		cmp     eax, ecx
		jle     MFExit
		call    RetMouseX
		mov		ecx, MFMenuYPos
		movsx   ecx, [ecx]
		mov     edx, [esp + ecx * 4 + 3Ch]
		add     edx, 1Bh
		mov		ecx, PosByLang
		mov		ecx, [ecx]
		add     edx, ecx
		cmp     eax, edx
		jge     MFExit
		call    RetMouseY
		mov		ecx, MFMenuYPos
		movsx   ecx, [ecx]
		imul    ecx, 1Bh
		add     ecx, 53h
		cmp     eax, ecx
		jle     MFExit
		call    RetMouseY
		mov		edx, MFMenuYPos
		movsx   edx, [edx]
		imul    edx, 1Bh
		add     edx, 6Eh
		cmp     eax, edx
		jge     MFExit
		mov     esi, 2
	MFExit:
		mov		[esp + 08h], esi
		mov		ecx, MFSomeByte
		mov		al, [ecx]
		jmp		MouseFixRetAddr
	}
}

void *BloodStrFixRetAddr;

__declspec(naked) void __stdcall BloodStrFixASM()
{
	__asm
	{
		add     edi, 10Ch
		push    edi
		jmp		BloodStrFixRetAddr
	}
}

void *BloodStrFixRetAddr2;

__declspec(naked) void __stdcall BloodStrFixASM2()
{
	__asm
	{
		add     edi, 10Ch
		push    edi
		jmp		BloodStrFixRetAddr2
	}
}

void *BloodStrFixRetAddr3;

__declspec(naked) void __stdcall BloodStrFixASM3()
{
	__asm
	{
		add     edi, 10Ch
		push    edi
		jmp		BloodStrFixRetAddr3
	}
}

void *BloodStrFixRetAddr4;

__declspec(naked) void __stdcall BloodStrFixASM4()
{
	__asm
	{
		add     edi, 10Ch
		push    edi
		jmp		BloodStrFixRetAddr4
	}
}

HRESULT PatchCustomExeStr()
{
	void *DLangAddrA = (void*)SearchAndGetAddresses(0x0040730A, 0x0040730A, 0x0040731A, LangSearchBytesA, sizeof(LangSearchBytesA), 0x00, __FUNCTION__);

	// Checking address pointer
	if (!DLangAddrA)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return E_FAIL;
	}

	langMin = (UnlockJapLang == false);
	gLangID = (BYTE *)*(DWORD *)((BYTE*)DLangAddrA - 2);

	// Check if resource files need to be created
	{
		char txtpath[MAX_PATH];
		strcpy_s(txtpath, MAX_PATH, GetModPath("data"));
		if (!PathFileExistsA(txtpath))
		{
			Logging::Log() << __FUNCTION__ << " Error: Could not find mod path!";
			return E_FAIL;
		}
		strcat_s(txtpath, MAX_PATH, "\\etc");
		if (!PathFileExistsA(txtpath) && !CreateDirectoryA(txtpath, nullptr))
		{
			Logging::Log() << __FUNCTION__ << " Error: Creating '" << txtpath << "'!";
			return E_FAIL;
		}
		strcat_s(txtpath, MAX_PATH, "\\resource");
		if (!PathFileExistsA(txtpath) && !CreateDirectoryA(txtpath, nullptr))
		{
			Logging::Log() << __FUNCTION__ << " Error: Creating '" << txtpath << "'!";
			return E_FAIL;
		}
		strcat_s(txtpath, MAX_PATH, "\\");
		for (auto item : LangList)
		{
			// Get modpath
			char txtmodname[MAX_PATH];
			strcpy_s(txtmodname, MAX_PATH, txtpath);
			strcat_s(txtmodname, MAX_PATH, item.Name);

			// Check if config file does not exist
			if (!PathFileExistsA(txtmodname))
			{
				ExtractFileFromResource(item.RcData, txtmodname);
			}
		}
	}

	int i = 0;
	for (auto item : LangList)
	{
		char txtname[MAX_PATH];
		strcpy_s(txtname, MAX_PATH, "data\\etc\\resource\\");
		strcat_s(txtname, MAX_PATH, item.Name);

		char Filename[MAX_PATH];
		ifstream file(GetFileModPath(txtname, Filename));

		if (!file.is_open())
		{
			Logging::Log() << __FUNCTION__ << " Error: Could not find text file: " << item.Name;
			return E_FAIL;
		}

		// Read each file into array
		int l = 0;
		string line;
		while (getline(file, line))
		{
			if (!line.empty() && !((*(char*)line.c_str() == '/') && (*(char*)(line.c_str() + 1) == '/')))
			{
				if (l >= STR_PER_LANG)
				{
					Logging::Log() << __FUNCTION__ << " Error: Too many lines in text file: " << item.Name;
					break;
				}
				exeStrPtr[i] = (char*)malloc(line.length() + 10);
				if (exeStrPtr[i])
				{
					strcpy_s(exeStrPtr[i], line.length() + 10, line.c_str());
				}
				i++;
				l++;
			}
		}
		file.close();

		// Add any missing lines
		if (l < STR_PER_LANG - 1)
		{
			Logging::Log() << __FUNCTION__ << " Error: Missing lines in text file: " << item.Name;
			line.assign("Blank");
			for (int x = l; x < STR_PER_LANG; x++)
			{
				exeStrPtr[i] = (char*)malloc(line.length() + 10);
				if (exeStrPtr[i])
				{
					strcpy_s(exeStrPtr[i], line.length() + 10, line.c_str());
				}
				i++;
			}
		}
	}

	if (i != (STR_PER_LANG * 6))
	{
		Logging::Log() << __FUNCTION__ << " Error: Wrong text file!";
		return E_FAIL;
	}

	Logging::Log() << "Enabling Custom Exe Strings...";

	// Pause menu
	LangsPauseRetAddr = (void *)((BYTE*)DLangAddrA + 0x0F);
	WriteJMPtoMemory((BYTE*)DLangAddrA + 6, *LangsPauseASM, 9);

	// Errors
	void *DLangAddrB = (void*)SearchAndGetAddresses(0x00407629, 0x00407629, 0x00407639, LangSearchBytesB, sizeof(LangSearchBytesB), 0x00, __FUNCTION__);
	if (DLangAddrB)
	{
		LangsErrorsRetAddr = (void *)((BYTE*)DLangAddrB + 9);
		isMultiLang = *(WORD *)((BYTE*)LangsErrorsRetAddr + 0x70) == 0xF685;
		WriteJMPtoMemory((BYTE*)DLangAddrB, *LangsErrorsASM, 9);
	}

	// Button
	BYTE codeA[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	void *DLangAddrC = CheckMultiMemoryAddress((void*)0x005AEE9F, 0x00000000, 0x00000000, (void*)LangSearchBytesC, sizeof(LangSearchBytesC), __FUNCTION__);
	if (DLangAddrC)
	{
		WriteJMPtoMemory((BYTE*)DLangAddrC + 0x10, *LangsButtonASM, 5);
		LangsButtonRetAddr = (void *)((BYTE*)DLangAddrC + 0x15);
	}
	else
	{
		void *DLangAddrD = CheckMultiMemoryAddress(0x00000000, (void*)0x005AF759, (void*)0x005AF079, (void*)LangSearchBytesD, sizeof(LangSearchBytesD), __FUNCTION__);
		if (DLangAddrD)
		{
			UpdateMemoryAddress((void *)((BYTE*)DLangAddrD + 0x0C), (void *)codeA, 9);
			WriteJMPtoMemory((BYTE*)DLangAddrD + 0x1B, *LangsButtonASM, 5);
			LangsButtonRetAddr = (void *)((BYTE*)DLangAddrD + 0x20);
		}
	}

	// Quick save
	void *DLangAddrE = CheckMultiMemoryAddress((void*)0x0044C688, 0x00000000, 0x00000000, (void*)LangSearchBytesE, sizeof(LangSearchBytesE), __FUNCTION__);
	if (DLangAddrE)
	{
		WriteJMPtoMemory((BYTE*)DLangAddrE + 0x28, *LangsGameSavedASM, 5);
		LangsGameSavedRetAddr = (void *)((BYTE*)DLangAddrE + 0x2D);
		DLangAddrE = CheckMultiMemoryAddress((void*)0x0044C6D8, 0x00000000, 0x00000000, (void*)LangSearchBytesE, sizeof(LangSearchBytesE), __FUNCTION__);
		if (DLangAddrE)
		{
			WriteJMPtoMemory((BYTE*)DLangAddrE + 0x28, *LangsCantSaveASM, 5);
			LangsCantSaveRetAddr = (void *)((BYTE*)DLangAddrE + 0x2D);
			DLangAddrE = CheckMultiMemoryAddress((void*)0x0044C728, 0x00000000, 0x00000000, (void*)LangSearchBytesE, sizeof(LangSearchBytesE), __FUNCTION__);
			if (DLangAddrE)
			{
				WriteJMPtoMemory((BYTE*)DLangAddrE + 0x28, *LangsNoQuickSaveASM, 5);
				LangsNoQuickSaveRetAddr = (void *)((BYTE*)DLangAddrE + 0x2D);
			}
		}
	}
	else
	{
		void *DLangAddrF = CheckMultiMemoryAddress(0x00000000, (void*)0x0044C827, (void*)0x0044C827, (void*)LangSearchBytesF, sizeof(LangSearchBytesF), __FUNCTION__);
		if (DLangAddrF)
		{
			UpdateMemoryAddress((void *)((BYTE*)DLangAddrF + 5), (void *)codeA, 9);
			WriteJMPtoMemory((BYTE*)DLangAddrF + 0x0E, *LangsGameSavedASM2, 5);
			LangsGameSavedRetAddr2 = (void *)((BYTE*)DLangAddrF + 0x13);
			DLangAddrF = (void *)((BYTE*)DLangAddrF + 0x90);
			UpdateMemoryAddress((void *)((BYTE*)DLangAddrF + 5), (void *)codeA, 9);
			WriteJMPtoMemory((BYTE*)DLangAddrF + 0x0E, *LangsCantSaveASM2, 5);
			LangsCantSaveRetAddr2 = (void *)((BYTE*)DLangAddrF + 0x13);
			DLangAddrF = (void *)((BYTE*)DLangAddrF + 0x90);
			UpdateMemoryAddress((void *)((BYTE*)DLangAddrF + 5), (void *)codeA, 9);
			WriteJMPtoMemory((BYTE*)DLangAddrF + 0x0E, *LangsNoQuickSaveASM2, 5);
			LangsNoQuickSaveRetAddr2 = (void *)((BYTE*)DLangAddrF + 0x13);
		}
	}

	// Unlock language selector
	void *DLangAddrG = CheckMultiMemoryAddress(0x00000000, 0x00000000, (void*)0x00463F77, (void*)LangSearchBytesG, sizeof(LangSearchBytesG), __FUNCTION__);
	if (DLangAddrG)
	{
		UpdateMemoryAddress((void *)((BYTE*)DLangAddrG + 0x19), (void *)&langMin, 1);
		UpdateMemoryAddress((void *)((BYTE*)DLangAddrG + 0x5F), (void *)&langMin, 1);
		// Fix mouse hitboxes for arrows in Game Options menu
		PosByLang = (int *)*(DWORD *)((BYTE*)DLangAddrG + 0x0702);
		MFSomeByte = (BYTE *)*(DWORD *)((BYTE*)DLangAddrG - 0x0C78);
		MFMenuYPos = (BYTE *)*(DWORD *)((BYTE*)DLangAddrG - 0x0C90);
		RetMouseB = (DWORD(*)(int a))(((BYTE*)DLangAddrG - 0x0CDE) + *(int *)((BYTE*)DLangAddrG - 0x0CE2));
		RetMouseX = (DWORD(*)(int a))(((BYTE*)DLangAddrG - 0x0CD2) + *(int *)((BYTE*)DLangAddrG - 0x0CD6));
		RetMouseY = (DWORD(*)(int a))(((BYTE*)DLangAddrG - 0x0CA9) + *(int *)((BYTE*)DLangAddrG - 0x0CAD));
		MouseFixRetAddr = (void *)((BYTE*)DLangAddrG - 0x0C74);
		WriteJMPtoMemory((BYTE*)DLangAddrG - 0x0C79, *MouseFixASM2, 5);
	}
	else
	{
		void *DLangAddrH = CheckMultiMemoryAddress((void*)0x004F74BA, (void*)0x004F77EA, 0x00000000, (void*)LangSearchBytesH, sizeof(LangSearchBytesH), __FUNCTION__);
		if (DLangAddrH)
		{
			gLangID_S = (DWORD *)*(DWORD *)((BYTE*)DLangAddrH + 0x32);
			void *DLangAddrI = CheckMultiMemoryAddress((void*)0x00463D20, 0x00000000, 0x00000000, (void*)LangSearchBytesI, sizeof(LangSearchBytesI), __FUNCTION__);
			if (DLangAddrI)
			{
				SomeWord = (WORD *)*(DWORD *)((BYTE*)DLangAddrI + 0xBF);
				SomeByte = (BYTE *)*(DWORD *)((BYTE*)DLangAddrI + 0xCB);
				SomeDataA = (DWORD *)*(DWORD *)((BYTE*)DLangAddrI + 0xFC);
				SomeDataB = (DWORD *)*(DWORD *)((BYTE*)DLangAddrI + 0xD7);
				FunctionA = (DWORD(*)(int a, int b, int c))(((BYTE*)DLangAddrI + 0xE1) + *(int *)((BYTE*)DLangAddrI + 0xDD));
				FunctionB = (DWORD(*)(int a))(((BYTE*)DLangAddrI + 0xEF) + *(int *)((BYTE*)DLangAddrI + 0xEB));
				FunctionC = (DWORD(*)(int a))(((BYTE*)DLangAddrI - 0x0548) + *(int *)((BYTE*)DLangAddrI - 0x054C));
				FunctionD = (DWORD(*)(int a, int b))(((BYTE*)DLangAddrI - 0x540) + *(int *)((BYTE*)DLangAddrI - 0x0544));
				LabelA = (DWORD *)(((BYTE*)DLangAddrI + 0xD5) + *(int *)((BYTE*)DLangAddrI + 0xD1));
				LabelB = (DWORD *)(((BYTE*)DLangAddrI + 0x03) + *(int *)((BYTE*)DLangAddrI - 0x01));
				WriteJMPtoMemory((BYTE*)DLangAddrI - 0xE8, *LangsSelectorASM, 5);
				// Fix blood setting arrows position in Game Options menu
				PosByLang = (int *)*(DWORD *)((BYTE*)DLangAddrI - 0x0456);
				BloodArrowFixRetAddr = (void *)((BYTE*)DLangAddrI + 0x095D);
				DWORD BloodArrowFixAddr = (DWORD)*BloodArrowFixASM;
				UpdateMemoryAddress((void *)((BYTE*)DLangAddrI + 0x0CD8), (void *)&BloodArrowFixAddr, 4);
				BloodArrowFixRetAddr2 = (void *)((BYTE*)DLangAddrI + 0x09C8);
				WriteJMPtoMemory((BYTE*)DLangAddrI + 0x09C3, *BloodArrowFixASM2, 5);
				// Fix mouse hitboxes for arrows in Game Options menu
				MFSomeByte = (BYTE *)*(DWORD *)((BYTE*)DLangAddrI - 0x0C9E);
				MFMenuYPos = (BYTE *)*(DWORD *)((BYTE*)DLangAddrI - 0x0CB2);
				RetMouseB = (DWORD(*)(int a))(((BYTE*)DLangAddrI - 0x0D00) + *(int *)((BYTE*)DLangAddrI - 0x0D04));
				RetMouseX = (DWORD(*)(int a))(((BYTE*)DLangAddrI - 0x0CF4) + *(int *)((BYTE*)DLangAddrI - 0x0CF8));
				RetMouseY = (DWORD(*)(int a))(((BYTE*)DLangAddrI - 0x0CCB) + *(int *)((BYTE*)DLangAddrI - 0x0CCF));
				MouseFixRetAddr = (void *)((BYTE*)DLangAddrI - 0x0C9A);
				WriteJMPtoMemory((BYTE*)DLangAddrI - 0x0C9F, *MouseFixASM, 5);
			}
			else
			{
				void *DLangAddrJ = CheckMultiMemoryAddress(0x00000000, (void*)0x00463E16, 0x00000000, (void*)LangSearchBytesJ, sizeof(LangSearchBytesJ), __FUNCTION__);
				if (DLangAddrJ)
				{
					SomeWord = (WORD *)*(DWORD *)((BYTE*)DLangAddrJ + 0x010D);
					SomeByte = (BYTE *)*(DWORD *)((BYTE*)DLangAddrJ + 0x0119);
					SomeDataA = (DWORD *)*(DWORD *)((BYTE*)DLangAddrJ + 0x0138);
					SomeDataB = (DWORD *)*(DWORD *)((BYTE*)DLangAddrJ + 0x0125);
					FunctionA = (DWORD(*)(int a, int b, int c))(((BYTE*)DLangAddrJ + 0x012F) + *(int *)((BYTE*)DLangAddrJ + 0x012B));
					FunctionB = (DWORD(*)(int a))(((BYTE*)DLangAddrJ + 0x0150) + *(int *)((BYTE*)DLangAddrJ + 0x014C));
					FunctionC = (DWORD(*)(int a))(((BYTE*)DLangAddrJ - 0x03CE) + *(int *)((BYTE*)DLangAddrJ - 0x03D2));
					FunctionD = (DWORD(*)(int a, int b))(((BYTE*)DLangAddrJ - 0x3C6) + *(int *)((BYTE*)DLangAddrJ - 0x03CA));
					LabelA = (DWORD *)(((BYTE*)DLangAddrJ + 0x9A) + *(int *)((BYTE*)DLangAddrJ + 0x96));
					LabelB = (DWORD *)(((BYTE*)DLangAddrJ + 0x0186) + *(int *)((BYTE*)DLangAddrJ + 0x0182));
					WriteJMPtoMemory((BYTE*)DLangAddrJ + 0x95, *LangsSelectorASM, 5);
					// Fix mouse points for arrows in Game Options menu
					PosByLang = (int *)*(DWORD *)((BYTE*)DLangAddrJ + 0x0660);
					MFSomeByte = (BYTE *)*(DWORD *)((BYTE*)DLangAddrJ - 0x0B24);
					MFMenuYPos = (BYTE *)*(DWORD *)((BYTE*)DLangAddrJ - 0x0B38);
					RetMouseB = (DWORD(*)(int a))(((BYTE*)DLangAddrJ - 0x0B86) + *(int *)((BYTE*)DLangAddrJ - 0x0B8A));
					RetMouseX = (DWORD(*)(int a))(((BYTE*)DLangAddrJ - 0x0B7A) + *(int *)((BYTE*)DLangAddrJ - 0x0B7E));
					RetMouseY = (DWORD(*)(int a))(((BYTE*)DLangAddrJ - 0x0B51) + *(int *)((BYTE*)DLangAddrJ - 0x0B55));
					MouseFixRetAddr = (void *)((BYTE*)DLangAddrJ - 0x0B20);
					WriteJMPtoMemory((BYTE*)DLangAddrJ - 0x0B25, *MouseFixASM, 5);
				}
			}
		}
	}

	// Fix blood setting string position in Game Options menu
	void *DLangAddrK = CheckMultiMemoryAddress((void*)0x00461FFB, 0x00000000, 0x00000000, (void*)LangSearchBytesK, sizeof(LangSearchBytesK), __FUNCTION__);
	if (DLangAddrK)
	{
		BYTE codeB[] = { 0x56, 0x90, 0x90, 0x90, 0x90 };
		UpdateMemoryAddress((void *)((BYTE*)DLangAddrK + 0x07), (void *)&codeB, 5);
		UpdateMemoryAddress((void *)((BYTE*)DLangAddrK + 0x1F), (void *)&codeB, 5);
		UpdateMemoryAddress((void *)((BYTE*)DLangAddrK + 0x38), (void *)&codeB, 5);
		UpdateMemoryAddress((void *)((BYTE*)DLangAddrK + 0x4B), (void *)&codeB, 5);
		WriteJMPtoMemory((BYTE*)DLangAddrK + 0x028D, *BloodStrFixASM, 5);
		BloodStrFixRetAddr = (void *)((BYTE*)DLangAddrK + 0x028D + 5);
		WriteJMPtoMemory((BYTE*)DLangAddrK + 0x02B1, *BloodStrFixASM2, 5);
		BloodStrFixRetAddr2 = (void *)((BYTE*)DLangAddrK + 0x02B1 + 5);
		WriteJMPtoMemory((BYTE*)DLangAddrK + 0x02D5, *BloodStrFixASM3, 5);
		BloodStrFixRetAddr3 = (void *)((BYTE*)DLangAddrK + 0x02D5 + 5);
		WriteJMPtoMemory((BYTE*)DLangAddrK + 0x02F5, *BloodStrFixASM4, 5);
		BloodStrFixRetAddr4 = (void *)((BYTE*)DLangAddrK + 0x02F5 + 5);
	}

	// Return
	return S_OK;
}

char *getResolutionDescStr()
{
	return exeStrPtr[STR_PER_LANG * (int)*gLangID + 25];
}

char* getSpeakerConfigDescStr()
{
	return exeStrPtr[STR_PER_LANG * (int)*gLangID + 26];
}

char* getMasterVolumeNameStr()
{
	return exeStrPtr[STR_PER_LANG * (int)*gLangID + 27];
}

char* getMasterVolumeDescStr()
{
	return exeStrPtr[STR_PER_LANG * (int)*gLangID + 28];
}

constexpr BYTE TownWestGateEventSearchBytes[] = { 0x00, 0x00, 0x00, 0x90, 0x00, 0xC0, 0x3F, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x6E, 0x20 };
constexpr BYTE TownWestGateEventUpdateVal[] = { 0x00, 0x00, 0x00, 0x60, 0x00, 0x80, 0x13, 0x00 };

void PatchTownWestGateEvent()
{
	void* DTownWestGateEventAddr = (void*)SearchAndGetAddresses(0x008DB440, 0x008DF110, 0x008DE110, TownWestGateEventSearchBytes, sizeof(TownWestGateEventSearchBytes), 0x00, __FUNCTION__);

	// Checking address pointer
	if (!DTownWestGateEventAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	Logging::Log() << "Enabling Town West Gate Event Fix...";
	UpdateMemoryAddress(DTownWestGateEventAddr, (void*)TownWestGateEventUpdateVal, sizeof(TownWestGateEventUpdateVal));
}

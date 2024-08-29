/**
* Copyright (C) 2024 Elisha Riedlinger
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

float CandleFPS = 20.0f;
float CandleFPSResult = 0.0f;
void* CandleFrametimePointer = nullptr;
float* atticShadowCutoffPoint = nullptr;

__declspec(naked) void __stdcall CandleFlameSpeedASM()
{
	__asm
	{
	CandleFlameSpeedHook:
		cmp dword ptr ds : [esi + 0x1C] , 0x42C80000	// 100.0f : Loop limit
		jl CandleFlameSpeedMath
		mov dword ptr ds : [esi + 0x1C] , 0x00000000	// 0.0f: Resets value to zero for infinite looping
		jmp CandleFlameSpeedHook

	CandleFlameSpeedMath:
		push eax
		mov eax, dword ptr ds : [CandleFrametimePointer]
		fld dword ptr ds : [eax]						// Frametime
		pop eax
		fmul dword ptr ds : [CandleFPS]					// Candle FPS
		fstp dword ptr ds : [CandleFPSResult]			// Result
		fld dword ptr ds : [esi + 0x1C]
		fadd dword ptr ds : [CandleFPSResult]
		fstp dword ptr ds : [esi + 0x1C]
		fld dword ptr ds : [esi + 0x1C]
		fistp dword ptr ds : [esi + 0x20]
		mov eax, dword ptr ds : [esi + 0x20]
		ret
	}
}

__declspec(naked) void __stdcall CandleFlameOpacityASM()
{
	__asm
	{
		fadd dword ptr ds : [ecx + 0x08]
		mov eax, 0x432F0000								// 175.0f : Candle Opacity
		ret
	}
}

void RunAtticShadows()
{
	if (!atticShadowCutoffPoint)
	{
		return;
	}

	static bool ValueSet = false;
	if (GetRoomID() == R_MAN_ATTIC && GetInGameCameraPosY() == -1450.0f)
	{
		ValueSet = true;
		float Value = -10.0f;
		UpdateMemoryAddress(atticShadowCutoffPoint, &Value, sizeof(float));
	}
	else if (ValueSet)
	{
		ValueSet = false;
		float Value = 10.0f;
		UpdateMemoryAddress(atticShadowCutoffPoint, &Value, sizeof(float));
	}
	else if (*atticShadowCutoffPoint != -10.0f && GetRoomID() != R_MAN_ATTIC)
	{
		float Value = -10.0f;
		UpdateMemoryAddress(atticShadowCutoffPoint, &Value, sizeof(float));
	}
}

void PatchAtticShadows()
{
	DWORD CandleFlameSpeedAddr = 
		GameVersion == SH2V_10 ? 0x004D041A :
		GameVersion == SH2V_11 ? 0x004D06CA :
		GameVersion == SH2V_DC ? 0x004CFF8A : NULL;

	DWORD CandleFlameOpacityAddr = 
		GameVersion == SH2V_10 ? 0x004D017D :
		GameVersion == SH2V_11 ? 0x004D042D :
		GameVersion == SH2V_DC ? 0x004CFCED : NULL;

	void** lplpCandleFrametimePointer = (void**)(
		GameVersion == SH2V_10 ? 0x0044787D :
		GameVersion == SH2V_11 ? 0x00447990 :
		GameVersion == SH2V_DC ? 0x00447990 : NULL);

	float** lplpAtticShadowCutoff = (float**)(
		GameVersion == SH2V_10 ? 0x0049D8D0 :
		GameVersion == SH2V_11 ? 0x0049DB80 :
		GameVersion == SH2V_DC ? 0x0049D440 : NULL);

	// Check variable addresses
	if (!CandleFlameSpeedAddr || !CandleFlameOpacityAddr || !lplpCandleFrametimePointer || !lplpAtticShadowCutoff ||
		*(BYTE*)CandleFlameSpeedAddr != 0xE8 || *(WORD*)CandleFlameOpacityAddr != 0x41D8 || *(WORD*)((CandleFlameSpeedAddr + 0x1e)) != 0x46D9 ||
		*(WORD*)((DWORD)lplpCandleFrametimePointer - 2) != 0x1DD9 || *(WORD*)((DWORD)lplpAtticShadowCutoff - 2) != 0x0DD8)
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	CandleFrametimePointer = *lplpCandleFrametimePointer;
	atticShadowCutoffPoint = *lplpAtticShadowCutoff;

	// Update SH2 code
	Logging::Log() << "Enabling Attic Shadow Fix...";
	WriteCalltoMemory((BYTE*)CandleFlameSpeedAddr, CandleFlameSpeedASM);
	WriteCalltoMemory((BYTE*)CandleFlameOpacityAddr, CandleFlameOpacityASM, 0x6);
	BYTE NopData[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	UpdateMemoryAddress((void*)(CandleFlameSpeedAddr + 0x1e), &NopData, 3);
	UpdateMemoryAddress((void*)(CandleFlameSpeedAddr + 0x23), &NopData, 6);
	UpdateMemoryAddress((void*)(CandleFlameSpeedAddr + 0x2a), &NopData, 3);
}

/**
* Copyright (C) 2024 mercury501
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

#include "Common\Utils.h"
#include "Patches\Patches.h"

struct RPTVector4 {
	float x, y, z, w;
};

RPTVector4 FirstRPTVec = { 0.f, -1.f, 0.f, 1.f };
RPTVector4 SecondRPTVec = { 0.f, 2.9f, 0.f, 1.f };

BYTE* FirstRPTJmpAddr = nullptr;
BYTE* SecondRPTJmpAddr = nullptr;

BYTE* FirstRPTReturnAddr = nullptr;
BYTE* SecondRPTReturnAddr = nullptr;

__declspec(naked) void __stdcall InjectSecondRPTVec()
{
	__asm
	{		
		push offset SecondRPTVec

		jmp SecondRPTReturnAddr
	}
}

__declspec(naked) void __stdcall InjectFirstRPTVec()
{
	__asm
	{
		push offset FirstRPTVec

		jmp FirstRPTReturnAddr
	}
}

void PatchPhRotationAfterBossFight()
{
	Logging::Log() << "Patching PH Rotation After Boss Fight";

	if (GameVersion == SH2V_UNKNOWN)
	{
		Logging::Log() << __FUNCTION__ << " Couldn't determine game version.";
		return;
	}

	FirstRPTJmpAddr = GameVersion == SH2V_10 ?
		(BYTE*)0x005760C1 :
		GameVersion == SH2V_11 ?
		(BYTE*)0x00576937 :
		(BYTE*)0x00576257;

	SecondRPTJmpAddr = GameVersion == SH2V_10 ?
		(BYTE*)0x00576087 :
		GameVersion == SH2V_11 ?
		(BYTE*)0x00576971 :
		(BYTE*)0x00576291;

	FirstRPTReturnAddr = FirstRPTJmpAddr + 0x05;
	SecondRPTReturnAddr = SecondRPTJmpAddr + 0x05;

	WriteJMPtoMemory(FirstRPTJmpAddr, InjectFirstRPTVec, 5);
	WriteJMPtoMemory(SecondRPTJmpAddr, InjectSecondRPTVec, 5);
}
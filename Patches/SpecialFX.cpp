/**
* Copyright (C) 2019 Elisha Riedlinger
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

// Variables for ASM
constexpr float CustomAddress1 = 0.04f;
constexpr float PointThreeBarValue = 0.3333333333f;
constexpr float PointSixBarValue = 0.6666666666f;
constexpr float IntroCutsceneValue1 = -3.0f;
constexpr float IntroCutsceneValue2 = -1.5f;
constexpr float PyramidHeadCutsceneValue1 = 1.5f;
constexpr float PyramidHeadCutsceneValue2 = -1.5f;
constexpr float LyingFigureTunnelCutsceneValue1 = -3.0f;
constexpr float LyingFigureTunnelCutsceneValue2 = -1.0f;
constexpr float HotelRoom312Value1 = -8.0f;
constexpr float HotelRoom312Value2 = -4.0f;
constexpr float MotionBlurValue = 0.25f;
void *jmpCustomAddress2Addr;
void *jmpCustomAddress3Addr;
void *jmpMotionBlurAddr;
void *MotionBlurDWORDAddr;
void *callCustomMotionBlurAddr;
void *jmpCustomMotionBlur1Addr;
void *jmpCustomMotionBlur2Addr;
void *jmpCustomMotionBlur3Addr;
void *jmpCustomMotionBlur4Addr;
void *jmpCustomMotionBlur5Addr;
void *jmpCustomMotionBlur6Addr;
void *jmpEddieBossDeathAddr;
void *jmpEddieBossDeathTimerAddr;
void *BloomColorRed;
void *BloomColorGreen;
void *BloomColorBlue;
void *jmpBloomCustomColorAddr;
void *jmpBloomDefaultColorAddr;

// Second custom addresses ASM (00631614)
__declspec(naked) void __stdcall CustomAddress2ASM()
{
	__asm
	{
		push edx
		mov edx, dword ptr ds : [CutsceneIDAddr]	// moves cutscene ID pointer to edx
		cmp dword ptr ds : [edx], 0x01				// IntroCutscene
		je NEAR IntroCutscene
		cmp dword ptr ds : [edx], 0x0E				// PyramidHeadCutscene
		je NEAR PyramidHeadCutscene
		cmp dword ptr ds : [edx], 0x06				// LyingFigureTunnelCutscene
		je NEAR LyingFigureTunnelCutscene
		mov edx, dword ptr ds : [RoomIDAddr]		// moves room ID pointer to edx
		cmp dword ptr ds : [edx], 0xA2				// Room ID; Hotel Room 312
		je NEAR HotelRoom312
		fld PointThreeBarValue						// 0.3333333333 float
		jmp NEAR ExitASM

	IntroCutscene:
		fld IntroCutsceneValue1
		jmp NEAR ExitASM

	PyramidHeadCutscene:
		fld PyramidHeadCutsceneValue1
		jmp NEAR ExitASM

	LyingFigureTunnelCutscene:
		fld LyingFigureTunnelCutsceneValue1
		jmp NEAR ExitASM

	HotelRoom312:
		fld HotelRoom312Value1

	ExitASM:
		pop edx
		jmp jmpCustomAddress2Addr
	}
}

// Third custom addresses ASM (00633460)
__declspec(naked) void __stdcall CustomAddress3ASM()
{
	__asm
	{
		push edx
		mov edx, dword ptr ds : [CutsceneIDAddr]	// moves cutscene ID pointer to edx
		cmp dword ptr ds : [edx], 0x01				// IntroCutscene
		je NEAR IntroCutscene
		cmp dword ptr ds : [edx], 0x0E				// PyramidHeadCutscene
		je NEAR PyramidHeadCutscene
		cmp dword ptr ds : [edx], 0x06				// LyingFigureTunnelCutscene
		je NEAR LyingFigureTunnelCutscene
		mov edx, dword ptr ds : [RoomIDAddr]		// moves room ID pointer to edx
		cmp dword ptr ds : [edx], 0xA2				// Room ID; Hotel Room 312
		je NEAR HotelRoom312
		fmul PointSixBarValue						// 0.6666666666 float
		jmp NEAR ExitASM

	IntroCutscene:
		fmul IntroCutsceneValue2
		jmp NEAR ExitASM

	PyramidHeadCutscene:
		fmul PyramidHeadCutsceneValue2
		jmp NEAR ExitASM

	LyingFigureTunnelCutscene:
		fmul LyingFigureTunnelCutsceneValue2
		jmp NEAR ExitASM

	HotelRoom312:
		fmul HotelRoom312Value2

	ExitASM:
		pop edx
		test eax, eax
		jmp jmpCustomAddress3Addr
	}
}

// Universal Motion Blur Intensity ASM
__declspec(naked) void __stdcall MotionBlurASM()
{
	__asm
	{
		fild dword ptr[esp + 0x04]
		fmul MotionBlurValue					// 0.25 float; lower value = more motion blur
		fistp dword ptr[esp + 0x04]
		mov al, byte ptr[esp + 0x04]
		push edx
		mov edx, dword ptr ds : [MotionBlurDWORDAddr]
		mov dword ptr ds : [edx], 0x00000000
		pop edx
		jmp jmpMotionBlurAddr
	}
}

// Maria/Mary Boss Transformation and Death and Hospital Otherworld Motion Blur Correction ASMs
#define DECLARE_MARY_BOSS_ASM(num, val) \
	__declspec(naked) void __stdcall MaryBossMotion ## num ## ASM() \
	{ \
		__asm { push val } \
		__asm { call callCustomMotionBlurAddr } \
		__asm { jmp jmpCustomMotionBlur ## num ## Addr } \
	}

#define VISIT_MARY_BOSS_ASM(visit) \
	visit(1, 0x00000100) \
	visit(2, 0x00000100) \
	visit(3, 0x00000100) \
	visit(4, 0x00000100) \
	visit(5, 0x00000078) \
	visit(6, 0x00000080)

VISIT_MARY_BOSS_ASM(DECLARE_MARY_BOSS_ASM);

// Eddie Boss Death Sequence ASM
__declspec(naked) void __stdcall EddieBossDeathASM()
{
	__asm {
		mov al, [esi + 0x03]
		cmp al, 0x28				// motion blur timer
		jg NEAR MotionTimer
		push 0x05					// animation delay
		push esi
		jmp jmpEddieBossDeathAddr
	MotionTimer:
		jmp jmpEddieBossDeathTimerAddr
	}
}

// Eddie Boss Death Sequence ASM
__declspec(naked) void __stdcall BloomColorASM()
{
	__asm {
		push edx
		mov edx, dword ptr ds : [RoomIDAddr]		// moves room ID pointer to edx
		cmp dword ptr ds : [edx], 0xB5				// Hotel 3rd Floor Hallway
		je NEAR CustomBloomColor					// jumps to CustomBloomColor
		cmp dword ptr ds : [edx], 0xA2				// Hotel Room 312
		je NEAR CustomBloomColor					// jumps to CustomBloomColor
		jmp NEAR DefaultBloomColor					// jumps to DefaultBloomColor

	CustomBloomColor:
		cmp bl, 0x00
		je NEAR DefaultBloomColor					// jumps to DefaultBloomColor
		mov edx, dword ptr ds : [BloomColorRed]
		mov byte ptr ds : [edx], 0xFF				// 255 Red
		mov edx, dword ptr ds : [BloomColorGreen]
		mov byte ptr ds : [edx], 0xFF				// 255 Green
		mov edx, dword ptr ds : [BloomColorBlue]
		mov byte ptr ds : [edx], 0xE3				// 227 Blue
		pop edx
		jmp jmpBloomCustomColorAddr

	DefaultBloomColor:
		mov edx, dword ptr ds : [BloomColorRed]
		mov byte ptr ds : [edx], bl
		pop edx
		jmp jmpBloomDefaultColorAddr
	}
}


// Update SH2 code to reenable special FX
void UpdateSpecialFX()
{
	// Get first custom address set (006A1488)
	DWORD CustomAddr1 = 0x004795E5;
	DWORD CustomAddr2 = CustomAddr1 + 0x0E;

	// Get second custom address set (00631614)
	DWORD CustomAddress2Ptr = 0x0047775D;
	jmpCustomAddress2Addr = (void*)(CustomAddress2Ptr + 6);

	// Get third custom address set (00633460)
	DWORD CustomAddress3Ptr = CustomAddress2Ptr + 0x1F;
	jmpCustomAddress3Addr = (void*)(CustomAddress3Ptr + 6);

	// Get Universal Motion Blur Intensity address
	DWORD MotionBlurPtr = 0x004782AE;
	jmpMotionBlurAddr = (void*)(MotionBlurPtr + 0x0E);
	memcpy(&MotionBlurDWORDAddr, (void*)(MotionBlurPtr + 6), sizeof(DWORD));

	// Get Maria/Mary Boss Transformation and Death and Hospital Otherworld Motion Blur Correction addresses
	DWORD CustomMotionBlur1Ptr = 0x0043FDB4;
	jmpCustomMotionBlur1Addr = (void*)(CustomMotionBlur1Ptr + 7);
	DWORD CustomMotionBlur2Ptr = 0x00440B5C;
	jmpCustomMotionBlur2Addr = (void*)(CustomMotionBlur2Ptr + 7);
	DWORD CustomMotionBlur3Ptr = 0x0043EA44;
	jmpCustomMotionBlur3Addr = (void*)(CustomMotionBlur3Ptr + 7);
	DWORD CustomMotionBlur4Ptr = 0x005747D9;
	jmpCustomMotionBlur4Addr = (void*)(CustomMotionBlur4Ptr + 7);
	DWORD CustomMotionBlur5Ptr = 0x004A8219;
	jmpCustomMotionBlur5Addr = (void*)(CustomMotionBlur5Ptr + 7);
	DWORD CustomMotionBlur6Ptr = 0x0058B9BE;
	jmpCustomMotionBlur6Addr = (void*)(CustomMotionBlur6Ptr + 7);
	memcpy(&callCustomMotionBlurAddr, (void*)(CustomMotionBlur2Ptr + 3), sizeof(DWORD));
	callCustomMotionBlurAddr = (void*)(CustomMotionBlur2Ptr + 7 + (DWORD)callCustomMotionBlurAddr);

	// Get Maria Behind Jail Cell Motion Blur Correction addresses
	DWORD MariaBehindJailCell1Ptr = 0x008D1758;
	DWORD MariaBehindJailCell2Ptr = MariaBehindJailCell1Ptr + 0x0C;
	DWORD MariaBehindJailCell3Ptr = MariaBehindJailCell2Ptr + 0x04;

	// Get Angela Abstract Daddy Motion Blur Correction address
	DWORD AngelaAbstractDaddy1Ptr = 0x008D0610;
	DWORD AngelaAbstractDaddy2Ptr = AngelaAbstractDaddy1Ptr + 0x08;
	DWORD AngelaAbstractDaddy3Ptr = AngelaAbstractDaddy2Ptr + 0x04;
	DWORD AngelaAbstractDaddy4Ptr = AngelaAbstractDaddy3Ptr + 0x0C;
	DWORD AngelaAbstractDaddy5Ptr = AngelaAbstractDaddy4Ptr + 0x04;
	DWORD AngelaAbstractDaddy6Ptr = AngelaAbstractDaddy5Ptr + 0x0C;
	DWORD AngelaAbstractDaddy7Ptr = AngelaAbstractDaddy6Ptr + 0x04;
	DWORD AngelaAbstractDaddy8Ptr = AngelaAbstractDaddy7Ptr + 0x04;

	// Get Eddie Boss Death Sequence address
	DWORD EddieBossDeathPtr = 0x004B0A2A;
	jmpEddieBossDeathAddr = (void*)(EddieBossDeathPtr + 0x05);
	jmpEddieBossDeathTimerAddr = (void*)(EddieBossDeathPtr + 0x29);

	// Get Bloom Color address
	DWORD BloomColorPtr = 0x0047C122;
	jmpBloomCustomColorAddr = (void*)(BloomColorPtr + 0x12);
	jmpBloomDefaultColorAddr = (void*)(BloomColorPtr + 0x06);
	memcpy(&BloomColorRed, (void*)(BloomColorPtr + 2), sizeof(DWORD));
	BloomColorGreen = (void*)((DWORD)BloomColorRed + 1);
	BloomColorBlue = (void*)((DWORD)BloomColorRed + 2);

	// Get room ID address
	RoomIDAddr = GetRoomIDPointer();

	// Get cutscene ID address
	CutsceneIDAddr = GetCutsceneIDPointer();

	// Checking address pointer
	if (!RoomIDAddr || !CutsceneIDAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get room ID or cutscene ID address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Enabling post Processing Special FX...";

	// Write first custom address set
	void *ValueAddr = (void*)&CustomAddress1;
	UpdateMemoryAddress((void*)CustomAddr1, &ValueAddr, sizeof(float));
	UpdateMemoryAddress((void*)CustomAddr2, &ValueAddr, sizeof(float));

	// Write second custom address set
	WriteJMPtoMemory((BYTE*)CustomAddress2Ptr, *CustomAddress2ASM, 6);

	// Write third custom address set
	WriteJMPtoMemory((BYTE*)CustomAddress3Ptr, *CustomAddress3ASM, 6);

	// Write Universal Motion Blur Intensity address
	WriteJMPtoMemory((BYTE*)MotionBlurPtr, *MotionBlurASM, 0x0E);

	// Write Maria/Mary Boss Transformation and Death and Hospital Otherworld Motion Blur Correction addresses
	WriteJMPtoMemory((BYTE*)CustomMotionBlur1Ptr, *MaryBossMotion1ASM, 7);
	WriteJMPtoMemory((BYTE*)CustomMotionBlur2Ptr, *MaryBossMotion2ASM, 7);
	WriteJMPtoMemory((BYTE*)CustomMotionBlur3Ptr, *MaryBossMotion3ASM, 7);
	WriteJMPtoMemory((BYTE*)CustomMotionBlur4Ptr, *MaryBossMotion4ASM, 7);
	WriteJMPtoMemory((BYTE*)CustomMotionBlur5Ptr, *MaryBossMotion5ASM, 7);
	WriteJMPtoMemory((BYTE*)CustomMotionBlur6Ptr, *MaryBossMotion6ASM, 7);

	// Write Maria Behind Jail Cell Motion Blur Correction addresses
	float Value = 80.0f;
	UpdateMemoryAddress((void*)MariaBehindJailCell1Ptr, &Value, sizeof(float));
	Value = 100.0f;
	UpdateMemoryAddress((void*)MariaBehindJailCell2Ptr, &Value, sizeof(float));
	Value = 400.0f;
	UpdateMemoryAddress((void*)MariaBehindJailCell3Ptr, &Value, sizeof(float));

	// Write Angela Abstract Daddy Motion Blur Correction address
	Value = 400.0f;
	UpdateMemoryAddress((void*)AngelaAbstractDaddy1Ptr, &Value, sizeof(float));
	Value = 30.0f;
	UpdateMemoryAddress((void*)AngelaAbstractDaddy2Ptr, &Value, sizeof(float));
	Value = 400.0f;
	UpdateMemoryAddress((void*)AngelaAbstractDaddy3Ptr, &Value, sizeof(float));
	Value = 90.0f;
	UpdateMemoryAddress((void*)AngelaAbstractDaddy4Ptr, &Value, sizeof(float));
	Value = 180.0f;
	UpdateMemoryAddress((void*)AngelaAbstractDaddy5Ptr, &Value, sizeof(float));
	Value = 90.0f;
	UpdateMemoryAddress((void*)AngelaAbstractDaddy6Ptr, &Value, sizeof(float));
	Value = 400.0f;
	UpdateMemoryAddress((void*)AngelaAbstractDaddy7Ptr, &Value, sizeof(float));
	Value = 160.0f;
	UpdateMemoryAddress((void*)AngelaAbstractDaddy8Ptr, &Value, sizeof(float));

	// Write Eddie Boss Death Sequence address
	WriteJMPtoMemory((BYTE*)EddieBossDeathPtr, *EddieBossDeathASM, 5);

	// Write Bloom Color address
	WriteJMPtoMemory((BYTE*)BloomColorPtr, *BloomColorASM, 6);
}

void UpdateHotelRoom312FogVolumeFix(DWORD *SH2_RoomID)
{
	// Get Fog Volume Intensity address
	static BYTE *Address1 = nullptr;
	if (!Address1)
	{
		static bool RunOnce = false;
		if (RunOnce)
		{
			return;
		}
		RunOnce = true;

		// Get address for blood position
		constexpr BYTE SearchBytes[]{ 0xD9, 0x5E, 0x04, 0x89, 0x5E, 0x30, 0x89, 0x5E, 0x34, 0x89, 0x5E, 0x38, 0x89, 0x5E, 0x3C, 0xC7, 0x46, 0x38 };
		Address1 = (BYTE*)ReadSearchedAddresses(0x00485907, 0x00485BA7, 0x00485DB7, SearchBytes, sizeof(SearchBytes), 0x29);
		if (!Address1)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
	}

	if (*SH2_RoomID == 0xA2 && *Address1 != 0)
	{
		*Address1 = 0;
	}
}

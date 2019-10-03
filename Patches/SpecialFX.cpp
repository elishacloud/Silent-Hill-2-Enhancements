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
constexpr float CustomAddress1Value = 0.04f;
constexpr float PointThreeBarValue = 0.3333333333f;
constexpr float PointSixBarValue = 0.6666666666f;
constexpr float MotionBlurValue = 0.25f;
float IntroCutsceneValue1 = 0;
float IntroCutsceneValue2 = 0;
float PyramidHeadCutsceneValue1 = 0;
float PyramidHeadCutsceneValue2 = 0;
float LyingFigureTunnelCutsceneValue1 = 0;
float LyingFigureTunnelCutsceneValue2 = 0;
float HotelRoom312Value1 = 0;
float HotelRoom312Value2 = 0;
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
	constexpr BYTE SearchBytesCustomAddr1[]{ 0x89, 0x4C, 0x24, 0x08, 0x8B, 0x08, 0x50, 0xDB, 0x44, 0x24, 0x0C, 0x89, 0x54, 0x24, 0x0C, 0xD8, 0x0D };
	DWORD CustomAddr1 = SearchAndGetAddresses(0x004795D4, 0x00479874, 0x00479A84, SearchBytesCustomAddr1, sizeof(SearchBytesCustomAddr1), 0x11);
	DWORD CustomAddr2 = CustomAddr1 + 0x0E;

	// Get second custom address set (00631614)
	constexpr BYTE SearchBytesCustomAddr2[]{ 0x8B, 0xCF, 0xD9, 0xE0, 0xBB, 0x01, 0x00, 0x00, 0x00, 0xD9, 0x44, 0x24, 0x58, 0xD3, 0xE3, 0xDC, 0xC0, 0xA1 };
	DWORD CustomAddress2Ptr = SearchAndGetAddresses(0x00477747, 0x004779E7, 0x00477BF7, SearchBytesCustomAddr2, sizeof(SearchBytesCustomAddr2), 0x16);
	jmpCustomAddress2Addr = (void*)(CustomAddress2Ptr + 6);

	// Get third custom address set (00633460)
	DWORD CustomAddress3Ptr = CustomAddress2Ptr + 0x1F;
	jmpCustomAddress3Addr = (void*)(CustomAddress3Ptr + 6);

	// Check addresses
	if (!CustomAddr1 || !CustomAddress2Ptr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get Universal Motion Blur Intensity address
	constexpr BYTE SearchBytesMotionBlur[]{ 0x8A, 0x4C, 0x24, 0x08, 0xB8, 0x03, 0x00, 0x00, 0x00, 0xA3 };
	DWORD MotionBlurPtr = SearchAndGetAddresses(0x004782E0, 0x00478580, 0x00478790, SearchBytesMotionBlur, sizeof(SearchBytesMotionBlur), -0x32);
	if (!MotionBlurPtr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpMotionBlurAddr = (void*)(MotionBlurPtr + 0x0E);
	memcpy(&MotionBlurDWORDAddr, (void*)(MotionBlurPtr + 6), sizeof(DWORD));

	// Get Maria/Mary Boss Transformation and Death and Hospital Otherworld Motion Blur Correction addresses
	constexpr BYTE SearchBytesMotionBlur1Ptr[]{ 0x8B, 0xF0, 0x83, 0xC4, 0x14, 0xDF, 0xE0, 0xF6, 0xC4, 0x01, 0x0F, 0x85 };
	DWORD CustomMotionBlur1Ptr = SearchAndGetAddresses(0x0043FD9C, 0x0043FF5C, 0x0043FF5C, SearchBytesMotionBlur1Ptr, sizeof(SearchBytesMotionBlur1Ptr), 0x18);
	jmpCustomMotionBlur1Addr = (void*)(CustomMotionBlur1Ptr + 7);
	DWORD CustomMotionBlur2Ptr = SearchAndGetAddresses(0x00440B40, 0x00440D00, 0x00440D00, SearchBytesMotionBlur1Ptr, sizeof(SearchBytesMotionBlur1Ptr), 0x1C);
	jmpCustomMotionBlur2Addr = (void*)(CustomMotionBlur2Ptr + 7);
	DWORD CustomMotionBlur3Ptr = SearchAndGetAddresses(0x0043EA2C, 0x0043EBEC, 0x0043EBEC, SearchBytesMotionBlur1Ptr, sizeof(SearchBytesMotionBlur1Ptr), 0x18);
	jmpCustomMotionBlur3Addr = (void*)(CustomMotionBlur3Ptr + 7);
	DWORD CustomMotionBlur4Ptr = SearchAndGetAddresses(0x005747C1, 0x00575071, 0x00574991, SearchBytesMotionBlur1Ptr, sizeof(SearchBytesMotionBlur1Ptr), 0x18);
	jmpCustomMotionBlur4Addr = (void*)(CustomMotionBlur4Ptr + 7);
	constexpr BYTE SearchBytesMotionBlur5Ptr[]{ 0x00, 0x85, 0xC0, 0x74, 0x0B };
	DWORD CustomMotionBlur5Ptr = SearchAndGetAddresses(0x004A8214, 0x004A84C4, 0x004A7D84, SearchBytesMotionBlur5Ptr, sizeof(SearchBytesMotionBlur5Ptr), 0x05);
	jmpCustomMotionBlur5Addr = (void*)(CustomMotionBlur5Ptr + 7);
	constexpr BYTE SearchBytesMotionBlur6Ptr[]{ 0x00, 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x75, 0x0A, 0x81, 0x0D };
	DWORD CustomMotionBlur6Ptr = SearchAndGetAddresses(0x0058B9DA, 0x0058C28A, 0x0058BBAA, SearchBytesMotionBlur6Ptr, sizeof(SearchBytesMotionBlur6Ptr), -0x1C);
	jmpCustomMotionBlur6Addr = (void*)(CustomMotionBlur6Ptr + 7);
	if (!CustomMotionBlur1Ptr || !CustomMotionBlur2Ptr || !CustomMotionBlur3Ptr || !CustomMotionBlur4Ptr || !CustomMotionBlur5Ptr || !CustomMotionBlur6Ptr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	memcpy(&callCustomMotionBlurAddr, (void*)(CustomMotionBlur2Ptr + 3), sizeof(DWORD));
	callCustomMotionBlurAddr = (void*)(CustomMotionBlur2Ptr + 7 + (DWORD)callCustomMotionBlurAddr);

	// Get Maria Behind Jail Cell Motion Blur Correction addresses
	constexpr BYTE SearchBytesMariaBehindJail[]{ 0x00, 0xDF, 0xE0, 0xF6, 0xC4, 0x05, 0x7A, 0x15, 0xD9, 0x05 };
	DWORD MariaBehindJailCell1Ptr = ReadSearchedAddresses(0x0058349E, 0x00583D4E, 0x0058366E, SearchBytesMariaBehindJail, sizeof(SearchBytesMariaBehindJail), 0x0A);
	DWORD MariaBehindJailCell2Ptr = MariaBehindJailCell1Ptr + 0x0C;
	DWORD MariaBehindJailCell3Ptr = MariaBehindJailCell2Ptr + 0x04;
	if (!MariaBehindJailCell1Ptr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get Angela Abstract Daddy Motion Blur Correction address
	constexpr BYTE SearchBytesAngelaAbstractDaddy[]{ 0x00, 0xDF, 0xE0, 0xF6, 0xC4, 0x05, 0x7A, 0x50, 0xD9, 0x05 };
	DWORD AngelaAbstractDaddy1Ptr = ReadSearchedAddresses(0x00581672, 0x00581F22, 0x00581842, SearchBytesAngelaAbstractDaddy, sizeof(SearchBytesAngelaAbstractDaddy), -0x2C);
	DWORD AngelaAbstractDaddy2Ptr = AngelaAbstractDaddy1Ptr + 0x08;
	DWORD AngelaAbstractDaddy3Ptr = AngelaAbstractDaddy2Ptr + 0x04;
	DWORD AngelaAbstractDaddy4Ptr = AngelaAbstractDaddy3Ptr + 0x0C;
	DWORD AngelaAbstractDaddy5Ptr = AngelaAbstractDaddy4Ptr + 0x04;
	DWORD AngelaAbstractDaddy6Ptr = AngelaAbstractDaddy5Ptr + 0x0C;
	DWORD AngelaAbstractDaddy7Ptr = AngelaAbstractDaddy6Ptr + 0x04;
	DWORD AngelaAbstractDaddy8Ptr = AngelaAbstractDaddy7Ptr + 0x04;
	if (!AngelaAbstractDaddy1Ptr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get Eddie Boss Death Sequence address
	constexpr BYTE SearchBytesEddieBossDeath[]{ 0x8B, 0x4E, 0x10, 0x8A, 0x46, 0x03, 0x83, 0xC9, 0x08, 0xFE, 0xC0, 0x89, 0x4E, 0x10, 0x88, 0x46, 0x03, 0x56 };
	DWORD EddieBossDeathPtr = SearchAndGetAddresses(0x004B0A42, 0x004B0CF2, 0x004B05B2, SearchBytesEddieBossDeath, sizeof(SearchBytesEddieBossDeath), -0x18);
	jmpEddieBossDeathAddr = (void*)(EddieBossDeathPtr + 0x05);
	jmpEddieBossDeathTimerAddr = (void*)(EddieBossDeathPtr + 0x29);
	if (!EddieBossDeathPtr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get Bloom Color address
	constexpr BYTE SearchBytesBloomColor[]{ 0x00, 0x8A, 0x4C, 0x24, 0x1C, 0x8A, 0x54, 0x24, 0x20, 0x88, 0x1D };
	DWORD BloomColorPtr = SearchAndGetAddresses(0x0047C107, 0x0047C3A7, 0x0047C5B7, SearchBytesBloomColor, sizeof(SearchBytesBloomColor), 0x1B);
	if (!BloomColorPtr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
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
	void *ValueAddr = (void*)&CustomAddress1Value;
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

void UpdateSpecialFXScale(DWORD Height)
{
	static DWORD LastHeight = 0;
	if (LastHeight == Height)
	{
		return;
	}
	LastHeight = Height;

	float DisplayRatio = (float)LastHeight / 1080.0f;

	IntroCutsceneValue1 = -3.0f * DisplayRatio;
	IntroCutsceneValue2 = -1.5f * DisplayRatio;
	PyramidHeadCutsceneValue1 = 1.5f * DisplayRatio;
	PyramidHeadCutsceneValue2 = -1.5f * DisplayRatio;
	LyingFigureTunnelCutsceneValue1 = -3.0f * DisplayRatio;
	LyingFigureTunnelCutsceneValue2 = -1.0f * DisplayRatio;
	HotelRoom312Value1 = -8.0f * DisplayRatio;
	HotelRoom312Value2 = -4.0f * DisplayRatio;
}

void UpdateHotelRoom312FogVolumeFix(DWORD *SH2_RoomID)
{
	// Get Fog Volume Intensity address
	static BYTE *Address1 = nullptr;
	if (!Address1)
	{
		RUNONCE();

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

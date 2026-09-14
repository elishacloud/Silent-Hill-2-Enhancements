/**
* Copyright (C) 2026 Murugo
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
#include <algorithm>
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

namespace {
    constexpr int kEnteredFloodedHotelBasementGameFlag = 0x1F6;

    // Variables for ASM
    static float WaterSpeedFactor = 0.65f;

    __declspec(naked) void __stdcall SetWaterMoveSpeedASM()
    {
        __asm
        {
            fmul dword ptr ds : [WaterSpeedFactor]
            ret
        }
    }
}

// Uses interpolated speed multipler to adjust James' speed in water
void PatchWaterMoveSpeed()
{
    constexpr BYTE SearchBytes2D[]{ 0x8B, 0x86, 0xB0, 0x00, 0x00, 0x00, 0x89, 0x46, 0x40, 0x83, 0xC4, 0x04 };
    BYTE* SetWaterMoveSpeed2DAddr = (BYTE*)SearchAndGetAddresses(0x0054923E, 0x0054956E, 0x00548E8E, SearchBytes2D, sizeof(SearchBytes2D), -0x26C, __FUNCTION__);

    constexpr BYTE SearchBytes3D[]{ 0xD8, 0x8E, 0xAC, 0x00, 0x00, 0x00, 0x8B, 0x8E, 0xB0, 0x00, 0x00, 0x00 };
    BYTE* SetWaterMoveSpeed3DAddr = (BYTE*)SearchAndGetAddresses(0x0054F3B0, 0x0054F6E0, 0x0054F000, SearchBytes3D, sizeof(SearchBytes3D), -0xBE, __FUNCTION__);

    if (!SetWaterMoveSpeed2DAddr || !SetWaterMoveSpeed3DAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }

    Logging::Log() << "Patching Water Move Speed Fix...";
    WriteCalltoMemory(SetWaterMoveSpeed2DAddr, *SetWaterMoveSpeedASM, 0x06);
    WriteCalltoMemory(SetWaterMoveSpeed3DAddr, *SetWaterMoveSpeedASM, 0x06);
}

// Fixes James' movement speed in certain flooded rooms
void RunWaterMoveSpeed()
{
    static BYTE* PlayerInWaterPtr = nullptr;
    if (!PlayerInWaterPtr)
    {
        RUNONCE();

        constexpr BYTE SearchBytes[]{ 0x83, 0xC8, 0xFF, 0x83, 0xF8, 0xFF, 0x0F, 0x95, 0xC1, 0xC6, 0x05 };
        PlayerInWaterPtr = (BYTE*)ReadSearchedAddresses(0x005469ED, 0x00546D1D, 0x0054663D, SearchBytes, sizeof(SearchBytes), 0x0B, __FUNCTION__);
        if (!PlayerInWaterPtr)
        {
            Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
            return;
        }
    }

    const DWORD RoomID = GetRoomID();
    if (GetRoomID() == R_HTL_ALT_EMPLOYEE_STAIRS)
    {
        const float JamesPosY = GetJamesPosY();
        *PlayerInWaterPtr = JamesPosY > -300.0f ? 1 : 0;
        WaterSpeedFactor = std::clamp(1.0f - (JamesPosY + 300.0f) / 250.0f * 0.35f, 0.65f, 1.0f);
    }
    else if (RoomID == R_STRANGE_AREA_2_B)
    {
        const float JamesPosY = GetJamesPosY();
        *PlayerInWaterPtr = JamesPosY > -250.0f ? 1 : 0;
        WaterSpeedFactor = std::clamp(1.0f - (JamesPosY + 250.0f) / 200.0f * 0.35f, 0.65f, 1.0f);
    }
    else
    {
        WaterSpeedFactor = 0.65f;
        if (RoomID == R_HTL_ALT_ELEVATOR && CheckGameFlag(kEnteredFloodedHotelBasementGameFlag))
        {
            *PlayerInWaterPtr = 1;
        }
    }
}

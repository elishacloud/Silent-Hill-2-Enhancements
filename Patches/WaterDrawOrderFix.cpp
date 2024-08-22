/**
* Copyright (C) 2024 Murugo
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
#include <unordered_set>
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Variables for ASM
constexpr float kWaterDepthFactor = 0.5f;
constexpr float kUnderwaterEffectDepthFactor = 0.25f;
constexpr float kUnderwaterEffectDepthFactor2 = 0.75f;

DWORD DrawDistanceAddr = 0;

bool CurrentRoomHasWater()
{
    // Exclude R_FOREST_CEMETERY, R_TOWN_LAKE, and R_FINAL_BOSS_RM since the player does not interact
    // directly with the water in these rooms.
    static const std::unordered_set<DWORD> rooms_with_water = {
        R_APT_W_STAIRCASE_N,
        R_STRANGE_AREA_2_B,
        R_LAB_BOTTOM_H,
        R_LAB_BOTTOM_G,
        R_LAB_BOTTOM_F,
        R_LAB_BOTTOM_E,
        R_LAB_BOTTOM_C,
        R_LAB_BOTTOM_I,
        R_HTL_ALT_EMPLOYEE_STAIRS,
        R_HTL_ALT_BAR,
        R_HTL_ALT_BAR_KITCHEN,
        R_HTL_ALT_ELEVATOR,
        R_HTL_ALT_EMPLOYEE_HALL_BF,
    };
    return rooms_with_water.find(GetRoomID()) != rooms_with_water.end();
}

// Prevents blood drop effects from appearing in rooms with water.
__declspec(naked) void __stdcall InitBloodDropEffectASM()
{
    __asm
    {
        mov word ptr[esi + 0x08], 1  // Particle state = 1 (initialized)
        call CurrentRoomHasWater
        test al, al
        jz ExitASM
        inc word ptr[esi + 0x08]  // Particle state = 2 (destroy)

    ExitASM:
        mov eax, edi
        ret
    }
}

// Increases the calculated draw distance of water objects for depth sorting.
__declspec(naked) void __stdcall AdjustWaterDepthASM()
{
    __asm
    {
        mov esi, dword ptr ds : [DrawDistanceAddr]
        fadd dword ptr ds : [esi]
        fmul dword ptr ds : [kWaterDepthFactor]  // Depth = (Depth + DrawDistance) / 2
        mov esi, eax
        fmul dword ptr ds : [esi + 0x08]  // st(0) = Depth sort index
        ret
    }
}

// Increases the calculated draw distance of underwater particle effects for depth sorting (always draw before water).
__declspec(naked) void __stdcall AdjustUnderwaterEffectDepthASM()
{
    __asm
    {
        push eax
        call CurrentRoomHasWater
        test al, al
        pop eax
        jz ExitASM

        fmul dword ptr ds : [kUnderwaterEffectDepthFactor]
        mov esi, dword ptr ds : [DrawDistanceAddr]
        fld dword ptr ds : [esi]
        fmul dword ptr ds : [kUnderwaterEffectDepthFactor2]
        faddp st(1), st(0)  // Depth = (Depth + 3 * DrawDistance) / 4

    ExitASM:
        mov esi, eax
        fmul dword ptr ds : [esi + 0x08]  // st(0) = Depth sort index
        ret
    }
}

// Fixes draw order of water and underwater particle effects with respect to other effects in the scene.
void PatchWaterDrawOrderFix()
{
    constexpr BYTE InitBloodDropEffectSearchBytes[]{ 0x5F, 0x66, 0xC7, 0x46, 0x08, 0x02, 0x00, 0x5E, 0xC3 };
    const DWORD InitBloodDropEffectAddr = SearchAndGetAddresses(0x004CFC33, 0x004CFEE3, 0x004CF7A3, InitBloodDropEffectSearchBytes, sizeof(InitBloodDropEffectSearchBytes), 0x16, __FUNCTION__);

    constexpr BYTE WaterDepthSearchBytesA[]{ 0xD9, 0x44, 0x24, 0x3C, 0x8B, 0xF0, 0xD8, 0x4E, 0x08 };
    constexpr BYTE WaterDepthSearchBytesBC[]{ 0xD9, 0x44, 0x24, 0x38, 0x8B, 0xF0, 0xD8, 0x4E, 0x08 };
    const DWORD WaterDepthAddrA = SearchAndGetAddresses(0x004D2CDE, 0x004D2F8E, 0x004D284E, WaterDepthSearchBytesA, sizeof(WaterDepthSearchBytesA), 0x04, __FUNCTION__);
    const DWORD WaterDepthAddrB = SearchAndGetAddresses(0x004D7DA2, 0x004D8052, 0x004D7912, WaterDepthSearchBytesBC, sizeof(WaterDepthSearchBytesBC), 0x04, __FUNCTION__);
    const DWORD WaterDepthAddrC = SearchAndGetAddresses(0x004D9542, 0x004D97F2, 0x004D90B2, WaterDepthSearchBytesBC, sizeof(WaterDepthSearchBytesBC), 0x04, __FUNCTION__);

    constexpr BYTE BloodPoolDepthSearchBytes[]{ 0x8B, 0x8C, 0x24, 0x88, 0x00, 0x00, 0x00, 0xC7, 0x84, 0x24, 0x8C };
    const DWORD BloodPoolDepthAddr = SearchAndGetAddresses(0x004CD10F, 0x004CD3BF, 0x004CCC7F, BloodPoolDepthSearchBytes, sizeof(BloodPoolDepthSearchBytes), 0x1F, __FUNCTION__);

    constexpr BYTE DustDepthSearchBytes[]{ 0x81, 0xC4, 0x14, 0x01, 0x00, 0x00, 0xC3, 0x90 };
    const DWORD DustDepthAddr = SearchAndGetAddresses(0x004E02D7, 0x004E0587, 0x004DFE47, DustDepthSearchBytes, sizeof(DustDepthSearchBytes), -0x1E, __FUNCTION__);

    constexpr BYTE DrawDistanceSearchBytes[]{ 0x8B, 0x44, 0x24, 0x78, 0x85, 0xC0, 0xD9, 0xFF };
    DrawDistanceAddr = ReadSearchedAddresses(0x004ED5D4, 0x004ED884, 0x004ED144, DrawDistanceSearchBytes, sizeof(DrawDistanceSearchBytes), 0x0A, __FUNCTION__);

    if (!InitBloodDropEffectAddr || !WaterDepthAddrA || !WaterDepthAddrB || !WaterDepthAddrC || !BloodPoolDepthAddr || !DustDepthAddr || !DrawDistanceAddr)
    {
        Logging::Log() << __FUNCTION__ << "Error: failed to find memory address!";
        return;
    }

    Logging::Log() << "Enabling Water Draw Order Fix...";
    WriteCalltoMemory((BYTE*)InitBloodDropEffectAddr, *InitBloodDropEffectASM, 0x06);
    WriteCalltoMemory((BYTE*)WaterDepthAddrA, *AdjustWaterDepthASM, 0x05);
    WriteCalltoMemory((BYTE*)WaterDepthAddrB, *AdjustWaterDepthASM, 0x05);
    WriteCalltoMemory((BYTE*)WaterDepthAddrC, *AdjustWaterDepthASM, 0x05);
    WriteCalltoMemory((BYTE*)BloodPoolDepthAddr, *AdjustUnderwaterEffectDepthASM, 0x05);
    WriteCalltoMemory((BYTE*)DustDepthAddr, *AdjustUnderwaterEffectDepthASM, 0x05);
}

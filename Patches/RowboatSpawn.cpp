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
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

namespace {
    constexpr int kEnteredRowboatGameFlag = 0x17B;

    BYTE* GameFlagPtr = 0;

    bool IsGameFlagSet(int flag)
    {
        return GameFlagPtr[flag >> 3] & (1 << (flag & 0x07));
    }

    void MoveRowboatToHotelDock(BYTE* RowboatSubCharPtr)
    {
        if (!IsGameFlagSet(kEnteredRowboatGameFlag)) return;

        float* RowboatPos = (float*)(RowboatSubCharPtr + 0x1C);
        RowboatPos[0] = -8915.900391f;
        RowboatPos[1] = 3690.0f;
        RowboatPos[2] = -19412.69922f;

        float* RowboatRot = (float*)(RowboatSubCharPtr + 0x2C);
        RowboatRot[0] = 0.0f;
        RowboatRot[1] = 3.141593f;
        RowboatRot[2] = 0.0f;
    }

    __declspec(naked) void __stdcall RowboatSpawnASM()
    {
        __asm
        {
            mov eax, dword ptr ds : [esp]
            call MoveRowboatToHotelDock
            add esp, 0x4C
            ret
        }
    }
}

// Spawns the rowboat at the dock outside of Lakeview Hotel when the player re-enters the area.
void PatchRowboatSpawn()
{
    constexpr BYTE GameFlagSearchBytes[]{ 0x83, 0xFE, 0x01, 0x55, 0x57, 0xBD, 0x00, 0x01, 0x00, 0x00 };
    GameFlagPtr = (BYTE*)ReadSearchedAddresses(0x0048AA9E, 0x0048AD3E, 0x0048AF4E, GameFlagSearchBytes, sizeof(GameFlagSearchBytes), 0x24, __FUNCTION__);

    constexpr BYTE SpawnRowboatSearchBytes[]{ 0xC7, 0x44, 0x24, 0x38, 0xDB, 0x0F, 0xC9, 0x3F };
    DWORD RowboatSpawnAddr = SearchAndGetAddresses(0x0057E64B, 0x0057EEFB, 0x0057E81B, SpawnRowboatSearchBytes, sizeof(SpawnRowboatSearchBytes), 0x15, __FUNCTION__);
    if (!GameFlagPtr || !RowboatSpawnAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }

    Logging::Log() << "Patching Rowboat Spawn Fix...";
    UpdateMemoryAddress((BYTE*)(RowboatSpawnAddr - 0x83), "\x90\x90\x90\x90\x90\x90", 6);
    WriteJMPtoMemory((BYTE*)RowboatSpawnAddr, *RowboatSpawnASM, 0x07);
}

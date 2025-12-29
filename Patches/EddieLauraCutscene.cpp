/**
* Copyright (C) 2025 Murugo
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
#define NOMINMAX

#include <Windows.h>
#include <algorithm>
#include "Patches.h"
#include "Common\Settings.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

namespace {

constexpr float kEddieLauraCutsceneFadeStart = 1190.0f;

struct AnimInfo
{
    uint16_t id;
    uint16_t frame_count;
    int16_t speed;
    uint16_t start_frame;
    uint16_t end_frame;
    char loop;
    char pad;
};

bool IsFadeOutActive()
{
    return GetCutsceneTimer() > kEddieLauraCutsceneFadeStart - 1.0f;
}

// Variables for ASM
static DWORD jmpCutsceneFadeOutReturnAddr = 0;

// Fades the screen out toward the end of the cutscene.
__declspec(naked) void __stdcall CutsceneFadeOutASM()
{
    __asm
    {
        call IsFadeOutActive
        test al, al
        jnz FadeOut
        cmp edi, esi
        jz ExitASM

    FadeOut:
        jmp jmpCutsceneFadeOutReturnAddr

    ExitASM:
        pop edi
        xor eax, eax
        pop esi
        ret
    }
}

}

// Restores the fade-out when using the extended cutscene between Eddie and Laura
// in the bowling alley.
void PatchEddieLauraCutscene()
{
    constexpr BYTE SearchBytes[]{ 0x3B, 0xFE, 0x0F, 0x84, 0xB6, 0x00, 0x00, 0x00 };
    const DWORD CutsceneFadeOutAddr = SearchAndGetAddresses(0x0059179E, 0x0059204E, 0x0059196E, SearchBytes, sizeof(SearchBytes), 0x00, __FUNCTION__);
    if (!CutsceneFadeOutAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }
    jmpCutsceneFadeOutReturnAddr = CutsceneFadeOutAddr + 0x08;

    Logging::Log() << "Patching Eddie and Laura Bowling Alley Cutscene...";
    WriteJMPtoMemory((BYTE*)CutsceneFadeOutAddr, *CutsceneFadeOutASM, 0x08);
}

// Adjusts the length of animations for actors in the cutscene with Eddie and Laura
// in the bowling alley.
void RunEddieLauraCutscene()
{
    static float* MaxCutsceneFrame = nullptr;

    // Anim table entries for the second half of CS_BOWL_LAURA_EDDIE.
    static AnimInfo* EddieAnimInfo = nullptr;
    static AnimInfo* LauraAnimInfo = nullptr;
    static AnimInfo* PizzaAnimInfo = nullptr;

    if (!MaxCutsceneFrame || !EddieAnimInfo || !LauraAnimInfo || !PizzaAnimInfo)
    {
        RUNONCE();

        {
            constexpr BYTE SearchBytes[]{ 0x83, 0xC4, 0x04, 0x25, 0xFF, 0xFB, 0xFF, 0xFF };
            MaxCutsceneFrame = (float*)ReadSearchedAddresses(0x004A1F36, 0x004A21E6, 0x004A1AA6, SearchBytes, sizeof(SearchBytes), 0x4F, __FUNCTION__);
        }
        {
            constexpr BYTE SearchBytes[]{ 0xA0, 0x11, 0xAA, 0x01, 0x00, 0x08, 0x00, 0x00, 0xA9, 0x01, 0x00, 0x00 };
            EddieAnimInfo = (AnimInfo*)SearchAndGetAddresses(0x006E4CB0, 0x006E5E98, 0x006E4E98, SearchBytes, sizeof(SearchBytes), 0x00, __FUNCTION__);
        }
        {
            constexpr BYTE SearchBytes[]{ 0xE3, 0x09, 0xAA, 0x01, 0x00, 0x08, 0x00, 0x00, 0xA9, 0x01, 0x00, 0x00 };
            LauraAnimInfo = (AnimInfo*)SearchAndGetAddresses(0x006E670C, 0x006E78F4, 0x006E68F4, SearchBytes, sizeof(SearchBytes), 0x00, __FUNCTION__);
        }
        {
            constexpr BYTE SearchBytes[]{ 0xD6, 0x27, 0xAA, 0x01, 0x00, 0x08, 0x00, 0x00, 0xA9, 0x01, 0x00, 0x00 };
            PizzaAnimInfo = (AnimInfo*)SearchAndGetAddresses(0x006DAC58, 0x006DBE40, 0x006DAE40, SearchBytes, sizeof(SearchBytes), 0x00, __FUNCTION__);
        }

        if (!MaxCutsceneFrame || !EddieAnimInfo || !LauraAnimInfo || !PizzaAnimInfo)
        {
            Logging::Log() << "__FUNCTION__" << " Error: failed to find memory address!";
            return;
        }
    }

    if (GetCutsceneID() != CS_BOWL_LAURA_EDDIE || *MaxCutsceneFrame == 0.0f)
    {
        return;
    }
    if (EddieLauraCutscene)
    {
        const uint16_t NewFrameCount = (uint16_t)*MaxCutsceneFrame - 764;
        const uint16_t NewEndFrame = NewFrameCount - 1;
        for (AnimInfo* info : { EddieAnimInfo, LauraAnimInfo, PizzaAnimInfo })
        {
            UpdateMemoryAddress(&info->frame_count, &NewFrameCount, sizeof(uint16_t));
            UpdateMemoryAddress(&info->end_frame, &NewEndFrame, sizeof(uint16_t));
        }
    }
    else
    {
        // If disabled, clip the cutscene to its original length even if the .DDS file has more frames.
        const float NewFrameCount = std::min(*MaxCutsceneFrame, kEddieLauraCutsceneFadeStart);
        UpdateMemoryAddress(MaxCutsceneFrame, &NewFrameCount, sizeof(float));
    }
}

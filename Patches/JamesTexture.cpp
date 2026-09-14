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
#include "Patches\Patches.h"
#include "Logging\Logging.h"

constexpr int kJamesJacketOldTextureId = 1240;
constexpr int kJamesJacketNewTextureId = 9980;
static BYTE*(*shGetTexture)(UINT) = nullptr;

// Variables for ASM
void* jmpReplaceJamesTextureReturnAddr = nullptr;

BYTE* GetJamesReplacementTexture(UINT textureId)
{
    // Replace James' jacket texture during the noose cutscene to hide parts of the jacket
    // model from clipping through both shoulders.
    if (textureId == kJamesJacketOldTextureId && GetCutsceneID() == CS_PS_NOOSE_PULL)
    {
        BYTE* texture = shGetTexture(kJamesJacketNewTextureId);
        if (texture != nullptr)
        {
            return texture;
        }
    }
    return shGetTexture(textureId);
}

__declspec(naked) void __stdcall ReplaceJamesTextureASM()
{
    __asm
    {
        mov eax, dword ptr ds : [ebx + esi * 0x08]
        mov ecx, dword ptr ds : [edi + eax * 0x04]
        push ecx
        call GetJamesReplacementTexture
        add esp, 0x04
        jmp jmpReplaceJamesTextureReturnAddr
    }
}

void PatchJamesTexture()
{
    constexpr BYTE SearchBytes[]{ 0x8B, 0x04, 0xF3, 0x8B, 0x0C, 0x87, 0x51 };
    const DWORD InjectAddr = SearchAndGetAddresses(0x0050DAB0, 0x0050DDE0, 0x0050D700, SearchBytes, sizeof(SearchBytes), 0x00, __FUNCTION__);
    if (!InjectAddr)
    {
        Logging::Log() << __FUNCTION__ << "Error: failed to find memory address!";
        return;
    }
    shGetTexture = (BYTE*(*)(UINT))((BYTE*)(InjectAddr + 0x0C) + *(DWORD*)(InjectAddr + 0x08));
    jmpReplaceJamesTextureReturnAddr = (void*)(InjectAddr + 0x0F);

    Logging::Log() << "Enabling texture replacement for James...";
    WriteJMPtoMemory((BYTE*)InjectAddr, *ReplaceJamesTextureASM, 0x06);
}

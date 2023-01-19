/**
* Copyright (C) 2023 Murugo
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
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "Patches\Patches.h"

typedef int32_t(__cdecl* GetMouseHorizontalRawPositionProc)();

GetMouseHorizontalRawPositionProc GetMouseHorizontalRawPosition = nullptr;
int16_t* SelectionIndex = nullptr;
void* CommandWindowMouseFixReturnAddr = nullptr;
DWORD IsMouseMovingFuncAddr = 0;

// Checks the cursor position and updates the selected command in the inventory command window when 3 options are available.
void Handle3CommandWindow() {
    if (SelectionIndex == nullptr || GetMouseHorizontalRawPosition == nullptr)
        return;

    // GetMouseHorizontalRawPosition() returns the horizontal position of the cursor without the
    // offset added by the WidescreenFixesPack. This allows us to check the absolute position of
    // the cursor at any aspect ratio.
    const int32_t MouseX = GetMouseHorizontalRawPosition();
    if (MouseX <= 355 || MouseX >= 440)
        return;
    const int32_t MouseY = GetMouseVerticalPosition();
    if (MouseY <= 62 || MouseY >= 140)
        return;
    if (MouseY < 88)
        *SelectionIndex = 0;
    else if (MouseY < 114)
        *SelectionIndex = 1;
    else
        *SelectionIndex = 2;
}

__declspec(naked) void __stdcall CommandWindowMouseFixASM()
{
    __asm
    {
        push eax
        mov eax, dword ptr ds : [IsMouseMovingFuncAddr]
        call eax
        test eax, eax
        pop eax
        jz Return
        cmp eax, 0x02
        jnz ExitAsm
        call Handle3CommandWindow

    Return:
        pop edi
        pop esi
        pop ebp
        add esp, 0x18
        ret

    ExitAsm:
        jmp CommandWindowMouseFixReturnAddr
    }
}

// Patch bugs with mouse interaction in the inventory command window:
// * Allow mouse selection when 3 options are present.
// * Allow keyboard/gamepad selection if the mouse is hovering over an option in the window.
void PatchCommandWindowMouseFix()
{
    constexpr BYTE CommandMouseInputSearchBytes[]{ 0x5F, 0x5E, 0x5D, 0x83, 0xC4, 0x18, 0xC3, 0x83, 0xF8, 0x01 };
    const DWORD CommandMouseInputAddr = SearchAndGetAddresses(0x00472428, 0x004726C8, 0x004728D8, CommandMouseInputSearchBytes, sizeof(CommandMouseInputSearchBytes), 0x07);
    if (!CommandMouseInputAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }
    memcpy(&SelectionIndex, (void*)(CommandMouseInputAddr - 0x0B), sizeof(DWORD));
    CommandWindowMouseFixReturnAddr = (void*)(CommandMouseInputAddr + 0x05);

    DWORD GetMouseXRelativeAddr = 0;
    memcpy(&GetMouseXRelativeAddr, (void*)(CommandMouseInputAddr - 0x2E), sizeof(DWORD));
    GetMouseHorizontalRawPosition = (GetMouseHorizontalRawPositionProc)(CommandMouseInputAddr + GetMouseXRelativeAddr - 0x2A);

    constexpr BYTE IsMouseMovingSearchBytes[]{ 0x8B, 0xC8, 0x81, 0xE9, 0x88, 0x00, 0x00, 0x00 };
    const DWORD IsMouseMovingSearchAddr = SearchAndGetAddresses(0x0044FD38, 0x0044FF98, 0x0044FF98, IsMouseMovingSearchBytes, sizeof(IsMouseMovingSearchBytes), -0x0D);
    if (!CommandMouseInputAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }
    DWORD IsMouseMovingRelativeAddr = 0;
    memcpy(&IsMouseMovingRelativeAddr, (void*)(IsMouseMovingSearchAddr), sizeof(DWORD));
    IsMouseMovingFuncAddr = IsMouseMovingRelativeAddr + IsMouseMovingSearchAddr + 0x04;

    Logging::Log() << "Patching Command Window Mouse Fix...";
    WriteJMPtoMemory((BYTE*)CommandMouseInputAddr, *CommandWindowMouseFixASM, 0x05);
}

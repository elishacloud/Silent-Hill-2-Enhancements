/**
* Copyright (C) 2022 Murugo
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
DWORD ActiveMarkerStateAddr = 0;
DWORD PauseCounterAddr = 0;
DWORD ZoomOrPanStateAddr = 0;
DWORD MarkerIndexAddr = 0;
DWORD DeltaFrameAddr = 0;
DWORD State4ReturnAddr = 0;
DWORD StateJumpTableAddr = 0;

float ZoomPanTimerMaxValue = 0.99999f;

// ASM which replaces frame timers in the active marker state handler.
__declspec(naked) void __stdcall ActiveMarkerStateASM()
{
    __asm
    {
        cmp edx, 0x01
        je ActiveMarkerState1
        cmp edx, 0x02
        je ActiveMarkerState2
        cmp edx, 0x03
        je ActiveMarkerState3
        cmp edx, 0x04
        je ActiveMarkerState4
        cmp edx, 0x00
        jnz HandlerResume

        // Wait 1/30 sec before moving to the next state.
        push eax
        push 0x02
        call PauseForFrames
        add esp, 0x04
        test eax, eax
        pop eax
        jz HandlerExit
        jmp HandlerResume

    ActiveMarkerState1:
        // Wait 17/30 sec before moving to the next state.
        push eax
        push 0x22
        call PauseForFrames
        add esp, 0x04
        test eax, eax
        pop eax
        jz HandlerExit
        mov ecx, dword ptr ds : [ZoomOrPanStateAddr]
        mov dword ptr ds : [ecx], eax
        mov ecx, dword ptr ds : [ActiveMarkerStateAddr]
        mov dword ptr ds : [ecx], 0x02
        mov ecx, dword ptr ds : [PauseCounterAddr]
        mov dword ptr ds : [ecx], 0x00
        jmp HandlerExit

    ActiveMarkerState2:
        // Wait 1/30 sec before checking whether the zoom is complete.
        push eax
        push 0x02
        call PauseForFrames
        add esp, 0x04
        test eax, eax
        pop eax
        jz HandlerExit
        mov ecx, dword ptr ds : [ZoomOrPanStateAddr]
        cmp dword ptr ds : [ecx], 0x02
        jnz HandlerExit
        mov ecx, dword ptr ds : [ActiveMarkerStateAddr]
        mov dword ptr ds : [ecx], 0x03
        mov ecx, dword ptr ds : [PauseCounterAddr]
        mov dword ptr ds : [ecx], 0x00
        jmp HandlerExit

    ActiveMarkerState3:
        // Fade in active marker.
        mov edx, dword ptr ds : [MarkerIndexAddr]
        mov edx, dword ptr ds : [edx]
        push eax
        mov eax, dword ptr ds : [DeltaFrameAddr]
        mov ah, byte ptr ds : [eax]
        shl ah, 0x02
        push ebx
        mov bl, [ecx + edx * 0x08 + 0x04]
        add bl, ah
        mov[ecx + edx * 0x08 + 0x04], bl
        cmp bl, 0x80
        pop ebx
        pop eax
        jb HandlerExit
        mov byte ptr ds : [ecx + edx * 0x08 + 0x04], 0x80
        mov ecx, dword ptr ds : [ActiveMarkerStateAddr]
        mov dword ptr ds : [ecx], 0x04
        jmp HandlerExit

    ActiveMarkerState4:
        // Wait 5/6 sec before moving to the next state.
        push eax
        push 0x32
        call PauseForFrames
        add esp, 0x04
        test eax, eax
        pop eax
        jz HandlerExit
        jmp State4ReturnAddr

    PauseForFrames:
        push ecx
        mov eax, dword ptr ds : [PauseCounterAddr]
        mov ecx, dword ptr ds : [eax]
        push edx
        mov edx, dword ptr ds : [DeltaFrameAddr]
        add ecx, dword ptr ds : [edx]
        pop edx
        mov dword ptr ds : [eax], ecx
        mov eax, 0x00
        cmp ecx, [esp + 0x08]
        pop ecx
        jge PauseForFramesDone
        ret
    PauseForFramesDone:
        mov eax, 0x01
        ret

    HandlerResume:
        mov ebx, dword ptr ds : [StateJumpTableAddr]
        mov edx, dword ptr ds : [ActiveMarkerStateAddr]
        mov edx, dword ptr ds : [edx]
        jmp dword ptr ds : [edx * 0x04 + ebx]

    HandlerExit:
        pop edi
        pop esi
        pop ecx
        ret
    }
}

DWORD GetDeltaFrameAddress()
{
    constexpr BYTE SearchBytesDeltaFrame[]{ 0x8B, 0x44, 0x24, 0x04, 0x8B, 0xC8, 0xA3 };
    DWORD Addr = ReadSearchedAddresses(0x004477DD, 0x0044797D, 0x0044797D, SearchBytesDeltaFrame, sizeof(SearchBytesDeltaFrame), -0x04);
    if (!Addr)
    {
        Logging::Log() << __FUNCTION__ << "Error: failed to find memory address!";
        return 0;
    }
    return Addr;
}

// Patch map transcription animation to play at the correct rate at 60 FPS.
void PatchMapTranscription()
{
    constexpr BYTE SearchBytesMarkerStateHandler[]{ 0x8B, 0xD1, 0xC1, 0xFA, 0x13, 0x81, 0xE2, 0xFF, 0x1F, 0x00, 0x00 };
    DWORD MarkerStateHandlerAddr = SearchAndGetAddresses(0x0049C644, 0x0049C8F4, 0x0049C1B4, SearchBytesMarkerStateHandler, sizeof(SearchBytesMarkerStateHandler), -0x10);
    if (!MarkerStateHandlerAddr)
    {
        Logging::Log() << __FUNCTION__ << "Error: failed to find memory address!";
        return;
    }

    memcpy(&StateJumpTableAddr, (DWORD*)(MarkerStateHandlerAddr + 0x03), sizeof(DWORD));
    memcpy(&ActiveMarkerStateAddr, (DWORD*)(MarkerStateHandlerAddr + 0x54), sizeof(DWORD));
    memcpy(&PauseCounterAddr, (DWORD*)(MarkerStateHandlerAddr + 0x6D), sizeof(DWORD));
    memcpy(&ZoomOrPanStateAddr, (DWORD*)(MarkerStateHandlerAddr + 0xA2), sizeof(DWORD));
    memcpy(&MarkerIndexAddr, (DWORD*)(MarkerStateHandlerAddr + 0xBD), sizeof(DWORD));
    State4ReturnAddr = MarkerStateHandlerAddr + 0xFD;

    constexpr BYTE SearchBytesZoomPanTimerMaxValue[]{ 0x83, 0xEC, 0x10, 0x83, 0xF8, 0x01, 0x75 };
    DWORD ZoomTimerMaxWriteAddr = SearchAndGetAddresses(0x0049DC35, 0x0049DEE5, 0x0049D7A5, SearchBytesZoomPanTimerMaxValue, sizeof(SearchBytesZoomPanTimerMaxValue), 0x1B);
    if (!ZoomTimerMaxWriteAddr)
    {
        Logging::Log() << __FUNCTION__ << "Error: failed to find memory address!";
        return;
    }
    DWORD PanTimerMaxWriteAddr = ZoomTimerMaxWriteAddr + 0x68;
    float* ZoomPanTimerMaxValuePtr = &ZoomPanTimerMaxValue;

    DeltaFrameAddr = GetDeltaFrameAddress();
    if (DeltaFrameAddr == 0)
    {
        return;
    }

    Logging::Log() << "Enabling Map Transcribe Animation Speed Fix...";
    WriteJMPtoMemory((BYTE*)MarkerStateHandlerAddr, *ActiveMarkerStateASM, 0x07);
    UpdateMemoryAddress((DWORD*)ZoomTimerMaxWriteAddr, &ZoomPanTimerMaxValuePtr, sizeof(float));
    UpdateMemoryAddress((DWORD*)PanTimerMaxWriteAddr, &ZoomPanTimerMaxValuePtr, sizeof(float));
}

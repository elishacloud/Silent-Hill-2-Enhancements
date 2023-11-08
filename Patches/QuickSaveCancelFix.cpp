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
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Variables for ASM
BYTE *SaveLoadSubState = nullptr;
void *jmpSaveSubState5Addr = nullptr;
void *jmpSaveSubState6Addr = nullptr;
void *jmpSaveSubStateDefaultAddr = nullptr;
void *WriteFileFuncAddr = nullptr;
void *SysFileNameAddr = nullptr;
void *SysFileBytesAddr = nullptr;

// ASM function to move the write to sh2pc.sys to a later stage
__declspec(naked) void __stdcall SaveSubStateASM()
{
    __asm
    {
        mov eax, dword ptr ds : [SaveLoadSubState]
        movsx eax, byte ptr ds : [eax]
        cmp eax, 0x06
        je HandleState6
        cmp eax, 0x05
        jne ExitAsm
        // Skip the early write to sh2pc.sys
        jmp jmpSaveSubState5Addr

    HandleState6:
        // Write sh2pc.sys and sh2pcsave??.dat serially on the same frame
        push 0xC00
        mov eax, dword ptr ds : [SysFileBytesAddr]
        push eax
        mov eax, dword ptr ds : [SysFileNameAddr]
        push eax
        mov eax, dword ptr ds : [WriteFileFuncAddr]
        call eax
        add esp, 0x0C
        cmp eax, 0x00
        jne Failure
        jmp jmpSaveSubState6Addr

    Failure:
        mov eax, 0x02
        ret
    
    ExitAsm:
        jmp jmpSaveSubStateDefaultAddr
    }
}

void PatchQuickSaveCancelFix()
{
    // Fix for writing sh2pc.sys and sh2pcsave??.dat on the same frame
    constexpr BYTE SaveSubStateSearchBytes[]{ 0x83, 0xF8, 0x06, 0x77, 0x47, 0xFF };
    DWORD SaveSubStateFuncAddr = SearchAndGetAddresses(0x00453137, 0x00453397, 0x00453397, SaveSubStateSearchBytes, sizeof(SaveSubStateSearchBytes), -0x07, __FUNCTION__);
    if (!SaveSubStateFuncAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }
    jmpSaveSubStateDefaultAddr = (void*)(SaveSubStateFuncAddr + 0x07);
    jmpSaveSubState5Addr = (void*)(SaveSubStateFuncAddr + 0x178);
    jmpSaveSubState6Addr = (void*)(SaveSubStateFuncAddr + 0x189);
    WriteFileFuncAddr = (void*)(*(DWORD*)(SaveSubStateFuncAddr + 0x88) + SaveSubStateFuncAddr + 0x8C);
    SysFileNameAddr = (void*)*(DWORD*)(SaveSubStateFuncAddr + 0xE3);
    SysFileBytesAddr = (void*)*(DWORD*)(SaveSubStateFuncAddr + 0xDE);

    Logging::Log() << "Enabling Quick Save Cancel Fix...";
    WriteJMPtoMemory((BYTE*)SaveSubStateFuncAddr, *SaveSubStateASM, 7);
}

void RunQuickSaveCancelFix()
{
    // Get save/load state addresses
    static BYTE *SaveLoadState = nullptr;
    if (!SaveLoadState)
    {
        RUNONCE();

        // Get address for save/load state
        constexpr BYTE SearchBytes[]{ 0x83, 0xF8, 0x14, 0x0F, 0x87, 0xF1, 0x0A, 0x00, 0x00 };
        SaveLoadState = (BYTE*)ReadSearchedAddresses(0x00453D70, 0x00453FD0, 0x00453FD0, SearchBytes, sizeof(SearchBytes), -0x04, __FUNCTION__);
        if (!SaveLoadState)
        {
            Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
            return;
        }
        SaveLoadSubState = SaveLoadState + 1;
    }

    // Cancel an active quick save before entering another room or when opening the save menu
    if (GetIsWritingQuicksave() == 1 && (GetEventIndex() == 1 || GetEventIndex() == 9))
    {
        *GetIsWritingQuicksavePointer() = 0;
        *SaveLoadState = 0;
        *SaveLoadSubState = 0;
    }
}

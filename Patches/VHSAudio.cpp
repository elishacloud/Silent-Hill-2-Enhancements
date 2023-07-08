/**
* Copyright (C) 2023 mercury501
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

#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\utility.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "Patches\Patches.h"
#include "Logging\Logging.h"
#include "Common\Utils.h"

injector::hook_back<int32_t(__cdecl*)(char*)> orgLoadMovie;

BYTE* SetAudioTrackReturn = nullptr;
BYTE* SetAudioTrackMurderReturn = nullptr;
char* MurderFileName = "data\\movie\\murder.bik";

bool loadingMurder = false;

__declspec(naked) void __stdcall SetAudioTrack()
{

    __asm
    {   // Restore the overwritten instructions
        add esp, 0x04
        xor ebx, ebx
    }

    if (loadingMurder)
    {
        __asm
        {   // Inject audio track 2
            mov [esp + 0x10], 0x01
            jmp SetAudioTrackMurderReturn
        }
    }

    __asm
    {
        jmp SetAudioTrackReturn
    }
}

int32_t __cdecl LoadMovie_Hook(char* FileName)
{
	Logging::LogDebug() << __FUNCTION__ << " File name: " << FileName;

    loadingMurder = strcmp(FileName, MurderFileName) == 0;

	return orgLoadMovie.fun(FileName);
}

void PatchVHSAudio()
{

    BYTE* SetAudioTrackAddr = (BYTE*)0x0043d704; //TODO address
    SetAudioTrackReturn = SetAudioTrackAddr + 0x05;
    SetAudioTrackMurderReturn = (BYTE*)0x0043D802;
    
    WriteJMPtoMemory(SetAudioTrackAddr, SetAudioTrack, 0x05);

	orgLoadMovie.fun = injector::MakeCALL((uint32_t*)0x0043d6ff, LoadMovie_Hook, true).get(); //TODO address
}
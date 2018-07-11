/**
* Copyright (C) 2018 Elisha Riedlinger
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
#include "PS2NoiseFilter.h"
#include "..\Common\Utils.h"
#include "..\Common\Logging.h"

BYTE tmpAddr;
DWORD tmpVar;
constexpr float BrightnessControl = 7.4f;

#pragma warning(suppress: 4725)
__declspec(naked) void __stdcall PS2NoiseFilter_vDC()
{
	__asm
	{
		mov tmpAddr, al
		fild dword ptr tmpAddr
		fdiv dword ptr BrightnessControl
		fistp dword ptr tmpVar
		mov eax, tmpVar
		MOV BYTE PTR DS : [0x009458C5], AL
		jmp jmpAddr_vDC
	}
}
#pragma warning(suppress: 4725)
__declspec(naked) void __stdcall PS2NoiseFilter_v10()
{
	__asm
	{
		mov tmpAddr, al
		fild dword ptr tmpAddr
		fdiv dword ptr BrightnessControl
		fistp dword ptr tmpVar
		mov eax, tmpVar
		MOV BYTE PTR DS : [0x00942CC5], AL
		jmp jmpAddr_v10
	}
}

#pragma warning(suppress: 4725)
__declspec(naked) void __stdcall PS2NoiseFilter_v11()
{
	__asm
	{
		mov tmpAddr, al
		fild dword ptr tmpAddr
		fdiv dword ptr BrightnessControl
		fistp dword ptr tmpVar
		mov eax, tmpVar
		MOV BYTE PTR DS : [0x009468C5], AL
		jmp jmpAddr_v11
	}
}

void UpdatePS2Filter()
{
	// For version DC
	if (CheckMemoryAddress((void*)p_NoiseFilterEDX_vDC, (void*)NoiseFilterEDX[0], sizeof(NoiseFilterEDX[0])) &&
		CheckMemoryAddress((void*)p_NoiseFilterMOV_vDC, (void*)NoiseFilterMOV_vDC[0], sizeof(NoiseFilterMOV_vDC[0])) &&
		CheckMemoryAddress((void*)p_NoiseFilterJMP_vDC, (void*)NoiseFilterJMP_vDC, sizeof(NoiseFilterJMP_vDC)))
	{
		Log() << "Updating for PS2 Noise Filter DC...";

		// Update EDX, MOV and JMP
		UpdateMemoryAddress((void*)p_NoiseFilterEDX_vDC, (void*)NoiseFilterEDX[1], sizeof(NoiseFilterEDX[1]));
		UpdateMemoryAddress((void*)p_NoiseFilterMOV_vDC, (void*)NoiseFilterMOV_vDC[1], sizeof(NoiseFilterMOV_vDC[1]));
		WriteJMPtoMemory((BYTE*)p_NoiseFilterJMP_vDC, *PS2NoiseFilter_vDC);
	}

	// For version v1.0
	else if (CheckMemoryAddress((void*)p_NoiseFilterEDX_v10, (void*)NoiseFilterEDX[0], sizeof(NoiseFilterEDX[0])) &&
		CheckMemoryAddress((void*)p_NoiseFilterMOV_v10, (void*)NoiseFilterMOV_v10[0], sizeof(NoiseFilterMOV_v10[0])) &&
		CheckMemoryAddress((void*)p_NoiseFilterJMP_v10, (void*)NoiseFilterJMP_v10, sizeof(NoiseFilterJMP_v10)))
	{
		Log() << "Updating for PS2 Noise Filter v1.0...";

		// Update EDX, MOV and JMP
		UpdateMemoryAddress((void*)p_NoiseFilterEDX_v10, (void*)NoiseFilterEDX[1], sizeof(NoiseFilterEDX[1]));
		UpdateMemoryAddress((void*)p_NoiseFilterMOV_v10, (void*)NoiseFilterMOV_v10[1], sizeof(NoiseFilterMOV_v10[1]));
		WriteJMPtoMemory((BYTE*)p_NoiseFilterJMP_v10, *PS2NoiseFilter_v10);
	}

	// For version v1.1
	else if (CheckMemoryAddress((void*)p_NoiseFilterEDX_v11, (void*)NoiseFilterEDX[0], sizeof(NoiseFilterEDX[0])) &&
		CheckMemoryAddress((void*)p_NoiseFilterMOV_v11, (void*)NoiseFilterMOV_v11[0], sizeof(NoiseFilterMOV_v11[0])) &&
		CheckMemoryAddress((void*)p_NoiseFilterJMP_v11, (void*)NoiseFilterJMP_v11, sizeof(NoiseFilterJMP_v11)))
	{
		Log() << "Updating for PS2 Noise Filter v1.1...";

		// Update EDX, MOV and JMP
		UpdateMemoryAddress((void*)p_NoiseFilterEDX_v11, (void*)NoiseFilterEDX[1], sizeof(NoiseFilterEDX[1]));
		UpdateMemoryAddress((void*)p_NoiseFilterMOV_v11, (void*)NoiseFilterMOV_v11[1], sizeof(NoiseFilterMOV_v11[1]));
		WriteJMPtoMemory((BYTE*)p_NoiseFilterJMP_v11, *PS2NoiseFilter_v11);
	}

	// Failed
	else
	{
		Log() << "Error: failed to find address for PS2 Noise Filter!";
	}
}

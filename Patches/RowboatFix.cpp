/**
* Copyright (C) 2024 BigManJapan, mercury501
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
#include "Patches\Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"


injector::hook_back<void(__cdecl*)(void)> orgHandleMovement;
BYTE* HandleMovementAddr = nullptr;
BYTE* GreatKnifeAddr = nullptr;

void __cdecl HandleMovement_Hook()
{
	if ((GetBoatFlag() == 0x01 && GetRoomID() == R_TOWN_LAKE))
	{
		UpdateMemoryAddress(GreatKnifeAddr, "\xFF", 0x01);
	}
	else
	{
		UpdateMemoryAddress(GreatKnifeAddr, "\x08", 0x01);
	}

	orgHandleMovement.fun();
}

void PatchGreatKnifeBoatSpeed()
{
	Logging::Log() << "Patching boat speed with great knife...";

	HandleMovementAddr = (BYTE*)0x005363ac;
	GreatKnifeAddr = (BYTE*)0x0052ea64;

	orgHandleMovement.fun = injector::MakeCALL(HandleMovementAddr, HandleMovement_Hook, true).get();
}
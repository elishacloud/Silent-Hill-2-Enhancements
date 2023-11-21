/**
* Copyright (C) 2023 Elisha Riedlinger, mercury501
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
#include "Logging\Logging.h"

#pragma warning(disable : 4100)
int32_t PlaySaveSound_Hook(int32_t SoundId, float volume, DWORD param3)
{
	//TODO  orgPlaySoundFun.fun(S_SAVE_GAME, volume, param3);
	Logging::Log() << __FUNCTION__;
	return 0x10;
}

#pragma warning(disable : 4100)
int32_t PlayLoadSound_Hook(int32_t SoundId, float volume, DWORD param3)
{
	//TODO  orgPlaySoundFun.fun(S_SAVE_GAME, volume, param3);
	Logging::Log() << __FUNCTION__;
	return 0x10;
}

void PatchCustomSFXs()
{
	Logging::Log() << "Patching custom SFXs...";

	uint32_t* LoadGameSoundPauseMenu = (uint32_t*)0x00454A64; //TODO addresses
	uint32_t* LoadGameSoundNewGame = (uint32_t*)0x0049870c;
	uint32_t* LoadGameSoundContinue = (uint32_t*)0x00497f84;
	uint32_t* SaveGameSoundRedSquares = (uint32_t*)0x004476DB;

	// Hook load/save game sounds
	injector::MakeCALL(LoadGameSoundPauseMenu, PlayLoadSound_Hook, true);
	injector::MakeCALL(LoadGameSoundNewGame, PlayLoadSound_Hook, true);
	injector::MakeCALL(LoadGameSoundContinue, PlayLoadSound_Hook, true);
	injector::MakeCALL(SaveGameSoundRedSquares, PlaySaveSound_Hook, true);
}
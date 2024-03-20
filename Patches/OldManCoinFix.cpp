/**
* Copyright (C) 2024 Murugo
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

// Game flag set after James throws the canned juice down the garbage chute.
constexpr WORD kCannedJuiceGameFlag = 0x5F;

// Prevents the Old Man Coin from appearing in the garbage chute outside Wood Side Apartments until
// after James uses the canned juice in the second floor laundry room.
void PatchOldManCoinFix()
{
    constexpr BYTE SearchBytes[]{ 0x1F, 0x07, 0x00, 0x00, 0x76, 0x00, 0x00, 0x00 };
    DWORD OldManCoinPreFlagAddr = SearchAndGetAddresses(0x008DE900, 0x008E25D0, 0x008E15D0, SearchBytes, sizeof(SearchBytes), 0x06, __FUNCTION__);
    if (!OldManCoinPreFlagAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }

    Logging::Log() << "Patching Old Man Coin Fix...";
    UpdateMemoryAddress((void*)OldManCoinPreFlagAddr, &kCannedJuiceGameFlag, sizeof(WORD));
}

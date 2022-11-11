/**
* Copyright (C) 2022 Elisha Riedlinger
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
*
* Includes source code from WidescreenFixes Pack
* https://github.com/ThirteenAG/WidescreenFixesPack
*/

#include "WidescreenFixesPack.h"
#include <vector>
#include "Common\Utils.h"
#include "Patches\Patches.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

bool IsUALPresent()
{
	return true;
}

std::tuple<int32_t, int32_t> GetDesktopRes()
{
	LONG screenWidth = 0, screenHeight = 0;
	GetDesktopRes(screenWidth, screenHeight);

	// Update patches for resolution change
	UpdateResolutionPatches(screenWidth, screenHeight);

	return std::make_tuple((int32_t)screenWidth, (int32_t)screenHeight);
}

std::string format(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	std::vector<char> v(1024);
	while (true)
	{
		va_list args2;
		va_copy(args2, args);
		int res = vsnprintf(v.data(), v.size(), fmt, args2);
		if ((res >= 0) && (res < static_cast<int>(v.size())))
		{
			va_end(args);
			va_end(args2);
			return std::string(v.data());
		}
		size_t size;
		if (res < 0)
		{
			size = v.size() * 2;
		}
		else
		{
			size = static_cast<size_t>(res) + 1;
		}
		v.clear();
		v.resize(size);
		va_end(args2);
	}
}

int GetValue(std::string_view, std::string_view szKey, int)
{
	int ret = 0;

	if (szKey.compare("ResX") == 0)
	{
		ret = ResX;
	}
	else if (szKey.compare("ResY") == 0)
	{
		ret = ResY;
	}
	else if (szKey.compare("FMVWidescreenMode") == 0)
	{
		ret = 0;
	}
	else if (szKey.compare("FMVWidescreenEnhancementPackCompatibility") == 0)
	{
		ret = 0;
	}
	else if (szKey.compare("Fix2D") == 0)
	{
		ret = Fix2D;
	}
	else if (szKey.compare("DisableCutsceneBorders") == 0)
	{
		ret = DisableCutsceneBorders;
	}
	else if (szKey.compare("SingleCoreAffinity") == 0)
	{
		ret = 0;	// Always disable single core affinity here
	}
	else if (szKey.compare("DisableSafeMode") == 0)
	{
		ret = DisableSafeMode;
	}
	else if (szKey.compare("FastTransitions") == 0)
	{
		ret = FastTransitions;
	}
	else if (szKey.compare("CreateLocalFix") == 0)
	{
		ret = CreateLocalFix;
	}
	else if (szKey.compare("FPSLimit") == 0)
	{
		ret = FPSLimit;
	}
	else if (szKey.compare("PS2CameraSpeed") == 0)
	{
		ret = PS2CameraSpeed;
	}
	else if (szKey.compare("GamepadControlsFix") == 0)
	{
		ret = GamepadControlsFix;
	}
	else if (szKey.compare("LightingFix") == 0)
	{
		ret = LightingFix;
	}
	else if (szKey.compare("ReduceCutsceneFOV") == 0)
	{
		ret = ReduceCutsceneFOV;
	}
	else if (szKey.compare("SteamCrashFix") == 0)
	{
		ret = SteamCrashFix;
	}
	else if (szKey.compare("IncreaseNoiseEffectRes") == 0)
	{
		ret = IncreaseNoiseEffectRes;
	}
	else if (szKey.compare("FullscreenImages") == 0)
	{
		ret = 0;	// Always disable full screen images here
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: setting not found! '" << szKey << "'";
	}

	return ret;
}

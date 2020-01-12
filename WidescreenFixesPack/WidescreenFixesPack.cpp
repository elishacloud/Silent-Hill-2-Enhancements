#include "IniReader.h"
#include <vector>
#include "Common\Settings.h"
#include "Logging\Logging.h"

bool IsUALPresent()
{
	return true;
}

std::tuple<int32_t, int32_t> GetDesktopRes()
{
	HMONITOR monitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);
	MONITORINFO info = {};
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &info);
	int32_t DesktopResW = info.rcMonitor.right - info.rcMonitor.left;
	int32_t DesktopResH = info.rcMonitor.bottom - info.rcMonitor.top;
	return std::make_tuple(DesktopResW, DesktopResH);
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
			size = v.size() * 2;
		else
			size = static_cast<size_t>(res) + 1;
		v.clear();
		v.resize(size);
		va_end(args2);
	}
}

int CIniReader::ReadInteger(std::string_view, std::string_view szKey, int)
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
		ret = FMVWidescreenMode;
	}
	else if (szKey.compare("FMVWidescreenEnhancementPackCompatibility") == 0)
	{
		ret = FMVWidescreenEnhancementPackCompatibility;
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
		ret = FullscreenImages;
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: setting not found!";
	}

	return ret;
}

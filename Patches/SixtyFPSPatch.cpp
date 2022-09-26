#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\calling.hpp"
#include "External\injector\include\injector\hooking.hpp"
#include "External\injector\include\injector\utility.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "Logging\Logging.h"
#include "Patches.h"

injector::hook_back<float(__cdecl*)(void)> GetFogAnimationRate;
injector::hook_back<float(__cdecl*)(void)> GetBulletShellAnimationRateOne;
injector::hook_back<float(__cdecl*)(void)> GetBulletShellAnimationRateTwo;
injector::hook_back<float(__cdecl*)(void)> GetBulletShellAnimationRateThree;
injector::hook_back<float(__cdecl*)(void)> GetBulletShellAnimationRateFour;

bool once = true;

void PatchWater()
{
	DWORD OldProtect, Back, dwRelAddr;
	float* WaterAnimationSpeedPtr = GetWaterAnimationSpeedPointer();

	VirtualProtect(WaterAnimationSpeedPtr, 0x04, PAGE_EXECUTE_READWRITE, &OldProtect);
	
	*WaterAnimationSpeedPtr = 0.0166665;

	VirtualProtect(WaterAnimationSpeedPtr, 0x04, OldProtect, &Back);
}

float __cdecl GetHalvedAnimationRate_Hook()
{
	return GetFogAnimationRate.fun() / 2;
}

float __cdecl GetDoubledAnimationRate_Hook()
{
	return GetFogAnimationRate.fun() * 2;
}

void PatchSixtyFPS()
{
	auto pattern = hook::pattern("E8 0B E7 FB FF");
	GetFogAnimationRate.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetHalvedAnimationRate_Hook, true).get();

	pattern = hook::pattern("E8 7F 69 F5 FF");
	GetBulletShellAnimationRateOne.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetDoubledAnimationRate_Hook, true).get();

	pattern = hook::pattern("E8 36 69 F5 FF");
	GetBulletShellAnimationRateTwo.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetDoubledAnimationRate_Hook, true).get();
	
	pattern = hook::pattern("E8 19 69 F5 FF");
	GetBulletShellAnimationRateThree.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetDoubledAnimationRate_Hook, true).get();

	pattern = hook::pattern("E8 08 69 F5 FF");
	GetBulletShellAnimationRateFour.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetDoubledAnimationRate_Hook, true).get();
	
	PatchWater();
}
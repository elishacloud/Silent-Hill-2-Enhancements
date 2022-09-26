#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\calling.hpp"
#include "External\injector\include\injector\hooking.hpp"
#include "External\injector\include\injector\utility.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "Logging\Logging.h"
#include "Patches.h"
#include "Common/Utils.h"

injector::hook_back<float(__cdecl*)(void)> GetFogAnimationRate;
injector::hook_back<float(__cdecl*)(void)> GetBulletShellAnimationRateOne;
injector::hook_back<float(__cdecl*)(void)> GetBulletShellAnimationRateTwo;
injector::hook_back<float(__cdecl*)(void)> GetBulletShellAnimationRateThree;
injector::hook_back<float(__cdecl*)(void)> GetBulletShellAnimationRateFour;

bool once = true;

void PatchWater()
{

	float* WaterAnimationSpeedPtr = GetWaterAnimationSpeedPointer();
	float NewValue = 0.0166665;

	UpdateMemoryAddress(WaterAnimationSpeedPtr, &NewValue, sizeof(float));
}

void PatchFlashlightOnSpeed()
{
	int16_t* FlashlightOnSpeedPtr = GetFlashlightOnSpeedPointer();
	int16_t NewValue = 0x78;

	UpdateMemoryAddress(FlashlightOnSpeedPtr, &NewValue, sizeof(int16_t));
}

void PatchLowHealthIndicatorFlash()
{
	float* LowHealthIndicatorSpeedPtr = GetLowHealthIndicatorFlashSpeedPointer();
	float NewValue = 0.0166665;
	
	UpdateMemoryAddress(LowHealthIndicatorSpeedPtr, &NewValue, sizeof(float));
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
	//TODO patterns for 1.1 and DC
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

	PatchFlashlightOnSpeed();

	PatchLowHealthIndicatorFlash();
}
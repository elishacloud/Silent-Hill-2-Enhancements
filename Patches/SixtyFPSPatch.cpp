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

void PatchStaircaseFlamesLighting()
{
	float* StaircaseFlamesLightingPtr = GetStaircaseFlamesLightingPointer();
	float NewValue = 0.00027777222565;

	UpdateMemoryAddress(StaircaseFlamesLightingPtr, &NewValue, sizeof(float));
}

void PatchWaterLevelSpeed()
{
	float* LoweringStepsPtr = GetWaterLevelLoweringStepsPointer();
	float* RisingStepsPtr = GetWaterLevelRisingStepsPointer();

	float NewLowValue = 20.;
	float NewRiseValue = 5.333333492;

	UpdateMemoryAddress(LoweringStepsPtr, &NewLowValue, sizeof(float));
	UpdateMemoryAddress(RisingStepsPtr, &NewRiseValue, sizeof(float));
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
	Logging::Log() << "Fixing 60 FPS...";

	constexpr BYTE FogAnimationRateSearchBytes[]{ 0x83, 0xC4, 0x04, 0x84, 0xDB, 0x5B, 0x8A, 0x54, 0x24, 0x0C };
	DWORD* AnimationRate = (DWORD*)SearchAndGetAddresses(0x4890D0, 0x489370, 0x489580, FogAnimationRateSearchBytes, sizeof(FogAnimationRateSearchBytes), 0x50);
	if (AnimationRate)
	{
		GetFogAnimationRate.fun = injector::MakeCALL(AnimationRate, GetHalvedAnimationRate_Hook, true).get();
	}
	else 
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Animation Rate Function address!";
	}
	

	constexpr BYTE BulletAnimationOneSearchBytes[]{ 0x88, 0x8C, 0x24, 0xB6, 0x00, 0x00, 0x00, 0x89 };
	DWORD* BulletAnimationOne = (DWORD*)SearchAndGetAddresses(0x4F0E12, 0x4F10C2, 0x4F0982, BulletAnimationOneSearchBytes, sizeof(BulletAnimationOneSearchBytes), 0xE3);
	if (BulletAnimationOne)
	{
		GetBulletShellAnimationRateOne.fun = injector::MakeCALL(BulletAnimationOne, GetDoubledAnimationRate_Hook, true).get();
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Bullet Animation Rate One Function address!";
	}

	constexpr BYTE BulletAnimationTwoSearchBytes[]{ 0xD8, 0x45, 0x08, 0xD9, 0x5D, 0x08, 0xD9, 0x44, 0x24, 0x1C };
	DWORD* BulletAnimationTwo = (DWORD*)SearchAndGetAddresses(0x4F0EE5, 0x4F1195, 0x4F0A55, BulletAnimationTwoSearchBytes, sizeof(BulletAnimationTwoSearchBytes), -0x39);
	if (BulletAnimationTwo)
	{
		GetBulletShellAnimationRateTwo.fun = injector::MakeCALL(BulletAnimationTwo, GetDoubledAnimationRate_Hook, true).get();
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Bullet Animation Rate Two Function address!";
	}
	
	constexpr BYTE BulletAnimationThreeSearchBytes[]{ 0xD8, 0x45, 0x08, 0xD9, 0x5D, 0x08, 0xD9, 0x44, 0x24, 0x1C };
	DWORD* BulletAnimationThree = (DWORD*)SearchAndGetAddresses(0x4F0EE5, 0x4F1195, 0x4F0A55, BulletAnimationThreeSearchBytes, sizeof(BulletAnimationThreeSearchBytes), 0x2D);
	if (BulletAnimationThree)
	{
		GetBulletShellAnimationRateThree.fun = injector::MakeCALL(BulletAnimationThree, GetDoubledAnimationRate_Hook, true).get();
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Bullet Animation Rate Three Function address!";
	}

	constexpr BYTE BulletAnimationFourSearchBytes[]{ 0xD8, 0x45, 0x08, 0xD9, 0x5D, 0x08, 0xD9, 0x44, 0x24, 0x1C };
	DWORD* BulletAnimationFour = (DWORD*)SearchAndGetAddresses(0x4F0EE5, 0x4F1195, 0x4F0A55, BulletAnimationFourSearchBytes, sizeof(BulletAnimationFourSearchBytes), 0x3E);
	if (BulletAnimationFour)
	{
		GetBulletShellAnimationRateFour.fun = injector::MakeCALL(BulletAnimationFour, GetDoubledAnimationRate_Hook, true).get();
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Bullet Animation Rate Four Function address!";
	}
	
	PatchWater();

	PatchFlashlightOnSpeed();

	PatchLowHealthIndicatorFlash();

	PatchStaircaseFlamesLighting();

	PatchWaterLevelSpeed();
}
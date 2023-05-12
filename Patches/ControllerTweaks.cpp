/**
* Copyright (C) 2020 Adrian Zdanowicz
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
#define _USE_MATH_DEFINES // For cmath
#include <Windows.h>
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"
#include "External/Hooking.Patterns/Hooking.Patterns.h"
#include "InputTweaks.h"
#include "Wrappers\d3d8\Overlay.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

struct AnalogState
{
	short X, Y;
	float angle, length;
};

struct GamePadState // Very incomplete
{
	AnalogState m_leftStick;
	AnalogState m_rightStick;
};

static_assert( sizeof(AnalogState) == 0xC, "Wrong size: AnalogState" );

DIJOYSTATE2* dinputJoyState;
bool OverriddenKeyboard = false;
bool OverriddenRunOption = false;
const int StickTolerance = 13000;

static void (*orgProcessDInputData)(GamePadState*);
void ProcessDInputData_Hook(GamePadState* state)
{
	DIJOYSTATE2& joystickState = *dinputJoyState;

	if (DPadMovementFix)
	{
		static bool IsAssigningPOVButton = false;		// Helps prevent the case where the selection moves during POV button assignement
		const DWORD povAngle = joystickState.rgdwPOV[0];
		const bool centered = (LOWORD(povAngle) == 0xFFFF);
		if ( !centered )
		{
			const bool WaitingInputAssignment = (GetEventIndex() == EVENT_OPTION_FMV && GetInputAssignmentFlag() == 1);		// In the Options (includes Control Options) screen and game is currently awaiting an input assignment
			const bool IsNotIngame = (GetEventIndex() != EVENT_IN_GAME || GetMenuEvent() == 7 || (GetEventIndex() == EVENT_IN_GAME && IsInFullScreenImageEvent()));		// Not currently in-game, so in a menu || Within any selection on the main menu screen || In-game but a text prompt is currently on-screen.

			if ((DPadMovementFix == DPAD_MOVEMENT_MODE) ||
				(DPadMovementFix == DPAD_HYBRID_MODE && IsNotIngame && !WaitingInputAssignment && !IsAssigningPOVButton))
			{
				// Override analog values with DPad values
				const double angleRadians = static_cast<double>(povAngle) * M_PI / (180.0 * 100.0);

				joystickState.lX = static_cast<LONG>(std::sin(angleRadians) * 32767.0);
				joystickState.lY = static_cast<LONG>(std::cos(angleRadians) * -32767.0);
			}
			else
			{
				// Check if waiting for input assignment
				if (WaitingInputAssignment)
				{
					IsAssigningPOVButton = true;
				}

				// Override button values with DPad values
				if ((povAngle > 31500 && povAngle <= 36000) || (povAngle >= 0 && povAngle <= 4500))
				{
					joystickState.rgbButtons[28] = 0x80;	// The high-order bit of the byte is set if the corresponding button is down
				}
				else if (povAngle > 4500 && povAngle <= 13500)
				{
					joystickState.rgbButtons[29] = 0x80;
				}
				else if (povAngle > 13500 && povAngle <= 22500)
				{
					joystickState.rgbButtons[30] = 0x80;
				}
				else if (povAngle > 22500 && povAngle <= 31500)
				{
					joystickState.rgbButtons[31] = 0x80;
				}
			}
		}
		else
		{
			// Reset flag if not assigning it
			IsAssigningPOVButton = false;
		}
	}

	// Input Tweaks
	if (EnableEnhancedMouse && 
		(((std::abs(joystickState.lX) < StickTolerance) && (std::abs(joystickState.lY) < StickTolerance)) || !IsControllerConnected) &&
		(GetEnableInput() == 0xFFFFFFFF || InputTweaksRef.ElevatorFix()))
	{
		float MouseX = InputTweaksRef.GetMouseAnalogX();

		// Mouse turning
		if (GetControlType() == ROTATIONAL_CONTROL)
		{
			joystickState.lX = static_cast<LONG>(MouseX * 32767.0);
		}

		// Boat stage and search view movement fix
		if (((GetBoatFlag() == 0x01 && GetRoomID() == 0x0E) || GetSearchViewFlag() == 0x06))
		{
			if (GetRunOption() == OPT_ANALOG && EnableToggleSprint)
			{
				*GetRunOptionPointer() = OPT_WALK;
				OverriddenRunOption = true;
			}

			joystickState.lY = static_cast<LONG>(InputTweaksRef.GetForwardAnalog() * 32767.0);
			joystickState.lX = static_cast<LONG>(InputTweaksRef.GetTurningAnalog() * 32767.0);

			if (GetControlType() == ROTATIONAL_CONTROL && MouseX != 0)
			{
				joystickState.lX = static_cast<LONG>(MouseX * 32767.0);
			}

			OverriddenKeyboard = true;
		}
		else if (OverriddenKeyboard) // right after dismounting the boat or exiting search view, clear the analog stick
		{
			joystickState.lY = 0;
			joystickState.lX = 0;

			OverriddenKeyboard = false;
			InputTweaksRef.ClearOverrideSprint();

			if (OverriddenRunOption)
			{
				*GetRunOptionPointer() = OPT_ANALOG;
				OverriddenRunOption = false;
			}
		}
	}

	// Full screen images mouse movement fix
	if (EnableEnhancedMouse &&
		(((std::abs(joystickState.lX) < StickTolerance) && (std::abs(joystickState.lY) < StickTolerance)) || !IsControllerConnected) &&
		GetFullscreenImageEvent() == 0x02)
	{
		joystickState.lX = static_cast<LONG>(0);
	}

	// Populate debug values
	ControllerConnectedFlag = IsControllerConnected;
	JoystickX = joystickState.lX;
	JoystickY = joystickState.lY;

	// Populate right stick with data
	switch (RestoreSearchCamMovement)
	{
	case 1: // XRot/YRot
		state->m_rightStick.X = static_cast<short>(joystickState.lRx);
		state->m_rightStick.Y = static_cast<short>(joystickState.lRy);
		break;
	case 2: // Z axis/ZRot
		state->m_rightStick.X = static_cast<short>(joystickState.lZ);
		state->m_rightStick.Y = static_cast<short>(joystickState.lRz);
		break;
	case 3: // ZRot/Z axis
		state->m_rightStick.X = static_cast<short>(joystickState.lRz);
		state->m_rightStick.Y = static_cast<short>(joystickState.lZ);
		break;
	default:
		break;
	}

	orgProcessDInputData(state);
}

void __stdcall GetDeviceState_Hook(IDirectInputDevice8A* device)
{
	DIJOYSTATE2& joystickState = *dinputJoyState;
	bool success = false;
	if (SUCCEEDED(device->Acquire()))
	{
		if(SUCCEEDED(device->GetDeviceState(sizeof(joystickState), &joystickState)))
		{
			success = true;
		}
	}

	if (!success)
	{
		memset(&joystickState, 0, sizeof(joystickState));
		std::fill(std::begin(joystickState.rgdwPOV), std::end(joystickState.rgdwPOV), DWORD(-1));
	}
}

bool* usingGamepad;
int* moveDirection2;
bool* strafingLeft;
bool* strafingRight;

bool isMovingAround()
{
	return *usingGamepad || *strafingLeft || *strafingRight;
}

static BOOL (*orgUsingSearchCamera)();
BOOL UsingSearchCamera_SeparateAnalogs()
{
	return isMovingAround() ? FALSE : orgUsingSearchCamera();
}

// FALSE - allow search camera
// TRUE - do not allow
BOOL StartSearchCamera_Hook()
{
	if ( isMovingAround() ) return FALSE;

	return *moveDirection2 != 0 && *moveDirection2 != 3;
}

void PatchControllerTweaks()
{
	using namespace hook;

	DWORD PollDInputDevicesAddr;
	{
		auto PollDInputDevicesPattern = pattern( "33 DB 3B C3 74 33" ).count(1); // 0x45893B
		if (PollDInputDevicesPattern.size() != 1)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
		PollDInputDevicesAddr = reinterpret_cast<DWORD>(PollDInputDevicesPattern.get_first( -0xB ));
	}

	dinputJoyState = *(DIJOYSTATE2**)(PollDInputDevicesAddr + 0x5A + 1);

	const DWORD HandleDInputAddr = PollDInputDevicesAddr + 0x159;

	// Shared
	int32_t jmpAddress = 0;
	memcpy( &jmpAddress, (void*)(HandleDInputAddr + 1), sizeof(jmpAddress) );

	orgProcessDInputData = decltype(orgProcessDInputData)(jmpAddress + HandleDInputAddr + 5);
	WriteCalltoMemory((BYTE*)HandleDInputAddr, ProcessDInputData_Hook);

	// Default to centered POV hats, as the game will never write to joy state if controller is not there - but it will read from it
	std::fill(std::begin(dinputJoyState->rgdwPOV), std::end(dinputJoyState->rgdwPOV), DWORD(-1));

	if (DPadMovementFix)
	{
		// If controller is unplugged while playing, clear gamepad input
		// push eax
		// call GetDeviceState_Hook
		// jmp loc_4589CA
		DWORD GetDeviceState_Addr = PollDInputDevicesAddr + 0x7F;

		BYTE pushEax[] = { 0x50 };
		UpdateMemoryAddress((void*)GetDeviceState_Addr, pushEax, sizeof(pushEax)); GetDeviceState_Addr += 1;
		WriteCalltoMemory((BYTE*)GetDeviceState_Addr, GetDeviceState_Hook); GetDeviceState_Addr += 5;

		BYTE jmpLoc[] = { 0xEB, 0x13 };
		UpdateMemoryAddress((void*)GetDeviceState_Addr, jmpLoc, sizeof(jmpLoc));
	}

	if (RestoreSearchCamMovement != 0)
	{
		// Map right stick to search camera
		{
			auto UsingGamepadPattern = pattern( "88 15 ? ? ? ? 75 09" ).count(1); // 0x52EA69
			if (UsingGamepadPattern.size() != 1)
			{
				Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
				return;
			}
			usingGamepad = *UsingGamepadPattern.get_first<bool*>( 2 );
		}
		
		{
			auto MoveDirectionPattern = pattern( "A1 ? ? ? ? 83 F8 08 7C 27" ).count(1); // 0x5568C5
			if (MoveDirectionPattern.size() != 1)
			{
				Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
				return;
			}	
			moveDirection2 = *MoveDirectionPattern.get_first<int*>( 1 );
		}

		{
			auto StrafingPattern = pattern( "A2 ? ? ? ? 88 0D ? ? ? ? 74 0F" ).count(1); // 0x52EC74
			if (StrafingPattern.size() != 1)
			{
				Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
				return;
			}
			strafingLeft = *StrafingPattern.get_first<bool*>( 1 );
			strafingRight = *StrafingPattern.get_first<bool*>( 5 + 2 );
		}

		{
			auto RightStickXPattern = pattern( "84 C0 7C 51 0F BE C8" ).count(1); // 0x52E93B
			auto RightStickYPattern = pattern( "D9 1D ? ? ? ? 6A 01 6A 00" ).count(1); // 0x52E9DD
			if (RightStickXPattern.size() != 1 || RightStickYPattern.size() != 1)
			{
				Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
				return;
			}

			float* rightStickXFloat = *RightStickXPattern.get_first<float*>( -6 + 2 );
			float* rightStickYFloat = *RightStickYPattern.get_first<float*>( 2 );

			auto StickPattern1 = pattern( "D8 1D ? ? ? ? DF E0 F6 C4 44 7A 6F" ).count(1); // 0x535CD5
			auto StickPattern2 = pattern( "D9 05 ? ? ? ? DF E0 F6 C4 05 7A 0E" ).count(1); // 0x535D69
			if (StickPattern1.size() != 1 || StickPattern2.size() != 1)
			{
				Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
				return;
			}

			auto StickPattern1Match = StickPattern1.get_one();
			auto StickPattern2Match = StickPattern2.get_one();

			UpdateMemoryAddress(StickPattern1Match.get<void>( -0x6 + 2 ), &rightStickXFloat, sizeof(rightStickXFloat) );
			UpdateMemoryAddress(StickPattern2Match.get<void>( -0x18 + 2 ), &rightStickXFloat, sizeof(rightStickXFloat) );

			UpdateMemoryAddress(StickPattern1Match.get<void>( 0xD + 2 ), &rightStickYFloat, sizeof(rightStickYFloat) );
			UpdateMemoryAddress(StickPattern2Match.get<void>( 2 ), &rightStickYFloat, sizeof(rightStickYFloat) );
		}

		// Allow for simultaneous walking and moving camera
		{
			// Rotational walk
			auto RotationalWalk1 = pattern( "85 C0 74 16 A1 ? ? ? ? 85 C0 0F 84 ? ? ? ? 83 F8 03 0F 84 ? ? ? ? 68" ).count(1); // 0x54E4B7 (-5)
			auto RotationalWalk2 = pattern( "74 35 E8 ? ? ? ? 85 C0" ).count(1); // 0x54F13D (+2)
			auto RotationalWalk3 = pattern( "75 7A E8 ? ? ? ? 85 C0" ).count(1); // 0x54F207 (+2)
			if (RotationalWalk1.size() != 1 || RotationalWalk2.size() != 1 || RotationalWalk3.size() != 1)
			{
				Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
				return;
			}

			const DWORD srcAddr = reinterpret_cast<DWORD>( RotationalWalk1.get_first( -5 ));
			memcpy( &jmpAddress, (void*)(srcAddr + 1), sizeof(jmpAddress) );
			orgUsingSearchCamera = decltype(orgUsingSearchCamera)(jmpAddress + srcAddr + 5);

			
			WriteCalltoMemory((BYTE*)srcAddr, UsingSearchCamera_SeparateAnalogs);
			WriteCalltoMemory(RotationalWalk2.get_first<BYTE>( 2 ), UsingSearchCamera_SeparateAnalogs);
			WriteCalltoMemory(RotationalWalk3.get_first<BYTE>( 2 ), UsingSearchCamera_SeparateAnalogs);

			// Directional walk
			auto DirectionalWalk1 = pattern( "85 C0 74 16 A1 ? ? ? ? 85 C0 0F 84 ? ? ? ? 83 F8 03 0F 84 ? ? ? ? 8D 44 24 18" ).count(1); // 0x547FC0 (-5)
			auto DirectionalWalk2 = pattern( "E8 ? ? ? ? 85 C0 75 2B 39 35" ).count(1); // 0x548D70
			auto DirectionalWalk3 = pattern( "75 7B E8" ).count(1); // 0x548EC4 (+2)
			if (DirectionalWalk1.size() != 1 || DirectionalWalk2.size() != 1)
			{
				Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
				return;
			}

			auto DirectionalWalk1Match = DirectionalWalk1.get_one();
			auto DirectionalWalk2Match = DirectionalWalk2.get_one();
			auto DirectionalWalk3Match = DirectionalWalk3.get_one();
			
			WriteCalltoMemory(DirectionalWalk1Match.get<BYTE>( -5 ), UsingSearchCamera_SeparateAnalogs);
			WriteCalltoMemory(DirectionalWalk1Match.get<BYTE>( 0x3A ), UsingSearchCamera_SeparateAnalogs);
			WriteCalltoMemory(DirectionalWalk1Match.get<BYTE>( 0xBA ), UsingSearchCamera_SeparateAnalogs);
			WriteCalltoMemory(DirectionalWalk2Match.get<BYTE>( 0 ), UsingSearchCamera_SeparateAnalogs);
			WriteCalltoMemory(DirectionalWalk2Match.get<BYTE>( 0x42 ), UsingSearchCamera_SeparateAnalogs);
			WriteCalltoMemory(DirectionalWalk2Match.get<BYTE>( 0x87 ), UsingSearchCamera_SeparateAnalogs);
			WriteCalltoMemory(DirectionalWalk3Match.get<BYTE>( 2 ), UsingSearchCamera_SeparateAnalogs);
		}

		{
			auto UpdateSearchCameraPattern = pattern( "A1 ? ? ? ? 85 C0 74 09 83 F8 03" ).count(1); // 0x535CBD
			if (UpdateSearchCameraPattern.size() != 1)
			{
				Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
				return;
			}

			auto UpdateSearchCameraMatch = UpdateSearchCameraPattern.get_first<BYTE>();

			const BYTE nops[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
			WriteCalltoMemory(UpdateSearchCameraMatch, StartSearchCamera_Hook);
			UpdateMemoryAddress(UpdateSearchCameraMatch + 5 + 2, nops, sizeof(nops) );
		}

		// Southpaw option
		if (Southpaw)
		{
			auto SouthpawPattern = pattern( "C7 05 ? ? ? ? ? ? ? ? C3 90 83 EC 24" ).count(1); // 0x45BCA4
			if (SouthpawPattern.size() == 1) // Don't treat failure as fatal, since it's a very minor change
			{
				const BOOL option = TRUE;
				UpdateMemoryAddress(SouthpawPattern.get_first( 6 ), &option, sizeof(option));
			}
		}
	}
}

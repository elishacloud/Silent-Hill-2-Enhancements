/**
* Copyright (C) 2022 mercury501
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

#include "InputTweaks.h"

InputTweaks InputTweaksRef;
KeyBindsHandler KeyBinds;

BYTE* KeyBindsAddr = nullptr;

const int AnalogThreshold = 15;
const int InputDebounce = 50;
const int PauseMenuMouseThreshold = 15;
const float AnalogHalfTilt = 0.5;
const float AnalogFullTilt = 1;
const float FloatTolerance = 0.10;

bool once = false;

auto LastMouseXRead = std::chrono::system_clock::now();
auto LastMouseYRead = std::chrono::system_clock::now();
auto LastWeaponSwap = std::chrono::system_clock::now();
auto LastMousePauseChange = std::chrono::system_clock::now();
int LastMouseVerticalPos = 0xF0;
int LastMouseHorizontalPos = 0xFC;
int MouseXAxis = 0;
int MouseYAxis = 0;
long int MouseWheel = 0;

AnalogStick VirtualRightStick;

bool PauseMenuVerticalChanged = false;
bool PauseMenuHorizontalChanged = false;
bool CheckKeyBindsFlag = false;

bool SetLMButton = false;
bool SetLeftKey = false;
bool SetRightKey = false;
bool SetUpKey = false;
bool SetDownKey = false;
bool CleanKeys = false;

Input Sprint;
Input RMB;
Input DebugCombo;
Input InfoCombo;

int ForwardBackwardsAxis = 0;
int LeftRightAxis = 0;

injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerLXAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerLYAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerRXAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerRYAxis;
injector::hook_back<void(__cdecl*)(void)> orgUpdateMousePosition;

BYTE* AnalogStringOne;
BYTE* AnalogStringTwo;
BYTE* AnalogStringThree;

int8_t GetControllerLXAxis_Hook(DWORD* arg)
{
	// Alt Tab Rotating Fix
	if (GetForegroundWindow() != GameWindowHandle)
	{
		SetLMButton = false;
		RMB.State = false;
		return 0;
	}
		
	return orgGetControllerLXAxis.fun(arg);
}

int8_t GetControllerLYAxis_Hook(DWORD* arg)
{
	return orgGetControllerLYAxis.fun(arg);
}

int8_t GetControllerRXAxis_Hook(DWORD* arg)
{
	if (GetSearchViewFlag() == 0x6 && !VirtualRightStick.IsCentered())
	{
		return VirtualRightStick.XAxis;
	}
	else
	{
		return orgGetControllerRXAxis.fun(arg);
	}
}

int8_t GetControllerRYAxis_Hook(DWORD* arg)
{
	if (GetSearchViewFlag() == 0x6 && !VirtualRightStick.IsCentered())
	{
		return VirtualRightStick.YAxis;
	}
	else
	{
		return orgGetControllerRYAxis.fun(arg);
	}
}

void UpdateMousePosition_Hook()
{
	orgUpdateMousePosition.fun();

	if (GetEventIndex() == EVENT_PAUSE_MENU || GetEventIndex() == EVENT_MEMO_LIST)
	{
		auto Now = std::chrono::system_clock::now();
		auto DeltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastMousePauseChange);

		// Vertical navigation
		if (PauseMenuVerticalChanged && DeltaMs.count() > InputDebounce)
		{
			LastMouseVerticalPos = GetMouseVerticalPosition();

			PauseMenuVerticalChanged = false;
		}

		if (std::abs(GetMouseVerticalPosition() - LastMouseVerticalPos) > PauseMenuMouseThreshold && !PauseMenuVerticalChanged)
		{
			if (GetMouseVerticalPosition() > LastMouseVerticalPos && (GetPauseMenuButtonIndex() != 0x04 || GetEventIndex() != EVENT_PAUSE_MENU))
			{
				SetDownKey = true;
			}
			else if (GetMouseVerticalPosition() < LastMouseVerticalPos && (GetPauseMenuButtonIndex() != 0x00 || GetEventIndex() != EVENT_PAUSE_MENU))
			{
				SetUpKey = true;
			} 

			LastMousePauseChange = Now;
			PauseMenuVerticalChanged = true;
			
		}
		else
		{
			LastMouseVerticalPos = GetMouseVerticalPosition();
		}

		// Horizontal navigation
		if (PauseMenuHorizontalChanged && DeltaMs.count() > InputDebounce)
		{
			LastMouseHorizontalPos = GetMouseHorizontalPosition();

			PauseMenuHorizontalChanged = false;
		}

		if (std::abs(GetMouseHorizontalPosition() - LastMouseHorizontalPos) > PauseMenuMouseThreshold && !PauseMenuHorizontalChanged)
		{
			if (GetMouseHorizontalPosition() > LastMouseHorizontalPos && GetPauseMenuQuitIndex() == 0)
			{
				SetRightKey = true;
			}
			else if (GetMouseHorizontalPosition() < LastMouseHorizontalPos && GetPauseMenuQuitIndex() == 1)
			{
				SetLeftKey = true;
			}
			
			LastMousePauseChange = Now;
			PauseMenuHorizontalChanged = true;
		}
		else
		{
			LastMouseHorizontalPos = GetMouseHorizontalPosition();
		}

		// Reset mouse position to avoid edges
		if (GetMouseHorizontalPosition() > 0x15F || GetMouseHorizontalPosition() < 0x10)
		{
			*GetMouseHorizontalPositionPointer() = 0xFC;
			LastMouseHorizontalPos = 0xFC;
		}
		if (GetMouseVerticalPosition() > 0x1D0 || GetMouseVerticalPosition() < 0x10)
		{
			*GetMouseVerticalPositionPointer() = 0xF0;
			LastMouseVerticalPos = 0xF0;
		}
	}
}

void InputTweaks::TweakGetDeviceState(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbData, LPVOID lpvData)
{

	if (GetEventIndex() == EVENT_OPTION_FMV)
		CheckKeyBindsFlag = true;
	if (GetEventIndex() != EVENT_OPTION_FMV && CheckKeyBindsFlag)
	{
		CheckKeyBindsFlag = false;
		CheckNumberKeyBinds();
	}

	// For keyboard
	if (ProxyInterface == KeyboardInterfaceAddress)
	{
		auto Now = std::chrono::system_clock::now();
		auto DeltaMsWeaponSwap = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastWeaponSwap);

		// Save Keyboard Data
		KeyboardData = (BYTE*)lpvData;

		// Ignore Alt + Enter combo
		if ((IsKeyPressed(DIK_LMENU) || IsKeyPressed(DIK_RMENU)) && IsKeyPressed(DIK_RETURN) && 
			DynamicResolution && ScreenMode != EXCLUSIVE_FULLSCREEN)
		{
			ClearKey(DIK_LMENU);
			ClearKey(DIK_RMENU);
			ClearKey(DIK_RETURN);
			Logging::LogDebug() << __FUNCTION__ << " Detected ALT + ENTER...";
		}

		// Ignore Ctrl + G combo
		if (IsKeyPressed(DIK_LCONTROL) && IsKeyPressed(DIK_G) && EnableDebugOverlay)
		{
			ClearKey(DIK_LCONTROL);
			ClearKey(DIK_G);
			DebugCombo.State = true;
			Logging::LogDebug() << __FUNCTION__ << " Detected CTRL + G...";
		}
		else
		{
			DebugCombo.State = false;
		}

		// Ignore Ctrl + I combo
		if (IsKeyPressed(DIK_LCONTROL) && IsKeyPressed(DIK_I) && EnableInfoOverlay)
		{
			ClearKey(DIK_LCONTROL);
			ClearKey(DIK_I);
			InfoCombo.State = true;
			Logging::LogDebug() << __FUNCTION__ << " Detected CTRL + I...";
		}
		else
		{
			InfoCombo.State = false;
		}

		// Check if Debug Combo is held down
		DebugCombo.UpdateHolding();

		// Check if Info Combo is held down
		InfoCombo.UpdateHolding();

		// Check if RMB is held down
		RMB.UpdateHolding();

		// Check if Sprint is held down
		Sprint.UpdateHoldingByValue(IsKeyPressed(KeyBinds.GetKeyBind(KEY_RUN)));

		// Check for toggle sprint
		if (EnableToggleSprint && GetRunOption() == OPT_ANALOG)
		{
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_RUN)) && !Sprint.Holding && GetEventIndex() == EVENT_IN_GAME)
			{
				Sprint.ToggleState();
			}
		}

		// Check for forward/backwards movement to fix boat stage
		ForwardBackwardsAxis = 0;
		LeftRightAxis = 0;

		if (GetBoatFlag() == 0x01 && GetRoomID() == 0x0E)
		{
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_MOVE_FORWARDS)))
			{
				ForwardBackwardsAxis += 1;
				ClearKey(KeyBinds.GetKeyBind(KEY_MOVE_FORWARDS));
			}
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_MOVE_BACKWARDS)))
			{
				ForwardBackwardsAxis -= 1;
				ClearKey(KeyBinds.GetKeyBind(KEY_MOVE_BACKWARDS));
			}

			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_TURN_LEFT)))
			{
				LeftRightAxis += 1;
				ClearKey(KeyBinds.GetKeyBind(KEY_TURN_LEFT));
			}
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_TURN_RIGHT)))
			{
				LeftRightAxis -= 1;
				ClearKey(KeyBinds.GetKeyBind(KEY_TURN_RIGHT));
			}
		}

		// Activate Overlays
		if (DebugCombo.State && !DebugCombo.Holding && EnableDebugOverlay)
			ShowDebugOverlay = !ShowDebugOverlay;
		if (InfoCombo.State && !InfoCombo.Holding && EnableInfoOverlay)
			ShowInfoOverlay = !ShowInfoOverlay;

		// Inject Key Presses
		if (EnableEnhancedMouse && GetEventIndex() != EVENT_MAP && GetEventIndex() != EVENT_INVENTORY && GetEventIndex() != EVENT_OPTION_FMV && 
			GetEventIndex() != EVENT_FMV && GetCutsceneID() == 0x0) 
		{
			if (SetLMButton)
				SetKey(KeyBinds.GetKeyBind(KEY_ACTION));
			if (RMB.State)
			{
				if (SetRMBAimFunction())
				{
					SetKey(KeyBinds.GetKeyBind(KEY_READY_WEAPON));
				}
				else
				{
					if (!RMB.Holding) {
						SetKey(KeyBinds.GetKeyBind(KEY_CANCEL));
					}
				}
			}
		}

		// Strafe keys become movement keys on menus
		if (GetEventIndex() != EVENT_IN_GAME || (GetEventIndex() == EVENT_IN_GAME && IsInFullScreenImageEvent()))
		{
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_STRAFE_LEFT)))
				SetKey(KeyBinds.GetKeyBind(KEY_TURN_LEFT));
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_STRAFE_RIGHT)))
				SetKey(KeyBinds.GetKeyBind(KEY_TURN_RIGHT));
		}

		if (SetUpKey)
		{
			SetKey(KeyBinds.GetKeyBind(KEY_MOVE_FORWARDS));
			SetUpKey = false;
		}

		if (SetDownKey)
		{
			SetKey(KeyBinds.GetKeyBind(KEY_MOVE_BACKWARDS));
			SetDownKey = false;
		}

		if (SetLeftKey)
		{
			SetKey(KeyBinds.GetKeyBind(KEY_TURN_LEFT));
			SetLeftKey = false; 
		}

		if (SetRightKey)
		{
			SetKey(KeyBinds.GetKeyBind(KEY_TURN_RIGHT));
			SetRightKey = false;
		}

		if (EnableToggleSprint && GetRunOption() == OPT_ANALOG)
		{
			ClearKey(KeyBinds.GetKeyBind(KEY_RUN));
		}

		if (EnableToggleSprint && Sprint.State && GetRunOption() == OPT_ANALOG && IsMovementPressed())
		{
			SetKey(KeyBinds.GetKeyBind(KEY_RUN));
		}

		// Mouse wheel weapon swapping
		if (EnableMouseWheelSwap && MouseWheel != 0 && DeltaMsWeaponSwap.count() > InputDebounce && GetEventIndex() == EVENT_IN_GAME)
		{
			if (MouseWheel > 0)
				SetKey(KeyBinds.GetKeyBind(KEY_NEXT_WEAPON));
			else
				SetKey(KeyBinds.GetKeyBind(KEY_PREV_WEAPON));
			
			LastWeaponSwap = Now;
			MouseWheel = 0;
		}

		if (GetForegroundWindow() != GameWindowHandle)
		{
			ClearKey(KeyBinds.GetKeyBind(KEY_ACTION));
			ClearKey(KeyBinds.GetKeyBind(KEY_READY_WEAPON));
		}

		if (MemoScreenFix && GetEventIndex() == EVENT_MEMO_LIST)
		{
			if (IsKeyPressed(DIK_UP))
				SetKey(KeyBinds.GetKeyBind(KEY_MOVE_FORWARDS));
			if (IsKeyPressed(DIK_DOWN))
				SetKey(KeyBinds.GetKeyBind(KEY_MOVE_BACKWARDS));
			if (IsKeyPressed(DIK_RETURN))
				SetKey(KeyBinds.GetKeyBind(KEY_ACTION));
		}

		// Fix for input sticking after alt+tab while holding aim and action
		if (CleanKeys)
		{
			ClearKey(KeyBinds.GetKeyBind(KEY_ACTION));
			ClearKey(KeyBinds.GetKeyBind(KEY_READY_WEAPON));
			CleanKeys = false;
		}

		// Clear Keyboard Data pointer
		KeyboardData = nullptr;
	}

	if (!once)
	{
		once = true;
		
		orgGetControllerLXAxis.fun = injector::MakeCALL(GetLeftAnalogXFunctionPointer(), GetControllerLXAxis_Hook, true).get();
		orgGetControllerLYAxis.fun = injector::MakeCALL(GetLeftAnalogYFunctionPointer(), GetControllerLYAxis_Hook, true).get();
		orgGetControllerRXAxis.fun = injector::MakeCALL(GetRightAnalogXFunctionPointer(), GetControllerRXAxis_Hook, true).get();
		orgGetControllerRYAxis.fun = injector::MakeCALL(GetRightAnalogYFunctionPointer(), GetControllerRYAxis_Hook, true).get();

		orgUpdateMousePosition.fun = injector::MakeCALL(GetUpdateMousePositionFunctionPointer(), UpdateMousePosition_Hook, true).get();

		CheckNumberKeyBinds();

		if (EnableToggleSprint)
		{
			if (!GetAnalogStringAddr()) return;

			// Change the Analog string (0x68) with Toggle (0x2F)
			UpdateMemoryAddress((void*)AnalogStringOne, "\x2F", 1);
			UpdateMemoryAddress((void*)AnalogStringTwo, "\x2F", 1);
			UpdateMemoryAddress((void*)AnalogStringThree, "\x2F", 1);
		}
	}

}

void InputTweaks::TweakGetDeviceData(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	// For mouse
	if (ProxyInterface == MouseInterfaceAddress)
	{
		// Save Mouse Data
		MouseData = rgdod;
		MouseDataSize = *pdwInOut;

		MouseXAxis = GetMouseRelXChange();
		ReadMouseButtons();

		if (GetSearchViewFlag() == 0x6)
		{
			VirtualRightStick.AddXValue(MouseXAxis);
			VirtualRightStick.AddYValue(GetMouseRelYChange());
		}
		else
		{
			VirtualRightStick.Recenter();
		}

		// Clear Mouse Data
		MouseData = nullptr;
		MouseDataSize = NULL;
	}
}

void InputTweaks::ClearKey(int KeyIndex)
{
	if (KeyIndex > 0x100 || !KeyboardData)
		return;

	KeyboardData[KeyIndex] = KEY_CLEAR;
}

void InputTweaks::SetKey(int KeyIndex)
{
	if (KeyIndex > 0x100 || !KeyboardData || KeyIndex == 0x0)
		return;

	KeyboardData[KeyIndex] = KEY_SET;
}

bool InputTweaks::IsKeyPressed(int KeyIndex)
{
	if (KeyIndex > 0x100 || !KeyboardData)
		return false;

	return KeyboardData[KeyIndex] == KEY_SET;
}

void InputTweaks::SetKeyboardInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface)
{
	KeyboardInterfaceAddress = ProxyInterface;
}

void InputTweaks::SetMouseInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface)
{
	MouseInterfaceAddress = ProxyInterface;
}

void InputTweaks::RemoveAddr(LPDIRECTINPUTDEVICE8A ProxyInterface)
{
	if (MouseInterfaceAddress == ProxyInterface)
		MouseInterfaceAddress = nullptr;
	if (KeyboardInterfaceAddress == ProxyInterface)
		KeyboardInterfaceAddress = nullptr;
}

int32_t InputTweaks::GetMouseRelXChange()
{
	auto Now = std::chrono::system_clock::now();
	auto DeltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastMouseXRead);
	LastMouseXRead = Now;

	if (DeltaMs.count() == 0)
		return MouseXAxis;

	int32_t AxisSum = 0;

	if (!MouseData || MouseDataSize == 0)
		return AxisSum;

	for (int i = 0; i < MouseDataSize; i++)
		if (MouseData[i].dwOfs == DIMOFS_X)
			AxisSum += (int32_t) MouseData[i].dwData;

	return AxisSum;
}

int32_t InputTweaks::GetMouseRelYChange()
{
	auto Now = std::chrono::system_clock::now();
	auto DeltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastMouseYRead);
	LastMouseYRead = Now;

	if (DeltaMs.count() == 0)
		return MouseYAxis;

	int32_t AxisSum = 0;

	if (!MouseData || MouseDataSize == 0)
		return AxisSum;

	for (int i = 0; i < MouseDataSize; i++)
		if (MouseData[i].dwOfs == DIMOFS_Y)
			AxisSum += (int32_t)MouseData[i].dwData;

	return AxisSum;
}

void InputTweaks::ReadMouseButtons()
{		
	if (!MouseData || GetForegroundWindow() != GameWindowHandle) return;

	for (int i = 0; i < MouseDataSize; i++)
		switch (MouseData[i].dwOfs)
		{
		case DIMOFS_BUTTON0:
		{
			SetLMButton = MouseData[i].dwData == KEY_SET;
			break;
		}
		case DIMOFS_BUTTON1:
		{
			RMB.State = MouseData[i].dwData == KEY_SET;
			break;
		}
		case DIMOFS_Z:
		{
			MouseWheel = MouseData[i].dwData;
		}

		default:
		{
			break;
		}

		}
		
}

float InputTweaks::GetMouseAnalogX()
{
	if (MouseXAxis == 0 || GetSearchViewFlag() == 0x06 || GetEventIndex() != EVENT_IN_GAME)
	{
		MouseXAxis = 0;
		return 0;
	}

	int TempAxis = MouseXAxis;
	MouseXAxis = 0;

	if (TempAxis > 0)
		return TempAxis > AnalogThreshold ? AnalogFullTilt : AnalogHalfTilt;

	return TempAxis < -AnalogThreshold ? -AnalogFullTilt : -AnalogHalfTilt;
}

float InputTweaks::GetForwardAnalog()
{
	return ForwardBackwardsAxis > 0 ? -1. : ForwardBackwardsAxis < 0 ? 1. : 0;
}

float InputTweaks::GetTurningAnalog()
{

	return LeftRightAxis > 0 ? -1. : LeftRightAxis < 0 ? 1. : 0;
}

void InputTweaks::ClearMouseInputs()
{
	SetLMButton = false;
	RMB.State = false;
	CleanKeys = true;
}

void InputTweaks::CheckNumberKeyBinds()
{
	BYTE* NumberKeyBinds = GetNumKeysWeaponBindStartPointer();
	BYTE* ActionKeyBinds = KeyBinds.GetKeyBindsPointer();
	boolean FoundNumber = false;

	// Iterate over Number Keybinds
	for (int i = 0; i < 0xA; i++)
	{
		FoundNumber = false;

		// Iterate over Action Keybinds
		for (int j = 0; j < 0x16; j++)
		{
			if (*(ActionKeyBinds + (j * 0x8)) == DefaultNumberKeyBinds[i])
			{
				FoundNumber = true;
				break;
			}
		}

		if (FoundNumber)
		{
			*(NumberKeyBinds + (i * 0x8)) = 0x00;
		}
		else
		{
			*(NumberKeyBinds + (i * 0x8)) = DefaultNumberKeyBinds[i];
		}
	}
}

bool InputTweaks::ElevatorFix()
{
	return (GetRoomID() == 0x46 && GetTalkShowHostState() == 0x01);
}

bool InputTweaks::HotelFix()
{
	return (GetRoomID() == 0xB8 && 
		(((std::abs(GetInGameCameraPosY() - (-840.)) < FloatTolerance) || (std::abs(GetInGameCameraPosY() - (-1350.)) < FloatTolerance))) &&
		IsInFullScreenImageEvent());
}

std::string InputTweaks::GetRightClickState()
{
	std::string Output = "Cancel";

	if (SetRMBAimFunction())
		Output = "Ready Weapon";

	if (!EnableEnhancedMouse)
		Output = "Not Enabled";

	return Output;
}

std::string InputTweaks::GetToggleSprintState()
{
	std::string Output = "Not Active";

	if (Sprint.State)
		Output = "Active";

	if (!EnableToggleSprint)
		Output = "Not Enabled";

	return Output;
}

bool InputTweaks::JamesVaultingBuildingsFix()
{
	return (GetRoomID() == 0x07 && std::abs(GetInGameCameraPosY() - (-3315.999)) < FloatTolerance);
}

bool InputTweaks::RosewaterParkFix()
{
	return (GetRoomID() == 0x08 && std::abs(GetInGameCameraPosY() - (150.)) < FloatTolerance && std::abs(GetJamesPosZ() - (78547.117)) < FloatTolerance);
}

bool InputTweaks::HospitalMonologueFix()
{
	return (GetRoomID() == 0x08 && std::abs(GetJamesPosZ() - (-6000.)) < FloatTolerance && IsInFullScreenImageEvent());
}

bool InputTweaks::FleshRoomFix()
{
	return (GetRoomID() == 0x79 || GetRoomID() == 0x8A || GetRoomID() == 0x8C);
}

bool InputTweaks::SetRMBAimFunction()
{
	return (GetEventIndex() == EVENT_IN_GAME &&
		(ElevatorFix() || (HotelFix())) || JamesVaultingBuildingsFix() || 
		RosewaterParkFix() || HospitalMonologueFix() || FleshRoomFix() ||
		!IsInFullScreenImageEvent());
}

bool InputTweaks::IsInFullScreenImageEvent()
{
	return GetFullscreenImageEvent() == 0x02;
}

bool InputTweaks::GetAnalogStringAddr()
{
	constexpr BYTE AnalogStringOneSearchBytes[]{ 0x68, 0x9A, 0x00, 0x00, 0x00, 0x56, 0x6A, 0x68 };
	BYTE *AnalogString = (BYTE*)SearchAndGetAddresses(0x461F63, 0x4621D5, 0x4621D5, AnalogStringOneSearchBytes, sizeof(AnalogStringOneSearchBytes), 0x07);

	if (!AnalogString)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Analog String One address!";
		return false;
	}
	else
	{
		AnalogStringOne = (BYTE*)((DWORD)AnalogString);
	}

	constexpr BYTE AnalogStringTwoSearchBytes[]{ 0x68, 0x98, 0x00, 0x00, 0x00, 0x81, 0xC7, 0x0C, 0x01, 0x00, 0x00, 0x57, 0x6A, 0x68 };
	AnalogString = (BYTE*)SearchAndGetAddresses(0x4621D1, 0x462433, 0x462433, AnalogStringTwoSearchBytes, sizeof(AnalogStringTwoSearchBytes), 0x0D);

	if (!AnalogString)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Analog String Two address!";
		return false;
	}
	else
	{
		AnalogStringTwo = (BYTE*)((DWORD)AnalogString);
	}

	constexpr BYTE AnalogStringThreeSearchBytes[]{ 0x00, 0x6A, 0x68 };
	AnalogString = (BYTE*)SearchAndGetAddresses(0x464465, 0x4646DE, 0x4648E6, AnalogStringThreeSearchBytes, sizeof(AnalogStringThreeSearchBytes), 0x02);

	if (!AnalogString)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Analog String Three address!";
		return false;
	}
	else
	{
		AnalogStringThree = (BYTE*)((DWORD)AnalogString);
	}

	return true;
}

bool InputTweaks::IsMovementPressed()
{
	return IsKeyPressed(KeyBinds.GetKeyBind(KEY_MOVE_FORWARDS)) || IsKeyPressed(KeyBinds.GetKeyBind(KEY_MOVE_BACKWARDS)) ||
		IsKeyPressed(KeyBinds.GetKeyBind(KEY_TURN_LEFT)) || IsKeyPressed(KeyBinds.GetKeyBind(KEY_TURN_RIGHT)) || 
		IsKeyPressed(KeyBinds.GetKeyBind(KEY_STRAFE_RIGHT)) || IsKeyPressed(KeyBinds.GetKeyBind(KEY_STRAFE_LEFT));
}

BYTE KeyBindsHandler::GetKeyBind(int KeyIndex)
{
	BYTE *pButton = GetKeyBindsPointer();

	if (pButton && *(pButton + (KeyIndex * 0x08)) == 0)
		Logging::Log() << "Null keybind, on index: " << KeyIndex;

	return (pButton) ? *(pButton + (KeyIndex * 0x08)) : 0;
}

BYTE* KeyBindsHandler::GetKeyBindsPointer()
{
	if (KeyBindsAddr)
	{
		return KeyBindsAddr;
	}

	// Get Turn Left Button address
	constexpr BYTE TurnLeftButtonSearchBytes[]{ 0x56, 0x8B, 0x74, 0x24, 0x08, 0x83, 0xFE, 0x16, 0x7D, 0x3F };
	BYTE *Binds = (BYTE*)ReadSearchedAddresses(0x5AEF90, 0x5AF8C0, 0x5AF1E0, TurnLeftButtonSearchBytes, sizeof(TurnLeftButtonSearchBytes), 0x1D);

	// Checking address pointer
	if (!Binds)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find KeyBinds address!";
		return nullptr;
	}

	KeyBindsAddr = (BYTE*)((DWORD)Binds);

	return KeyBindsAddr;
}
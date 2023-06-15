/**
* Copyright (C) 2023 mercury501
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
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "InputTweaks.h"

InputTweaks InputTweaksRef;
KeyBindsHandler KeyBinds;

BYTE* KeyBindsAddr = nullptr;

const int AnalogThreshold = 15;
const int InputDebounce = 50;
const float AnalogHalfTilt = 0.5f;
const float AnalogFullTilt = 1.0f;
const float FloatTolerance = 0.10f;

const int MemoEvenTop = 80;
const int MemoOddTop = 64;
const int MemoLeft = 50;
const int MemoHeight = 32;
const int MemoWidth = 580;

const int PauseMenuTop = 216;
const int PauseMenuLeft = 281;
const int PauseMenuHeight = 30;
const int PauseMenuWidth = 118;

const int QuitMenuTop = 245;
const int QuitMenuWidth = 58;

Hitboxes PauseMenu = Hitboxes(PauseMenuTop, PauseMenuLeft, PauseMenuHeight, PauseMenuWidth, 5, 1);
Hitboxes QuitMenu = Hitboxes(QuitMenuTop, PauseMenuLeft, PauseMenuHeight, QuitMenuWidth, 1, 2);
MemoHitboxes MemoMenu = MemoHitboxes(MemoEvenTop, MemoOddTop, MemoLeft, MemoHeight, MemoWidth);
bool EnteredPuzzle = false;

int LastFrameCursorHorizontalPos = 0;
int LastFrameCursorVerticalPos = 0;

auto LastMouseXRead = std::chrono::system_clock::now();
auto LastMouseYRead = std::chrono::system_clock::now();
auto LastWeaponSwap = std::chrono::system_clock::now();
auto LastMousePauseChange = std::chrono::system_clock::now();
int MouseXAxis = 0;
int MouseYAxis = 0;
long int MouseWheel = 0;

AnalogStick VirtualRightStick;

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
Input EscInput;
Input CancelInput;

int ForwardBackwardsAxis = 0;
int LeftRightAxis = 0;
bool OverrideSprint = false;

const int AutoHideCursorMs = 3 * 1000;
auto LastCursorMovement = std::chrono::system_clock::now();
int LastCursorXPos = 0;
int LastCursorYPos = 0;
bool HideMouseCursor = false;
CursorPositionHandler CursorPosHandler;

injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerLXAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerLYAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerRXAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerRYAxis;
injector::hook_back<void(__cdecl*)(void)> orgUpdateMousePosition;
injector::hook_back<void(__cdecl*)(void)> orgDrawCursor;
injector::hook_back<void(__cdecl*)(void)> orgSetShowCursorFlag;
injector::hook_back<int32_t(__cdecl*)(void)> orgCanSave;

BYTE* AnalogStringOne;
BYTE* AnalogStringTwo;
BYTE* AnalogStringThree;

uint8_t keyNotSetWarning[21] = { 0 };

int32_t CanSave_Hook()
{
	return orgCanSave.fun();
}

void DrawCursor_Hook()
{
	if (HideMouseCursor)
		return;

	orgDrawCursor.fun();
}

void SetShowCursorFlag_Hook(void)
{
	orgSetShowCursorFlag.fun();
}

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
	// Injecting the virtual analog stick for search view
	if (GetSearchViewFlag() == 0x6 && !VirtualRightStick.IsCentered() && EnableEnhancedMouse)
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
	// Injecting the virtual analog stick for search view
	if (GetSearchViewFlag() == 0x6 && !VirtualRightStick.IsCentered() && EnableEnhancedMouse)
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
	int CurrentMouseHorizontalPos = GetMouseHorizontalPosition();
	int CurrentMouseVerticalPos = GetMouseVerticalPosition();

	orgUpdateMousePosition.fun();

	// During normal gameplay or reading memo, restore the cursor's position to last frame's, for cursor position consistency
	if (((GetEventIndex() == EVENT_IN_GAME && !IsInFullScreenImageEvent()) && GetMenuEvent() != 0x07) || (GetReadingMemoFlag() != 0x00 && GetEventIndex() == EVENT_MEMO_LIST))
	{
		*GetMouseHorizontalPositionPointer() = CurrentMouseHorizontalPos;
		*GetMouseVerticalPositionPointer() = CurrentMouseVerticalPos;
	}

	auto Now = std::chrono::system_clock::now();

	// Center the mouse cursor when entering a puzzle
	if (GetEventIndex() == EVENT_IN_GAME && IsInFullScreenImageEvent() && GetMenuEvent() == 0x0D)
	{
		if (!EnteredPuzzle)
			CursorPosHandler.CenterCursor();

		EnteredPuzzle = true;
	}
	else
	{
		EnteredPuzzle = false;

		if (AutoHideMouseCursor)
		{
			// Auto hide mouse cursor, move to top left and remember its position
			if ((GetEventIndex() == EVENT_IN_GAME && !IsInFullScreenImageEvent()) && GetMenuEvent() != 0x07) // During normal gameplay
			{
				CursorPosHandler.MoveCursorToOrigin();
				HideMouseCursor = true;
			} // If cursor is visible and has moved, update saved cursor position
			else if (!HideMouseCursor && (GetMouseVerticalPosition() != LastCursorYPos || GetMouseHorizontalPosition() != LastCursorXPos))
			{
				LastCursorXPos = GetMouseHorizontalPosition();
				LastCursorYPos = GetMouseVerticalPosition();

				CursorPosHandler.UpdateCursorPos();

				LastCursorMovement = Now;
			} // If cursor is not visible, and has moved from 0:0, set cursor to visible and restore its position
			else if (HideMouseCursor && (GetMouseVerticalPosition() != 0 || GetMouseHorizontalPosition() != 0))
			{

				CursorPosHandler.RestoreCursorPos();

				LastCursorXPos = GetMouseHorizontalPosition();
				LastCursorYPos = GetMouseVerticalPosition();

				LastCursorMovement = Now;
				HideMouseCursor = false;
			} // If too much time has passed, hide the cursor
			else if ((std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastCursorMovement).count() > AutoHideCursorMs))
			{
				CursorPosHandler.MoveCursorToOrigin();

				HideMouseCursor = true;
			}
		}
	}
	
	if (EnhanceMouseCursor)
	{
		CurrentMouseHorizontalPos = GetMouseHorizontalPosition();
		CurrentMouseVerticalPos = GetMouseVerticalPosition();

		// Check if mouse has moved, to enable keyboard selection when the cursor is hovering a hitbox
		bool HasCursorMoved = (CurrentMouseHorizontalPos != LastFrameCursorHorizontalPos) || (CurrentMouseVerticalPos != LastFrameCursorVerticalPos);

		// Handling of vertical and horizontal navigation for Pause screen
		if (GetEventIndex() == EVENT_PAUSE_MENU && HasCursorMoved)
		{
			if (PauseMenu.IsMouseInBounds(CurrentMouseHorizontalPos, CurrentMouseVerticalPos) &&
				GetQuitSubmenuFlag() == 0x00)
			{
				int futureIndex = PauseMenu.GetVerticalIndex(CurrentMouseVerticalPos);

				// If Save Game is to be selected, but saving is disabled, don't select the option
				if (!(futureIndex == 0x01 && orgCanSave.fun() != 0x01))
				{
#pragma warning(disable : 4244)
					* GetPauseMenuButtonIndexPointer() = futureIndex;
				}
			}

			// Pause menu quit submenu
			if (QuitMenu.IsMouseInBounds(CurrentMouseHorizontalPos, CurrentMouseVerticalPos) &&
				GetQuitSubmenuFlag() == 0x01)
			{
#pragma warning(disable : 4244)
				*GetPauseMenuQuitIndexPointer() = QuitMenu.GetHorizontalIndex(CurrentMouseHorizontalPos);
			}
		}

		// Handling of vertical and horizontal navigation for Memo list
		if (GetEventIndex() == EVENT_MEMO_LIST && GetMenuEvent() == 0x0D && GetReadingMemoFlag() == 0 && GetTransitionState() == 0 && HasCursorMoved)
		{
			int CollectedMemos = CountCollectedMemos();
			int NormalizedMemos = (CollectedMemos > 11) ? 11 : CollectedMemos;
			int SelectedHitbox = MemoMenu.GetEnabledVerticalIndex(CurrentMouseVerticalPos, NormalizedMemos);
			int TopOrBot = MemoMenu.IsMouseTopOrBot(CurrentMouseHorizontalPos, CurrentMouseVerticalPos);

			// Check wether the cursor is on top/bottom hitboxes
			if (TopOrBot == 0)
			{
				if (SelectedHitbox <= 1)
					TopOrBot = 1;
				else if (SelectedHitbox >= 9)
					TopOrBot = -1;
			}

			if (MemoMenu.IsMouseInBounds(CurrentMouseHorizontalPos, CurrentMouseVerticalPos, NormalizedMemos) ||
				(CollectedMemos >= 11 && TopOrBot != 0 && MemoMenu.IsMouseHorizontallyInBounds(CurrentMouseHorizontalPos, NormalizedMemos)))
			{
				if (CollectedMemos < 11)
				{
					*GetMemoListIndexPointer() = MemoMenu.GetEnabledVerticalIndex(CurrentMouseVerticalPos, NormalizedMemos);
				}
				else 
				{
					if (TopOrBot != 0)
					{
						// top
						if (TopOrBot == 1)
						{
							*GetMemoListIndexPointer() = MemoMenu.GetClampedMemoIndex(-1,
								CollectedMemos, *GetMemoListIndexPointer());
							*GetMemoListHitboxPointer() = 3; // Select the top memo

							*GetMouseVerticalPositionPointer() = MemoMenu.GetTop() + (MemoMenu.GetHeight() * 2.5);
						}
						else // bottom
						{
							*GetMemoListIndexPointer() = MemoMenu.GetClampedMemoIndex(1,
								CollectedMemos, *GetMemoListIndexPointer());
							*GetMemoListHitboxPointer() = -3; // Select the bottom memo

							*GetMouseVerticalPositionPointer() = MemoMenu.GetTop() + (MemoMenu.GetHeight() * 8.5);
						}
					}
					else if (*GetMemoListHitboxPointer() != MemoMenu.ConvertHitboxValue(SelectedHitbox))
					{
						if (SelectedHitbox > 1 && SelectedHitbox < 9)
						{
							*GetMemoListIndexPointer() = MemoMenu.GetClampedMemoIndex(SelectedHitbox - MemoMenu.ConvertHitboxValue(*GetMemoListHitboxPointer()),
								CollectedMemos, *GetMemoListIndexPointer());
							*GetMemoListHitboxPointer() = MemoMenu.ConvertHitboxValue(SelectedHitbox);
						}
					}
				}
			}
		}

		LastFrameCursorHorizontalPos = GetMouseHorizontalPosition();
		LastFrameCursorVerticalPos = GetMouseVerticalPosition();
	}
}

void InputTweaks::TweakGetDeviceState(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbData, LPVOID lpvData)
{
	UNREFERENCED_PARAMETER(cbData);

	// Check number keybinds after exiting the Options screen
	if (GetEventIndex() == EVENT_OPTION_FMV)
		CheckKeyBindsFlag = true;
	if (GetEventIndex() != EVENT_OPTION_FMV && CheckKeyBindsFlag)
	{
		CheckKeyBindsFlag = false;
		CheckNumberKeyBinds();
	}

	// For controller
	if (ProxyInterface == ControllerInterfaceAddress)
	{
		// Save controller data
		ControllerData = (DIJOYSTATE*)lpvData;

		// Clear the the pause button if a quicksave is in progress
		if (GameLoadFix && (GetIsWritingQuicksave() == 1 || GetTextAddr() == 1))
			ControllerData->rgbButtons[KeyBinds.GetPauseButtonBind()] = KEY_CLEAR;

        // Clear the pause button if either the player or Maria (NPC) is dying
        if (GameLoadFix && (GetPlayerIsDying() != 0 || GetMariaNpcIsDying() != 0))
        {
            ControllerData->rgbButtons[KeyBinds.GetPauseButtonBind()] = KEY_CLEAR;
        }

		// Clear controller data
		ControllerData = nullptr;
	}

	// For keyboard
	if (ProxyInterface == KeyboardInterfaceAddress)
	{
		auto Now = std::chrono::system_clock::now();
		auto DeltaMsWeaponSwap = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastWeaponSwap);

		// Save Keyboard Data
		KeyboardData = (BYTE*)lpvData;

		// Fix for ALT + TAB
		if (GetForegroundWindow() != GameWindowHandle)
		{
			SetLMButton = false;
			RMB.State = false;
		}

		// Reset Sprint toggle
		if (EnableToggleSprint && GetRunOption() != OPT_ANALOG && !OverrideSprint)
			Sprint.State = false;

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

		// Update Esc/Cancel input states
		EscInput.State = IsKeyPressed(KeyBinds.GetKeyBind(KEY_SKIP));
		EscInput.UpdateHolding();
		CancelInput.State = IsKeyPressed(KeyBinds.GetKeyBind(KEY_CANCEL));
		CancelInput.UpdateHolding();

		// Clear the ESC and SKIP key if a quicksave is in progress, or if holding the button entering the result screen
		if (GameLoadFix && (GetIsWritingQuicksave() == 1 || GetTextAddr() == 1) ||
			(GetEventIndex() == EVENT_GAME_RESULT_TWO && (EscInput.Holding || CancelInput.Holding)))
		{
			ClearKey(KeyBinds.GetKeyBind(KEY_SKIP));
			ClearKey(KeyBinds.GetKeyBind(KEY_CANCEL));
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
		if (EnableToggleSprint && (GetRunOption() == OPT_ANALOG || OverrideSprint))
		{
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_RUN)) && !Sprint.Holding && GetEventIndex() == EVENT_IN_GAME)
			{
				Sprint.ToggleState();
			}
		}

		// Check for forward/backwards movement to fix boat stage
		ForwardBackwardsAxis = 0;
		LeftRightAxis = 0;

		if (((GetBoatFlag() == 0x01 && GetRoomID() == 0x0E) || GetSearchViewFlag() == 0x06) && EnableEnhancedMouse)
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

			if (GetRunOption() == OPT_ANALOG)
				OverrideSprint = true;
		}

		// Activate Overlays
		if (DebugCombo.State && !DebugCombo.Holding && EnableDebugOverlay)
			ShowDebugOverlay = !ShowDebugOverlay;
		if (InfoCombo.State && !InfoCombo.Holding && EnableInfoOverlay)
			ShowInfoOverlay = !ShowInfoOverlay;

		// Inject Key Presses

		// Inject ready weapon or cancel based on context, on RMB press
		if (EnableEnhancedMouse && GetEventIndex() != EVENT_MAP && GetEventIndex() != EVENT_INVENTORY && GetEventIndex() != EVENT_OPTION_FMV && 
			GetEventIndex() != EVENT_FMV && GetCutsceneID() == 0x0) 
		{
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

		// Inject action, same condition as RMB but without CutsceneID == 0x00 to enable input on FIE in the middle of cutscenes, eg Laura's letter in the hotel
		if (EnableEnhancedMouse && GetEventIndex() != EVENT_MAP && GetEventIndex() != EVENT_INVENTORY && GetEventIndex() != EVENT_OPTION_FMV &&
			GetEventIndex() != EVENT_FMV)
		{
			if (SetLMButton)
				SetKey(KeyBinds.GetKeyBind(KEY_ACTION));
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

		// Setting sprint button for the Toggle Sprint function
		if (EnableToggleSprint && (GetRunOption() == OPT_ANALOG || OverrideSprint) && GetEventIndex() != EVENT_OPTION_FMV)
		{
			ClearKey(KeyBinds.GetKeyBind(KEY_RUN));
		}

		if (EnableToggleSprint && Sprint.State && ((GetRunOption() == OPT_ANALOG && IsMovementPressed()) || OverrideSprint) && GetEventIndex() == EVENT_IN_GAME)
		{
			SetKey(KeyBinds.GetKeyBind(KEY_RUN));
		}

		// Mouse wheel weapon swapping
		if (MouseWheel != 0 && DeltaMsWeaponSwap.count() > InputDebounce)
		{
			// Inject up and down in the memo screen
			switch (GetEventIndex()) 
			{
			case EVENT_MEMO_LIST:
				if (MouseWheel > 0)
					SetKey(KeyBinds.GetKeyBind(KEY_MOVE_FORWARDS));
				else
					SetKey(KeyBinds.GetKeyBind(KEY_MOVE_BACKWARDS));

				break;

			case EVENT_IN_GAME:
				if (EnableMouseWheelSwap)
				{
					if (MouseWheel > 0)
						SetKey(KeyBinds.GetKeyBind(KEY_NEXT_WEAPON));
					else
						SetKey(KeyBinds.GetKeyBind(KEY_PREV_WEAPON));

					LastWeaponSwap = Now;
				}
				break;

			default:
				break;
			}
			MouseWheel = 0;
		}

		// Fix for Alt-Tabbing issues
		if (GetForegroundWindow() != GameWindowHandle)
		{
			ClearKey(KeyBinds.GetKeyBind(KEY_ACTION));
			ClearKey(KeyBinds.GetKeyBind(KEY_READY_WEAPON));
		}

		// Fix for memo screen arrow keys navigation
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

        // Clear the ESC and SKIP key if either the player or Maria (NPC) is dying
        if (GameLoadFix && (GetPlayerIsDying() != 0 || GetMariaNpcIsDying() != 0))
        {
            ClearKey(KeyBinds.GetKeyBind(KEY_SKIP));
            ClearKey(KeyBinds.GetKeyBind(KEY_CANCEL));
        }

		// Clear Keyboard Data pointer
		KeyboardData = nullptr;
	}
}

void PatchInputTweaks()
{
	// Hooking controller input functions
	orgGetControllerLXAxis.fun = injector::MakeCALL(GetLeftAnalogXFunctionPointer(), GetControllerLXAxis_Hook, true).get();
	orgGetControllerLYAxis.fun = injector::MakeCALL(GetLeftAnalogYFunctionPointer(), GetControllerLYAxis_Hook, true).get();
	orgGetControllerRXAxis.fun = injector::MakeCALL(GetRightAnalogXFunctionPointer(), GetControllerRXAxis_Hook, true).get();
	orgGetControllerRYAxis.fun = injector::MakeCALL(GetRightAnalogYFunctionPointer(), GetControllerRYAxis_Hook, true).get();

	orgUpdateMousePosition.fun = injector::MakeCALL(GetUpdateMousePositionFunctionPointer(), UpdateMousePosition_Hook, true).get();

	InputTweaksRef.CheckNumberKeyBinds();

	if (EnableToggleSprint)
	{
		if (!InputTweaksRef.GetAnalogStringAddr()) return;

		// Change the Analog string (0x68) with Toggle (0x2F)
		UpdateMemoryAddress((void*)AnalogStringOne, "\x2F", 1);
		UpdateMemoryAddress((void*)AnalogStringTwo, "\x2F", 1);
		UpdateMemoryAddress((void*)AnalogStringThree, "\x2F", 1);
	}

	// Hooking the mouse visibility function
	if (EnableEnhancedMouse)
	{
		orgDrawCursor.fun = injector::MakeCALL(GetDrawCursorPointer(), DrawCursor_Hook, true).get();
		orgSetShowCursorFlag.fun = injector::MakeCALL(GetSetShowCursorPointer(), SetShowCursorFlag_Hook, true).get();
		orgCanSave.fun = injector::MakeCALL(GetCanSaveFunctionPointer(), CanSave_Hook, true).get();
	}
}

void InputTweaks::TweakGetDeviceData(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbObjectData, 
	LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	UNREFERENCED_PARAMETER(dwFlags);
	UNREFERENCED_PARAMETER(cbObjectData);

	// For mouse
	if (ProxyInterface == MouseInterfaceAddress)
	{
		// Save Mouse Data for axis movement
		MouseData = rgdod;
		MouseDataSize = *pdwInOut;

		// Get the current mouse state, to handle LMB/RMB
		MouseInterfaceAddress->GetDeviceState(sizeof(MouseState), &MouseState);

		// Save current mouse state
		MouseXAxis = GetMouseRelXChange();
		ReadMouseButtons();

		// In search view, inject mouse movement into the right analog stick
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

void InputTweaks::SetControllerInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface)
{
	ControllerInterfaceAddress = ProxyInterface;
}

void InputTweaks::RemoveInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface)
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

	for (UINT i = 0; i < MouseDataSize; i++)
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

	for (UINT i = 0; i < MouseDataSize; i++)
		if (MouseData[i].dwOfs == DIMOFS_Y)
			AxisSum += (int32_t)MouseData[i].dwData;

	return AxisSum;
}

void InputTweaks::ReadMouseButtons()
{		
	if (!MouseData || GetForegroundWindow() != GameWindowHandle) return;

	for (UINT i = 0; i < MouseDataSize; i++)
		switch (MouseData[i].dwOfs)
		{
#pragma warning(suppress: 4644)
		case DIMOFS_Z:
		{
			MouseWheel = MouseData[i].dwData;
		}

		default:
		{
			break;
		}
		}

	SetLMButton = (MouseState.rgbButtons[0] == KEY_SET);
	RMB.State = (MouseState.rgbButtons[1] == KEY_SET);
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
	return ForwardBackwardsAxis > 0 ? -1.0f : ForwardBackwardsAxis < 0 ? 1.0f : 0.0f;
}

float InputTweaks::GetTurningAnalog()
{

	return LeftRightAxis > 0 ? -1.0f : LeftRightAxis < 0 ? 1.0f : 0.0f;
}

void InputTweaks::ClearMouseInputs()
{
	SetLMButton = false;
	RMB.State = false;
	CleanKeys = true;
}

void InputTweaks::CheckNumberKeyBinds()
{
	// Fix for binding actions to the number keys
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
			*(NumberKeyBinds + (i * 0x8)) = (BYTE)DefaultNumberKeyBinds[i];
		}
	}
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

	if (Sprint.State && (GetRunOption() == OPT_ANALOG || OverrideSprint))
		Output = "Active";

	if (!EnableToggleSprint)
		Output = "Not Enabled";

	return Output;
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
		(ElevatorFix() || HotelFix() || JamesVaultingBuildingsFix() || 
		RosewaterParkFix() || HospitalMonologueFix() || FleshRoomFix() ||
		!IsInFullScreenImageEvent()));
}

bool InputTweaks::GetAnalogStringAddr()
{
	constexpr BYTE AnalogStringOneSearchBytes[]{ 0x68, 0x9A, 0x00, 0x00, 0x00, 0x56, 0x6A, 0x68 };
	BYTE *AnalogString = (BYTE*)SearchAndGetAddresses(0x00461F63, 0x004621D5, 0x004621D5, AnalogStringOneSearchBytes, sizeof(AnalogStringOneSearchBytes), 0x07, __FUNCTION__);

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
	AnalogString = (BYTE*)SearchAndGetAddresses(0x004621D1, 0x00462433, 0x00462433, AnalogStringTwoSearchBytes, sizeof(AnalogStringTwoSearchBytes), 0x0D, __FUNCTION__);

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
	AnalogString = (BYTE*)SearchAndGetAddresses(0x00464465, 0x004646DE, 0x004648E6, AnalogStringThreeSearchBytes, sizeof(AnalogStringThreeSearchBytes), 0x02, __FUNCTION__);

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

void InputTweaks::SetOverrideSprint()
{
	OverrideSprint = true;
}

void InputTweaks::ClearOverrideSprint()
{
	OverrideSprint = false;
}

bool InputTweaks::GetRMBState()
{
	return RMB.State;
}

bool InputTweaks::GetLMBState()
{
	return SetLMButton;
}

void InputTweaks::InitializeHitboxes(float AspectRatio)
{
	PauseMenu = Hitboxes(PauseMenuTop, PauseMenuLeft, PauseMenuHeight, PauseMenuWidth, 5, 1, AspectRatio);
	QuitMenu = Hitboxes(QuitMenuTop, PauseMenuLeft, PauseMenuHeight, QuitMenuWidth, 1, 2, AspectRatio);
	MemoMenu = MemoHitboxes(MemoEvenTop, MemoOddTop, MemoLeft, MemoHeight, MemoWidth, AspectRatio);
}

BYTE KeyBindsHandler::GetKeyBind(int KeyIndex)
{
	BYTE* pButton = GetKeyBindsPointer();

	if ((pButton && *(pButton + (KeyIndex * 0x08)) == 0) && keyNotSetWarning[KeyIndex] == 0)
	{
		Logging::Log() << "ERROR: Keybind " << KEY_NAMES[KeyIndex] << " not set";
		keyNotSetWarning[KeyIndex] = 1;
	}

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
	BYTE* Binds = (BYTE*)ReadSearchedAddresses(0x005AEF90, 0x005AF8C0, 0x005AF1E0, TurnLeftButtonSearchBytes, sizeof(TurnLeftButtonSearchBytes), 0x1D, __FUNCTION__);

	// Checking address pointer
	if (!Binds)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find KeyBinds address!";
		return nullptr;
	}

	KeyBindsAddr = (BYTE*)((DWORD)Binds);

	return KeyBindsAddr;
}

BYTE KeyBindsHandler::GetPauseButtonBind()
{
	return *(this->GetKeyBindsPointer() + 0xF0);
}

int CountCollectedMemos()
{
	auto* psVar2 = GetMemoCountIndexPointer();
	int LoopCondition = ((int)psVar2) + 0x390;

	auto* MemosArray = GetMemoInventoryPointer();
	int TotalMemos = 0;

	do {

		if (((unsigned int)(MemosArray)[(int)*psVar2 >> 5] >> ((byte)*psVar2 & 0x1f) & 1) != 0)
			TotalMemos = TotalMemos + 1;

		if (((unsigned int)(MemosArray)[(int)psVar2[8] >> 5] >> ((byte)psVar2[8] & 0x1f) & 1) != 0)
			TotalMemos = TotalMemos + 1;

		if (((unsigned int)(MemosArray)[(int)psVar2[0x10] >> 5] >> ((byte)psVar2[0x10] & 0x1f) & 1) != 0)
			TotalMemos = TotalMemos + 1;

		psVar2 += 0x18;
	} while ((int)psVar2 < LoopCondition);

	return TotalMemos;
}
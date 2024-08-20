/**
* Copyright (C) 2024 mercury501
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

#pragma warning( disable : 4244 )
#include "Overlay.h"

bool ControllerConnectedFlag = false;
int JoystickX = 0;
int JoystickY = 0;

void Overlay::DrawOverlays(LPDIRECT3DDEVICE8 ProxyInterface, LONG Width, LONG Height)
{
	if (LastWidth != Width || LastHeight != Height)
	{
		LastWidth = Width;
		LastHeight = Height;
		InitializeDataStructs();
		InputTweaksRef.InitializeHitboxes((float)Width / (float)Height);
	}

	// Nvidia fix
	ProxyInterface->GetRenderState(D3DRS_FOGENABLE, &FogEnableValue);
	ProxyInterface->SetRenderState(D3DRS_FOGENABLE, 0x0);
	ProxyInterface->GetRenderState(D3DRS_MULTISAMPLEANTIALIAS, &MultiSampleValue);
	ProxyInterface->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	if (SetATOC)
	{
		ProxyInterface->SetRenderState(D3DRS_ADAPTIVETESS_Y, FOURCC_ATOC);
	}
	else if (IsFixGPUAntiAliasingEnabled)
	{
		ProxyInterface->SetRenderState(D3DRS_POINTSIZE, FOURCC_A2M_ENABLE);
	}

	// Debug Overlay
	if (ShowDebugOverlay)
	{
		DrawDebugOverlay(ProxyInterface);
	}

	// Info Overlay
	if (ShowInfoOverlay)
	{
		DrawInfoOverlay(ProxyInterface);
	}

	// Reset Direct3D state
	ProxyInterface->SetRenderState(D3DRS_FOGENABLE, FogEnableValue);
	ProxyInterface->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, MultiSampleValue);
	if (SetATOC)
	{
		ProxyInterface->SetRenderState(D3DRS_ADAPTIVETESS_Y, D3DFMT_UNKNOWN);
	}
	else if (IsFixGPUAntiAliasingEnabled)
	{
		ProxyInterface->SetRenderState(D3DRS_POINTSIZE, FOURCC_A2M_DISABLE);
	}
}

void Overlay::DrawInfoOverlay(LPDIRECT3DDEVICE8 ProxyInterface)
{
	int SpecialItems = bitCount(GetSecretItemsCollected());

	std::string OvlString = "STATS INFO (CTRL + I) ";
	OvlString.append("\rAction Difficulty: ");
	OvlString.append(ActionDifficulty[GetActionDifficulty()]);

	OvlString.append("\rRiddle Difficulty: ");
	OvlString.append(RiddleDifficulty[GetRiddleDifficulty()]);

	OvlString.append("\rSaves: ");
	OvlString.append(std::to_string(GetNumberOfSaves()));

	OvlString.append("\rTotal Time: ");
	OvlString.append(SecondsToTimeString(GetInGameTime()));
	
	OvlString.append("\rWalking Distance: ");
	OvlString.append(FloatToStr(GetWalkingDistance() / KMConstant, 2));
	OvlString.append("km");

	OvlString.append("\rRunning Distance: ");
	OvlString.append(FloatToStr(GetRunningDistance() / KMConstant, 2));
	OvlString.append("km");

	OvlString.append("\rItems: ");
	OvlString.append(std::to_string(GetItemsCollected()));
	
	if (SpecialItems > 0)
	{
		OvlString.append("(+");
		OvlString.append(std::to_string(SpecialItems));
		OvlString.append(")");
	}

	OvlString.append("\rShooting Kills: ");
	OvlString.append(std::to_string(GetShootingKills()));

	OvlString.append("\rFighting Kills: ");
	OvlString.append(std::to_string(GetMeleeKills()));

	OvlString.append("\rBoat Stage Time: ");
	OvlString.append(SecondsToMsTimeString(GetBoatStageTime()));

	OvlString.append("\rBoat Max Speed: ");
	OvlString.append(FloatToStr(GetBoatMaxSpeed(), 2));
	OvlString.append("m/s");

	OvlString.append("\rTotal Damage: ");
	OvlString.append(FloatToStr(GetDamagePointsTaken(), 2));

	InfoOverlayTextStruct.String = OvlString.c_str();
	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);
}

void Overlay::DrawDebugOverlay(LPDIRECT3DDEVICE8 ProxyInterface)
{
	// Calculate FPS if it has been more than one second since last update
	auto currentTime = std::chrono::steady_clock::now();
	if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime).count() >= 500)
	{
		LastFPS = (float)AverageFPSCounter;
		lastUpdateTime = currentTime;
	}

	float CharYPos = GetJamesPosY();

	// Lock value at 0 if close enough, to avoid a rapidly changing number.
	if (abs(CharYPos) < AntiJitterValue)
	{
		CharYPos = 0;
	}
	// If value changes only a small amount then lock to last value.
	if (abs(CharYPos - LastCharYPos) < AntiJitterValue)
	{
		CharYPos = LastCharYPos;
	}
	LastCharYPos = CharYPos;

	std::string OvlString = "DEBUG INFO (CTRL + G) ";

	OvlString.append("\rExecutable Version: ");
	OvlString.append(GameVersion == SH2V_10 ? "1.0" : 
					GameVersion == SH2V_11 ? "1.1" : 
					GameVersion == SH2V_DC ? "DC" : 
					"Unknown");

	OvlString.append("\rFPS: ");
	OvlString.append(FloatToStr(LastFPS, FPSFloatPrecision));

	OvlString.append("\rGame Resolution: ");
	OvlString.append(std::to_string(LastWidth));
	OvlString.append("x");
	OvlString.append(std::to_string(LastHeight));

	OvlString.append("\rRoom ID: 0x");
	OvlString.append(IntToHexStr(GetRoomID()));

	OvlString.append("\rCutscene ID: 0x");
	OvlString.append(IntToHexStr(GetCutsceneID()));

	OvlString.append("\rEvent ID: 0x");
	OvlString.append(IntToHexStr(GetEventIndex()));

	OvlString.append("\rMenu Event ID: 0x");
	OvlString.append(IntToHexStr(GetMenuEvent()));

	OvlString.append("\rFullscreen Image Event: 0x");
	OvlString.append(IntToHexStr(GetFullscreenImageEvent()));

	OvlString.append("\rMemos collected: ");
	OvlString.append(std::to_string(CountCollectedMemos()));

	OvlString.append("\rChar X Position: ");
	OvlString.append(FloatToStr(GetJamesPosX(), FloatPrecision));

	OvlString.append("\rChar Y Position: ");
	OvlString.append(FloatToStr(CharYPos, FloatPrecision));

	OvlString.append("\rChar Z Position: ");
	OvlString.append(FloatToStr(GetJamesPosZ(), FloatPrecision));

	OvlString.append("\rCamera X Position: ");
	OvlString.append(FloatToStr(GetInGameCameraPosX(), FloatPrecision));

	OvlString.append("\rCamera Y Position: ");
	OvlString.append(FloatToStr(GetInGameCameraPosY(), FloatPrecision));

	OvlString.append("\rCamera Z Position: ");
	OvlString.append(FloatToStr(GetInGameCameraPosZ(), FloatPrecision));

	OvlString.append("\rRight Click Function: ");
	OvlString.append(InputTweaksRef.GetRightClickState());

	OvlString.append("\rToggle Sprint: ");
	OvlString.append(InputTweaksRef.GetToggleSprintState());

	OvlString.append("\rController connected: ");
	OvlString.append(ControllerConnectedFlag ? "True" : "False");

	OvlString.append("\rJoystick LY: ");
	OvlString.append(std::to_string(JoystickY));

	OvlString.append("\rJoystick LX: ");
	OvlString.append(std::to_string(JoystickX));

	OvlString.append("\rLeft Mouse B.: ");
	OvlString.append(InputTweaksRef.GetLMBState() ? "True" : "False");
	OvlString.append("\rRight Mouse B.: ");
	OvlString.append(InputTweaksRef.GetRMBState() ? "True" : "False");

	OvlString.append("\rMouse X Pos.: ");
	OvlString.append(std::to_string(GetMouseHorizontalPosition()));
	OvlString.append("\rMouse Y Pos.: ");
	OvlString.append(std::to_string(GetMouseVerticalPosition()));

	// Temporary Debug String, to use wherever
	OvlString.append(AuxDebugOvlString);

	DebugOverlayTextStruct.String = OvlString.c_str();
	DrawDebugText(ProxyInterface, DebugOverlayTextStruct);
}

void Overlay::DrawDebugText(LPDIRECT3DDEVICE8 ProxyInterface, Overlay::D3D8TEXT FontStruct)
{
	RECT DropShadowRect = FontStruct.Rect;
	DropShadowRect.top = DropShadowRect.top + DropShadowOffset;
	DropShadowRect.left = DropShadowRect.left + DropShadowOffset;

	// This flag is set when changing resolution, we have to reload the font
	if (ResetDebugFontFlag)
	{
		ResetDebugFontFlag = false;
		DebugFont->OnResetDevice();
	}

	if (ProxyInterface != nullptr && DebugFont == nullptr)
	{
		HFONT FontCharacteristics = CreateFontA(16, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, 0, FontName);
		if (FontCharacteristics != NULL)
		{
			Logging::LogDebug() << __FUNCTION__ << " Creating Debug font: " << FontName;
			D3DXCreateFont(ProxyInterface, FontCharacteristics, &DebugFont);
			DeleteObject(FontCharacteristics);
		}
	}

	if (DebugFont != nullptr)
	{
		// Draw the drop shadow text
		DebugFont->DrawTextA(FontStruct.String, -1, &DropShadowRect, FontStruct.Format, TextColors.Black);
		// Draw the main text
		DebugFont->DrawTextA(FontStruct.String, -1, &FontStruct.Rect, FontStruct.Format, FontStruct.Color);
	}
}

std::string Overlay::IntToHexStr(int IntValue)
{
	std::stringstream Stream;
	Stream << std::hex << IntValue;

	std::string OutputString(Stream.str());

	return OutputString;
}

std::string Overlay::FloatToStr(float FloatValue, int precision)
{
	std::stringstream Stream;
	Stream.precision(precision);
	Stream << std::fixed << FloatValue;

	std::string OutputString(Stream.str());

	return OutputString;
}

void Overlay::ResetFont()
{
	if (DebugFont)
	{
		DebugFont->OnLostDevice();
		ResetDebugFontFlag = true;
	}

	if (IGTFont)
	{
		IGTFont->OnLostDevice();
		ResetIGTFontFlag = true;
	}
}

std::string Overlay::SecondsToTimeString(int time)
{
	std::string TimeString = "";
	int hours, minutes, seconds;

	hours = time / 3600;
	time -= hours * 3600;
	minutes = time / 60;
	time -= minutes * 60;
	seconds = time;

	TimeString.append(std::to_string(hours));
	TimeString.append("h ");
	TimeString.append(std::to_string(minutes));
	TimeString.append("m ");
	TimeString.append(std::to_string(seconds));
	TimeString.append("s");

	return TimeString;
}

std::string Overlay::SecondsToMsTimeString(float time)
{
	std::string TimeString = "";
	int minutes, seconds, tenths;

	minutes = time / 60;
	time -= minutes * 60;
	seconds = time;
	tenths = (time - seconds) * 100;

	TimeString.append(std::to_string(minutes));
	TimeString.append("m ");
	TimeString.append(std::to_string(seconds));
	TimeString.append("s ");
	TimeString.append(std::to_string(tenths));

	return TimeString;
}

int Overlay::bitCount(uint8_t num)
{
	int count = 0;
	while (num > 0) {          
		if ((num & 0x1) == 1)    
			count++;
		num >>= 1;             
	}
	return count;	
}

void Overlay::InitializeDataStructs()
{
	Logging::LogDebug() << __FUNCTION__ << " Initializing Overlay Text Structs...";

	InfoOverlayTextStruct.Format = DT_NOCLIP | DT_LEFT;
	InfoOverlayTextStruct.Rect.left = LastWidth - 205;
	InfoOverlayTextStruct.Rect.top = rectOffset;
	InfoOverlayTextStruct.Rect.right = LastWidth;
	InfoOverlayTextStruct.Rect.bottom = rectOffset + 15;
	InfoOverlayTextStruct.Color = TextColors.Tiel;

	DebugOverlayTextStruct.Format = DT_NOCLIP | DT_LEFT;
	DebugOverlayTextStruct.Rect.left = rectOffset;
	DebugOverlayTextStruct.Rect.top = rectOffset;
	DebugOverlayTextStruct.Rect.right = rectOffset + 300;
	DebugOverlayTextStruct.Rect.bottom = rectOffset + 15;
	DebugOverlayTextStruct.Color = TextColors.Green;
}

std::string Overlay::GetIGTString()
{
	float time = GetInGameTime();
	std::string TimeString = "";
	int hours, minutes;

	hours = time / 3600;
	time -= hours * 3600;
	minutes = time / 60;
	time -= minutes * 60;

	TimeString.append(std::to_string(hours));
	TimeString.append(":");
	
	if (minutes < 10)
	{
		TimeString.append("0");
	}

	TimeString.append(std::to_string(minutes));
	TimeString.append(":");
	
	if (time < 10)
	{
		TimeString.append("0");
	}

	TimeString.append(FloatToStr(time, 3));

	return TimeString;
}

void Overlay::RenderMouseCursor()
{
	if ((GetEventIndex() != EVENT_PAUSE_MENU && GetEventIndex() != EVENT_MEMO_LIST) || GetReadingMemoFlag() != 0 || GetTransitionState() != FADE_NONE)
		return;

	*GetMousePointerVisibleFlagPointer() = 1;
	SetShowCursorFlag_Hook();
	DrawCursor_Hook();
}
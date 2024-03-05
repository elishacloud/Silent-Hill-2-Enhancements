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

#pragma once
#include "d3d8wrapper.h"
#include "Patches\InputTweaks.h"
#include <sstream>
#include <chrono>
#include <iomanip>

class Overlay
{
public:
	struct D3D8TEXT
	{
		LPCSTR     String = "";
		RECT			  Rect;
		DWORD           Format;
		D3DCOLOR         Color;
	};

	void DrawOverlays(LPDIRECT3DDEVICE8 ProxyInterface);
	void ResetFont();
	void RenderMouseCursor();

private:
	std::string ActionDifficulty[4]
	{
		"Beginner",
		"Easy",
		"Normal",
		"Hard"
	};

	std::string RiddleDifficulty[4]
	{
		"Easy",
		"Normal",
		"Hard",
		"Extra"
	};

	// LowWhite, MediumWhite, FullWhite
	D3DCOLOR WhiteArray[3] = { 
		D3DCOLOR_ARGB(255, 50, 50, 50), 
		D3DCOLOR_ARGB(255, 128, 128, 128), 
		D3DCOLOR_ARGB(255, 255, 255, 255) 
	};

	struct OverlayTextColors {
		D3DCOLOR Black = D3DCOLOR_ARGB(255, 0, 0, 0);
		D3DCOLOR Green = D3DCOLOR_ARGB(255, 153, 255, 153);
		D3DCOLOR Tiel = D3DCOLOR_ARGB(255, 153, 217, 234);
	} TextColors;

	void DrawDebugOverlay(LPDIRECT3DDEVICE8 ProxyInterface);
	void DrawInfoOverlay(LPDIRECT3DDEVICE8 ProxyInterface);
	void DrawMenuTestOverlay(LPDIRECT3DDEVICE8 ProxyInterface);

	void DrawDebugText(LPDIRECT3DDEVICE8 ProxyInterface, D3D8TEXT TextStruct);
	void DrawMenuTestText(LPDIRECT3DDEVICE8 ProxyInterface, D3D8TEXT TextStruct);
	void DrawIGTText(LPDIRECT3DDEVICE8 ProxyInterface, D3D8TEXT TextStruct);

	std::string IntToHexStr(int IntValue);
	std::string FloatToStr(float FloatValue, int precision);
	std::string SecondsToTimeString(int time);
	std::string SecondsToMsTimeString(float time);
	std::string GetIGTString();
	int bitCount(uint8_t num);

	bool ChangeMenuTestColor();
	void InitializeDataStructs();

	const int rectOffset = 40;
	const int FloatPrecision = 4;
	const int FPSFloatPrecision = 2;
	const int KMConstant = 500000;
	const float AntiJitterValue = 0.0001f;
	const int DropShadowOffset = 1;

	float LastCharYPos;

	LPCSTR FontName = "Arial";

	LPD3DXFONT DebugFont = nullptr;
	LPD3DXFONT MenuTestFont = nullptr;
	LPD3DXFONT IGTFont = nullptr;

	bool ResetDebugFontFlag = false;
	bool ResetMenuTestFontFlag = false;
	bool ResetIGTFontFlag = false;

	std::chrono::system_clock::time_point LastColorChange = std::chrono::system_clock::now();
	int WhiteArrayIndex = 2;

	Overlay::D3D8TEXT MenuTestTextStruct;
	Overlay::D3D8TEXT InfoOverlayTextStruct;
	Overlay::D3D8TEXT DebugOverlayTextStruct;
	Overlay::D3D8TEXT ControlMenuTestTextStruct;
	LONG LastBufferWidth = 0;
	LONG LastBufferHeight = 0;

	DWORD FogEnableValue;
	DWORD MultiSampleValue;
};

extern bool ControllerConnectedFlag;
extern int JoystickX;
extern int JoystickY;

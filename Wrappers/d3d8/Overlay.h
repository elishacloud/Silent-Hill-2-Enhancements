#pragma once
#include "d3d8wrapper.h"
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

	void DrawDebugOverlay(LPDIRECT3DDEVICE8 ProxyInterface);
	void DrawInfoOverlay(LPDIRECT3DDEVICE8 ProxyInterface);
	void DrawMenuTestOverlay(LPDIRECT3DDEVICE8 ProxyInterface);
	void DrawDebugText(LPDIRECT3DDEVICE8 ProxyInterface, D3D8TEXT TextStruct);
	void DrawMenuTestText(LPDIRECT3DDEVICE8 ProxyInterface, D3D8TEXT TextStruct);
	std::string IntToHexStr(int IntValue);
	std::string FloatToStr(float FloatValue, int precision);
	std::string SecondsToTimeString(int time);
	std::string SecondsToMsTimeString(int time);
	std::string GetIGTString();
	std::string BoatStageTimeString(float time);
	int bitCount(uint8_t num);
	bool ChangeMenuTestColor();
	void InitializeDataStructs();
};

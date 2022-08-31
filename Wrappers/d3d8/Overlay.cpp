#include "Overlay.h"

LPD3DXFONT font = NULL;
bool ResetFontFlag = false;

void Overlay::DrawInfoOverlay(LPDIRECT3DDEVICE8 ProxyInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	int rectOffset = 40, FloatPrecision = 4, KMConstant = 500000;

	D3D8TEXT TextStruct;
	TextStruct.Format = DT_NOCLIP | DT_SINGLELINE;
	TextStruct.Rect.left = ResolutionWidth - 185;
	TextStruct.Rect.top = rectOffset;
	TextStruct.Rect.right = ResolutionWidth;
	TextStruct.Rect.bottom = rectOffset + 15;

	std::string OvlString = "INFO MENU (Crtl + I) ";

	TextStruct.String = OvlString.c_str();

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Action Difficulty: ";
	OvlString.append(ActionDifficulty[GetActionDifficulty()]);

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Riddle Difficulty: ";
	OvlString.append(RiddleDifficulty[GetRiddleDifficulty()]);

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Saves: ";
	OvlString.append(std::to_string(GetNumberOfSaves()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Total Time: ";
	OvlString.append(SecondsToTimeString(GetInGameTime()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Walking Distance: ";
	OvlString.append(FloatToStr(GetWalkingDistance() / KMConstant, 2));
	OvlString.append("km");

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Running Distance: ";
	OvlString.append(FloatToStr(GetRunningDistance() / KMConstant, 2));
	OvlString.append("km");

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Items: ";
	OvlString.append(std::to_string(GetItemsCollected()));
	OvlString.append("(+");
	OvlString.append(std::to_string(GetSecretItemsCollected()));
	OvlString.append(")");

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Shooting Kills: ";
	OvlString.append(std::to_string(GetShootingKills()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Fighting Kills: ";
	OvlString.append(std::to_string(GetMeleeKills()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Boat Stage Time: ";
	OvlString.append(SecondsToMsTimeString(GetBoatStageTime()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Boat Max Speed: ";
	OvlString.append(FloatToStr(GetBoatMaxSpeed(), 2));
	OvlString.append("m/s");

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Total Damage: ";
	OvlString.append(FloatToStr(GetDamagePointsTaken(), 2));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);
}

void Overlay::DrawDebugOverlay(LPDIRECT3DDEVICE8 ProxyInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	int rectOffset = 40, FloatPrecision = 4;
	float CharYPos = GetJamesPosY();
	float AntiJitterValue = 0.0001;

	// Lock value at 0 if close enough, to avoid a rapidly changing number.
	if (CharYPos > -AntiJitterValue && CharYPos < AntiJitterValue)
	{
		CharYPos = 0;
	}

	D3D8TEXT TextStruct;
	TextStruct.Format = DT_NOCLIP | DT_SINGLELINE;
	TextStruct.Rect.left = rectOffset;
	TextStruct.Rect.top = rectOffset;
	TextStruct.Rect.right = rectOffset + 300;
	TextStruct.Rect.bottom = rectOffset + 15;

	std::string OvlString = "DEBUG MENU (Crtl + D) ";

	TextStruct.String = OvlString.c_str();

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Game Resolution: ";
	OvlString.append(std::to_string(ResolutionWidth));
	OvlString.append("x");
	OvlString.append(std::to_string(ResolutionHeight));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Room ID: 0x";
	OvlString.append(IntToHexStr(GetRoomID()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Cutscene ID: 0x";
	OvlString.append(IntToHexStr(GetCutsceneID()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "FPS: ";
	OvlString.append(FloatToStr(GetFPSCounter(), FloatPrecision));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Char X Position: ";
	OvlString.append(FloatToStr(GetJamesPosX(), FloatPrecision));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Char Y Position: ";
	OvlString.append(FloatToStr(CharYPos, FloatPrecision));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Char Z Position: ";
	OvlString.append(FloatToStr(GetJamesPosZ(), FloatPrecision));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

}


void Overlay::DrawDebugText(LPDIRECT3DDEVICE8 ProxyInterface, Overlay::D3D8TEXT FontStruct)
{
	Logging::LogDebug() << __FUNCTION__;

	D3DCOLOR GreenColor = D3DCOLOR_ARGB(255, 153, 255, 153);
	D3DCOLOR BlackColor = D3DCOLOR_ARGB(255, 0, 0, 0);
	int DropShadowOffset = 1;

	RECT DropShadowRect = FontStruct.Rect;
	DropShadowRect.top = DropShadowRect.top + DropShadowOffset;
	DropShadowRect.left = DropShadowRect.left + DropShadowOffset;

	if (ResetFontFlag)
	{
		ResetFontFlag = false;
		font->OnResetDevice();

	}

	if (ProxyInterface != NULL && font == NULL)
	{
		HFONT FontCharacteristics = CreateFontA(14, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "Arial");
		if (FontCharacteristics != NULL)
		{
			Logging::LogDebug() << __FUNCTION__ << " Creating font...";
			D3DXCreateFont(ProxyInterface, FontCharacteristics, &font);
			DeleteObject(FontCharacteristics);
		}
	}

	if (font != NULL)
	{
		font->DrawTextA(FontStruct.String, -1, &DropShadowRect, FontStruct.Format, BlackColor);
		font->DrawTextA(FontStruct.String, -1, &FontStruct.Rect, FontStruct.Format, GreenColor);
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
	if (font)
	{
		font->OnLostDevice();
		ResetFontFlag = true;
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
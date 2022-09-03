#include "Overlay.h"

const int rectOffset		= 40;
const int TextSpacing		= 15;
const int FloatPrecision	= 4;
const int KMConstant		= 500000;
const float AntiJitterValue	= 0.0001f;
const int DropShadowOffset	= 1;

LPD3DXFONT font = nullptr;
bool ResetFontFlag = false;

void Overlay::DrawInfoOverlay(LPDIRECT3DDEVICE8 ProxyInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	D3D8TEXT TextStruct;
	TextStruct.Format = DT_NOCLIP | DT_SINGLELINE;
	TextStruct.Rect.left = BufferWidth - 205;
	TextStruct.Rect.top = rectOffset;
	TextStruct.Rect.right = BufferWidth;
	TextStruct.Rect.bottom = rectOffset + 15;
	TextStruct.Color = D3DCOLOR_ARGB(255, 153, 217, 234);

	std::string OvlString = "INFO MENU (CTRL + I) ";

	TextStruct.String = OvlString.c_str();

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Action Difficulty: ";
	OvlString.append(ActionDifficulty[GetActionDifficulty()]);

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Riddle Difficulty: ";
	OvlString.append(RiddleDifficulty[GetRiddleDifficulty()]);

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Saves: ";
	OvlString.append(std::to_string(GetNumberOfSaves()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Total Time: ";
	OvlString.append(SecondsToTimeString((int)GetInGameTime()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Walking Distance: ";
	OvlString.append(FloatToStr(GetWalkingDistance() / KMConstant, 2));
	OvlString.append("km");

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Running Distance: ";
	OvlString.append(FloatToStr(GetRunningDistance() / KMConstant, 2));
	OvlString.append("km");

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Items: ";
	OvlString.append(std::to_string(GetItemsCollected()));
	OvlString.append("(+");
	OvlString.append(std::to_string(bitCount(GetSecretItemsCollected())));
	OvlString.append(")");

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Shooting Kills: ";
	OvlString.append(std::to_string(GetShootingKills()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Fighting Kills: ";
	OvlString.append(std::to_string(GetMeleeKills()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Boat Stage Time: ";
	OvlString.append(SecondsToMsTimeString((int)GetBoatStageTime()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Boat Max Speed: ";
	OvlString.append(FloatToStr(GetBoatMaxSpeed(), 2));
	OvlString.append("m/s");

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Total Damage: ";
	OvlString.append(FloatToStr(GetDamagePointsTaken(), 2));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);
}

void Overlay::DrawDebugOverlay(LPDIRECT3DDEVICE8 ProxyInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	float CharYPos = GetJamesPosY();

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
	TextStruct.Color = D3DCOLOR_ARGB(255, 153, 255, 153);

	std::string OvlString = "DEBUG MENU (CTRL + D) ";

	TextStruct.String = OvlString.c_str();

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Game Resolution: ";
	OvlString.append(std::to_string(BufferWidth));
	OvlString.append("x");
	OvlString.append(std::to_string(BufferHeight));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Room ID: 0x";
	OvlString.append(IntToHexStr(GetRoomID()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Cutscene ID: 0x";
	OvlString.append(IntToHexStr(GetCutsceneID()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "FPS: ";
	OvlString.append(FloatToStr(GetFPSCounter(), FloatPrecision));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Char X Position: ";
	OvlString.append(FloatToStr(GetJamesPosX(), FloatPrecision));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Char Y Position: ";
	OvlString.append(FloatToStr(CharYPos, FloatPrecision));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Char Z Position: ";
	OvlString.append(FloatToStr(GetJamesPosZ(), FloatPrecision));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, TextStruct);
}

void Overlay::DrawDebugText(LPDIRECT3DDEVICE8 ProxyInterface, Overlay::D3D8TEXT FontStruct)
{
	Logging::LogDebug() << __FUNCTION__;

	D3DCOLOR BlackColor = D3DCOLOR_ARGB(255, 0, 0, 0);
	
	RECT DropShadowRect = FontStruct.Rect;
	DropShadowRect.top = DropShadowRect.top + DropShadowOffset;
	DropShadowRect.left = DropShadowRect.left + DropShadowOffset;

	if (ResetFontFlag)
	{
		ResetFontFlag = false;
		font->OnResetDevice();

	}

	if (ProxyInterface != nullptr && font == nullptr)
	{
		HFONT FontCharacteristics = CreateFontA(16, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "Arial");
		if (FontCharacteristics != nullptr)
		{
			Logging::LogDebug() << __FUNCTION__ << " Creating font...";
			D3DXCreateFont(ProxyInterface, FontCharacteristics, &font);
			DeleteObject(FontCharacteristics);
		}
	}

	if (font != nullptr)
	{
		font->DrawTextA(FontStruct.String, -1, &DropShadowRect, FontStruct.Format, BlackColor);
		font->DrawTextA(FontStruct.String, -1, &FontStruct.Rect, FontStruct.Format, FontStruct.Color);
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

std::string Overlay::SecondsToMsTimeString(int time)
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

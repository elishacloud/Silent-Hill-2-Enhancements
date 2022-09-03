#include "Overlay.h"

LPD3DXFONT font = NULL;
bool ResetFontFlag = false;
auto LastColorChange = std::chrono::system_clock::now();
unsigned long frames = 0;
Overlay::D3D8TEXT MenuTestTextStruct;
Overlay::D3D8TEXT InfoOverlayTextStruct;
Overlay::D3D8TEXT DebugOverlayTextStruct;
// LowWhite, MediumWhite, FullWhite
D3DCOLOR WhiteArray[3] = { D3DCOLOR_ARGB(255, 50, 50, 50), D3DCOLOR_ARGB(255, 128, 128, 128), D3DCOLOR_ARGB(255, 255, 255, 255) };
int WhiteArrayIndex = 2;

void Overlay::DrawOverlays(LPDIRECT3DDEVICE8 ProxyInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	InitializeDataStructs();

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

	// Menu Test
	if (EnableMenuTest)
	{
		DrawMenuTestOverlay(ProxyInterface);
	}
}

void Overlay::DrawInfoOverlay(LPDIRECT3DDEVICE8 ProxyInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	int FloatPrecision = 4, KMConstant = 500000;

	std::string OvlString = "INFO MENU (CTRL + I) ";

	InfoOverlayTextStruct.String = OvlString.c_str();

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Action Difficulty: ";
	OvlString.append(ActionDifficulty[GetActionDifficulty()]);

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Riddle Difficulty: ";
	OvlString.append(RiddleDifficulty[GetRiddleDifficulty()]);

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Saves: ";
	OvlString.append(std::to_string(GetNumberOfSaves()));

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Total Time: ";
	OvlString.append(SecondsToTimeString(GetInGameTime()));

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Walking Distance: ";
	OvlString.append(FloatToStr(GetWalkingDistance() / KMConstant, 2));
	OvlString.append("km");

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Running Distance: ";
	OvlString.append(FloatToStr(GetRunningDistance() / KMConstant, 2));
	OvlString.append("km");

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Items: ";
	OvlString.append(std::to_string(GetItemsCollected()));
	OvlString.append("(+");
	OvlString.append(std::to_string(bitCount(GetSecretItemsCollected())));
	OvlString.append(")");

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Shooting Kills: ";
	OvlString.append(std::to_string(GetShootingKills()));

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Fighting Kills: ";
	OvlString.append(std::to_string(GetMeleeKills()));

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Boat Stage Time: ";
	OvlString.append(SecondsToMsTimeString(GetBoatStageTime()));

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Boat Max Speed: ";
	OvlString.append(FloatToStr(GetBoatMaxSpeed(), 2));
	OvlString.append("m/s");

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Total Damage: ";
	OvlString.append(FloatToStr(GetDamagePointsTaken(), 2));

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, InfoOverlayTextStruct);
}

void Overlay::DrawMenuTestOverlay(LPDIRECT3DDEVICE8 ProxyInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	int FloatPrecision = 4;

	frames++;

	if (ChangeMenuTestColor())
	{
		switch (WhiteArrayIndex)
		{
		case 0:
		case 1:
			WhiteArrayIndex++;
			break;
		case 2:
			WhiteArrayIndex = 0;
			break;
		}

	}

	MenuTestTextStruct.Color = WhiteArray[WhiteArrayIndex];

	std::string OvlString = "Menu Test v0.1";

	MenuTestTextStruct.String = OvlString.c_str();

	DrawDropShadowedText(ProxyInterface, MenuTestTextStruct);

	OvlString = "Frames: ";
	OvlString.append(std::to_string(frames));

	MenuTestTextStruct.String = OvlString.c_str();
	MenuTestTextStruct.Rect.top += 15;
	MenuTestTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, MenuTestTextStruct);

}

void Overlay::DrawDebugOverlay(LPDIRECT3DDEVICE8 ProxyInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	int FloatPrecision = 4;
	float CharYPos = GetJamesPosY();
	float AntiJitterValue = 0.0001;

	// Lock value at 0 if close enough, to avoid a rapidly changing number.
	if (CharYPos > -AntiJitterValue && CharYPos < AntiJitterValue)
	{
		CharYPos = 0;
	}

	std::string OvlString = "DEBUG MENU (CTRL + D) ";

	DebugOverlayTextStruct.String = OvlString.c_str();

	DrawDropShadowedText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "Game Resolution: ";
	OvlString.append(std::to_string(ResolutionWidth));
	OvlString.append("x");
	OvlString.append(std::to_string(ResolutionHeight));

	DebugOverlayTextStruct.String = OvlString.c_str();
	DebugOverlayTextStruct.Rect.top += 15;
	DebugOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "Room ID: 0x";
	OvlString.append(IntToHexStr(GetRoomID()));

	DebugOverlayTextStruct.String = OvlString.c_str();
	DebugOverlayTextStruct.Rect.top += 15;
	DebugOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "Cutscene ID: 0x";
	OvlString.append(IntToHexStr(GetCutsceneID()));

	DebugOverlayTextStruct.String = OvlString.c_str();
	DebugOverlayTextStruct.Rect.top += 15;
	DebugOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "FPS: ";
	OvlString.append(FloatToStr(GetFPSCounter(), FloatPrecision));

	DebugOverlayTextStruct.String = OvlString.c_str();
	DebugOverlayTextStruct.Rect.top += 15;
	DebugOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "Char X Position: ";
	OvlString.append(FloatToStr(GetJamesPosX(), FloatPrecision));

	DebugOverlayTextStruct.String = OvlString.c_str();
	DebugOverlayTextStruct.Rect.top += 15;
	DebugOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "Char Y Position: ";
	OvlString.append(FloatToStr(CharYPos, FloatPrecision));

	DebugOverlayTextStruct.String = OvlString.c_str();
	DebugOverlayTextStruct.Rect.top += 15;
	DebugOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "Char Z Position: ";
	OvlString.append(FloatToStr(GetJamesPosZ(), FloatPrecision));

	DebugOverlayTextStruct.String = OvlString.c_str();
	DebugOverlayTextStruct.Rect.top += 15;
	DebugOverlayTextStruct.Rect.bottom += 15;

	DrawDropShadowedText(ProxyInterface, DebugOverlayTextStruct);

}


void Overlay::DrawDropShadowedText(LPDIRECT3DDEVICE8 ProxyInterface, Overlay::D3D8TEXT FontStruct)
{
	Logging::LogDebug() << __FUNCTION__;

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
		HFONT FontCharacteristics = CreateFontA(16, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "Arial");
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

bool Overlay::ChangeMenuTestColor()
{
	auto Now = std::chrono::system_clock::now();
	auto Ms = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastColorChange);

	// Every third of a second
	if (Ms.count() >= 300)
	{
		LastColorChange = Now;
		return true;
	}

	return false;
}

void Overlay::InitializeDataStructs()
{
	Logging::LogDebug() << __FUNCTION__;

	int rectOffset = 40;

	InfoOverlayTextStruct.Format = DT_NOCLIP | DT_SINGLELINE;
	InfoOverlayTextStruct.Rect.left = ResolutionWidth - 205;
	InfoOverlayTextStruct.Rect.top = rectOffset;
	InfoOverlayTextStruct.Rect.right = ResolutionWidth;
	InfoOverlayTextStruct.Rect.bottom = rectOffset + 15;
	InfoOverlayTextStruct.Color = D3DCOLOR_ARGB(255, 153, 217, 234);

	MenuTestTextStruct.Format = DT_NOCLIP | DT_SINGLELINE;
	MenuTestTextStruct.Rect.left = ResolutionWidth - 205;
	MenuTestTextStruct.Rect.top = ResolutionHeight - rectOffset;
	MenuTestTextStruct.Rect.right = ResolutionWidth;
	MenuTestTextStruct.Rect.bottom = MenuTestTextStruct.Rect.top + 15;
	MenuTestTextStruct.Color = WhiteArray[2];

	DebugOverlayTextStruct.Format = DT_NOCLIP | DT_SINGLELINE;
	DebugOverlayTextStruct.Rect.left = rectOffset;
	DebugOverlayTextStruct.Rect.top = rectOffset;
	DebugOverlayTextStruct.Rect.right = rectOffset + 300;
	DebugOverlayTextStruct.Rect.bottom = rectOffset + 15;
	DebugOverlayTextStruct.Color = D3DCOLOR_ARGB(255, 153, 255, 153);

}
#include "Overlay.h"

const int rectOffset		= 40;
const int TextSpacing		= 15;
const int FloatPrecision	= 4;
const int KMConstant		= 500000;
const float AntiJitterValue	= 0.0001f;
const int DropShadowOffset	= 1;

bool ResetFontFlag = false;
LPD3DXFONT DebugFont = nullptr;
LPD3DXFONT MenuTestFont = nullptr;
bool ResetDebugFontFlag = false;
bool ResetMenuTestFontFlag = false;
auto LastColorChange = std::chrono::system_clock::now();
unsigned long frames = 0;
Overlay::D3D8TEXT MenuTestTextStruct;
Overlay::D3D8TEXT InfoOverlayTextStruct;
Overlay::D3D8TEXT DebugOverlayTextStruct;
Overlay::D3D8TEXT ControlMenuTestTextStruct;
// LowWhite, MediumWhite, FullWhite
D3DCOLOR WhiteArray[3] = { D3DCOLOR_ARGB(255, 50, 50, 50), D3DCOLOR_ARGB(255, 128, 128, 128), D3DCOLOR_ARGB(255, 255, 255, 255) };
int WhiteArrayIndex = 2;

void Overlay::DrawOverlays(LPDIRECT3DDEVICE8 ProxyInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	InitializeDataStructs();

	// In the pause menu, skip drawing
	if (GetEventIndex() == 0x10) return;

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

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Action Difficulty: ";
	OvlString.append(ActionDifficulty[GetActionDifficulty()]);

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += 15;
	InfoOverlayTextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Riddle Difficulty: ";
	OvlString.append(RiddleDifficulty[GetRiddleDifficulty()]);

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Saves: ";
	OvlString.append(std::to_string(GetNumberOfSaves()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Total Time: ";
	OvlString.append(SecondsToTimeString((int)GetInGameTime()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Walking Distance: ";
	OvlString.append(FloatToStr(GetWalkingDistance() / KMConstant, 2));
	OvlString.append("km");

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Running Distance: ";
	OvlString.append(FloatToStr(GetRunningDistance() / KMConstant, 2));
	OvlString.append("km");

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Items: ";
	OvlString.append(std::to_string(GetItemsCollected()));
	OvlString.append("(+");
	OvlString.append(std::to_string(bitCount(GetSecretItemsCollected())));
	OvlString.append(")");

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Shooting Kills: ";
	OvlString.append(std::to_string(GetShootingKills()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Fighting Kills: ";
	OvlString.append(std::to_string(GetMeleeKills()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Boat Stage Time: ";
	OvlString.append(SecondsToMsTimeString((int)GetBoatStageTime()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Boat Max Speed: ";
	OvlString.append(FloatToStr(GetBoatMaxSpeed(), 2));
	OvlString.append("m/s");

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);

	OvlString = "Total Damage: ";
	OvlString.append(FloatToStr(GetDamagePointsTaken(), 2));

	InfoOverlayTextStruct.String = OvlString.c_str();
	InfoOverlayTextStruct.Rect.top += TextSpacing;
	InfoOverlayTextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, InfoOverlayTextStruct);
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

	ControlMenuTestTextStruct.Color = WhiteArray[WhiteArrayIndex];

	std::string OvlString = "IGT: ";
	OvlString.append(GetIGTString());

	MenuTestTextStruct.String = OvlString.c_str();

	DrawMenuTestText(ProxyInterface, MenuTestTextStruct);

	OvlString = "v0.1";

	MenuTestTextStruct.String = OvlString.c_str();
	MenuTestTextStruct.Rect.top += 15;
	MenuTestTextStruct.Rect.bottom += 15;

	DrawMenuTestText(ProxyInterface, MenuTestTextStruct);
	DrawMenuTestText(ProxyInterface, ControlMenuTestTextStruct);

	/*
	OvlString = "Frames: ";
	OvlString.append(std::to_string(frames));

	MenuTestTextStruct.String = OvlString.c_str();
	MenuTestTextStruct.Rect.top += TextSpacing;
	MenuTestTextStruct.Rect.bottom += TextSpacing;

	DrawMenuTestText(ProxyInterface, MenuTestTextStruct);
	*/
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

	std::string OvlString = "DEBUG MENU (CTRL + D) ";

	DebugOverlayTextStruct.String = OvlString.c_str();

	DrawDebugText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "Game Resolution: ";
	OvlString.append(std::to_string(BufferWidth));
	OvlString.append("x");
	OvlString.append(std::to_string(BufferHeight));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "Room ID: 0x";
	OvlString.append(IntToHexStr(GetRoomID()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "Cutscene ID: 0x";
	OvlString.append(IntToHexStr(GetCutsceneID()));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "FPS: ";
	OvlString.append(FloatToStr(GetFPSCounter(), FloatPrecision));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "Char X Position: ";
	OvlString.append(FloatToStr(GetJamesPosX(), FloatPrecision));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "Char Y Position: ";
	OvlString.append(FloatToStr(CharYPos, FloatPrecision));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += TextSpacing;
	TextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, DebugOverlayTextStruct);

	OvlString = "Char Z Position: ";
	OvlString.append(FloatToStr(GetJamesPosZ(), FloatPrecision));

	DebugOverlayTextStruct.String = OvlString.c_str();
	DebugOverlayTextStruct.Rect.top += TextSpacing;
	DebugOverlayTextStruct.Rect.bottom += TextSpacing;

	DrawDebugText(ProxyInterface, DebugOverlayTextStruct);

}


void Overlay::DrawDebugText(LPDIRECT3DDEVICE8 ProxyInterface, Overlay::D3D8TEXT FontStruct)
{
	Logging::LogDebug() << __FUNCTION__;

	D3DCOLOR BlackColor = D3DCOLOR_ARGB(255, 0, 0, 0);
	
	RECT DropShadowRect = FontStruct.Rect;
	DropShadowRect.top = DropShadowRect.top + DropShadowOffset;
	DropShadowRect.left = DropShadowRect.left + DropShadowOffset;

	if (ResetDebugFontFlag)
	{
		ResetDebugFontFlag = false;
		DebugFont->OnResetDevice();

	}

	if (ProxyInterface != nullptr && font == nullptr)
	{
		HFONT FontCharacteristics = CreateFontA(16, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "Arial");
		if (FontCharacteristics != nullptr)
		{
			Logging::LogDebug() << __FUNCTION__ << " Creating Debug font...";
			D3DXCreateFont(ProxyInterface, FontCharacteristics, &DebugFont);
			DeleteObject(FontCharacteristics);
		}
	}

	if (DebugFont != nullptr)
	{
		DebugFont->DrawTextA(FontStruct.String, -1, &DropShadowRect, FontStruct.Format, BlackColor);
		DebugFont->DrawTextA(FontStruct.String, -1, &FontStruct.Rect, FontStruct.Format, FontStruct.Color);
	}
}

void Overlay::DrawMenuTestText(LPDIRECT3DDEVICE8 ProxyInterface, Overlay::D3D8TEXT FontStruct)
{
	Logging::LogDebug() << __FUNCTION__;

	D3DCOLOR BlackColor = D3DCOLOR_ARGB(255, 0, 0, 0);

	int DropShadowOffset = 1;

	RECT DropShadowRect = FontStruct.Rect;
	DropShadowRect.top = DropShadowRect.top + DropShadowOffset;
	DropShadowRect.left = DropShadowRect.left + DropShadowOffset;

	if (ResetMenuTestFontFlag)
	{
		ResetMenuTestFontFlag = false;
		MenuTestFont->OnResetDevice();

	}

	if (ProxyInterface != NULL && MenuTestFont == NULL)
	{
		HFONT FontCharacteristics = CreateFontA(18, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "Arial");
		if (FontCharacteristics != NULL)
		{
			Logging::LogDebug() << __FUNCTION__ << " Creating  Menu Test font...";
			D3DXCreateFont(ProxyInterface, FontCharacteristics, &MenuTestFont);
			DeleteObject(FontCharacteristics);
		}
	}

	if (MenuTestFont != NULL)
	{
		MenuTestFont->DrawTextA(FontStruct.String, -1, &DropShadowRect, FontStruct.Format, BlackColor);
		MenuTestFont->DrawTextA(FontStruct.String, -1, &FontStruct.Rect, FontStruct.Format, FontStruct.Color);
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
		MenuTestFont->OnLostDevice();
		ResetDebugFontFlag = true;
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

	ControlMenuTestTextStruct.Format = DT_NOCLIP | DT_SINGLELINE;
	ControlMenuTestTextStruct.Rect.left = ResolutionWidth - 205 + 87;
	ControlMenuTestTextStruct.Rect.top = ResolutionHeight - rectOffset + 15;
	ControlMenuTestTextStruct.Rect.right = ResolutionWidth;
	ControlMenuTestTextStruct.Rect.bottom = MenuTestTextStruct.Rect.top + 15 + 15;
	ControlMenuTestTextStruct.Color = WhiteArray[2];
	ControlMenuTestTextStruct.String = ".";

	DebugOverlayTextStruct.Format = DT_NOCLIP | DT_SINGLELINE;
	DebugOverlayTextStruct.Rect.left = rectOffset;
	DebugOverlayTextStruct.Rect.top = rectOffset;
	DebugOverlayTextStruct.Rect.right = rectOffset + 300;
	DebugOverlayTextStruct.Rect.bottom = rectOffset + 15;
	DebugOverlayTextStruct.Color = D3DCOLOR_ARGB(255, 153, 255, 153);

}

std::string Overlay::GetIGTString()
{
	float time = GetInGameTime();
	std::string TimeString = "";
	int hours, minutes, seconds, milliseconds;

	hours = time / 3600;
	time -= hours * 3600;
	minutes = time / 60;
	time -= minutes * 60;

	TimeString.append(std::to_string(hours));
	TimeString.append(":");
	TimeString.append(std::to_string(minutes));
	TimeString.append(":");
	TimeString.append(FloatToStr(time, 3));

	return TimeString;
}

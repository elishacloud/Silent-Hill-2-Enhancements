#include "Overlay.h"

const int rectOffset		= 40;
const int FloatPrecision	= 4;
const int KMConstant		= 500000;
const float AntiJitterValue	= 0.0001f;
const int DropShadowOffset	= 1;

bool ResetFontFlag = false;
LPD3DXFONT DebugFont = nullptr;
LPD3DXFONT MenuTestFont = nullptr;
struct OverlayTextColors {
	D3DCOLOR Black = D3DCOLOR_ARGB(255, 0, 0, 0);
	D3DCOLOR Green = D3DCOLOR_ARGB(255, 153, 255, 153);
	D3DCOLOR Tiel = D3DCOLOR_ARGB(255, 153, 217, 234);
} TextColors;

bool ResetDebugFontFlag = false;
bool ResetMenuTestFontFlag = false;
bool ResetIGTFontFlag = false;

auto LastColorChange = std::chrono::system_clock::now();
int WhiteArrayIndex = 2;
unsigned long frames = 0;

Overlay::D3D8TEXT MenuTestTextStruct;
Overlay::D3D8TEXT InfoOverlayTextStruct;
Overlay::D3D8TEXT DebugOverlayTextStruct;
Overlay::D3D8TEXT ControlMenuTestTextStruct;

DWORD FogEnableValue;
char dir[MAX_PATH] = { 0 };
LPCSTR FontName = "Arial";
LPCSTR IGTFontName = "Arial";
LPCSTR AlternateIGTFontName = "Noto Sans Mono Thin.ttf";
LPCSTR AlternateIGTFontNameNoExt = "Noto Sans Mono Thin";

DWORD FogEnableValue;

void Overlay::DrawOverlays(LPDIRECT3DDEVICE8 ProxyInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	InitializeDataStructs();
	SetIGTFont();

	// In the pause menu, skip drawing
	if (GetEventIndex() == 0x10) return;

	// nVidia fix
	ProxyInterface->GetRenderState(D3DRS_FOGENABLE, &FogEnableValue);
	ProxyInterface->SetRenderState(D3DRS_FOGENABLE, 0x0);

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

	ProxyInterface->SetRenderState(D3DRS_FOGENABLE, FogEnableValue);
}

void Overlay::DrawInfoOverlay(LPDIRECT3DDEVICE8 ProxyInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	int FloatPrecision = 4, KMConstant = 500000;

	std::string OvlString = "INFO MENU (CTRL + I) ";
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
	OvlString.append("(+");
	OvlString.append(std::to_string(bitCount(GetSecretItemsCollected())));
	OvlString.append(")");

	OvlString.append("\rShooting Kills: ");
	OvlString.append(std::to_string(GetShootingKills()));

	OvlString.append("\rFighting Kills: ");
	OvlString.append(std::to_string(GetMeleeKills()));

	OvlString.append("\rBoat Stage Time: ");
	OvlString.append(BoatStageTimeString(GetBoatStageTime()));

	OvlString.append("\rBoat Max Speed: ");
	OvlString.append(FloatToStr(GetBoatMaxSpeed(), 2));
	OvlString.append("m/s");

	OvlString.append("\rTotal Damage: ");
	OvlString.append(FloatToStr(GetDamagePointsTaken(), 2));

	InfoOverlayTextStruct.String = OvlString.c_str();
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

	std::string OvlString = GetIGTString();

	MenuTestTextStruct.String = OvlString.c_str();

	DrawIGTText(ProxyInterface, MenuTestTextStruct);
	
	OvlString = "0.1";

	MenuTestTextStruct.String = OvlString.c_str();
	MenuTestTextStruct.Rect.top += 22;
	MenuTestTextStruct.Rect.left += 85;
	DrawMenuTestText(ProxyInterface, MenuTestTextStruct);

	DrawMenuTestText(ProxyInterface, ControlMenuTestTextStruct);

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

	OvlString.append("\rGame Resolution: ");
	OvlString.append(std::to_string(ResolutionWidth));
	OvlString.append("x");
	OvlString.append(std::to_string(BufferHeight));

	OvlString.append("\rRoom ID: 0x");
	OvlString.append(IntToHexStr(GetRoomID()));

	OvlString.append("\rCutscene ID: 0x");
	OvlString.append(IntToHexStr(GetCutsceneID()));

	OvlString.append("\rFPS: ");
	OvlString.append(FloatToStr(GetFPSCounter(), FloatPrecision));

	OvlString.append("\rChar X Position: ");
	OvlString.append(FloatToStr(GetJamesPosX(), FloatPrecision));

	OvlString.append("\rChar Y Position: ");
	OvlString.append(FloatToStr(CharYPos, FloatPrecision));

	OvlString.append("\rChar Z Position: ");
	OvlString.append(FloatToStr(GetJamesPosZ(), FloatPrecision));

	DebugOverlayTextStruct.String = OvlString.c_str();
	DrawDebugText(ProxyInterface, DebugOverlayTextStruct);

}

void Overlay::DrawDebugText(LPDIRECT3DDEVICE8 ProxyInterface, Overlay::D3D8TEXT FontStruct)
{
	Logging::LogDebug() << __FUNCTION__;
	
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
		HFONT FontCharacteristics = CreateFontA(16, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, PROOF_QUALITY, 0, FontName);
		if (FontCharacteristics != NULL)
		{
			Logging::LogDebug() << __FUNCTION__ << " Creating Debug font: " << FontName;
			D3DXCreateFont(ProxyInterface, FontCharacteristics, &DebugFont);
			DeleteObject(FontCharacteristics);
		}
	}

	if (DebugFont != nullptr)
	{
		DebugFont->DrawTextA(FontStruct.String, -1, &DropShadowRect, FontStruct.Format, TextColors.Black);
		DebugFont->DrawTextA(FontStruct.String, -1, &FontStruct.Rect, FontStruct.Format, FontStruct.Color);
	}
}

void Overlay::DrawMenuTestText(LPDIRECT3DDEVICE8 ProxyInterface, Overlay::D3D8TEXT FontStruct)
{
	Logging::LogDebug() << __FUNCTION__;

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
		HFONT FontCharacteristics = CreateFontA(14, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, PROOF_QUALITY, 0, FontName);
		if (FontCharacteristics != NULL)
		{
			Logging::LogDebug() << __FUNCTION__ << " Creating Menu Test font: " << FontName;
			D3DXCreateFont(ProxyInterface, FontCharacteristics, &MenuTestFont);
			DeleteObject(FontCharacteristics);
		}
	}

	if (MenuTestFont != NULL)
	{
		MenuTestFont->DrawTextA(FontStruct.String, -1, &DropShadowRect, FontStruct.Format, TextColors.Black);
		MenuTestFont->DrawTextA(FontStruct.String, -1, &FontStruct.Rect, FontStruct.Format, FontStruct.Color);
	}
}

void Overlay::DrawIGTText(LPDIRECT3DDEVICE8 ProxyInterface, Overlay::D3D8TEXT FontStruct)
{
	Logging::LogDebug() << __FUNCTION__;

	int DropShadowOffset = 1;

	RECT DropShadowRect = FontStruct.Rect;
	DropShadowRect.top = DropShadowRect.top + DropShadowOffset;
	DropShadowRect.left = DropShadowRect.left + DropShadowOffset;

	if (ResetIGTFontFlag)
	{
		ResetIGTFontFlag = false;
		IGTFont->OnResetDevice();
	}

	if (ProxyInterface != NULL && IGTFont == NULL)
	{
		HFONT FontCharacteristics = CreateFontA(22, 0, 0, 0, FW_REGULAR, 0, 0, 0, 0, 0, 0, PROOF_QUALITY, 0, IGTFontName);
		if (FontCharacteristics != NULL)
		{
			Logging::LogDebug() << __FUNCTION__ << " Creating Menu Test font: " << FontName;
			D3DXCreateFont(ProxyInterface, FontCharacteristics, &IGTFont);
			DeleteObject(FontCharacteristics);
		}
	}

	if (IGTFont != NULL)
	{
		IGTFont->DrawTextA(FontStruct.String, -1, &DropShadowRect, FontStruct.Format, TextColors.Black);
		IGTFont->DrawTextA(FontStruct.String, -1, &FontStruct.Rect, FontStruct.Format, FontStruct.Color);
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

	if (MenuTestFont)
	{
		MenuTestFont->OnLostDevice();
		ResetMenuTestFontFlag = true;
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

	int rectOffset = 40, MenuTestLeftOffset = 130;

	InfoOverlayTextStruct.Format = DT_NOCLIP | DT_LEFT;
	InfoOverlayTextStruct.Rect.left = ResolutionWidth - 205;
	InfoOverlayTextStruct.Rect.top = rectOffset;
	InfoOverlayTextStruct.Rect.right = ResolutionWidth;
	InfoOverlayTextStruct.Rect.bottom = rectOffset + 15;
	InfoOverlayTextStruct.Color = TextColors.Tiel;

	MenuTestTextStruct.Format = DT_NOCLIP | DT_LEFT;
	MenuTestTextStruct.Rect.left = ResolutionWidth - MenuTestLeftOffset;
	MenuTestTextStruct.Rect.top = ResolutionHeight - rectOffset - 15;
	MenuTestTextStruct.Rect.right = ResolutionWidth;
	MenuTestTextStruct.Rect.bottom = MenuTestTextStruct.Rect.top + 15;
	MenuTestTextStruct.Color = WhiteArray[2];

	ControlMenuTestTextStruct.Format = DT_NOCLIP | DT_LEFT;
	ControlMenuTestTextStruct.Rect.left = ResolutionWidth - MenuTestLeftOffset + 105;
	ControlMenuTestTextStruct.Rect.top = ResolutionHeight - rectOffset + 7;
	ControlMenuTestTextStruct.Rect.right = ResolutionWidth;
	ControlMenuTestTextStruct.Rect.bottom = MenuTestTextStruct.Rect.top + 15 + 35;
	ControlMenuTestTextStruct.Color = WhiteArray[2];
	ControlMenuTestTextStruct.String = ".";

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
	int hours, minutes, seconds, milliseconds;

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

std::string Overlay::GetFontPath()
{
	int LastSlashIndex = -1;
	std::string output = "";

	GetSH2FolderPath(dir, MAX_PATH);

	output = RemoveExeName(dir);
	output.append("sh2e\\font\\");
	output.append(AlternateIGTFontName);

	return output;
}

std::string Overlay::RemoveExeName(char* path)
{
	int LastSlashIndex = -1;
	std::string output = "";

	for (int i = 0; i < strlen(path); i++)
	{
		if (path[i] == '\\' || path[i] == '/')
			LastSlashIndex = i;
	}

	if (LastSlashIndex > 0)
	{
		dir[LastSlashIndex + 1] = '\0';
	}

	output.append(path);

	return output;
}

void Overlay::SetIGTFont()
{
	std::string path = GetFontPath();
	Logging::LogDebug() << __FUNCTION__ << path;

	if (AddFontResourceExA(path.c_str(), FR_PRIVATE, 0) > 0) 
	{
		IGTFontName = AlternateIGTFontNameNoExt;
	}

}

void Overlay::ReleaseFont()
{
	if (FontName == AlternateIGTFontNameNoExt)
		RemoveFontResourceExA(GetFontPath().c_str(), FR_PRIVATE, 0);
		
}

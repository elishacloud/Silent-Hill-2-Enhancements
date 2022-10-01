#pragma warning( disable : 4244 )
#include "Overlay.h"

const int rectOffset		= 40;
const int FloatPrecision	= 4;
const int KMConstant		= 500000;
const float AntiJitterValue	= 0.0001f;
const int DropShadowOffset	= 1;

LPCSTR FontName = "Arial";

LPD3DXFONT DebugFont = nullptr;
LPD3DXFONT MenuTestFont = nullptr;
LPD3DXFONT IGTFont = nullptr;

bool ResetDebugFontFlag = false;
bool ResetMenuTestFontFlag = false;
bool ResetIGTFontFlag = false;

auto LastColorChange = std::chrono::system_clock::now();
int WhiteArrayIndex = 2;

Overlay::D3D8TEXT MenuTestTextStruct;
Overlay::D3D8TEXT InfoOverlayTextStruct;
Overlay::D3D8TEXT DebugOverlayTextStruct;
Overlay::D3D8TEXT ControlMenuTestTextStruct;
LONG LastBufferWidth = 0;
LONG LastBufferHeight = 0;

DWORD FogEnableValue;

void Overlay::DrawOverlays(LPDIRECT3DDEVICE8 ProxyInterface)
{
	if (LastBufferWidth != BufferWidth || LastBufferHeight != BufferHeight)
		InitializeDataStructs();

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
	int SpecialItems = bitCount(GetSecretItemsCollected());

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

void Overlay::DrawMenuTestOverlay(LPDIRECT3DDEVICE8 ProxyInterface)
{
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

	std::string OvlString = "";

	if (EnableMenuTestIGT)
	{
		OvlString = GetIGTString();

		MenuTestTextStruct.String = OvlString.c_str();
		DrawIGTText(ProxyInterface, MenuTestTextStruct);
	}

	OvlString = "0.1";

	MenuTestTextStruct.String = OvlString.c_str();
	MenuTestTextStruct.Rect.top += 22;
	MenuTestTextStruct.Rect.left += 80;
	DrawMenuTestText(ProxyInterface, MenuTestTextStruct);
	MenuTestTextStruct.Rect.top -= 22;
	MenuTestTextStruct.Rect.left -= 80;

	// Pulsating dot
	DrawMenuTestText(ProxyInterface, ControlMenuTestTextStruct);

}

void Overlay::DrawDebugOverlay(LPDIRECT3DDEVICE8 ProxyInterface)
{
	float CharYPos = GetJamesPosY();

	// Lock value at 0 if close enough, to avoid a rapidly changing number.
	if (CharYPos > -AntiJitterValue && CharYPos < AntiJitterValue)
	{
		CharYPos = 0;
	}

	std::string OvlString = "DEBUG MENU (CTRL + D) ";

	OvlString.append("\rGame Resolution: ");
	OvlString.append(std::to_string(BufferWidth));
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

	OvlString.append(AuxDebugOvlString);

	DebugOverlayTextStruct.String = OvlString.c_str();
	DrawDebugText(ProxyInterface, DebugOverlayTextStruct);

}

void Overlay::DrawDebugText(LPDIRECT3DDEVICE8 ProxyInterface, Overlay::D3D8TEXT FontStruct)
{
	RECT DropShadowRect = FontStruct.Rect;
	DropShadowRect.top = DropShadowRect.top + DropShadowOffset;
	DropShadowRect.left = DropShadowRect.left + DropShadowOffset;

	if (ResetDebugFontFlag)
	{
		ResetDebugFontFlag = false;
		DebugFont->OnResetDevice();
	}

	if (ProxyInterface != nullptr && DebugFont == nullptr)
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
		HFONT FontCharacteristics = CreateFontA(22, 0, 0, 0, FW_REGULAR, 0, 0, 0, 0, 0, 0, PROOF_QUALITY, 0, FontName);
		if (FontCharacteristics != NULL)
		{
			Logging::LogDebug() << __FUNCTION__ << " Creating IGT font: " << FontName;
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

bool Overlay::ChangeMenuTestColor()
{
	auto Now = std::chrono::system_clock::now();
	auto DeltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastColorChange);

	// Every third of a second
	if (DeltaMs.count() >= 300)
	{
		LastColorChange = Now;
		return true;
	}

	return false;
}

void Overlay::InitializeDataStructs()
{
	Logging::LogDebug() << __FUNCTION__ << " Initializing Overlay Text Structs...";

	int MenuTestLeftOffset = 130;

	InfoOverlayTextStruct.Format = DT_NOCLIP | DT_LEFT;
	InfoOverlayTextStruct.Rect.left = BufferWidth - 205;
	InfoOverlayTextStruct.Rect.top = rectOffset;
	InfoOverlayTextStruct.Rect.right = BufferWidth;
	InfoOverlayTextStruct.Rect.bottom = rectOffset + 15;
	InfoOverlayTextStruct.Color = TextColors.Tiel;

	MenuTestTextStruct.Format = DT_NOCLIP | DT_LEFT;
	MenuTestTextStruct.Rect.left = BufferWidth - MenuTestLeftOffset;
	MenuTestTextStruct.Rect.top = BufferHeight - rectOffset - 15;
	MenuTestTextStruct.Rect.right = BufferWidth;
	MenuTestTextStruct.Rect.bottom = MenuTestTextStruct.Rect.top + 15;
	MenuTestTextStruct.Color = WhiteArray[2];

	ControlMenuTestTextStruct.Format = DT_NOCLIP | DT_LEFT;
	ControlMenuTestTextStruct.Rect.left = BufferWidth - MenuTestLeftOffset + 100;
	ControlMenuTestTextStruct.Rect.top = BufferHeight - rectOffset + 7;
	ControlMenuTestTextStruct.Rect.right = BufferWidth;
	ControlMenuTestTextStruct.Rect.bottom = MenuTestTextStruct.Rect.top + 15 + 30;
	ControlMenuTestTextStruct.Color = WhiteArray[2];
	ControlMenuTestTextStruct.String = ".";

	DebugOverlayTextStruct.Format = DT_NOCLIP | DT_LEFT;
	DebugOverlayTextStruct.Rect.left = rectOffset;
	DebugOverlayTextStruct.Rect.top = rectOffset;
	DebugOverlayTextStruct.Rect.right = rectOffset + 300;
	DebugOverlayTextStruct.Rect.bottom = rectOffset + 15;
	DebugOverlayTextStruct.Color = TextColors.Green;
	
	LastBufferWidth = BufferWidth;
	LastBufferHeight = BufferHeight;
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

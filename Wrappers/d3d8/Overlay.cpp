#include "Overlay.h"

LPD3DXFONT font = NULL;
bool ResetFontFlag = false;

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

	Overlay::D3D8TEXT TextStruct;
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

	int ShootingKills = GetShootingKills();
	int MeleeKills = GetMeleeKills();

	OvlString = "Kills Gun/Melee/Total: ";
	OvlString.append(std::to_string(ShootingKills));
	OvlString.append(" / ");
	OvlString.append(std::to_string(MeleeKills));
	OvlString.append(" / ");
	OvlString.append(std::to_string(ShootingKills + MeleeKills));

	TextStruct.String = OvlString.c_str();
	TextStruct.Rect.top += 15;
	TextStruct.Rect.bottom += 15;

	DrawDebugText(ProxyInterface, TextStruct);

	OvlString = "Boat Max Speed: ";
	OvlString.append(FloatToStr(GetBoatMaxSpeed(), 2));

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
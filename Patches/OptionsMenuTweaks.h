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
#include "Logging\Logging.h"
#include "Patches\Patches.h"
#include "Common\Utils.h"
#include "Wrappers\d3d8\d3d8wrapper.h"
#include "Common\Settings.h"

#define BEZEL_VERT_NUM 6
#define RECT_VERT_NUM 4

#define BUTTONS_NUM 22
#define BUTTON_ICONS_NUM 26
#define BUTTON_QUADS_NUM 11

enum ControllerButton
{
	BUTTON_SQUARE,
	BUTTON_CROSS,
	BUTTON_CIRCLE,
	BUTTON_TRIANGLE,
	BUTTON_L1,
	BUTTON_R1,
	BUTTON_L2,
	BUTTON_R2,
	BUTTON_SELECT,
	BUTTON_START,
	BUTTON_L3,
	BUTTON_R3,

	L_UP = 100,
	L_DOWN,
	L_LEFT,
	L_RIGHT,

	R_UP = 200,
	R_DOWN,
	R_LEFT,
	R_RIGHT,

	D_UP = 300,
	D_DOWN,
	D_LEFT,
	D_RIGHT,

	LEVER_DIRECTIONAL = 998,
	DPAD = 999,
};

enum OptionState
{
	STANDARD,
	SELECTED,
	LOCKED,
};

struct ColorVertex
{
	D3DXVECTOR3 coords;
	float rhw = 1.f;
	D3DCOLOR color;
};

struct TexturedVertex
{
	D3DXVECTOR3 coords;
	float rhw = 1.f;
	float u, v;
};

struct IconQuad
{
	TexturedVertex vertices[4];
	OptionState state;
};

struct Bezels
{
	ColorVertex TopVertices[BEZEL_VERT_NUM];
	ColorVertex BotVertices[BEZEL_VERT_NUM];
};

struct CenterRects
{
	ColorVertex vertices[RECT_VERT_NUM];
};

struct CO_TEXT
{
	LPCSTR     String = "";
	RECT			  Rect;
	DWORD           Format;
	D3DCOLOR         Color;
};

class MasterVolume
{
public:
	void HandleMasterVolume(LPDIRECT3DDEVICE8 ProxyInterface);
	void ChangeMasterVolumeValue(int delta);

	void HandleConfirmOptions(bool ConfirmChange);

private:
	bool EnteredOptionsMenu = false;
};

struct SliderColor
{
	D3DCOLOR color[2];
};

class MasterVolumeSlider
{
public:
	MasterVolumeSlider() = default;

	void DrawSlider(LPDIRECT3DDEVICE8 ProxyInterface, int value, bool ValueChanged);

private:
	long LastBufferWidth = 0;
	long LastBufferHeight = 0;

	// Color 0 = selected, 1 not selected
	D3DCOLOR LightGrayBezel[2] = { D3DCOLOR_ARGB(0x40, 0xB0, 0xB0, 0xB0), D3DCOLOR_ARGB(0x40, 0x44, 0x44, 0x44) };
	D3DCOLOR DarkGrayBezel[2] = { D3DCOLOR_ARGB(0x40, 0x40, 0x40, 0x40), D3DCOLOR_ARGB(0x40, 0x28, 0x28, 0x28) };

	D3DCOLOR LightGoldBezel[2] = { D3DCOLOR_ARGB(0x40, 0xB0, 0xB0, 0x58), D3DCOLOR_ARGB(0x40, 0x44, 0x44, 0x2E) };
	D3DCOLOR DarkGoldBezel[2] = { D3DCOLOR_ARGB(0x40, 0x40, 0x40, 0x20), D3DCOLOR_ARGB(0x40, 0x28, 0x28, 0x20) };

	D3DCOLOR InactiveGraySquare[2] = { D3DCOLOR_ARGB(0x40, 0x50, 0x50, 0x50), D3DCOLOR_ARGB(0x40, 0x2C, 0x2C, 0x2C) };
	D3DCOLOR ActiveGraySquare[2] = { D3DCOLOR_ARGB(0x40, 0x80, 0x80, 0x80), D3DCOLOR_ARGB(0x40, 0x38, 0x38, 0x38) };

	D3DCOLOR InactiveGoldSquare[2] = { D3DCOLOR_ARGB(0x40, 0x50, 0x50, 0x25), D3DCOLOR_ARGB(0x40, 0x2C, 0x2C, 0x20) };
	D3DCOLOR ActiveGoldSquare[2] = { D3DCOLOR_ARGB(0x40, 0x80, 0x80, 0x40), D3DCOLOR_ARGB(0x40, 0x38, 0x38, 0x28) };

	// Bezel L flipped horizontally
#pragma warning(disable : 4305)
	ColorVertex BezelVertices[BEZEL_VERT_NUM] =
	{
		{ D3DXVECTOR3( 10.547, -21.5625, 0.000) , 1.f, NULL},
		{ D3DXVECTOR3(  5.859, -17.8125, 0.000) , 1.f, NULL},
		{ D3DXVECTOR3( 10.547,  21.5625, 0.000) , 1.f, NULL},
		{ D3DXVECTOR3(  5.859,  17.8125, 0.000) , 1.f, NULL}, // Alignment vertex
		{ D3DXVECTOR3(-10.547,  21.5625, 0.000) , 1.f, NULL},
		{ D3DXVECTOR3( -5.859,  17.8125, 0.000) , 1.f, NULL}
	};
#pragma warning(disable : 4305)
	ColorVertex RectangleVertices[RECT_VERT_NUM] =
	{
		{ D3DXVECTOR3( 5.859,  17.8125, 0.000), 1.000, NULL}, // Alignment vertex
		{ D3DXVECTOR3( 5.859, -17.8125, 0.000), 1.000, NULL},
		{ D3DXVECTOR3(-5.859,  17.8125, 0.000), 1.000, NULL},
		{ D3DXVECTOR3(-5.859, -17.8125, 0.000), 1.000, NULL}
	};

	// Possible values 0x0 - 0xF
	Bezels FinalBezels[0x10];
	CenterRects FinalRects[0x10];

	void InitVertices();
};

class ButtonIcons
{
public:
	ButtonIcons() = default;

	void Init(LPDIRECT3DDEVICE8 ProxyInterface);
	void UpdateBinds();
	void ResetFont()
	{
		this->ResetFontFlag = true;
	}

	void HandleControllerIcons(LPDIRECT3DDEVICE8 ProxyInterface);
	void DrawIcons(LPDIRECT3DDEVICE8 ProxyInterface);

	void UpdateUVs()
	{
		for (int i = 0; i < BUTTON_QUADS_NUM; i++)
		{
			int CurrentIndex = GetControlOptionsSelectedOption() - 5 + i;

			if (CurrentIndex >= 0 && CurrentIndex <= 3)
			{
				this->quads[i].state = OptionState::LOCKED;
			}
			else if (i == 5 && GetControlOptionsIsToStopScrolling() != 1 && GetControlOptionsSelectedColumn() == 1)
			{
				this->quads[i].state = OptionState::SELECTED;
			}
			else
			{
				this->quads[i].state = OptionState::STANDARD;
			}

			//TODO avoid drawing if the input is not set, override game text value drawing only if what's drawing isn't ??? or -
			if (CurrentIndex < 0 || CurrentIndex >= BUTTONS_NUM)
			{
				this->SetQuadUV(i, 0.f, 0.f, 0.f, 0.f);
			} 
			else
			{
				this->SetQuadUV(i, this->GetUOffset(), this->GetVOffset(), this->GetUStartingValue(), this->GetVStartingValue(this->binds[CurrentIndex]));
			}
		}
	}

	void SetQuadUV(int index, float uOffset, float vOffset, float u, float v)
	{
		if (index > BUTTON_QUADS_NUM || index < 0)
		{
			Logging::Log() << __FUNCTION__ << " ERROR: index out of bounds.";
		}

		this->quads[index].vertices[0].u = u;
		this->quads[index].vertices[0].v = v;

		this->quads[index].vertices[1].u = u;
		this->quads[index].vertices[1].v = v + vOffset;

		this->quads[index].vertices[2].u = u + uOffset;
		this->quads[index].vertices[2].v = v + vOffset;

		this->quads[index].vertices[3].u = u + uOffset;
		this->quads[index].vertices[3].v = v;
	}

private:
	long LastBufferWidth = 0;
	long LastBufferHeight = 0;
	bool ResetFontFlag = false;

	IconQuad quads[BUTTON_QUADS_NUM];
	int quadsNum = BUTTON_QUADS_NUM;

	ColorVertex LineVertices[2][RECT_VERT_NUM] = {
	{
		{ D3DXVECTOR3(600.f,  1.f, 0.000), 1.000, D3DCOLOR_ARGB(0x40, 0x80, 0x80, 0x80)},
		{ D3DXVECTOR3(600.f, -1.f, 0.000), 1.000, D3DCOLOR_ARGB(0x40, 0x80, 0x80, 0x80)},
		{ D3DXVECTOR3(-600.f,  1.f, 0.000), 1.000, D3DCOLOR_ARGB(0x40, 0x80, 0x80, 0x80)},
		{ D3DXVECTOR3(-600.f, -1.f, 0.000), 1.000, D3DCOLOR_ARGB(0x40, 0x80, 0x80, 0x80)}}, 
		{{ D3DXVECTOR3(600.f,  1.f, 0.000), 1.000, D3DCOLOR_ARGB(0x40, 0x80, 0x80, 0x80)},
		{ D3DXVECTOR3(600.f, -1.f, 0.000), 1.000, D3DCOLOR_ARGB(0x40, 0x80, 0x80, 0x80)},
		{ D3DXVECTOR3(-600.f,  1.f, 0.000), 1.000, D3DCOLOR_ARGB(0x40, 0x80, 0x80, 0x80)},
		{ D3DXVECTOR3(-600.f, -1.f, 0.000), 1.000, D3DCOLOR_ARGB(0x40, 0x80, 0x80, 0x80)}} 
	};

	BYTE* ControllerBindsAddr = nullptr;
	ControllerButton binds[BUTTONS_NUM];
	int BindsNum = BUTTONS_NUM;

	LPDIRECT3DTEXTURE8  ButtonIconsTexture = NULL;
	DWORD SubtractionPixelShader = NULL;
	
	LPD3DXFONT ControlOptionsFont = nullptr;
	LPCSTR FontName = "Arial";
	CO_TEXT message;

	void DrawControlOptionsText(LPDIRECT3DDEVICE8 ProxyInterface, CO_TEXT FontStruct);

	float GetUStartingValue()
	{
		//TODO button set config
		int ButtonIconSetConfig = 0;

		if (ButtonIconSetConfig > 3 || ButtonIconSetConfig < 0)
		{
			Logging::Log() << __FUNCTION__ << " ERROR: Button Icon Set Value invalid: " << ButtonIconSetConfig;
			ButtonIconSetConfig = 0;
		}

		return (1.f / 4.f) * ButtonIconSetConfig;
	}

	float GetVOffset()
	{
		return 1.f / BUTTON_ICONS_NUM;
	}

	float GetUOffset()
	{
		return 1.f / 4.f;
	}

	float GetVStartingValue(ControllerButton button)
	{
		for (int i = 0; i < BUTTON_ICONS_NUM; i++)
		{
			if (this->TextureMap[i] == button)
			{
				return i * this->GetVOffset();
			}
		}

		Logging::Log() << __FUNCTION__ << " ERROR: Invalid keybind found: " << button;
		return 0.f;
	}

	ControllerButton TextureMap[BUTTON_ICONS_NUM] =
	{
		ControllerButton::L_UP,
		ControllerButton::L_RIGHT,
		ControllerButton::L_DOWN,
		ControllerButton::L_LEFT,

		//TODO
		ControllerButton::R_UP,
		ControllerButton::R_RIGHT,
		ControllerButton::R_DOWN,
		ControllerButton::R_LEFT,
		//TODO 
		ControllerButton::D_UP,
		ControllerButton::D_RIGHT,
		ControllerButton::D_DOWN,
		ControllerButton::D_LEFT,

		ControllerButton::BUTTON_TRIANGLE,
		ControllerButton::BUTTON_CIRCLE,
		ControllerButton::BUTTON_CROSS,
		ControllerButton::BUTTON_SQUARE,

		ControllerButton::BUTTON_L1,
		ControllerButton::BUTTON_R1,
		ControllerButton::BUTTON_L2,
		ControllerButton::BUTTON_R2,

		ControllerButton::BUTTON_SELECT,
		ControllerButton::BUTTON_START,

		ControllerButton::BUTTON_L3,
		ControllerButton::BUTTON_R3,

		//TODO PS and touchpad buttons
		ControllerButton::LEVER_DIRECTIONAL,
		ControllerButton::LEVER_DIRECTIONAL,
	};
};

extern MasterVolume MasterVolumeRef;
extern ButtonIcons ButtonIconsRef;
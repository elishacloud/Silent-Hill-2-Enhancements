/**
* Copyright (C) 2023 mercury501
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

struct MasterVertex
{
	D3DXVECTOR3 coords;
	float rhw = 1.f;
	D3DCOLOR color;
};

struct Bezels
{
	MasterVertex TopVertices[BEZEL_VERT_NUM];
	MasterVertex BotVertices[BEZEL_VERT_NUM];
};

struct CenterRects
{
	MasterVertex vertices[RECT_VERT_NUM];
};

class MasterVolume
{
private:
	bool EnteredOptionsMenu = false;

public:
	void HandleMasterVolume(LPDIRECT3DDEVICE8 ProxyInterface);
	void ChangeMasterVolumeValue(int delta);

	void HandleConfirmOptions(bool ConfirmChange);
};

struct SliderColor
{
	D3DCOLOR color[2];
};

class MasterVolumeSlider
{
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
	MasterVertex BezelVertices[BEZEL_VERT_NUM] =
	{
		{ D3DXVECTOR3( 10.547, -21.5625, 0.000) , 1.f, NULL},
		{ D3DXVECTOR3(  5.859, -17.8125, 0.000) , 1.f, NULL},
		{ D3DXVECTOR3( 10.547,  21.5625, 0.000) , 1.f, NULL},
		{ D3DXVECTOR3(  5.859,  17.8125, 0.000) , 1.f, NULL}, // Alignment vertex
		{ D3DXVECTOR3(-10.547,  21.5625, 0.000) , 1.f, NULL},
		{ D3DXVECTOR3( -5.859,  17.8125, 0.000) , 1.f, NULL}
	};
#pragma warning(disable : 4305)
	MasterVertex RectangleVertices[RECT_VERT_NUM] =
	{
		{ D3DXVECTOR3( 5.859,  17.8125, 0.000), 1.000, NULL}, // Alignment vertex
		{ D3DXVECTOR3( 5.859, -17.8125, 0.000), 1.000, NULL},
		{ D3DXVECTOR3(-5.859,  17.8125, 0.000), 1.000, NULL},
		{ D3DXVECTOR3(-5.859, -17.8125, 0.000), 1.000, NULL}
	};

	// Possible values 0x0 - 0xF
	Bezels FinalBezels[0x10];
	CenterRects FinalRects[0x10];

	void TranslateVertexBuffer(MasterVertex* vertices, int count, float x, float y);
	void RotateVertexBuffer(MasterVertex* vertices, int count, float angle);
	void ScaleVertexBuffer(MasterVertex* vertices, int count, float x, float y);

	void ApplyVertexBufferTransformation(MasterVertex* vertices, int count, D3DXMATRIX matrix);
	void SetVertexBufferColor(MasterVertex* vertices, int count, DWORD color);
	void CopyVertexBuffer(MasterVertex* source, MasterVertex* destination, int count);

	void InitVertices();

public:
	MasterVolumeSlider() = default;

	void DrawSlider(LPDIRECT3DDEVICE8 ProxyInterface, int value, bool ValueChanged);	
};

extern MasterVolume MasterVolumeRef;
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

struct MasterVertex
{
	D3DXVECTOR3 coords;
	float rhw = 1.f;
	DWORD color;
};

class MasterVolume
{
private:
	void DrawMasterVolumeSlider(LPDIRECT3DDEVICE8 ProxyInterface);

	void TranslateVertexBuffer(MasterVertex* vertices, int count, float x, float y);
	void RotateVertexBuffer(MasterVertex* vertices, int count, float angle);
	void ScaleVertexBuffer(MasterVertex* vertices, int count, float x, float y);
	
	void ApplyVertexBufferTransformation(MasterVertex* vertices, int count, D3DXMATRIX matrix);
	void SetVertexBufferColor(MasterVertex* vertices, int count, DWORD color);

public:
	void HandleMasterVolumeSlider(LPDIRECT3DDEVICE8 ProxyInterface);
};
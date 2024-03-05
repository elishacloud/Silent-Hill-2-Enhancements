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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches\Patches.h"

/*
	00465621 option high res name selected
	00465065 option high res name unselected
	00465262 option high res value unselected
	00465947 option high res value selected, shared: drawtextoverlayhook
	0046563e option high res description

	00465ec1 change option value
*/

class DisplayMode
{
public:
	DisplayMode()
	{
		options[0] = "Windowed"; //getDisplayModeOptionWindowedStr();
		options[1] = "Windowed Fullscreen"; //getDisplayModeOptionFullscreenWindowedStr();
		options[2] = "Fullscreen"; //getDisplayModeOptionFullscreenStr();

		CurrentOption = 0; //TODO last selected option
	}

	void IncrementOption()
	{
		if (CurrentOption == 2)
		{
			CurrentOption = 0;
		} 
		else
		{
			CurrentOption += 1;
		}
	}

	void DecrementOption()
	{
		if (CurrentOption == 0)
		{
			CurrentOption = 2;
		}
		else
		{
			CurrentOption -= 1;
		}
	}

	char* GetCurrentOptionStr()
	{
		return options[CurrentOption];
	}
private:
	char* options[3];
	int CurrentOption;

};

DisplayMode display_mode;

char* GetCurrentDisplayOptionStr()
{
	return display_mode.GetCurrentOptionStr();
}

void PatchDisplayMode()
{

}

void HandleDisplayMode()
{

}
/**
* Copyright (C) 2024 Elisha Riedlinger
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

#include "dinput8wrapper.h"

void genericQueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if (!ppvObj || !*ppvObj)
	{
		return;
	}

#define QUERYINTERFACE(x) \
	if (riid == IID_ ## x) \
		{ \
			*ppvObj = ProxyAddressLookupTableDinput8.FindAddress<m_ ## x>(*ppvObj); \
		}

#define CREATEINTERFACE(x) \
	if (riid == IID_ ## x) \
		{ \
			*ppvObj = new m_ ## x((x*)*ppvObj); \
		}

	QUERYINTERFACE(IDirectInput8A);
	QUERYINTERFACE(IDirectInputDevice8A);
	QUERYINTERFACE(IDirectInputEffect);
}

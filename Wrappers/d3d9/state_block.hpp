/**
* Copyright (C) 2014 Patrick Mours. All rights reserved.
* License: https://github.com/crosire/reshade#license
*
* Copyright (C) 2021 Elisha Riedlinger
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

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>
#include "com_ptr.hpp"

namespace reshade::d3d9
{
	class state_block
	{
	public:
		explicit state_block(IDirect3DDevice9 *device);
		~state_block();

		bool init_state_block();
		void release_state_block();

		void capture();
		void apply_and_release();

	private:
		void release_all_device_objects();

		com_ptr<IDirect3DDevice9> _device;
		com_ptr<IDirect3DStateBlock9> _state_block;
		UINT _num_simultaneous_rendertargets;
		D3DVIEWPORT9 _viewport;
		com_ptr<IDirect3DSurface9> _depth_stencil;
		com_ptr<IDirect3DSurface9> _render_targets[8];
	};
}

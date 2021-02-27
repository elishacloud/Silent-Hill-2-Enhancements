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

#include "state_block.hpp"
#include <algorithm>

reshade::d3d9::state_block::state_block(IDirect3DDevice9 *device)
{
	ZeroMemory(this, sizeof(*this));

	_device = device;

	D3DCAPS9 caps;
	device->GetDeviceCaps(&caps);
	_num_simultaneous_rendertargets = std::min(caps.NumSimultaneousRTs, DWORD(8));
}
reshade::d3d9::state_block::~state_block()
{
	release_all_device_objects();
}

void reshade::d3d9::state_block::capture()
{
	_state_block->Capture();

	_device->GetViewport(&_viewport);

	for (DWORD target = 0; target < _num_simultaneous_rendertargets; target++)
		_device->GetRenderTarget(target, &_render_targets[target]);
	_device->GetDepthStencilSurface(&_depth_stencil);
}
void reshade::d3d9::state_block::apply_and_release()
{
	_state_block->Apply();

	for (DWORD target = 0; target < _num_simultaneous_rendertargets; target++)
		_device->SetRenderTarget(target, _render_targets[target].get());
	_device->SetDepthStencilSurface(_depth_stencil.get());

	// Set viewport after render targets have been set, since 'SetRenderTarget' causes the viewport to be set to the full size of the render target
	_device->SetViewport(&_viewport);

	release_all_device_objects();
}

bool reshade::d3d9::state_block::init_state_block()
{
	return SUCCEEDED(_device->CreateStateBlock(D3DSBT_ALL, &_state_block));
}
void reshade::d3d9::state_block::release_state_block()
{
	_state_block.reset();
}
void reshade::d3d9::state_block::release_all_device_objects()
{
	_depth_stencil.reset();
	for (auto &render_target : _render_targets)
		render_target.reset();
}

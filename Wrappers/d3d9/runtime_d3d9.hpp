/**
* Copyright (C) 2014 Patrick Mours. All rights reserved.
* License: https://github.com/crosire/reshade#license
*
* Copyright (C) 2022 Elisha Riedlinger
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

#include "ReShade\Runtime\runtime.hpp"
#include "state_block.hpp"
#include "buffer_detection.hpp"

namespace reshade::d3d9
{
	class runtime_d3d9 : public runtime
	{
	public:
		runtime_d3d9(IDirect3DDevice9 *device, IDirect3DSwapChain9 *swapchain, buffer_detection *bdc);
		~runtime_d3d9();

		bool on_init(const D3DPRESENT_PARAMETERS &pp);
		bool get_gamma();
		void reset_gamma(bool reload);
		void on_reset();
		void on_present();

	private:
		bool init_effect(size_t index) override;
		void unload_effect(size_t index) override;
		void unload_effects() override;

		bool init_texture(texture &texture) override;
		void upload_texture(const texture &texture, const uint8_t *pixels) override;
		void destroy_texture(texture &texture) override;

		void render_technique(technique &technique) override;

		state_block _app_state;
		com_ptr<IDirect3D9> _d3d;
		const com_ptr<IDirect3DDevice9> _device;
		const com_ptr<IDirect3DSwapChain9> _swapchain;
		buffer_detection *const _buffer_detection;

		unsigned int _max_vertices = 0;
		unsigned int _num_samplers;
		unsigned int _num_simultaneous_rendertargets;
		unsigned int _behavior_flags;

		D3DFORMAT _backbuffer_format = D3DFMT_UNKNOWN;
		com_ptr<IDirect3DSurface9> _backbuffer;
		com_ptr<IDirect3DSurface9> _backbuffer_resolved;
		com_ptr<IDirect3DTexture9> _backbuffer_texture;
		com_ptr<IDirect3DSurface9> _backbuffer_texture_surface;

		HMODULE _d3d_compiler = nullptr;
		com_ptr<IDirect3DSurface9> _effect_stencil;
		com_ptr<IDirect3DVertexBuffer9> _effect_vertex_buffer;
		com_ptr<IDirect3DVertexDeclaration9> _effect_vertex_layout;

		std::unordered_map<DWORD, std::vector<BYTE>> _compile_cache;

		void update_depth_texture_bindings(com_ptr<IDirect3DSurface9> surface);

		com_ptr<IDirect3DTexture9> _depth_texture;
		com_ptr<IDirect3DSurface9> _depth_surface;

		bool _gamma_set = false;
		bool _disable_intz = false;
		bool _reset_buffer_detection = false;
		bool _filter_aspect_ratio = true;
		IDirect3DSurface9 *_depth_surface_override = nullptr;
	};
}

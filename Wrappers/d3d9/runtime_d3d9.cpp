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

#define RESHADE_FILE_LIST
#include "d3d9wrapper.h"
#include "d3dx9.h"
#include "Resource.h"

namespace reshade::d3d9
{
	struct d3d9_tex_data
	{
		com_ptr<IDirect3DTexture9> texture;
		com_ptr<IDirect3DSurface9> surface;
	};

	struct d3d9_pass_data
	{
		com_ptr<IDirect3DStateBlock9> stateblock;
		com_ptr<IDirect3DPixelShader9> pixel_shader;
		com_ptr<IDirect3DVertexShader9> vertex_shader;
		IDirect3DSurface9 *render_targets[8] = {};
		IDirect3DTexture9 *sampler_textures[16] = {};
	};

	struct d3d9_technique_data
	{
		DWORD num_samplers = 0;
		DWORD sampler_states[16][12] = {};
		IDirect3DTexture9 *sampler_textures[16] = {};
		DWORD constant_register_count = 0;
		std::vector<d3d9_pass_data> passes;
	};
}

reshade::d3d9::runtime_d3d9::runtime_d3d9(IDirect3DDevice9 *device, IDirect3DSwapChain9 *swapchain, buffer_detection *bdc) :
	_device(device), _swapchain(swapchain), _buffer_detection(bdc),
	_app_state(device)
{
	assert(bdc != nullptr);
	assert(device != nullptr);
	assert(swapchain != nullptr);

	_device->GetDirect3D(&_d3d);
	assert(_d3d != nullptr);

	D3DCAPS9 caps = {};
	_device->GetDeviceCaps(&caps);
	D3DDEVICE_CREATION_PARAMETERS creation_params = {};
	_device->GetCreationParameters(&creation_params);

	_renderer_id = 0x9000;

	if (D3DADAPTER_IDENTIFIER9 adapter_desc;
		SUCCEEDED(_d3d->GetAdapterIdentifier(creation_params.AdapterOrdinal, 0, &adapter_desc)))
	{
		_vendor_id = adapter_desc.VendorId;
		_device_id = adapter_desc.DeviceId;

		// Only the last 5 digits represents the version specific to a driver
		// See https://docs.microsoft.com/windows-hardware/drivers/display/version-numbers-for-display-drivers
		const DWORD driver_version = LOWORD(adapter_desc.DriverVersion.LowPart) + (HIWORD(adapter_desc.DriverVersion.LowPart) % 10) * 10000;
		Logging::Log() << "Running on " << adapter_desc.Description << " Driver " << (driver_version / 100) << '.' << (driver_version % 100);
	}

	_num_samplers = caps.MaxSimultaneousTextures;
	_num_simultaneous_rendertargets = std::min(caps.NumSimultaneousRTs, static_cast<DWORD>(8));
	_behavior_flags = creation_params.BehaviorFlags;

	subscribe_to_load_config([this](const ini_file &config) {
		config.get("D3D9", "DisableINTZ", _disable_intz);
		config.get("D3D9", "DepthCopyBeforeClears", _buffer_detection->preserve_depth_buffers);
		config.get("D3D9", "DepthCopyAtClearIndex", _buffer_detection->depthstencil_clear_index);
		config.get("D3D9", "UseAspectRatioHeuristics", _filter_aspect_ratio);

		if (_buffer_detection->depthstencil_clear_index == std::numeric_limits<UINT>::max())
			_buffer_detection->depthstencil_clear_index  = 0;
	});
	subscribe_to_save_config([this](ini_file &config) {
		config.set("D3D9", "DisableINTZ", _disable_intz);
		config.set("D3D9", "DepthCopyBeforeClears", _buffer_detection->preserve_depth_buffers);
		config.set("D3D9", "DepthCopyAtClearIndex", _buffer_detection->depthstencil_clear_index);
		config.set("D3D9", "UseAspectRatioHeuristics", _filter_aspect_ratio);
	});
}
reshade::d3d9::runtime_d3d9::~runtime_d3d9()
{
	if (_d3d_compiler != nullptr)
		FreeLibrary(_d3d_compiler);
}

bool reshade::d3d9::runtime_d3d9::on_init(const D3DPRESENT_PARAMETERS &pp)
{
	RECT window_rect = {};
	GetClientRect(pp.hDeviceWindow, &window_rect);

	_width = pp.BackBufferWidth;
	_height = pp.BackBufferHeight;
	_window_width = window_rect.right;
	_window_height = window_rect.bottom;
	_color_bit_depth = pp.BackBufferFormat == D3DFMT_A2B10G10R10 || pp.BackBufferFormat == D3DFMT_A2R10G10B10 ? 10 : 8;
	_backbuffer_format = pp.BackBufferFormat;

	// Get back buffer surface
	HRESULT hr = _swapchain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &_backbuffer);
	assert(SUCCEEDED(hr));

	if (pp.MultiSampleType != D3DMULTISAMPLE_NONE || (pp.BackBufferFormat == D3DFMT_X8R8G8B8 || pp.BackBufferFormat == D3DFMT_X8B8G8R8))
	{
		switch (_backbuffer_format)
		{
		case D3DFMT_X8R8G8B8:
			_backbuffer_format = D3DFMT_A8R8G8B8;
			break;
		case D3DFMT_X8B8G8R8:
			_backbuffer_format = D3DFMT_A8B8G8R8;
			break;
		}

		if (FAILED(_device->CreateRenderTarget(_width, _height, _backbuffer_format, D3DMULTISAMPLE_NONE, 0, FALSE, &_backbuffer_resolved, nullptr)))
			return false;
	}
	else
	{
		_backbuffer_resolved = _backbuffer;
	}

	// Create back buffer shader texture
	if (FAILED(_device->CreateTexture(_width, _height, 1, D3DUSAGE_RENDERTARGET, _backbuffer_format, D3DPOOL_DEFAULT, &_backbuffer_texture, nullptr)))
		return false;

	hr = _backbuffer_texture->GetSurfaceLevel(0, &_backbuffer_texture_surface);
	assert(SUCCEEDED(hr));

	// Create effect depth-stencil surface
	if (FAILED(_device->CreateDepthStencilSurface(_width, _height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, FALSE, &_effect_stencil, nullptr)))
		return false;

	// Create vertex layout for vertex buffer which holds vertex indices
	const D3DVERTEXELEMENT9 declaration[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	if (FAILED(_device->CreateVertexDeclaration(declaration, &_effect_vertex_layout)))
		return false;

	// Create state block object
	if (!_app_state.init_state_block())
		return false;

	return runtime::on_init(pp.hDeviceWindow);
}
bool reshade::d3d9::runtime_d3d9::get_gamma()
{
	return _gamma_set;
}
void reshade::d3d9::runtime_d3d9::reset_gamma(bool reload)
{
	subscribe_to_save_config([this, reload](ini_file &config) {

		config.reset_config();

		if (reload)
		{
			_gamma_set = true;

			runtime::on_reset(false);

			runtime::on_init(nullptr);

			update_and_render_effects();
		}
	});
}
void reshade::d3d9::runtime_d3d9::on_reset()
{
	runtime::on_reset();

	_app_state.release_state_block();

	_backbuffer.reset();
	_backbuffer_resolved.reset();
	_backbuffer_texture.reset();
	_backbuffer_texture_surface.reset();

	_max_vertices = 0;
	_effect_stencil.reset();
	_effect_vertex_buffer.reset();
	_effect_vertex_layout.reset();

	_depth_texture.reset();
	_depth_surface.reset();

	_has_depth_texture = false;
	_depth_surface_override = nullptr;

	_compile_cache.clear();
}

void reshade::d3d9::runtime_d3d9::on_present()
{
	if (!_is_initialized)
	{
		return;
	}

	assert(_buffer_detection != nullptr);
	_vertices = _buffer_detection->total_vertices();
	_drawcalls = _buffer_detection->total_drawcalls();

	// Disable INTZ replacement while high network activity is detected, since the option is not available in the UI then, but artifacts may occur without it
	_buffer_detection->disable_intz = _disable_intz;

	update_depth_texture_bindings(_buffer_detection->find_best_depth_surface(_filter_aspect_ratio ? _width : 0, _height, _depth_surface_override));

	_app_state.capture();
	BOOL software_rendering_enabled = FALSE;
	if ((_behavior_flags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0)
	{
		software_rendering_enabled = _device->GetSoftwareVertexProcessing(),
		_device->SetSoftwareVertexProcessing(FALSE); // Disable software vertex processing since it is incompatible with programmable shaders
	}

	// Resolve MSAA back buffer if MSAA is active
	if (_backbuffer_resolved != _backbuffer)
	{
		_device->StretchRect(_backbuffer.get(), nullptr, _backbuffer_resolved.get(), nullptr, D3DTEXF_NONE);
	}

	update_and_render_effects();
	runtime::on_present();

	// Stretch main render target back into MSAA back buffer if MSAA is active
	if (_backbuffer_resolved != _backbuffer)
	{
		_device->StretchRect(_backbuffer_resolved.get(), nullptr, _backbuffer.get(), nullptr, D3DTEXF_NONE);
	}

	// Apply previous state from application
	_app_state.apply_and_release();
	if ((_behavior_flags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0)
	{
		_device->SetSoftwareVertexProcessing(software_rendering_enabled);
	}

	// Can only reset the tracker after the state block has been applied, to ensure current depth-stencil binding is updated correctly
	if (_reset_buffer_detection)
	{
		_buffer_detection->reset(true);
		_reset_buffer_detection = false;
	}
}

bool reshade::d3d9::runtime_d3d9::init_effect(size_t index)
{
	effect &effect = _effects[index];

	bool is_gamma = (_gamma_set && effect.source_file.compare(GammaEffectName + ".fx") == 0);

	// Add specialization constant defines to source code
	effect.preamble +=
		"#define COLOR_PIXEL_SIZE 1.0 / " + std::to_string(_width) + ", 1.0 / " + std::to_string(_height) + "\n"
		"#define DEPTH_PIXEL_SIZE COLOR_PIXEL_SIZE\n"
		"#define SV_DEPTH_PIXEL_SIZE DEPTH_PIXEL_SIZE\n"
		"#define SV_TARGET_PIXEL_SIZE COLOR_PIXEL_SIZE\n";

	const std::string hlsl_vs = effect.preamble + effect.module.hlsl;
	const std::string hlsl_ps = effect.preamble + "#define POSITION VPOS\n" + effect.module.hlsl;

	std::unordered_map<std::string, com_ptr<IUnknown>> entry_points;

	// Compile the generated HLSL source code to DX byte code
	DWORD loop_count = 0;
	for (const reshadefx::entry_point &entry_point : effect.module.entry_points)
	{
		DWORD entry_index = (index << 16) + loop_count;
		size_t hlsl_size = 0;
		const char *profile = nullptr, *hlsl = nullptr;
		com_ptr<ID3DBlob> compiled, d3d_errors;
		LPVOID compiled_buffer = nullptr;

		switch (entry_point.type)
		{
		case reshadefx::shader_type::vs:
			hlsl = hlsl_vs.c_str();
			hlsl_size = hlsl_vs.size();
			profile = "vs_3_0";
			break;
		case reshadefx::shader_type::ps:
			hlsl = hlsl_ps.c_str();
			hlsl_size = hlsl_ps.size();
			profile = "ps_3_0";
			break;
		case reshadefx::shader_type::cs:
			effect.errors += "Compute shaders are not supported in ";
			effect.errors += "D3D9";
			effect.errors += '.';
			return false;
		}

		HRESULT hr = D3D_OK;

		if (_compile_cache[entry_index].size() == 0 || is_gamma)
		{
			hr = D3DCompile(
				hlsl, hlsl_size,
				nullptr, nullptr, nullptr,
				entry_point.name.c_str(),
				profile,
				_performance_mode ? D3DCOMPILE_OPTIMIZATION_LEVEL3 : D3DCOMPILE_OPTIMIZATION_LEVEL1, 0,
				&compiled, &d3d_errors);

			if (d3d_errors != nullptr) // Append warnings to the output error string as well
			{
				effect.errors.append(static_cast<const char *>(d3d_errors->GetBufferPointer()), d3d_errors->GetBufferSize() - 1); // Subtracting one to not append the null-terminator as well
			}

			// No need to setup resources if any of the shaders failed to compile
			if (FAILED(hr))
			{
				Logging::Log() << "Error: Failed to failed to compile shader! HRESULT is: " << (D3DERR)hr << '.';
				return false;
			}
			
			if (com_ptr<ID3DBlob> d3d_disassembled; SUCCEEDED(D3DDisassemble(compiled->GetBufferPointer(), compiled->GetBufferSize(), 0, nullptr, &d3d_disassembled)))
			{
				effect.assembly[entry_point.name] = std::string(static_cast<const char *>(d3d_disassembled->GetBufferPointer()));
			}
			else
			{
				Logging::Log() << "Error: Failed to failed to disassemble shader! HRESULT is: " << (D3DERR)hr << '.';
			}

			// Cashe shader
			_compile_cache[entry_index].resize(compiled->GetBufferSize());
			memcpy(&_compile_cache[entry_index][0], compiled->GetBufferPointer(), compiled->GetBufferSize());

			compiled_buffer = compiled->GetBufferPointer();
		}
		else
		{
			compiled_buffer = &_compile_cache[entry_index][0];
		}

		// Create runtime shader objects from the compiled DX byte code
		switch (entry_point.type)
		{
		case reshadefx::shader_type::vs:
			hr = _device->CreateVertexShader(static_cast<const DWORD *>(compiled_buffer), reinterpret_cast<IDirect3DVertexShader9 **>(&entry_points[entry_point.name]));
			break;
		case reshadefx::shader_type::ps:
			hr = _device->CreatePixelShader(static_cast<const DWORD *>(compiled_buffer), reinterpret_cast<IDirect3DPixelShader9 **>(&entry_points[entry_point.name]));
			break;
		}

		if (FAILED(hr))
		{
			Logging::Log() << "Error: Failed to create shader for entry point '" << entry_point.name << "'! HRESULT is " << (D3DERR)hr << '.';
			return false;
		}

		++loop_count;
	}

	if (is_gamma)
	{
		_gamma_set = false;
	}

	d3d9_technique_data technique_init;
	assert(effect.module.num_texture_bindings == 0);
	assert(effect.module.num_storage_bindings == 0);
	technique_init.num_samplers = effect.module.num_sampler_bindings;
	technique_init.constant_register_count = static_cast<DWORD>((effect.uniform_data_storage.size() + 15) / 16);

	for (const reshadefx::sampler_info &info : effect.module.samplers)
	{
		if (info.binding >= ARRAYSIZE(technique_init.sampler_states))
		{
			Logging::Log() << "Error: Cannot bind sampler '" << info.unique_name << "' since it exceeds the maximum number of allowed sampler slots in " << "D3D9" << " (" << info.binding << ", allowed are up to " << ARRAYSIZE(technique_init.sampler_states) << ").";
			return false;
		}

		const texture &texture = look_up_texture_by_name(info.texture_name);

		technique_init.sampler_textures[info.binding] = static_cast<d3d9_tex_data *>(texture.impl)->texture.get();

		// Since textures with auto-generated mipmap levels do not have a mipmap maximum, limit the bias here so this is not as obvious
		assert(texture.levels > 0);
		const float lod_bias = std::min(texture.levels - 1.0f, info.lod_bias);

		technique_init.sampler_states[info.binding][D3DSAMP_ADDRESSU] = static_cast<D3DTEXTUREADDRESS>(info.address_u);
		technique_init.sampler_states[info.binding][D3DSAMP_ADDRESSV] = static_cast<D3DTEXTUREADDRESS>(info.address_v);
		technique_init.sampler_states[info.binding][D3DSAMP_ADDRESSW] = static_cast<D3DTEXTUREADDRESS>(info.address_w);
		technique_init.sampler_states[info.binding][D3DSAMP_BORDERCOLOR] = 0;
		technique_init.sampler_states[info.binding][D3DSAMP_MAGFILTER] = 1 + ((static_cast<unsigned int>(info.filter) & 0x0C) >> 2);
		technique_init.sampler_states[info.binding][D3DSAMP_MINFILTER] = 1 + ((static_cast<unsigned int>(info.filter) & 0x30) >> 4);
		technique_init.sampler_states[info.binding][D3DSAMP_MIPFILTER] = 1 + ((static_cast<unsigned int>(info.filter) & 0x03));
		technique_init.sampler_states[info.binding][D3DSAMP_MIPMAPLODBIAS] = *reinterpret_cast<const DWORD *>(&lod_bias);
		technique_init.sampler_states[info.binding][D3DSAMP_MAXMIPLEVEL] = static_cast<DWORD>(std::max(0.0f, info.min_lod));
		technique_init.sampler_states[info.binding][D3DSAMP_MAXANISOTROPY] = 1;
		technique_init.sampler_states[info.binding][D3DSAMP_SRGBTEXTURE] = info.srgb;
	}

	UINT max_vertices = 3;

	for (technique &technique : _techniques)
	{
		if (technique.impl != nullptr || technique.effect_index != index)
			continue;

		// Copy construct new technique implementation instead of move because effect may contain multiple techniques
		auto impl = new d3d9_technique_data(technique_init);
		technique.impl = impl;

		impl->passes.resize(technique.passes.size());
		for (size_t pass_index = 0; pass_index < technique.passes.size(); ++pass_index)
		{
			d3d9_pass_data &pass_data = impl->passes[pass_index];
			const reshadefx::pass_info &pass_info = technique.passes[pass_index];

			max_vertices = std::max(max_vertices, pass_info.num_vertices);

			entry_points.at(pass_info.ps_entry_point)->QueryInterface(&pass_data.pixel_shader);
			entry_points.at(pass_info.vs_entry_point)->QueryInterface(&pass_data.vertex_shader);

			pass_data.render_targets[0] = _backbuffer_resolved.get();

			for (UINT k = 0; k < ARRAYSIZE(pass_data.sampler_textures); ++k)
				pass_data.sampler_textures[k] = impl->sampler_textures[k];

			for (UINT k = 0; k < 8 && !pass_info.render_target_names[k].empty(); ++k)
			{
				if (k > _num_simultaneous_rendertargets)
				{
					Logging::Log() << "Error: Device only supports " << _num_simultaneous_rendertargets << " simultaneous render targets, but pass " << pass_index << " in technique '" << technique.name << "' uses more, which are ignored.";
					break;
				}

				d3d9_tex_data *const tex_impl = static_cast<d3d9_tex_data *>(
					look_up_texture_by_name(pass_info.render_target_names[k]).impl);

				// Unset textures that are used as render target
				for (DWORD s = 0; s < impl->num_samplers; ++s)
					if (tex_impl->texture == pass_data.sampler_textures[s])
						pass_data.sampler_textures[s] = nullptr;

				pass_data.render_targets[k] = tex_impl->surface.get();
			}

			HRESULT hr = _device->BeginStateBlock();
			if (SUCCEEDED(hr))
			{
				_device->SetVertexShader(pass_data.vertex_shader.get());
				_device->SetPixelShader(pass_data.pixel_shader.get());

				const auto convert_blend_op = [](reshadefx::pass_blend_op value) {
					switch (value)
					{
					default:
					case reshadefx::pass_blend_op::add: return D3DBLENDOP_ADD;
					case reshadefx::pass_blend_op::subtract: return D3DBLENDOP_SUBTRACT;
					case reshadefx::pass_blend_op::rev_subtract: return D3DBLENDOP_REVSUBTRACT;
					case reshadefx::pass_blend_op::min: return D3DBLENDOP_MIN;
					case reshadefx::pass_blend_op::max: return D3DBLENDOP_MAX;
					}
				};
				const auto convert_blend_func = [](reshadefx::pass_blend_func value) {
					switch (value)
					{
					default:
					case reshadefx::pass_blend_func::one: return D3DBLEND_ONE;
					case reshadefx::pass_blend_func::zero: return D3DBLEND_ZERO;
					case reshadefx::pass_blend_func::src_color: return D3DBLEND_SRCCOLOR;
					case reshadefx::pass_blend_func::src_alpha: return D3DBLEND_SRCALPHA;
					case reshadefx::pass_blend_func::inv_src_color: return D3DBLEND_INVSRCCOLOR;
					case reshadefx::pass_blend_func::inv_src_alpha: return D3DBLEND_INVSRCALPHA;
					case reshadefx::pass_blend_func::dst_alpha: return D3DBLEND_DESTALPHA;
					case reshadefx::pass_blend_func::dst_color: return D3DBLEND_DESTCOLOR;
					case reshadefx::pass_blend_func::inv_dst_alpha: return D3DBLEND_INVDESTALPHA;
					case reshadefx::pass_blend_func::inv_dst_color: return D3DBLEND_INVDESTCOLOR;
					}
				};
				const auto convert_stencil_op = [](reshadefx::pass_stencil_op value) {
					switch (value)
					{
					default:
					case reshadefx::pass_stencil_op::keep: return D3DSTENCILOP_KEEP;
					case reshadefx::pass_stencil_op::zero: return D3DSTENCILOP_ZERO;
					case reshadefx::pass_stencil_op::invert: return D3DSTENCILOP_INVERT;
					case reshadefx::pass_stencil_op::replace: return D3DSTENCILOP_REPLACE;
					case reshadefx::pass_stencil_op::incr: return D3DSTENCILOP_INCR;
					case reshadefx::pass_stencil_op::incr_sat: return D3DSTENCILOP_INCRSAT;
					case reshadefx::pass_stencil_op::decr: return D3DSTENCILOP_DECR;
					case reshadefx::pass_stencil_op::decr_sat: return D3DSTENCILOP_DECRSAT;
					}
				};
				const auto convert_stencil_func = [](reshadefx::pass_stencil_func value) {
					switch (value)
					{
					default:
					case reshadefx::pass_stencil_func::always: return D3DCMP_ALWAYS;
					case reshadefx::pass_stencil_func::never: return D3DCMP_NEVER;
					case reshadefx::pass_stencil_func::equal: return D3DCMP_EQUAL;
					case reshadefx::pass_stencil_func::not_equal: return D3DCMP_NOTEQUAL;
					case reshadefx::pass_stencil_func::less: return D3DCMP_LESS;
					case reshadefx::pass_stencil_func::less_equal: return D3DCMP_LESSEQUAL;
					case reshadefx::pass_stencil_func::greater: return D3DCMP_GREATER;
					case reshadefx::pass_stencil_func::greater_equal: return D3DCMP_GREATEREQUAL;
					}
				};

				_device->SetRenderState(D3DRS_ZENABLE, FALSE);
				_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
				// D3DRS_SHADEMODE
				_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
				_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
				_device->SetRenderState(D3DRS_LASTPIXEL, TRUE);
				_device->SetRenderState(D3DRS_SRCBLEND, convert_blend_func(pass_info.src_blend));
				_device->SetRenderState(D3DRS_DESTBLEND, convert_blend_func(pass_info.dest_blend));
				_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
				// D3DRS_ALPHAREF
				// D3DRS_ALPHAFUNC
				_device->SetRenderState(D3DRS_DITHERENABLE, FALSE);
				_device->SetRenderState(D3DRS_ALPHABLENDENABLE, pass_info.blend_enable);
				_device->SetRenderState(D3DRS_FOGENABLE, FALSE);
				_device->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
				// D3DRS_FOGCOLOR
				// D3DRS_FOGTABLEMODE
				// D3DRS_FOGSTART
				// D3DRS_FOGEND
				// D3DRS_FOGDENSITY
				// D3DRS_RANGEFOGENABLE
				_device->SetRenderState(D3DRS_STENCILENABLE, pass_info.stencil_enable);
				_device->SetRenderState(D3DRS_STENCILFAIL, convert_stencil_op(pass_info.stencil_op_fail));
				_device->SetRenderState(D3DRS_STENCILZFAIL, convert_stencil_op(pass_info.stencil_op_depth_fail));
				_device->SetRenderState(D3DRS_STENCILPASS, convert_stencil_op(pass_info.stencil_op_pass));
				_device->SetRenderState(D3DRS_STENCILFUNC, convert_stencil_func(pass_info.stencil_comparison_func));
				_device->SetRenderState(D3DRS_STENCILREF, pass_info.stencil_reference_value);
				_device->SetRenderState(D3DRS_STENCILMASK, pass_info.stencil_read_mask);
				_device->SetRenderState(D3DRS_STENCILWRITEMASK, pass_info.stencil_write_mask);
				// D3DRS_TEXTUREFACTOR
				// D3DRS_WRAP0 - D3DRS_WRAP7
				_device->SetRenderState(D3DRS_CLIPPING, FALSE);
				_device->SetRenderState(D3DRS_LIGHTING, FALSE);
				// D3DRS_AMBIENT
				// D3DRS_FOGVERTEXMODE
				_device->SetRenderState(D3DRS_COLORVERTEX, FALSE);
				// D3DRS_LOCALVIEWER
				_device->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
				_device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
				_device->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
				_device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
				_device->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
				_device->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
				_device->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
				// D3DRS_POINTSIZE
				// D3DRS_POINTSIZE_MIN
				// D3DRS_POINTSPRITEENABLE
				// D3DRS_POINTSCALEENABLE
				// D3DRS_POINTSCALE_A - D3DRS_POINTSCALE_C
				// D3DRS_MULTISAMPLEANTIALIAS
				// D3DRS_MULTISAMPLEMASK
				// D3DRS_PATCHEDGESTYLE
				// D3DRS_DEBUGMONITORTOKEN
				// D3DRS_POINTSIZE_MAX
				// D3DRS_INDEXEDVERTEXBLENDENABLE
				_device->SetRenderState(D3DRS_COLORWRITEENABLE, pass_info.color_write_mask);
				// D3DRS_TWEENFACTOR
				_device->SetRenderState(D3DRS_BLENDOP, convert_blend_op(pass_info.blend_op));
				// D3DRS_POSITIONDEGREE
				// D3DRS_NORMALDEGREE
				_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
				_device->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, 0);
				_device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);
				// D3DRS_MINTESSELLATIONLEVEL
				// D3DRS_MAXTESSELLATIONLEVEL
				// D3DRS_ADAPTIVETESS_X - D3DRS_ADAPTIVETESS_W
				_device->SetRenderState(D3DRS_ENABLEADAPTIVETESSELLATION, FALSE);
				_device->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);
				// D3DRS_CCW_STENCILFAIL
				// D3DRS_CCW_STENCILZFAIL
				// D3DRS_CCW_STENCILPASS
				// D3DRS_CCW_STENCILFUNC
				_device->SetRenderState(D3DRS_COLORWRITEENABLE1, pass_info.color_write_mask); // See https://docs.microsoft.com/en-us/windows/win32/direct3d9/multiple-render-targets
				_device->SetRenderState(D3DRS_COLORWRITEENABLE2, pass_info.color_write_mask);
				_device->SetRenderState(D3DRS_COLORWRITEENABLE3, pass_info.color_write_mask);
				_device->SetRenderState(D3DRS_BLENDFACTOR, 0xFFFFFFFF);
				_device->SetRenderState(D3DRS_SRGBWRITEENABLE, pass_info.srgb_write_enable);
				_device->SetRenderState(D3DRS_DEPTHBIAS, 0);
				// D3DRS_WRAP8 - D3DRS_WRAP15
				_device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
				_device->SetRenderState(D3DRS_SRCBLENDALPHA, convert_blend_func(pass_info.src_blend_alpha));
				_device->SetRenderState(D3DRS_DESTBLENDALPHA, convert_blend_func(pass_info.dest_blend_alpha));
				_device->SetRenderState(D3DRS_BLENDOPALPHA, convert_blend_op(pass_info.blend_op_alpha));

				hr = _device->EndStateBlock(&pass_data.stateblock);
			}

			if (FAILED(hr))
			{
				Logging::Log() << "Error: Failed to create state block for pass " << pass_index << " in technique '" << technique.name << "'! HRESULT is " << (D3DERR)hr << '.';
				return false;
			}
		}
	}

	// Update vertex buffer which holds vertex indices
	if (max_vertices > _max_vertices)
	{
		_effect_vertex_buffer.reset();

		if (FAILED(_device->CreateVertexBuffer(max_vertices * sizeof(float), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_effect_vertex_buffer, nullptr)))
			return false;

		if (float *data;
			SUCCEEDED(_effect_vertex_buffer->Lock(0, max_vertices * sizeof(float), reinterpret_cast<void **>(&data), 0)))
		{
			for (UINT i = 0; i < max_vertices; i++)
				data[i] = static_cast<float>(i);
			_effect_vertex_buffer->Unlock();
		}

		_max_vertices = max_vertices;
	}

	return true;
}
void reshade::d3d9::runtime_d3d9::unload_effect(size_t index)
{
	for (technique &tech : _techniques)
	{
		if (tech.effect_index != index)
			continue;

		delete static_cast<d3d9_technique_data *>(tech.impl);
		tech.impl = nullptr;
	}

	runtime::unload_effect(index);
}
void reshade::d3d9::runtime_d3d9::unload_effects()
{
	for (technique &tech : _techniques)
	{
		delete static_cast<d3d9_technique_data *>(tech.impl);
		tech.impl = nullptr;
	}

	runtime::unload_effects();
}

bool reshade::d3d9::runtime_d3d9::init_texture(texture &texture)
{
	auto impl = new d3d9_tex_data();
	texture.impl = impl;

	switch (texture.impl_reference)
	{
	case texture_reference::back_buffer:
		impl->texture = _backbuffer_texture;
		impl->surface = _backbuffer_texture_surface;
		return true;
	case texture_reference::depth_buffer:
		impl->texture = _depth_texture;
		impl->surface = _depth_surface;
		return true;
	}

	UINT levels = texture.levels;
	DWORD usage = 0;
	D3DFORMAT format = D3DFMT_UNKNOWN;
	D3DDEVICE_CREATION_PARAMETERS cp;
	_device->GetCreationParameters(&cp);

	switch (texture.format)
	{
	case reshadefx::texture_format::r8:
		format = D3DFMT_X8R8G8B8; // Use 4-component format so that green/blue components are returned as zero and alpha as one (to match behavior from other APIs)
		break;
	case reshadefx::texture_format::r16f:
		format = D3DFMT_R16F;
		break;
	case reshadefx::texture_format::r32f:
		format = D3DFMT_R32F;
		break;
	case reshadefx::texture_format::rg8:
		format = D3DFMT_X8R8G8B8;
		break;
	case reshadefx::texture_format::rg16:
		format = D3DFMT_G16R16;
		break;
	case reshadefx::texture_format::rg16f:
		format = D3DFMT_G16R16F;
		break;
	case reshadefx::texture_format::rg32f:
		format = D3DFMT_G32R32F;
		break;
	case reshadefx::texture_format::rgba8:
		format = D3DFMT_A8R8G8B8;
		break;
	case reshadefx::texture_format::rgba16:
		format = D3DFMT_A16B16G16R16;
		break;
	case reshadefx::texture_format::rgba16f:
		format = D3DFMT_A16B16G16R16F;
		break;
	case reshadefx::texture_format::rgba32f:
		format = D3DFMT_A32B32G32R32F;
		break;
	case reshadefx::texture_format::rgb10a2:
		format = D3DFMT_A2B10G10R10;
		break;
	}

	if (levels > 1)
	{
		// Enable auto-generated mipmaps if the format supports it
		if (_d3d->CheckDeviceFormat(cp.AdapterOrdinal, cp.DeviceType, D3DFMT_X8R8G8B8, D3DUSAGE_AUTOGENMIPMAP, D3DRTYPE_TEXTURE, format) == D3D_OK)
		{
			usage |= D3DUSAGE_AUTOGENMIPMAP;
			levels = 0;
		}
		else
		{
			Logging::Log() << "Error: Auto-generated mipmap levels are not supported for format " << static_cast<unsigned int>(texture.format) << " of texture '" << texture.unique_name << "'.";
		}
	}

	if (texture.render_target)
	{
		// Make texture a render target if format allows it
		if (_d3d->CheckDeviceFormat(cp.AdapterOrdinal, cp.DeviceType, D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, format) == D3D_OK)
		{
			usage |= D3DUSAGE_RENDERTARGET;
		}
		else
		{
			Logging::Log() << "Error: Render target usage is not supported for format " << static_cast<unsigned int>(texture.format) << " of texture '" << texture.unique_name << "'.";
		}
	}

	HRESULT hr = _device->CreateTexture(texture.width, texture.height, levels, usage, format, D3DPOOL_DEFAULT, &impl->texture, nullptr);
	if (FAILED(hr))
	{
		Logging::Log() << "Error: Failed to create texture '" << texture.unique_name << "'! HRESULT is " << (D3DERR)hr << '.';
		Logging::Log() << "> Details: Width = " << texture.width << ", Height = " << texture.height << ", Levels = " << levels << ", Usage = " << usage << ", Format = " << format;
		return false;
	}

	hr = impl->texture->GetSurfaceLevel(0, &impl->surface);
	assert(SUCCEEDED(hr));

	// Clear texture to zero since by default its contents are undefined
	if (usage & D3DUSAGE_RENDERTARGET)
		_device->ColorFill(impl->surface.get(), nullptr, D3DCOLOR_ARGB(0, 0, 0, 0));

	return true;
}
void reshade::d3d9::runtime_d3d9::upload_texture(const texture &texture, const uint8_t *pixels)
{
	auto impl = static_cast<d3d9_tex_data *>(texture.impl);
	assert(impl != nullptr && texture.impl_reference == texture_reference::none && pixels != nullptr);

	D3DSURFACE_DESC desc; impl->texture->GetLevelDesc(0, &desc); // Get D3D texture format
	com_ptr<IDirect3DTexture9> intermediate;
	if (HRESULT hr = _device->CreateTexture(texture.width, texture.height, 1, 0, desc.Format, D3DPOOL_SYSTEMMEM, &intermediate, nullptr); FAILED(hr))
	{
		Logging::Log() << "Error: Failed to create system memory texture for updating texture '" << texture.unique_name << "'! HRESULT is " << (D3DERR)hr << '.';
		Logging::Log() << "> Details: Width = " << texture.width << ", Height = " << texture.height << ", Levels = " << "1" << ", Usage = " << "0" << ", Format = " << desc.Format;
		return;
	}

	D3DLOCKED_RECT mapped;
	if (FAILED(intermediate->LockRect(0, &mapped, nullptr, 0)))
		return;
	auto mapped_data = static_cast<uint8_t *>(mapped.pBits);

	switch (texture.format)
	{
	case reshadefx::texture_format::r8: // These are actually D3DFMT_X8R8G8B8, see 'init_texture'
		for (uint32_t y = 0, pitch = texture.width * 4; y < texture.height; ++y, mapped_data += mapped.Pitch, pixels += pitch)
			for (uint32_t x = 0; x < pitch; x += 4)
				mapped_data[x + 0] = 0, // Set green and blue channel to zero
				mapped_data[x + 1] = 0,
				mapped_data[x + 2] = pixels[x + 0],
				mapped_data[x + 3] = 0xFF;
		break;
	case reshadefx::texture_format::rg8:
		for (uint32_t y = 0, pitch = texture.width * 4; y < texture.height; ++y, mapped_data += mapped.Pitch, pixels += pitch)
			for (uint32_t x = 0; x < pitch; x += 4)
				mapped_data[x + 0] = 0, // Set blue channel to zero
				mapped_data[x + 1] = pixels[x + 1],
				mapped_data[x + 2] = pixels[x + 0],
				mapped_data[x + 3] = 0xFF;
		break;
	case reshadefx::texture_format::rgba8:
		for (uint32_t y = 0, pitch = texture.width * 4; y < texture.height; ++y, mapped_data += mapped.Pitch, pixels += pitch)
			for (uint32_t x = 0; x < pitch; x += 4)
				mapped_data[x + 0] = pixels[x + 2], // Flip RGBA input to BGRA
				mapped_data[x + 1] = pixels[x + 1],
				mapped_data[x + 2] = pixels[x + 0],
				mapped_data[x + 3] = pixels[x + 3];
		break;
	default:
		Logging::Log() << "Error: Texture upload is not supported for format " << static_cast<unsigned int>(texture.format) << " of texture '" << texture.unique_name << "'!";
		break;
	}

	intermediate->UnlockRect(0);

	if (HRESULT hr = _device->UpdateTexture(intermediate.get(), impl->texture.get()); FAILED(hr))
	{
		Logging::Log() << "Error: Failed to update texture '" << texture.unique_name << "' from system memory texture! HRESULT is " << (D3DERR)hr << '.';
		return;
	}
}
void reshade::d3d9::runtime_d3d9::destroy_texture(texture &texture)
{
	delete static_cast<d3d9_tex_data *>(texture.impl);
	texture.impl = nullptr;
}

void reshade::d3d9::runtime_d3d9::render_technique(technique &technique)
{
	const auto impl = static_cast<d3d9_technique_data *>(technique.impl);

	// Setup vertex input (used to have a vertex ID as vertex shader input)
	_device->SetStreamSource(0, _effect_vertex_buffer.get(), 0, sizeof(float));
	_device->SetVertexDeclaration(_effect_vertex_layout.get());

	// Setup shader constants
	if (impl->constant_register_count != 0)
	{
		const auto uniform_storage_data = reinterpret_cast<const float *>(_effects[technique.effect_index].uniform_data_storage.data());
		_device->SetPixelShaderConstantF(0, uniform_storage_data, impl->constant_register_count);
		_device->SetVertexShaderConstantF(0, uniform_storage_data, impl->constant_register_count);
	}

	bool is_effect_stencil_cleared = false;
	bool needs_implicit_backbuffer_copy = true; // First pass always needs the back buffer updated

	for (size_t pass_index = 0; pass_index < technique.passes.size(); ++pass_index)
	{
		if (needs_implicit_backbuffer_copy)
		{
			// Save back buffer of previous pass
			_device->StretchRect(_backbuffer_resolved.get(), nullptr, _backbuffer_texture_surface.get(), nullptr, D3DTEXF_NONE);
		}

		const d3d9_pass_data &pass_data = impl->passes[pass_index];
		const reshadefx::pass_info &pass_info = technique.passes[pass_index];

		// Setup state
		pass_data.stateblock->Apply();

		// Setup shader resources
		for (DWORD s = 0; s < impl->num_samplers; s++)
		{
			_device->SetTexture(s, pass_data.sampler_textures[s]);

			// Need to bind textures to vertex shader samplers too
			// See https://docs.microsoft.com/windows/win32/direct3d9/vertex-textures-in-vs-3-0
			if (s < 4)
				_device->SetTexture(D3DVERTEXTEXTURESAMPLER0 + s, pass_data.sampler_textures[s]);

			for (DWORD state = D3DSAMP_ADDRESSU; state <= D3DSAMP_SRGBTEXTURE; state++)
			{
				_device->SetSamplerState(s, static_cast<D3DSAMPLERSTATETYPE>(state), impl->sampler_states[s][state]);

				if (s < 4) // vs_3_0 supports up to four samplers in vertex shaders
					_device->SetSamplerState(D3DVERTEXTEXTURESAMPLER0 + s, static_cast<D3DSAMPLERSTATETYPE>(state), impl->sampler_states[s][state]);
			}
		}

		// Setup render targets (and viewport, which is implicitly updated by 'SetRenderTarget')
		for (DWORD target = 0; target < _num_simultaneous_rendertargets; target++)
			_device->SetRenderTarget(target, pass_data.render_targets[target]);

		D3DVIEWPORT9 viewport;
		_device->GetViewport(&viewport);
		_device->SetDepthStencilSurface(viewport.Width == _width && viewport.Height == _height && pass_info.stencil_enable ? _effect_stencil.get() : nullptr);

		if (pass_info.stencil_enable && viewport.Width == _width && viewport.Height == _height && !is_effect_stencil_cleared)
		{
			is_effect_stencil_cleared = true;

			_device->Clear(0, nullptr, (pass_info.clear_render_targets ? D3DCLEAR_TARGET : 0) | D3DCLEAR_STENCIL, 0, 1.0f, 0);
		}
		else if (pass_info.clear_render_targets)
		{
			_device->Clear(0, nullptr, D3DCLEAR_TARGET, 0, 0.0f, 0);
		}

		// Set __TEXEL_SIZE__ constant (see effect_codegen_hlsl.cpp)
		const float texel_size[4] = {
			-1.0f / viewport.Width,
			 1.0f / viewport.Height
		};
		_device->SetVertexShaderConstantF(255, texel_size, 1);

		// Draw primitives
		UINT primitive_count = pass_info.num_vertices;
		D3DPRIMITIVETYPE topology;
		switch (pass_info.topology)
		{
		case reshadefx::primitive_topology::point_list:
			topology = D3DPT_POINTLIST;
			break;
		case reshadefx::primitive_topology::line_list:
			topology = D3DPT_LINELIST;
			primitive_count /= 2;
			break;
		case reshadefx::primitive_topology::line_strip:
			topology = D3DPT_LINESTRIP;
			primitive_count -= 1;
			break;
		default:
		case reshadefx::primitive_topology::triangle_list:
			topology = D3DPT_TRIANGLELIST;
			primitive_count /= 3;
			break;
		case reshadefx::primitive_topology::triangle_strip:
			topology = D3DPT_TRIANGLESTRIP;
			primitive_count -= 2;
			break;
		}
		_device->DrawPrimitive(topology, 0, primitive_count);

		_vertices += pass_info.num_vertices;
		_drawcalls += 1;

		needs_implicit_backbuffer_copy = false;

		// Generate mipmaps for modified resources
		for (IDirect3DSurface9 *target : pass_data.render_targets)
		{
			if (target == nullptr)
				break;

			if (target == _backbuffer_resolved)
			{
				needs_implicit_backbuffer_copy = true;
				break;
			}

			if (com_ptr<IDirect3DBaseTexture9> texture;
				SUCCEEDED(target->GetContainer(IID_PPV_ARGS(&texture))) && texture->GetLevelCount() > 1)
			{
				texture->SetAutoGenFilterType(D3DTEXF_LINEAR);
				texture->GenerateMipSubLevels();
			}
		}
	}
}

void reshade::d3d9::runtime_d3d9::update_depth_texture_bindings(com_ptr<IDirect3DSurface9> depth_surface)
{
	if (depth_surface == _depth_surface)
		return;

	_depth_texture.reset();
	_depth_surface = std::move(depth_surface);
	_has_depth_texture = false;

	if (_depth_surface != nullptr)
	{
		if (HRESULT hr = _depth_surface->GetContainer(IID_PPV_ARGS(&_depth_texture)); FAILED(hr))
		{
			Logging::Log() << "Error: Failed to retrieve texture from depth surface! HRESULT is " << (D3DERR)hr << '.';
		}
		else
		{
			_has_depth_texture = true;
		}
	}

	// Update all references to the new texture
	for (const texture &tex : _textures)
	{
		if (tex.impl == nullptr ||
			tex.impl_reference != texture_reference::depth_buffer)
			continue;
		const auto tex_impl = static_cast<d3d9_tex_data *>(tex.impl);

		// Update references in technique list
		for (const technique &tech : _techniques)
		{
			const auto tech_impl = static_cast<d3d9_technique_data *>(tech.impl);
			if (tech_impl == nullptr)
				continue;

			for (d3d9_pass_data &pass_data : tech_impl->passes)
				// Replace all occurances of the old texture with the new one
				for (IDirect3DTexture9 *&sampler_tex : pass_data.sampler_textures)
					if (tex_impl->texture == sampler_tex)
						sampler_tex = _depth_texture.get();
		}

		tex_impl->texture = _depth_texture;
		tex_impl->surface = _depth_surface;
	}
}

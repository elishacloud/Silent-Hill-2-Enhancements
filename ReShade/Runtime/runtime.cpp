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
#include "runtime.hpp"
#include "runtime_config.hpp"
#include "runtime_objects.hpp"
#include "Resource.h"
#include "Logging\Logging.h"
#include "Common\Settings.h"
#include "External\reshade\source\effect_parser.hpp"
#include "External\reshade\source\effect_codegen.hpp"
#include "External\reshade\source\effect_preprocessor.hpp"
#include <thread>
#include <cassert>
#include <algorithm>
#include "stb_image.h"
#include "stb_image_dds.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"

 bool read_resource(DWORD id, std::string &data)
{
	const HRSRC info = FindResource(m_hModule, MAKEINTRESOURCE(id), RT_RCDATA);
	const HGLOBAL handle = LoadResource(m_hModule, info);

	DWORD file_size = SizeofResource(m_hModule, info);
	if (!file_size || !handle)
	{
		return false;
	}

	std::string_view file_data((char*)LockResource(handle), file_size);

	// Remove BOM (0xefbbbf means 0xfeff)
	if (file_data.size() >= 3 &&
		static_cast<unsigned char>(file_data[0]) == 0xef &&
		static_cast<unsigned char>(file_data[1]) == 0xbb &&
		static_cast<unsigned char>(file_data[2]) == 0xbf)
		file_data = std::string_view(file_data.data() + 3, file_data.size() - 3);

	data = file_data;
	return true;
}

reshade::runtime::runtime() :
	_start_time(std::chrono::high_resolution_clock::now()),
	_last_present_time(std::chrono::high_resolution_clock::now()),
	_last_frame_duration(std::chrono::milliseconds(1)),
	_reload_key_data(),
	_performance_mode_key_data(),
	_effects_key_data(),
	_prev_preset_key_data(),
	_next_preset_key_data()
{
	load_config();
}
reshade::runtime::~runtime()
{
	assert(_worker_threads.empty());
	assert(!_is_initialized && _techniques.empty());
}

bool reshade::runtime::on_init(void*)
{
	assert(!_is_initialized);

	// Reset frame count to zero so effects are loaded in 'update_and_render_effects'
	_framecount = 0;

	_is_initialized = true;
	_last_reload_time = std::chrono::high_resolution_clock::now();

	_preset_save_success = true;

	Logging::Log() << "Recreated runtime environment on runtime " << this << '.';

	return true;
}
void reshade::runtime::on_reset(bool clear_width_height)
{
	if (_is_initialized)
		// Update initialization state immediately, so that any effect loading still in progress can abort early
		_is_initialized = false;
	else
		return; // Nothing to do if the runtime was already destroyed or not successfully initialized in the first place

	unload_effects();

	if (clear_width_height)
	{
		_width = _height = 0;
	}

	Logging::Log() << "Destroyed runtime environment on runtime " << this << '.';
}
void reshade::runtime::on_present()
{
	// Get current time and date
	const std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	tm tm; localtime_s(&tm, &t);
	_date[0] = tm.tm_year + 1900;
	_date[1] = tm.tm_mon + 1;
	_date[2] = tm.tm_mday;
	_date[3] = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;

	_framecount++;
	const auto current_time = std::chrono::high_resolution_clock::now();
	_last_frame_duration = current_time - _last_present_time;
	_last_present_time = current_time;

	// Reset frame statistics
	_drawcalls = _vertices = 0;
}

bool reshade::runtime::load_effect(const std::string &name, DWORD id, size_t effect_index)
{
	effect &effect = _effects[effect_index]; // Safe to access this multi-threaded, since this is the only call working on this effect
	const std::string effect_name = name;
	effect = {};
	effect.compiled = true;

	if (_effect_load_skipping && !_load_option_disable_skipping && !_worker_threads.empty()) // Only skip during 'load_effects'
	{
		const ini_file &preset = ini_file::load_cache(); // ini_file::load_cache is not thread-safe, so load from file here

		if (std::vector<std::string> techniques;
			preset.get({}, "Techniques", techniques))
		{
			effect.skipped = std::find_if(techniques.cbegin(), techniques.cend(), [&effect_name](const std::string &technique) {
				const size_t at_pos = technique.find('@') + 1;
				return at_pos == 0 || technique.find(effect_name, at_pos) == at_pos; }) == techniques.cend();

			if (effect.skipped)
			{
				_reload_remaining_effects--;
				return false;
			}
		}
	}

	{ // Load, pre-process and compile the source file
		reshadefx::preprocessor pp;

		pp.add_macro_definition("__RESHADE__", std::to_string(RESHADE_MAJOR * 10000 + RESHADE_MINOR * 100 + RESHADE_REVISION));
		pp.add_macro_definition("__RESHADE_PERFORMANCE_MODE__", _performance_mode ? "1" : "0");
		pp.add_macro_definition("__VENDOR__", std::to_string(_vendor_id));
		pp.add_macro_definition("__DEVICE__", std::to_string(_device_id));
		pp.add_macro_definition("__RENDERER__", std::to_string(_renderer_id));
		pp.add_macro_definition("BUFFER_WIDTH", std::to_string(_width));
		pp.add_macro_definition("BUFFER_HEIGHT", std::to_string(_height));
		pp.add_macro_definition("BUFFER_RCP_WIDTH", "(1.0 / BUFFER_WIDTH)");
		pp.add_macro_definition("BUFFER_RCP_HEIGHT", "(1.0 / BUFFER_HEIGHT)");
		pp.add_macro_definition("BUFFER_COLOR_BIT_DEPTH", std::to_string(_color_bit_depth));

		std::vector<std::string> preprocessor_definitions = _global_preprocessor_definitions;
		preprocessor_definitions.insert(preprocessor_definitions.end(), _preset_preprocessor_definitions.begin(), _preset_preprocessor_definitions.end());

		for (const auto &definition : preprocessor_definitions)
		{
			if (definition.empty())
				continue; // Skip invalid definitions

			const size_t equals_index = definition.find('=');
			if (equals_index != std::string::npos)
				pp.add_macro_definition(
					definition.substr(0, equals_index),
					definition.substr(equals_index + 1));
			else
				pp.add_macro_definition(definition);
		}

		// Add some conversion macros for compatibility with older versions of ReShade
		pp.append_string(
			"#define tex2Doffset(s, coords, offset) tex2D(s, coords, offset)\n"
			"#define tex2Dlodoffset(s, coords, offset) tex2Dlod(s, coords, offset)\n"
			"#define tex2Dgather(s, t, c) tex2Dgather##c(s, t)\n"
			"#define tex2Dgatheroffset(s, t, o, c) tex2Dgather##c(s, t, o)\n"
			"#define tex2Dgather0 tex2DgatherR\n"
			"#define tex2Dgather1 tex2DgatherG\n"
			"#define tex2Dgather2 tex2DgatherB\n"
			"#define tex2Dgather3 tex2DgatherA\n");

		effect.source_file.assign(effect_name);

		std::string data;
		if (read_resource(id, data))
		{
			pp.push(std::move(data), name);
			pp.parse();
		}
		else
		{
			effect.compiled = false;
		}

		unsigned shader_model;
		if (_renderer_id == 0x9000)
			shader_model = 30; // D3D9
		else if (_renderer_id < 0xa100)
			shader_model = 40; // D3D10 (including feature level 9)
		else if (_renderer_id < 0xb000)
			shader_model = 41; // D3D10.1
		else if (_renderer_id < 0xc000)
			shader_model = 50; // D3D11
		else
			shader_model = 60; // D3D12

		std::unique_ptr<reshadefx::codegen> codegen;
		if ((_renderer_id & 0xF0000) == 0)
			codegen.reset(reshadefx::create_codegen_hlsl(shader_model, !_no_debug_info, _performance_mode));
		else if (_renderer_id < 0x20000)
			codegen.reset(reshadefx::create_codegen_glsl(!_no_debug_info, _performance_mode, false));
		else // Vulkan uses SPIR-V input
			codegen.reset(reshadefx::create_codegen_spirv(true, !_no_debug_info, _performance_mode, false, true));

		reshadefx::parser parser;

		// Compile the pre-processed source code (try the compile even if the preprocessor step failed to get additional error information)
		if (!parser.parse(std::move(pp.output()), codegen.get()) || !effect.compiled)
		{
			Logging::Log() << "Failed to compile " << name << ":\n" << pp.errors() << parser.errors();
			effect.compiled = false;
		}

		// Append preprocessor and parser errors to the error list
		effect.errors = std::move(pp.errors()) + std::move(parser.errors());

		// Keep track of used preprocessor definitions (so they can be displayed in the GUI)
		for (const auto &definition : pp.used_macro_definitions())
		{
			if (definition.first.size() <= 10 || definition.first[0] == '_' || !definition.first.compare(0, 8, "RESHADE_") || !definition.first.compare(0, 7, "BUFFER_"))
				continue;

			effect.definitions.push_back({ definition.first, trim(definition.second) });
		}

		// Keep track of included files
		effect.included_files = pp.included_files();
		std::sort(effect.included_files.begin(), effect.included_files.end()); // Sort file names alphabetically

		// Write result to effect module
		codegen->write_result(effect.module);
	}

	// Fill all specialization constants with values from the current preset
	if (_performance_mode && effect.compiled)
	{
		const ini_file &preset = ini_file::load_cache(); // ini_file::load_cache is not thread-safe, so load from file here

		for (reshadefx::uniform_info &constant : effect.module.spec_constants)
		{
			effect.preamble += "#define SPEC_CONSTANT_" + constant.name + ' ';

			switch (constant.type.base)
			{
			case reshadefx::type::t_int:
				preset.get(effect_name, constant.name, constant.initializer_value.as_int);
				break;
			case reshadefx::type::t_bool:
			case reshadefx::type::t_uint:
				preset.get(effect_name, constant.name, constant.initializer_value.as_uint);
				break;
			case reshadefx::type::t_float:
				preset.get(effect_name, constant.name, constant.initializer_value.as_float);
				break;
			}

			// Check if this is a split specialization constant and move data accordingly
			if (constant.type.is_scalar() && constant.offset != 0)
				constant.initializer_value.as_uint[0] = constant.initializer_value.as_uint[constant.offset];

			for (unsigned int i = 0; i < constant.type.components(); ++i)
			{
				switch (constant.type.base)
				{
				case reshadefx::type::t_bool:
					effect.preamble += constant.initializer_value.as_uint[i] ? "true" : "false";
					break;
				case reshadefx::type::t_int:
					effect.preamble += std::to_string(constant.initializer_value.as_int[i]);
					break;
				case reshadefx::type::t_uint:
					effect.preamble += std::to_string(constant.initializer_value.as_uint[i]);
					break;
				case reshadefx::type::t_float:
					effect.preamble += std::to_string(constant.initializer_value.as_float[i]);
					break;
				}

				if (i + 1 < constant.type.components())
					effect.preamble += ", ";
			}

			effect.preamble += '\n';
		}
	}

	// Create space for all variables (aligned to 16 bytes)
	effect.uniform_data_storage.resize((effect.module.total_uniform_size + 15) & ~15);

	for (uniform var : effect.module.uniforms)
	{
		var.effect_index = effect_index;

		// Copy initial data into uniform storage area
		reset_uniform_value(var);

		const std::string_view special = var.annotation_as_string("source");
		if (special.empty()) /* Ignore if annotation is missing */;
		else if (special == "frametime")
			var.special = special_uniform::frame_time;
		else if (special == "framecount")
			var.special = special_uniform::frame_count;
		else if (special == "random")
			var.special = special_uniform::random;
		else if (special == "pingpong")
			var.special = special_uniform::ping_pong;
		else if (special == "date")
			var.special = special_uniform::date;
		else if (special == "timer")
			var.special = special_uniform::timer;
		else if (special == "key")
			var.special = special_uniform::key;
		else if (special == "mousepoint")
			var.special = special_uniform::mouse_point;
		else if (special == "mousedelta")
			var.special = special_uniform::mouse_delta;
		else if (special == "mousebutton")
			var.special = special_uniform::mouse_button;
		else if (special == "mousewheel")
			var.special = special_uniform::mouse_wheel;
		else if (special == "freepie")
			var.special = special_uniform::freepie;
		else if (special == "overlay_open")
			var.special = special_uniform::overlay_open;
		else if (special == "bufready_depth")
			var.special = special_uniform::bufready_depth;

		effect.uniforms.push_back(std::move(var));
	}

	std::vector<texture> new_textures;
	new_textures.reserve(effect.module.textures.size());
	std::vector<technique> new_techniques;
	new_techniques.reserve(effect.module.techniques.size());

	for (texture texture : effect.module.textures)
	{
		texture.effect_index = effect_index;

		{	const std::lock_guard<std::mutex> lock(_reload_mutex); // Protect access to global texture list

			// Try to share textures with the same name across effects
			if (const auto existing_texture = std::find_if(_textures.begin(), _textures.end(),
				[&texture](const auto &item) { return item.unique_name == texture.unique_name; });
				existing_texture != _textures.end())
			{
				// Cannot share texture if this is a normal one, but the existing one is a reference and vice versa
				if (texture.semantic.empty() != (existing_texture->impl_reference == texture_reference::none))
				{
					effect.errors += "error: " + texture.unique_name + ": another effect (";
					effect.errors += _effects[existing_texture->effect_index].source_file.filename().u8string();
					effect.errors += ") already created a texture with the same name but different usage; rename the variable to fix this error\n";
					effect.compiled = false;
					break;
				}
				else if (texture.semantic.empty() && !existing_texture->matches_description(texture))
				{
					effect.errors += "warning: " + texture.unique_name + ": another effect (";
					effect.errors += _effects[existing_texture->effect_index].source_file.filename().u8string();
					effect.errors += ") already created a texture with the same name but different dimensions; textures are shared across all effects, so either rename the variable or adjust the dimensions so they match\n";
				}

				if (_color_bit_depth != 8)
				{
					for (const auto &sampler_info : effect.module.samplers)
					{
						if (sampler_info.srgb && sampler_info.texture_name == texture.unique_name)
						{
							effect.errors += "error: " + sampler_info.unique_name + ": texture does not support sRGB sampling (back buffer format is not RGBA8)";
							effect.compiled = false;
						}
					}
				}

				if (std::find(existing_texture->shared.begin(), existing_texture->shared.end(), effect_index) == existing_texture->shared.end())
					existing_texture->shared.push_back(effect_index);

				// Always make shared textures render targets, since they may be used as such in a different effect
				existing_texture->render_target = true;
				existing_texture->storage_access = true;
				continue;
			}
		}

		if (texture.annotation_as_int("pooled"))
		{
			const std::lock_guard<std::mutex> lock(_reload_mutex);

			// Try to find another pooled texture to share with
			if (const auto existing_texture = std::find_if(_textures.begin(), _textures.end(),
				[&texture](const auto &item) { return item.annotation_as_int("pooled") && item.matches_description(texture); });
				existing_texture != _textures.end())
			{
				// Overwrite referenced texture in samplers with the pooled one
				for (auto &sampler_info : effect.module.samplers)
					if (sampler_info.texture_name == texture.unique_name)
						sampler_info.texture_name  = existing_texture->unique_name;
				// Overwrite referenced texture in render targets with the pooled one
				for (auto &technique_info : effect.module.techniques)
					for (auto &pass_info : technique_info.passes)
						std::replace(std::begin(pass_info.render_target_names), std::end(pass_info.render_target_names),
							texture.unique_name, existing_texture->unique_name);

				if (std::find(existing_texture->shared.begin(), existing_texture->shared.end(), effect_index) == existing_texture->shared.end())
					existing_texture->shared.push_back(effect_index);

				existing_texture->render_target = true;
				existing_texture->storage_access = true;
				continue;
			}
		}

		if (texture.semantic == "COLOR")
			texture.impl_reference = texture_reference::back_buffer;
		else if (texture.semantic == "DEPTH")
			texture.impl_reference = texture_reference::depth_buffer;
		else if (!texture.semantic.empty())
			effect.errors += "warning: " + texture.unique_name + ": unknown semantic '" + texture.semantic + "'\n";

		// This is the first effect using this texture
		texture.shared.push_back(effect_index);

		new_textures.push_back(std::move(texture));
	}

	for (technique technique : effect.module.techniques)
	{
		technique.effect_index = effect_index;

		technique.hidden = technique.annotation_as_int("hidden") != 0;

		if (technique.annotation_as_int("enabled"))
			enable_technique(technique);

		new_techniques.push_back(std::move(technique));
	}

	if (effect.compiled)
		if (effect.errors.empty())
			Logging::Log() << "Successfully loaded " << name;
		else
			Logging::Log() << "Successfully loaded " << name << " with warnings:\n" << effect.errors;

	{	const std::lock_guard<std::mutex> lock(_reload_mutex);
		std::move(new_textures.begin(), new_textures.end(), std::back_inserter(_textures));
		std::move(new_techniques.begin(), new_techniques.end(), std::back_inserter(_techniques));

		_last_shader_reload_successfull &= effect.compiled;
		_reload_remaining_effects--;
	}

	return effect.compiled;
}
void reshade::runtime::load_effects()
{
	// Clear out any previous effects
	unload_effects();

	_last_shader_reload_successfull = true;

	// Reload preprocessor definitions from current preset before compiling
	{
		_preset_preprocessor_definitions.clear();

		const ini_file &preset = ini_file::load_cache();
		preset.get({}, "PreprocessorDefinitions", _preset_preprocessor_definitions);
	}

	// Build a list of effects by walking through the effect list
	std::vector<FILELIST> effects_no;
	for (auto &item : shaderList)
	{
		if (*item.enabled)
		{
			effects_no.push_back(item);
		}
	}

	_reload_total_effects = effects_no.size();
	_reload_remaining_effects = _reload_total_effects;

	if (_reload_total_effects == 0)
	{
		return; // No effect files found, so nothing more to do
	}

	// Allocate space for effects which are placed in this array during the 'load_effect' call
	_effects.resize(_reload_total_effects);

	// Now that we have a list of files, load them in parallel
	// Split workload into batches instead of launching a thread for every file to avoid launch overhead and stutters due to too many threads being in flight
	//const size_t num_splits = std::min<size_t>(effects_no.size(), std::max<size_t>(std::thread::hardware_concurrency(), 2u) - 1);

	// Keep track of the spawned threads, so the runtime cannot be destroyed while they are still running
	//for (size_t n = 0; n < num_splits; ++n)
		//_worker_threads.emplace_back([this, effects_no, num_splits, n](){
			// Abort loading when initialization state changes (indicating that 'on_reset' was called in the meantime)
			for (size_t i = 0; i < effects_no.size() && _is_initialized; ++i)
			{
				//if (i * num_splits / effects_no.size() == n)
				{
					load_effect(effects_no[i].name, effects_no[i].value, i);
				}
			}
		//});
}
void reshade::runtime::load_textures()
{
	_last_texture_reload_successfull = true;

	Logging::Log() << "Loading image files for textures ...";

	for (texture &texture : _textures)
	{
		if (texture.impl == nullptr || texture.impl_reference != texture_reference::none)
		{
			continue; // Ignore textures that are not created yet and those that are handled in the runtime implementation
		}

		std::string	texture_name(texture.annotation_as_string("source"));
		// Ignore textures that have no image file attached to them (e.g. plain render targets)
		if (texture_name.empty())
		{
			continue;
		}

		// Search for image
		DWORD id = 0;
		for (auto &item : textureList)
		{
			if (item.name.compare(texture_name) == 0)
			{
				id = item.value;
				break;
			}
		}

		if (!id)
		{
			Logging::Log() << "Source " << texture_name << " for texture '" << texture.unique_name << "' could not be found.";
			_last_texture_reload_successfull = false;
			continue;
		}

		// Read texture data into memory
		std::string mem;
		read_resource(id, mem);

		unsigned char * filedata = nullptr;
		int width = 0, height = 0, channels = 0;
		if (stbi_dds_test_memory((stbi_uc*)&mem[0], static_cast<int>(mem.size())))
		{
			filedata = stbi_dds_load_from_memory((stbi_uc*)&mem[0], static_cast<int>(mem.size()), &width, &height, &channels, STBI_rgb_alpha);
		}
		else
		{
			filedata = stbi_load_from_memory((stbi_uc*)&mem[0], static_cast<int>(mem.size()), &width, &height, &channels, STBI_rgb_alpha);
		}

		if (filedata == nullptr)
		{
			Logging::Log() << "Source " << texture_name << " for texture '" << texture.unique_name << "' could not be loaded! Make sure it is of a compatible file format.";
			_last_texture_reload_successfull = false;
			continue;
		}

		// Need to potentially resize image data to the texture dimensions
		if (texture.width != uint32_t(width) || texture.height != uint32_t(height))
		{
			Logging::Log() << "Resizing image data for texture '" << texture.unique_name << "' from " << width << "x" << height << " to " << texture.width << "x" << texture.height << " ...";

			std::vector<uint8_t> resized(texture.width * texture.height * 4);
			stbir_resize_uint8(filedata, width, height, 0, resized.data(), texture.width, texture.height, 0, 4);
			upload_texture(texture, resized.data());
		}
		else
		{
			upload_texture(texture, filedata);
		}

		stbi_image_free(filedata);

		texture.loaded = true;
	}

	_textures_loaded = true;
}

void reshade::runtime::unload_effect(size_t effect_index)
{
	assert(effect_index < _effects.size());

	// Lock here to be safe in case another effect is still loading
	const std::lock_guard<std::mutex> lock(_reload_mutex);

	// Destroy textures belonging to this effect
	_textures.erase(std::remove_if(_textures.begin(), _textures.end(),
		[this, effect_index](texture &tex) {
			tex.shared.erase(std::remove(tex.shared.begin(), tex.shared.end(), effect_index), tex.shared.end());
			if (tex.shared.empty()) {
				destroy_texture(tex);
				return true;
			}
			return false;
		}), _textures.end());
	// Clean up techniques belonging to this effect
	_techniques.erase(std::remove_if(_techniques.begin(), _techniques.end(),
		[effect_index](const technique &tech) {
			return tech.effect_index == effect_index;
		}), _techniques.end());

	// Do not clear source file, so that an 'unload_effect' immediately followed by a 'load_effect' which accesses that works
	effect &effect = _effects[effect_index];
	effect.rendering = false;
	effect.compiled = false;
	effect.errors.clear();
	effect.preamble.clear();
	effect.included_files.clear();
	effect.definitions.clear();
	effect.assembly.clear();
	effect.uniforms.clear();
	effect.uniform_data_storage.clear();
}
void reshade::runtime::unload_effects()
{
	// Make sure no threads are still accessing effect data
	for (std::thread &thread : _worker_threads)
		if (thread.joinable())
			thread.join();
	_worker_threads.clear();

	// Destroy all textures
	for (texture &tex : _textures)
		destroy_texture(tex);
	_textures.clear();
	_textures_loaded = false;
	// Clean up all techniques
	_techniques.clear();

	// Reset the effect list after all resources have been destroyed
	_effects.clear();
}

void reshade::runtime::update_and_render_effects()
{
	// Delay first load to the first render call to avoid loading while the application is still initializing
	if (_framecount == 0 && !_no_reload_on_init)
		load_effects();

	if (_reload_remaining_effects == 0)
	{
		// Clear the thread list now that they all have finished
		for (std::thread &thread : _worker_threads)
			if (thread.joinable())
				thread.join(); // Threads have exited, but still need to join them prior to destruction
		_worker_threads.clear();

		// Finished loading effects, so apply preset to figure out which ones need compiling
		load_current_preset();

		_last_reload_time = std::chrono::high_resolution_clock::now();
		_reload_total_effects = 0;
		_reload_remaining_effects = std::numeric_limits<size_t>::max();

		// Reset all effect loading options
		_load_option_disable_skipping = false;

	}
	else if (_reload_remaining_effects != std::numeric_limits<size_t>::max())
	{
		return; // Cannot render while effects are still being loaded
	}
	else if (!_reload_compile_queue.empty())
	{
		bool success = true;

		// Pop an effect from the queue
		const size_t effect_index = _reload_compile_queue.back();
		_reload_compile_queue.pop_back();
		effect &effect = _effects[effect_index];

		// Create textures now, since they are referenced when building samplers in the 'init_effect' call below
		for (texture &texture : _textures)
		{
			// Always create shared textures, since they may be in use by this effect already
			if (texture.impl == nullptr && (texture.effect_index == effect_index || texture.shared.size() > 1))
			{
				if (!init_texture(texture))
				{
					success = false;
					effect.errors += "Failed to create texture " + texture.unique_name;
					break;
				}
			}
		}

		// Compile the effect with the back-end implementation
		if (success && (success = init_effect(effect_index)) == false)
		{
			// De-duplicate error lines (D3DCompiler sometimes repeats the same error multiple times)
			for (size_t cur_line_offset = 0, next_line_offset, end_offset;
				(next_line_offset = effect.errors.find('\n', cur_line_offset)) != std::string::npos && (end_offset = effect.errors.find('\n', next_line_offset + 1)) != std::string::npos; cur_line_offset = next_line_offset + 1)
			{
				const std::string_view cur_line(effect.errors.c_str() + cur_line_offset, next_line_offset - cur_line_offset);
				const std::string_view next_line(effect.errors.c_str() + next_line_offset + 1, end_offset - next_line_offset - 1);

				if (cur_line == next_line)
				{
					effect.errors.erase(next_line_offset, end_offset - next_line_offset);
					next_line_offset = cur_line_offset - 1;
				}
			}

			if (effect.errors.empty())
				Logging::Log() << "Failed initializing " << effect.source_file << '.';
			else
				Logging::Log() << "Failed initializing " << effect.source_file << ":\n" << effect.errors;
		}

		if (success == false) // Something went wrong, do clean up
		{
			// Destroy all textures belonging to this effect
			for (texture &tex : _textures)
				if (tex.effect_index == effect_index && tex.shared.size() <= 1)
					destroy_texture(tex);
			// Disable all techniques belonging to this effect
			for (technique &tech : _techniques)
				if (tech.effect_index == effect_index)
					disable_technique(tech);

			effect.compiled = false;
			_last_shader_reload_successfull = false;
		}

		// An effect has changed, need to reload textures
		_textures_loaded = false;

	}
	else if (!_textures_loaded)
	{
		// Now that all effects were compiled, load all textures
		load_textures();
	}

	// Nothing to do here if effects are disabled globally
	if (!_effects_enabled)
		return;

	// Update special uniform variables
	for (effect &effect : _effects)
	{
		if (!effect.rendering)
			continue;

		for (uniform &variable : effect.uniforms)
		{
			switch (variable.special)
			{
				case special_uniform::frame_time:
				{
					set_uniform_value(variable, _last_frame_duration.count() * 1e-6f);
					break;
				}
				case special_uniform::frame_count:
				{
					if (variable.type.is_boolean())
						set_uniform_value(variable, (_framecount % 2) == 0);
					else
						set_uniform_value(variable, static_cast<unsigned int>(_framecount % UINT_MAX));
					break;
				}
				case special_uniform::random:
				{
					const int min = variable.annotation_as_int("min", 0, 0);
					const int max = variable.annotation_as_int("max", 0, RAND_MAX);
					set_uniform_value(variable, min + (std::rand() % (std::abs(max - min) + 1)));
					break;
				}
				case special_uniform::ping_pong:
				{
					const float min = variable.annotation_as_float("min", 0, 0.0f);
					const float max = variable.annotation_as_float("max", 0, 1.0f);
					const float step_min = variable.annotation_as_float("step", 0);
					const float step_max = variable.annotation_as_float("step", 1);
					float increment = step_max == 0 ? step_min : (step_min + std::fmodf(static_cast<float>(std::rand()), step_max - step_min + 1));
					const float smoothing = variable.annotation_as_float("smoothing");

					float value[2] = { 0, 0 };
					get_uniform_value(variable, value, 2);
					if (value[1] >= 0)
					{
						increment = std::max(increment - std::max(0.0f, smoothing - (max - value[0])), 0.05f);
						increment *= _last_frame_duration.count() * 1e-9f;

						if ((value[0] += increment) >= max)
							value[0] = max, value[1] = -1;
					}
					else
					{
						increment = std::max(increment - std::max(0.0f, smoothing - (value[0] - min)), 0.05f);
						increment *= _last_frame_duration.count() * 1e-9f;

						if ((value[0] -= increment) <= min)
							value[0] = min, value[1] = +1;
					}
					set_uniform_value(variable, value, 2);
					break;
				}
				case special_uniform::date:
				{
					set_uniform_value(variable, _date, 4);
					break;
				}
				case special_uniform::timer:
				{
					const unsigned long long timer_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_last_present_time - _start_time).count();
					set_uniform_value(variable, static_cast<unsigned int>(timer_ms));
					break;
				}
				case special_uniform::bufready_depth:
				{
					set_uniform_value(variable, _has_depth_texture);
					break;
				}
			}
		}
	}

	// Render all enabled techniques
	for (technique &technique : _techniques)
	{
		if (technique.impl == nullptr || !technique.enabled)
			continue; // Ignore techniques that are not fully loaded or currently disabled

		const auto time_technique_started = std::chrono::high_resolution_clock::now();
		render_technique(technique);
		const auto time_technique_finished = std::chrono::high_resolution_clock::now();

		technique.average_cpu_duration.append(std::chrono::duration_cast<std::chrono::nanoseconds>(time_technique_finished - time_technique_started).count());

		if (technique.time_left > 0)
		{
			technique.time_left -= std::chrono::duration_cast<std::chrono::milliseconds>(_last_frame_duration).count();
			if (technique.time_left <= 0)
				disable_technique(technique);
		}
	}
}

void reshade::runtime::enable_technique(technique &technique)
{
	assert(technique.effect_index < _effects.size());

	if (!_effects[technique.effect_index].compiled)
		return; // Cannot enable techniques that failed to compile

	Logging::Log() << "Enabling technique: " << technique.name;

	const bool status_changed = !technique.enabled;
	technique.enabled = true;
	technique.time_left = technique.annotation_as_int("timeout");

	// Queue effect file for compilation if it was not fully loaded yet
	if (technique.impl == nullptr && // Avoid adding the same effect multiple times to the queue if it contains multiple techniques that were enabled simultaneously
		std::find(_reload_compile_queue.begin(), _reload_compile_queue.end(), technique.effect_index) == _reload_compile_queue.end())
	{
		_reload_total_effects++;
		_reload_compile_queue.push_back(technique.effect_index);
	}

	if (status_changed) // Increase rendering reference count
		_effects[technique.effect_index].rendering++;
}
void reshade::runtime::disable_technique(technique &technique)
{
	assert(technique.effect_index < _effects.size());

	Logging::Log() << "Disabling technique: " << technique.name;

	const bool status_changed =  technique.enabled;
	technique.enabled = false;
	technique.time_left = 0;
	technique.average_cpu_duration.clear();
	technique.average_gpu_duration.clear();

	if (status_changed) // Decrease rendering reference count
		_effects[technique.effect_index].rendering--;
}

void reshade::runtime::subscribe_to_load_config(std::function<void(const ini_file &)> function)
{
	_load_config_callables.push_back(function);

	function(ini_file::load_cache());
}
void reshade::runtime::subscribe_to_save_config(std::function<void(ini_file &)> function)
{
	_save_config_callables.push_back(function);

	function(ini_file::load_cache());
}

void reshade::runtime::load_config()
{
	const ini_file &config = ini_file::load_cache();

	config.get("GENERAL", "NoDebugInfo", _no_debug_info);
	config.get("GENERAL", "NoReloadOnInit", _no_reload_on_init);

	config.get("GENERAL", "PerformanceMode", _performance_mode);
	config.get("GENERAL", "PreprocessorDefinitions", _global_preprocessor_definitions);
	config.get("GENERAL", "EffectLoadSkipping", _effect_load_skipping);
	config.get("GENERAL", "PresetTransitionDelay", _preset_transition_delay);

	for (const auto &callback : _load_config_callables)
		callback(config);
}

void reshade::runtime::load_current_preset()
{
	_preset_save_success = true;

	const ini_file config = ini_file::load_cache(); // Copy config, because reference becomes invalid in the next line
	const ini_file &preset = ini_file::load_cache();

	std::vector<std::string> technique_list;
	preset.get({}, "Techniques", technique_list);
	std::vector<std::string> sorted_technique_list;
	preset.get({}, "TechniqueSorting", sorted_technique_list);
	std::vector<std::string> preset_preprocessor_definitions;
	preset.get({}, "PreprocessorDefinitions", preset_preprocessor_definitions);

	// Recompile effects if preprocessor definitions have changed or running in performance mode (in which case all preset values are compile-time constants)
	if (_reload_remaining_effects != 0) // ... unless this is the 'load_current_preset' call in 'update_and_render_effects'
	{
		if (_performance_mode || preset_preprocessor_definitions != _preset_preprocessor_definitions)
		{
			_preset_preprocessor_definitions = std::move(preset_preprocessor_definitions);
			load_effects();
			return; // Preset values are loaded in 'update_and_render_effects' during effect loading
		}

		if (std::find_if(technique_list.begin(), technique_list.end(), [this](const std::string &technique) {
				if (const size_t at_pos = technique.find('@'); at_pos == std::string::npos)
					return true;
				else if (const auto it = std::find_if(_effects.begin(), _effects.end(),
					[effect_name = static_cast<std::string_view>(technique).substr(at_pos + 1)](const effect &effect) { return effect_name == effect.source_file.filename().u8string(); }); it == _effects.end())
					return true;
				else
					return it->skipped; }) != technique_list.end())
		{
			load_effects();
			return;
		}
	}

	if (sorted_technique_list.empty())
		config.get("GENERAL", "TechniqueSorting", sorted_technique_list);
	if (sorted_technique_list.empty())
		sorted_technique_list = technique_list;

	// Reorder techniques
	std::sort(_techniques.begin(), _techniques.end(), [this, &sorted_technique_list](const technique &lhs, const technique &rhs) {
		const std::string lhs_unique = lhs.name + '@' + _effects[lhs.effect_index].source_file.filename().u8string();
		const std::string rhs_unique = rhs.name + '@' + _effects[rhs.effect_index].source_file.filename().u8string();
		auto lhs_it = std::find(sorted_technique_list.begin(), sorted_technique_list.end(), lhs_unique);
		auto rhs_it = std::find(sorted_technique_list.begin(), sorted_technique_list.end(), rhs_unique);
		if (lhs_it == sorted_technique_list.end())
			lhs_it = std::find(sorted_technique_list.begin(), sorted_technique_list.end(), lhs.name);
		if (rhs_it == sorted_technique_list.end())
			rhs_it = std::find(sorted_technique_list.begin(), sorted_technique_list.end(), rhs.name);
		return lhs_it < rhs_it; });

	// Compute times since the transition has started and how much is left till it should end
	auto transition_time = std::chrono::duration_cast<std::chrono::microseconds>(_last_present_time - _last_preset_switching_time).count();
	auto transition_ms_left = _preset_transition_delay - transition_time / 1000;
	auto transition_ms_left_from_last_frame = transition_ms_left + std::chrono::duration_cast<std::chrono::microseconds>(_last_frame_duration).count() / 1000;

	if (_is_in_between_presets_transition && transition_ms_left <= 0)
		_is_in_between_presets_transition = false;

	for (effect &effect : _effects)
	{
		for (uniform &variable : effect.uniforms)
		{
			if (variable.special != special_uniform::none)
				continue;
			const std::string section = effect.source_file.filename().u8string();

			if (variable.supports_toggle_key())
			{
				// Load shortcut key, but first reset it, since it may not exist in the preset file
				std::memset(variable.toggle_key_data, 0, sizeof(variable.toggle_key_data));
				preset.get(section, "Key" + variable.name, variable.toggle_key_data);
			}

			if (!_is_in_between_presets_transition)
				// Reset values to defaults before loading from a new preset
				reset_uniform_value(variable);

			reshadefx::constant values, values_old;
			switch (variable.type.base)
			{
			case reshadefx::type::t_int:
				get_uniform_value(variable, values.as_int, variable.type.components());
				preset.get(section, variable.name, values.as_int);
				set_uniform_value(variable, values.as_int, variable.type.components());
				break;
			case reshadefx::type::t_bool:
			case reshadefx::type::t_uint:
				get_uniform_value(variable, values.as_uint, variable.type.components());
				preset.get(section, variable.name, values.as_uint);
				set_uniform_value(variable, values.as_uint, variable.type.components());
				break;
			case reshadefx::type::t_float:
				get_uniform_value(variable, values.as_float, variable.type.components());
				values_old = values;
				preset.get(section, variable.name, values.as_float);
				if (_is_in_between_presets_transition)
				{
					// Perform smooth transition on floating point values
					for (unsigned int i = 0; i < variable.type.components(); i++)
					{
						const auto transition_ratio = (values.as_float[i] - values_old.as_float[i]) / transition_ms_left_from_last_frame;
						values.as_float[i] = values.as_float[i] - transition_ratio * transition_ms_left;
					}
				}
				set_uniform_value(variable, values.as_float, variable.type.components());
				break;
			}
		}
	}

	for (technique &technique : _techniques)
	{
		const std::string unique_name =
			technique.name + '@' + _effects[technique.effect_index].source_file.filename().u8string();

		// Ignore preset if "enabled" annotation is set
		if ((technique.annotation_as_int("enabled") ||
			std::find(technique_list.begin(), technique_list.end(), unique_name) != technique_list.end() ||
			std::find(technique_list.begin(), technique_list.end(), technique.name) != technique_list.end()))
		{
			enable_technique(technique);
		}
		else
		{
			disable_technique(technique);
		}

		// Reset toggle key to the value set via annotation first, since it may not exist in the preset
		technique.toggle_key_data[0] = technique.annotation_as_int("toggle");
		technique.toggle_key_data[1] = technique.annotation_as_int("togglectrl");
		technique.toggle_key_data[2] = technique.annotation_as_int("toggleshift");
		technique.toggle_key_data[3] = technique.annotation_as_int("togglealt");
		if (!preset.get({}, "Key" + unique_name, technique.toggle_key_data) &&
			!preset.get({}, "Key" + technique.name, technique.toggle_key_data))
			std::memset(technique.toggle_key_data, 0, std::size(technique.toggle_key_data));
	}
}
void reshade::runtime::save_current_preset() const
{
	ini_file &preset = ini_file::load_cache();

	// Build list of active techniques and effects
	std::vector<std::string> technique_list, sorted_technique_list;
	std::unordered_set<size_t> effect_list;
	effect_list.reserve(_techniques.size());
	technique_list.reserve(_techniques.size());
	sorted_technique_list.reserve(_techniques.size());

	for (const technique &technique : _techniques)
	{
		const std::string unique_name =
			technique.name + '@' + _effects[technique.effect_index].source_file.filename().u8string();

		if (technique.enabled)
			technique_list.push_back(unique_name);
		if (technique.enabled || technique.toggle_key_data[0] != 0)
			effect_list.insert(technique.effect_index);

		// Keep track of the order of all techniques and not just the enabled ones
		sorted_technique_list.push_back(unique_name);

		if (technique.toggle_key_data[0] != 0)
			preset.set({}, "Key" + unique_name, technique.toggle_key_data);
		else if (int value = 0; preset.get({}, "Key" + unique_name, value) && value != 0)
			preset.set({}, "Key" + unique_name, 0); // Clear toggle key data
	}

	preset.set({}, "Techniques", std::move(technique_list));
	preset.set({}, "TechniqueSorting", std::move(sorted_technique_list));
	preset.set({}, "PreprocessorDefinitions", _preset_preprocessor_definitions);

	// TODO: Do we want to save spec constants here too? The preset will be rather empty in performance mode otherwise.
	for (size_t effect_index = 0; effect_index < _effects.size(); ++effect_index)
	{
		if (effect_list.find(effect_index) == effect_list.end())
			continue;

		const effect &effect = _effects[effect_index];

		for (const uniform &variable : effect.uniforms)
		{
			if (variable.special != special_uniform::none)
				continue;

			const std::string section = effect.source_file.filename().u8string();
			const unsigned int components = variable.type.components();
			reshadefx::constant values;

			if (variable.supports_toggle_key())
			{
				// save the shortcut key into the preset files
				if (variable.toggle_key_data[0] != 0)
					preset.set(section, "Key" + variable.name, variable.toggle_key_data);
				else if (int value = 0; preset.get(section, "Key" + variable.name, value) && value != 0)
					preset.set(section, "Key" + variable.name, 0); // Clear toggle key data
			}

			switch (variable.type.base)
			{
			case reshadefx::type::t_int:
				get_uniform_value(variable, values.as_int, components);
				preset.set(section, variable.name, values.as_int, components);
				break;
			case reshadefx::type::t_bool:
			case reshadefx::type::t_uint:
				get_uniform_value(variable, values.as_uint, components);
				preset.set(section, variable.name, values.as_uint, components);
				break;
			case reshadefx::type::t_float:
				get_uniform_value(variable, values.as_float, components);
				preset.set(section, variable.name, values.as_float, components);
				break;
			}
		}
	}
}

static inline bool force_floating_point_value(const reshadefx::type &type, uint32_t renderer_id)
{
	if (renderer_id == 0x9000)
		return true; // All uniform variables are floating-point in D3D9
	if (type.is_matrix() && (renderer_id & 0x10000))
		return true; // All matrices are floating-point in GLSL
	return false;
}

void reshade::runtime::get_uniform_value(const uniform &variable, uint8_t *data, size_t size, size_t base_index) const
{
	size = std::min(size, static_cast<size_t>(variable.size));
	assert(data != nullptr && (size % 4) == 0);

	auto &data_storage = _effects[variable.effect_index].uniform_data_storage;
	assert(variable.offset + size <= data_storage.size());

	const size_t array_length = (variable.type.is_array() ? variable.type.array_length : 1);
	assert(base_index < array_length);

	if (variable.type.is_matrix())
	{
		for (size_t a = base_index, i = 0; a < array_length; ++a)
			// Each row of a matrix is 16-byte aligned, so needs special handling
			for (size_t row = 0; row < variable.type.rows; ++row)
				for (size_t col = 0; i < (size / 4) && col < variable.type.cols; ++col, ++i)
					std::memcpy(
						data + ((a - base_index) * variable.type.components() + (row * variable.type.cols + col)) * 4,
						data_storage.data() + variable.offset + (a * (variable.type.rows * 4) + (row * 4 + col)) * 4, 4);
	}
	else if (array_length > 1)
	{
		for (size_t a = base_index, i = 0; a < array_length; ++a)
			// Each element in the array is 16-byte aligned, so needs special handling
			for (size_t row = 0; i < (size / 4) && row < variable.type.rows; ++row, ++i)
				std::memcpy(
					data + ((a - base_index) * variable.type.components() + row) * 4,
					data_storage.data() + variable.offset + (a * 4 + row) * 4, 4);
	}
	else
	{
		std::memcpy(data, data_storage.data() + variable.offset, size);
	}
}
void reshade::runtime::get_uniform_value(const uniform &variable, bool *values, size_t count, size_t array_index) const
{
	count = std::min(count, static_cast<size_t>(variable.size / 4));
	assert(values != nullptr);

	const auto data = static_cast<uint8_t *>(alloca(variable.size));
	get_uniform_value(variable, data, variable.size, array_index);

	for (size_t i = 0; i < count; i++)
		values[i] = reinterpret_cast<const uint32_t *>(data)[i] != 0;
}
void reshade::runtime::get_uniform_value(const uniform &variable, int32_t *values, size_t count, size_t array_index) const
{
	if (variable.type.is_integral() && !force_floating_point_value(variable.type, _renderer_id))
	{
		get_uniform_value(variable, reinterpret_cast<uint8_t *>(values), count * sizeof(int32_t), array_index);
		return;
	}

	count = std::min(count, static_cast<size_t>(variable.size / 4));
	assert(values != nullptr);

	const auto data = static_cast<uint8_t *>(alloca(variable.size));
	get_uniform_value(variable, data, variable.size, array_index);

	for (size_t i = 0; i < count; i++)
		values[i] = static_cast<int32_t>(reinterpret_cast<const float *>(data)[i]);
}
void reshade::runtime::get_uniform_value(const uniform &variable, uint32_t *values, size_t count, size_t array_index) const
{
	get_uniform_value(variable, reinterpret_cast<int32_t *>(values), count, array_index);
}
void reshade::runtime::get_uniform_value(const uniform &variable, float *values, size_t count, size_t array_index) const
{
	if (variable.type.is_floating_point() || force_floating_point_value(variable.type, _renderer_id))
	{
		get_uniform_value(variable, reinterpret_cast<uint8_t *>(values), count * sizeof(float), array_index);
		return;
	}

	count = std::min(count, static_cast<size_t>(variable.size / 4));
	assert(values != nullptr);

	const auto data = static_cast<uint8_t *>(alloca(variable.size));
	get_uniform_value(variable, data, variable.size, array_index);

	for (size_t i = 0; i < count; ++i)
		if (variable.type.is_signed())
			values[i] = static_cast<float>(reinterpret_cast<const int32_t *>(data)[i]);
		else
			values[i] = static_cast<float>(reinterpret_cast<const uint32_t *>(data)[i]);
}
void reshade::runtime::set_uniform_value(uniform &variable, const uint8_t *data, size_t size, size_t base_index)
{
	size = std::min(size, static_cast<size_t>(variable.size));
	assert(data != nullptr && (size % 4) == 0);

	auto &data_storage = _effects[variable.effect_index].uniform_data_storage;
	assert(variable.offset + size <= data_storage.size());

	const size_t array_length = (variable.type.is_array() ? variable.type.array_length : 1);
	assert(base_index < array_length);

	if (variable.type.is_matrix())
	{
		for (size_t a = base_index, i = 0; a < array_length; ++a)
			// Each row of a matrix is 16-byte aligned, so needs special handling
			for (size_t row = 0; row < variable.type.rows; ++row)
				for (size_t col = 0; i < (size / 4) && col < variable.type.cols; ++col, ++i)
					std::memcpy(
						data_storage.data() + variable.offset + (a * variable.type.rows * 4 + (row * 4 + col)) * 4,
						data + ((a - base_index) * variable.type.components() + (row * variable.type.cols + col)) * 4, 4);
	}
	else if (array_length > 1)
	{
		for (size_t a = base_index, i = 0; a < array_length; ++a)
			// Each element in the array is 16-byte aligned, so needs special handling
			for (size_t row = 0; i < (size / 4) && row < variable.type.rows; ++row, ++i)
				std::memcpy(
					data_storage.data() + variable.offset + (a * 4 + row) * 4,
					data + ((a - base_index) * variable.type.components() + row) * 4, 4);
	}
	else
	{
		std::memcpy(data_storage.data() + variable.offset, data, size);
	}
}
void reshade::runtime::set_uniform_value(uniform &variable, const bool *values, size_t count, size_t array_index)
{
	if (variable.type.is_floating_point() || force_floating_point_value(variable.type, _renderer_id))
	{
		const auto data = static_cast<float *>(alloca(count * sizeof(float)));
		for (size_t i = 0; i < count; ++i)
			data[i] = values[i] ? 1.0f : 0.0f;

		set_uniform_value(variable, reinterpret_cast<const uint8_t *>(data), count * sizeof(float), array_index);
	}
	else
	{
		const auto data = static_cast<uint32_t *>(alloca(count * sizeof(uint32_t)));
		for (size_t i = 0; i < count; ++i)
			data[i] = values[i] ? 1 : 0;

		set_uniform_value(variable, reinterpret_cast<const uint8_t *>(data), count * sizeof(uint32_t), array_index);
	}
}
void reshade::runtime::set_uniform_value(uniform &variable, const int32_t *values, size_t count, size_t array_index)
{
	if (variable.type.is_floating_point() || force_floating_point_value(variable.type, _renderer_id))
	{
		const auto data = static_cast<float *>(alloca(count * sizeof(float)));
		for (size_t i = 0; i < count; ++i)
			data[i] = static_cast<float>(values[i]);

		set_uniform_value(variable, reinterpret_cast<const uint8_t *>(data), count * sizeof(float), array_index);
	}
	else
	{
		set_uniform_value(variable, reinterpret_cast<const uint8_t *>(values), count * sizeof(int32_t), array_index);
	}
}
void reshade::runtime::set_uniform_value(uniform &variable, const uint32_t *values, size_t count, size_t array_index)
{
	if (variable.type.is_floating_point() || force_floating_point_value(variable.type, _renderer_id))
	{
		const auto data = static_cast<float *>(alloca(count * sizeof(float)));
		for (size_t i = 0; i < count; ++i)
			data[i] = static_cast<float>(values[i]);

		set_uniform_value(variable, reinterpret_cast<const uint8_t *>(data), count * sizeof(float), array_index);
	}
	else
	{
		set_uniform_value(variable, reinterpret_cast<const uint8_t *>(values), count * sizeof(uint32_t), array_index);
	}
}
void reshade::runtime::set_uniform_value(uniform &variable, const float *values, size_t count, size_t array_index)
{
	if (variable.type.is_floating_point() || force_floating_point_value(variable.type, _renderer_id))
	{
		set_uniform_value(variable, reinterpret_cast<const uint8_t *>(values), count * sizeof(float), array_index);
	}
	else
	{
		const auto data = static_cast<int32_t *>(alloca(count * sizeof(int32_t)));
		for (size_t i = 0; i < count; ++i)
			data[i] = static_cast<int32_t>(values[i]);

		set_uniform_value(variable, reinterpret_cast<const uint8_t *>(data), count * sizeof(int32_t), array_index);
	}
}

void reshade::runtime::reset_uniform_value(uniform &variable)
{
	if (!variable.has_initializer_value)
	{
		std::memset(_effects[variable.effect_index].uniform_data_storage.data() + variable.offset, 0, variable.size);
		return;
	}

	// Need to use typed setters, to ensure values are properly forced to floating point in D3D9
	for (size_t i = 0, array_length = (variable.type.is_array() ? variable.type.array_length : 1);
		i < array_length; ++i)
	{
		const reshadefx::constant &value = variable.type.is_array() ? variable.initializer_value.array_data[i] : variable.initializer_value;

		switch (variable.type.base)
		{
		case reshadefx::type::t_int:
			set_uniform_value(variable, value.as_int, variable.type.components(), i);
			break;
		case reshadefx::type::t_bool:
		case reshadefx::type::t_uint:
			set_uniform_value(variable, value.as_uint, variable.type.components(), i);
			break;
		case reshadefx::type::t_float:
			set_uniform_value(variable, value.as_float, variable.type.components(), i);
			break;
		}
	}
}

reshade::texture &reshade::runtime::look_up_texture_by_name(const std::string &unique_name)
{
	const auto it = std::find_if(_textures.begin(), _textures.end(),
		[&unique_name](const auto &item) { return item.unique_name == unique_name && item.impl != nullptr; });
	assert(it != _textures.end());
	return *it;
}

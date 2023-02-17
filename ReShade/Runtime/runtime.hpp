/**
* Copyright (C) 2014 Patrick Mours. All rights reserved.
* License: https://github.com/crosire/reshade#license
*
* Copyright (C) 2023 Elisha Riedlinger
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
#include <windows.h>
#include <mutex>
#include <memory>
#include <atomic>
#include <chrono>
#include <functional>

bool read_resource(DWORD id, std::string &data);

namespace reshade
{
	class ini_file; // Forward declarations to avoid excessive #include
	struct effect;
	struct uniform;
	struct texture;
	struct technique;

	// Platform independent base class for the main ReShade runtime.
	// This class needs to be implemented for all supported rendering APIs.
	class runtime
	{
	public:
		// Return whether the runtime is initialized.
		bool is_initialized() const { return _is_initialized; }

		// Return the frame width in pixels.
		unsigned int frame_width() const { return _width; }
		// Return the frame height in pixels.
		unsigned int frame_height() const { return _height; }

		// Create a new texture with the specified dimensions.
		virtual bool init_texture(texture &texture) = 0;
		// Upload the image data of a texture.
		virtual void upload_texture(const texture &texture, const uint8_t *pixels) = 0;
		// Destroy an existing texture.
		virtual void destroy_texture(texture &texture) = 0;

		// Get the value of a uniform variable.
		void get_uniform_value(const uniform &variable, uint8_t *data, size_t size, size_t base_index) const;
		void get_uniform_value(const uniform &variable, bool *values, size_t count, size_t array_index = 0) const;
		void get_uniform_value(const uniform &variable, int32_t *values, size_t count, size_t array_index = 0) const;
		void get_uniform_value(const uniform &variable, uint32_t *values, size_t count, size_t array_index = 0) const;
		void get_uniform_value(const uniform &variable, float *values, size_t count, size_t array_index = 0) const;
		// Update the value of a uniform variable.
		void set_uniform_value(uniform &variable, const uint8_t *data, size_t size, size_t base_index);
		void set_uniform_value(uniform &variable, const bool *values, size_t count, size_t array_index = 0);
		void set_uniform_value(uniform &variable, const int32_t *values, size_t count, size_t array_index = 0);
		void set_uniform_value(uniform &variable, const uint32_t *values, size_t count, size_t array_index = 0);
		void set_uniform_value(uniform &variable, const float *values, size_t count, size_t array_index = 0);
		template <typename T>
		std::enable_if_t<std::is_same_v<T, bool> || std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, float>>
		set_uniform_value(uniform &variable, T x, T y = T(0), T z = T(0), T w = T(0))
		{
			const T data[4] = { x, y, z, w };
			set_uniform_value(variable, data, 4);
		}

		// Reset a uniform variable to its initial value.
		void reset_uniform_value(uniform &variable);

		// <summary>
		void subscribe_to_load_config(std::function<void(const ini_file &)> function);
		// Register a function to be called when user configuration is stored.
		void subscribe_to_save_config(std::function<void(ini_file &)> function);

	protected:
		runtime();
		virtual ~runtime();

		// Callback function called when the runtime is initialized.
		bool on_init(void *);
		// Callback function called when the runtime is uninitialized.
		void on_reset(bool clear_width_height = true);
		// Callback function called every frame.
		void on_present();

		// Compile effect from the specified source file and initialize textures, uniforms and techniques.
		bool load_effect(const std::string &name, DWORD id, size_t effect_index);
		// Load all effects found in the effect search paths.
		void load_effects();
		// Initialize resources for the effect and load the effect module.
		virtual bool init_effect(size_t effect_index) = 0;
		// Unload the specified effect.
		virtual void unload_effect(size_t effect_index);
		// Unload all effects currently loaded.
		virtual void unload_effects();

		// Load image files and update textures with image data.
		void load_textures();

		// Apply post-processing effects to the frame.
		void update_and_render_effects();
		// Render all passes in a technique.
		virtual void render_technique(technique &technique) = 0;

		// Returns the texture object corresponding to the passed "unique_name".
		texture &look_up_texture_by_name(const std::string &unique_name);

		bool _is_initialized = false;
		bool _performance_mode = false;
		bool _has_depth_texture = false;
		unsigned int _width = 0;
		unsigned int _height = 0;
		unsigned int _window_width = 0;
		unsigned int _window_height = 0;
		unsigned int _vendor_id = 0;
		unsigned int _device_id = 0;
		unsigned int _renderer_id = 0;
		unsigned int _color_bit_depth = 8;

		uint64_t _framecount = 0;
		unsigned int _vertices = 0;
		unsigned int _drawcalls = 0;

		std::vector<effect> _effects;
		std::vector<texture> _textures;
		std::vector<technique> _techniques;

	private:
		// Compare current version against the latest published one.
		bool is_loading() const { return _reload_remaining_effects != std::numeric_limits<size_t>::max(); }

		// Enable a technique so it is rendered.
		void enable_technique(technique &technique);
		// Disable a technique so that it is no longer rendered.
		void disable_technique(technique &technique);

		// Load user configuration from disk.
		void load_config();

		// Load the selected preset and apply it.
		void load_current_preset();
		// Save the current value configuration to the currently selected preset.
		void save_current_preset() const;

		// === Status ===
		int _date[4] = {};
		bool _effects_enabled = true;
		bool _ignore_shortcuts = false;
		bool _force_shortcut_modifiers = true;
		unsigned int _effects_key_data[4];
		std::shared_ptr<class input> _input;
		std::chrono::high_resolution_clock::duration _last_frame_duration;
		std::chrono::high_resolution_clock::time_point _start_time;
		std::chrono::high_resolution_clock::time_point _last_present_time;

		// == Configuration ===
		bool _needs_update = false;
		unsigned long _latest_version[3] = {};
		std::vector<std::function<void(ini_file &)>> _save_config_callables;
		std::vector<std::function<void(const ini_file &)>> _load_config_callables;

		// === Effect Loading ===
		bool _no_debug_info = 0;
		bool _no_reload_on_init = false;
		bool _effect_load_skipping = false;
		bool _load_option_disable_skipping = false;
		bool _last_shader_reload_successfull = true;
		bool _last_texture_reload_successfull = true;
		bool _textures_loaded = false;
		unsigned int _reload_key_data[4];
		unsigned int _performance_mode_key_data[4];
		size_t _reload_total_effects = 1;
		std::vector<size_t> _reload_compile_queue;
		std::atomic<size_t> _reload_remaining_effects = 0;
		std::mutex _reload_mutex;
		std::vector<std::thread> _worker_threads;
		std::vector<std::string> _global_preprocessor_definitions;
		std::vector<std::string> _preset_preprocessor_definitions;
		std::chrono::high_resolution_clock::time_point _last_reload_time;

		// === Preset Switching ===
		bool _preset_save_success = true;
		bool _is_in_between_presets_transition = false;
		unsigned int _prev_preset_key_data[4];
		unsigned int _next_preset_key_data[4];
		unsigned int _preset_transition_delay = 1000;
		std::chrono::high_resolution_clock::time_point _last_preset_switching_time;
	};
}

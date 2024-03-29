#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		// Reflect
		enum MaterialFlags
		{
			MaterialFlagNone = 0,
			MaterialFlagMirror = 1 << 0
		};

		struct FileTexture
		{
			std::filesystem::path filename;
			Filter mag_filter = FilterLinear;
			Filter min_filter = FilterLinear;
			bool linear_mipmap = true;
			AddressMode address_mode = AddressRepeat;
		};

		// Reflect
		struct Material
		{
			// Reflect
			vec4 color = vec4(1.f);
			// Reflect
			virtual void set_color(const vec4& v) = 0;
			// Reflect RANGE=ZO
			float metallic = 0.f;
			// Reflect
			virtual void set_metallic(float v) = 0;
			// Reflect RANGE=ZO
			float roughness = 1.f;
			// Reflect
			virtual void set_roughness(float v) = 0;
			// Reflect
			vec4 emissive = vec4(0.f);
			// Reflect
			virtual void set_emissive(const vec4& v) = 0;
			// Reflect
			float tiling = 1.f;
			// Reflect
			virtual void set_tiling(float v) = 0;
			// Reflect
			RenderQueue render_queue = RenderQueue::Opaque;
			// Reflect
			virtual void set_render_queue(RenderQueue q) = 0;
			// Reflect
			bool mirror = false;
			// Reflect
			virtual void set_mirror(bool v) = 0;
			// Reflect
			int color_map = -1;
			// Reflect
			virtual void set_color_map(int i) = 0;
			// Reflect
			int normal_map = -1;
			// Reflect
			virtual void set_normal_map(int i) = 0;
			// Reflect
			float normal_map_strength = 1.f;
			// Reflect
			virtual void set_normal_map_strength(float v) = 0;
			// Reflect
			int metallic_map = -1;
			// Reflect
			virtual void set_metallic_map(int i) = 0;
			// Reflect
			int roughness_map = -1;
			// Reflect
			virtual void set_roughness_map(int i) = 0;
			// Reflect
			int emissive_map = -1;
			// Reflect
			virtual void set_emissive_map(int i) = 0;
			// Reflect
			float emissive_map_strength = 1.f;
			// Reflect
			virtual void set_emissive_map_strength(float v) = 0;
			// Reflect
			int alpha_map = -1;
			// Reflect
			virtual void set_alpha_map(int i) = 0;
			// Reflect
			int splash_map = -1;
			// Reflect
			virtual void set_splash_map(int i) = 0;

			// Reflect
			std::vector<std::pair<std::string, float>> float_variables;
			// Reflect
			std::vector<std::pair<std::string, int>> int_variables;

			// shader will insert this file to its content
			// Reflect
			std::filesystem::path code_file = L"flame/shaders/standard_mat.glsl";
			// Reflect
			virtual void set_code_file(const std::filesystem::path& path) = 0;
			// Reflect
			std::vector<std::string> defines;
			// Reflect
			virtual void set_defines(const std::vector<std::string>& defines) = 0;

			// Reflect
			std::vector<FileTexture> textures;
			// Reflect
			virtual void set_textures(const std::vector<FileTexture>& textures) = 0;

			virtual void generate_code(bool force = false) = 0;

			Listeners<void(uint)> data_listeners;

			std::filesystem::path filename;
			uint ref = 0;

			virtual ~Material() {}

			inline void data_changed(uint h)
			{
				data_listeners.call(h);
			}

			inline MaterialFlags get_flags() const
			{
				uint flags = 0;
				if (mirror) flags |= MaterialFlagMirror;
				return (MaterialFlags)flags;
			}

			virtual void save(const std::filesystem::path& filename) = 0;

			struct Create
			{
				virtual MaterialPtr operator()() = 0;
			};
			// Reflect static
			FLAME_GRAPHICS_API static Create& create;

			struct Get
			{
				virtual MaterialPtr operator()(const std::filesystem::path& filename) = 0;
			};
			// Reflect static
			FLAME_GRAPHICS_API static Get& get;

			struct Release
			{
				virtual void operator()(MaterialPtr material) = 0;
			};
			// Reflect static
			FLAME_GRAPHICS_API static Release& release;
		};
	}
}

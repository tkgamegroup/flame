#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Texture
		{
			std::filesystem::path filename;
			bool srgb = false;
			Filter mag_filter = FilterLinear;
			Filter min_filter = FilterLinear;
			bool linear_mipmap = true;
			AddressMode address_mode = AddressRepeat;
			bool auto_mipmap = false;
		};

		struct Material
		{
			vec4 color = vec4(1.f);
			float metallic = 0.f;
			float roughness = 1.f;
			bool opaque = true;
			bool sort = false;
			int color_map = -1;
			int normal_map = -1;
			int metallic_map = -1;
			int roughness_map = -1;
			int alpha_map = -1;
			float alpha_test = 0.f;

			vec4 float_values = vec4(0.f);
			ivec4 int_values = ivec4(0);

			// shader will insert this file to its content
			std::filesystem::path code_file = L"flame/shaders/default_mat.glsl";
			std::vector<std::string> shader_defines;

			std::vector<Texture> textures;

			std::filesystem::path filename;
			uint ref = 0;

			virtual ~Material() {}

			virtual void save(const std::filesystem::path& filename) = 0;

			struct Create
			{
				virtual MaterialPtr operator()() = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct Get
			{
				virtual MaterialPtr operator()(const std::filesystem::path& filename) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Release
			{
				virtual void operator()(MaterialPtr material) = 0;
			};
			FLAME_GRAPHICS_API static Release& release;
		};
	}
}

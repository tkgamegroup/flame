#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Material
		{
			struct Texture
			{
				std::filesystem::path filename;
				bool srgb = false;
				Filter mag_filter = FilterLinear;
				Filter min_filter = FilterLinear;
				bool linear_mipmap = true;
				AddressMode address_mode = AddressClampToEdge;
				bool auto_mipmap = false;
			};

			vec4 color = vec4(1.f);
			float metallic = 0.f;
			float roughness = 1.f;
			bool opaque = true;
			bool sort = false;

			std::filesystem::path shader_file = L"flame/assets/shaders/std_mat.glsl";
			std::vector<std::string> shader_defines;

			Texture textures[8] = {};

			std::filesystem::path filename;

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
		};
	}
}

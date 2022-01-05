#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		inline const auto MaxMaterialTexturesCount = 8U;

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

			std::filesystem::path pipeline_file = "standard_mataterial.glsl";
			std::string pipeline_defines;

			Texture textures[MaxMaterialTexturesCount] = {};

			std::filesystem::path filename;

			struct Get
			{
				virtual MaterialPtr operator()(const std::filesystem::path& filename) = 0;
			};
			FLAME_GRAPHICS_EXPORTS static Get& get;
		};
	}
}

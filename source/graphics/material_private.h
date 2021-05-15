#pragma once

#include "material.h"

namespace flame
{
	namespace graphics
	{
		struct MaterialPrivate : Material
		{
			struct Texture
			{
				std::filesystem::path filename;
				bool srgb = true;
				Filter mag_filter = FilterLinear;
				Filter min_filter = FilterLinear;
				bool linear_mipmap = true;
				AddressMode address_mode = AddressClampToEdge;
			};

			std::filesystem::path filename;

			vec4 color = vec4(1.f, 0.f, 0.5f, 1.f);
			float metallic = 0.f;
			float roughness = 1.f;
			float alpha_test = 0.f;
			bool double_side = false;

			std::filesystem::path pipeline_file = "standard_mat.glsl";
			std::string pipeline_defines;

			Texture textures[4] = {};

			vec4 get_color() const override { return color; }
			float get_metallic() const override { return metallic; }
			float get_roughness() const override { return roughness; }
			float get_alpha_test() const override { return alpha_test; }

			void get_pipeline_file(wchar_t* dst) const override;
			const char* get_pipeline_defines() const override { return pipeline_defines.c_str(); }

			void get_texture_file(uint idx, wchar_t* dst) const override;
			bool get_texture_srgb(uint idx) const override { return idx < 4 ? textures[idx].srgb : false; }
			SamplerPtr get_texture_sampler(DevicePtr device, uint idx) const override;

			static MaterialPrivate* get(const std::filesystem::path& filename);
		};

		extern MaterialPrivate* default_material;
		extern std::vector<std::unique_ptr<MaterialPrivate>> loaded_materials;
	}
}

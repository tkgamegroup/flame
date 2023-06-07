#pragma once

#include "material.h"

namespace flame
{
	namespace graphics
	{
		struct MaterialPrivate : Material
		{
			MaterialPrivate();
			~MaterialPrivate();

			void set_color(const vec4& v) override;
			void set_metallic(float v) override;
			void set_roughness(float v) override;
			void set_emissive(const vec4& v) override;
			void set_tiling(float v) override;
			void set_render_queue(RenderQueue q) override;
			void set_mirror(bool v) override;
			void set_color_map(int i) override;
			void set_normal_map(int i) override;
			void set_normal_map_strength(float v) override;
			void set_metallic_map(int i) override;
			void set_roughness_map(int i) override;
			void set_emissive_map(int i) override;
			void set_emissive_map_strength(float v) override;
			void set_alpha_map(int i) override;
			void set_splash_map(int i)override;

			void set_code_file(const std::filesystem::path& path) override;
			void set_defines(const std::vector<std::string>& defines) override;

			void set_textures(const std::vector<Texture>& textures) override;

			void save(const std::filesystem::path& filename) override;
		};

		extern MaterialPtr default_material;
		extern std::vector<MaterialPtr> materials;
		extern std::vector<std::unique_ptr<MaterialT>> loaded_materials;
	}
}

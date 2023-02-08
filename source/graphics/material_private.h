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

			void set_color_map(int i) override;
			void set_normal_map(int i) override;
			void set_metallic_map(int i) override;
			void set_roughness_map(int i) override;
			void set_emissive_map(int i) override;
			void set_alpha_map(int i) override;

			void set_textures(const std::vector<Texture>& textures) override;

			void save(const std::filesystem::path& filename) override;
		};

		extern MaterialPtr default_material;
		extern std::vector<MaterialPtr> materials;
		extern std::vector<std::unique_ptr<MaterialT>> loaded_materials;
	}
}

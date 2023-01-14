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

			void save(const std::filesystem::path& filename) override;
		};

		extern MaterialPtr default_material;
		extern std::vector<MaterialPtr> materials;
		extern std::vector<std::unique_ptr<MaterialT>> loaded_materials;
	}
}

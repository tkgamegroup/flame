#pragma once

#include "material.h"

namespace flame
{
	namespace graphics
	{
		struct MaterialPrivate : Material
		{
			uint ref = 0;

			void save(const std::filesystem::path& filename) override;
		};

		extern MaterialPtr default_material;
	}
}

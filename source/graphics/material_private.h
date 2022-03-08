#pragma once

#include "material.h"

namespace flame
{
	namespace graphics
	{
		struct MaterialPrivate : Material
		{
			void save(const std::filesystem::path& filename) override;
		};

		extern MaterialPtr default_material;
	}
}

#pragma once

#include "model.h"

namespace flame
{
	namespace graphics
	{
		struct ModelPrivate : Model
		{
			uint ref = 0;

			void save(const std::filesystem::path& filename) override;
		};
	}
}

#pragma once

#include "model.h"

namespace flame
{
	namespace graphics
	{
		struct ModelPrivate : Model
		{
			void save(const std::filesystem::path& filename, bool binary) override;
		};
	}
}

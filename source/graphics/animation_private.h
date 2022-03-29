#pragma once

#include "animation.h"

namespace flame
{
	namespace graphics
	{
		struct AnimationPrivate : Animation
		{
			uint ref = 0;

			void save(const std::filesystem::path& filename) override;
		};
	}
}

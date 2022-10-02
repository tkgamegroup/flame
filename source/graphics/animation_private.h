#pragma once

#include "animation.h"

namespace flame
{
	namespace graphics
	{
		struct AnimationPrivate : Animation
		{
			void save(const std::filesystem::path& filename) override;
		};
	}
}

#pragma once

#include "animation.h"

namespace flame
{
	namespace graphics
	{
		struct AnimationPrivate : Animation
		{
			uint ref = 0;
		};
	}
}

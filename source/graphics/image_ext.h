#pragma once

#include "image.h"

namespace flame
{
	namespace graphics
	{
		FLAME_GRAPHICS_EXPORTS float image_alpha_test_coverage(ImagePtr* img, uint level, float ref, uint channel, float scale);
		FLAME_GRAPHICS_EXPORTS void image_scale_alpha_to_coverage(ImagePtr* img, uint level, float desired, float ref, uint channel);
	}
}

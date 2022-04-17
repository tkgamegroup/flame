#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Debug
		{
			struct GetAllImages
			{
				virtual std::vector<ImagePtr> operator()() = 0;
			};
			FLAME_GRAPHICS_API static GetAllImages& get_all_images;
		};
	}
}

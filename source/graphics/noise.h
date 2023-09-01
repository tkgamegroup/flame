#pragma once

#include "texture.h"

namespace flame
{
	namespace graphics
	{
		FLAME_GRAPHICS_API float voronoi_noise(const vec2& coord);
		FLAME_GRAPHICS_API float perlin_noise(const vec2& coord);
	}
}

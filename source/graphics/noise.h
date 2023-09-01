#pragma once

#include "texture.h"

namespace flame
{
	namespace graphics
	{
		float voronoi_noise(const vec2& coord);
		float perlin_noise(const vec2& coord);
	}
}

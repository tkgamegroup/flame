#pragma once

#include "texture.h"

namespace flame
{
	namespace graphics
	{
		FLAME_GRAPHICS_API float voronoi_noise(const vec2& coord);
		FLAME_GRAPHICS_API float perlin_noise(const vec2& coord);
		inline vec2 perlin_gradient(const vec2& coord, float step)
		{
			return vec2(
				perlin_noise(coord + vec2(step, 0)) - perlin_noise(coord - vec2(step, 0)),
				perlin_noise(coord + vec2(0, step)) - perlin_noise(coord - vec2(0, step)
			));
		}
	}
}

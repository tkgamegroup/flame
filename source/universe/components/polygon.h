#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cPolygon : Component
	{
		// Reflect requires
		cElementPtr element = nullptr;

		vec2 pts[8];
		vec2 uvs[8];
		uint num_pts = 0;
		cvec4 color = cvec4(255);

		inline void add_pt(const vec2& pt, const vec2& uv)
		{
			pts[num_pts] = pt;
			uvs[num_pts] = uv;
			num_pts++;
		}

		graphics::ImagePtr image = nullptr;
		graphics::SamplerPtr sampler = nullptr;

		struct Create
		{
			virtual cPolygonPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}

#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cCurve : Component
	{
		// Reflect requires
		cNodePtr node = nullptr;

		// Reflect
		float segment_length = 1.f;
		// Reflect
		virtual void set_segment_length(float length) = 0;

		// Reflect
		float curvedness = 0.5f;
		// Reflect
		virtual void set_curvedness(float curvedness) = 0;

		// Reflect
		std::vector<vec3> ctrl_points;
		// Reflect
		virtual void set_ctrl_points(const std::vector<vec3>& points) = 0;

		struct Create
		{
			virtual cCurvePtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}

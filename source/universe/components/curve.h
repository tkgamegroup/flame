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
		uint tess = 8;
		// Reflect
		virtual void set_tess(uint tess) = 0;

		// Reflect
		float curveness = 0.2f;
		// Reflect
		virtual void set_curveness(float curveness) = 0;

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

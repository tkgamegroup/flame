#pragma once

#include "curve.h"

namespace flame
{
	struct cCurvePrivate : cCurve
	{
		Curve<vec3> curve;
		AABB bounds;

		~cCurvePrivate();
		void on_init() override;

		void update_curve();
		void set_tess(uint tess) override;
		void set_curveness(float curveness) override;
		void set_ctrl_points(const std::vector<vec3>& points) override;
	};
}

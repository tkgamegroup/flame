#pragma once

#include "curve.h"

namespace flame
{
	struct cCurvePrivate : cCurve
	{
		Curve<3> curve;
		AABB bounds;

		~cCurvePrivate();
		void on_init() override;

		void update_curve();
		void set_segment_length(float length) override;
		void set_curvedness(float curvedness) override;
		void set_ctrl_points(const std::vector<vec3>& points) override;
	};
}

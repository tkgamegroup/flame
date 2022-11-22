#include "curve_private.h"
#include "node_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cCurvePrivate::~cCurvePrivate()
	{
		node->drawers.remove("curve"_h);
		node->measurers.remove("curve"_h);
	}

	void cCurvePrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data) {
			if (curve.vertices.empty())
				return;

			switch (draw_data.pass)
			{
			case PassPrimitive:
			{
				auto transformed_vertices = curve.vertices;
				for (auto& v : transformed_vertices)
					v = vec3(node->transform * vec4(v, 1.f));
				draw_data.primitives.emplace_back("LineStrip"_h, transformed_vertices.data(), (uint)transformed_vertices.size(), cvec4(255, 255, 255, 255));
			}
				break;
			}
		}, "mesh"_h);
		node->measurers.add([this](AABB* ret) {
			if (curve.vertices.empty())
				return false;
			*ret = AABB(bounds.get_points(node->transform));
			return true;
		}, "curve"_h);
	}

	void cCurvePrivate::update_curve()
	{
		curve.update();
		bounds.reset();
		for (auto& v : curve.vertices)
			bounds.expand(v);
	}

	void cCurvePrivate::set_tess(uint _tess)
	{
		if (tess == _tess)
			return;
		tess = clamp(_tess, 1U, 64U);

		curve.tess = tess;
		update_curve();

		node->mark_transform_dirty();
		data_changed("tess"_h);
	}

	void cCurvePrivate::set_curveness(float _curveness)
	{
		if (curveness == _curveness)
			return;
		curveness = clamp(_curveness, 0.f, 1.f);

		curve.t = curveness;
		update_curve();

		node->mark_transform_dirty();
		data_changed("curveness"_h);
	}

	void cCurvePrivate::set_ctrl_points(const std::vector<vec3>& points)
	{
		ctrl_points = points;

		curve.ctrl_points = ctrl_points;
		update_curve();

		node->mark_transform_dirty();
		data_changed("ctrl_points"_h);
	}

	struct cCurveCreate : cCurve::Create
	{
		cCurvePtr operator()(EntityPtr e) override
		{
			return new cCurvePrivate();
		}
	}cCurve_create;
	cCurve::Create& cCurve::create = cCurve_create;
}

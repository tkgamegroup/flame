#pragma once

#include <flame/universe/components/element.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct sElementRendererPrivate;

	struct cElementPrivate : cElement
	{
		sElementRendererPrivate* _renderer = nullptr;

		float _x = 0.f;
		float _y = 0.f;
		float _width = 0.f;
		float _height = 0.f;
		float _pivotx = 0.f;
		float _pivoty = 0.f;
		float _scalex = 1.f;
		float _scaley = 1.f;
		float _rotation = 0.f;
		float _skewx = 0.f;
		float _skewy = 0.f;

		Mat<3, 2, float> _transform = Mat<3, 2, float>(1.f);
		bool _transform_dirty = true;
		Vec2f _p00, _p10, _p11, _p01;

		Vec3c _fill_color = Vec3c(255);

		void _set_x(float x);
		void _set_y(float y);
		void _set_width(float w);
		void _set_height(float h);
		void _set_pivotx(float p);
		void _set_pivoty(float p);
		void _set_scalex(float s);
		void _set_scaley(float s);
		void _set_rotation(float r);
		void _set_skewx(float s);
		void _set_skewy(float s);

		void _update_transform();
		const Mat<3, 2, float>& _get_transform();
		const Vec2f& _get_p00();
		const Vec2f& _get_p10();
		const Vec2f& _get_p11();
		const Vec2f& _get_p01();

		void _set_fill_color(const Vec3c& c);

		void _on_entered_world();

		void _draw(graphics::Canvas* canvas);

		static cElementPrivate* _create();

		float get_x() const override { return _x; }
		void set_x(float x) { _set_x(x); }

		float get_y() const override { return _y; }
		void set_y(float y) { _set_y(y); }

		float get_width() const override { return _width; }
		void set_width(float w) { _set_width(w); }

		float get_height() const override { return _height; }
		void set_height(float h) { _set_height(h); }

		float get_pivotx() const override { return _pivotx; }
		void set_pivotx(float p) override { _set_pivotx(p); }

		float get_pivoty() const override { return _pivoty; }
		void set_pivoty(float p) override { _set_pivoty(p); }

		float get_scalex() const override { return _scalex; }
		void set_scalex(float s) override { _set_scalex(s); }

		float get_scaley() const override { return _scaley; }
		void set_scaley(float s) override { _set_scaley(s); }

		float get_rotation() const override { return _rotation; }
		void set_rotation(float r) override { _set_rotation(r); }

		float get_skewx() const override { return _skewx; }
		void set_skewx(float s) override { _set_skewx(s); }

		float get_skewy() const override { return _skewy; }
		void set_skewy(float s) override { _set_skewy(s); }

		Vec2f get_p00() override { return _get_p00(); }
		Vec2f get_p10() override { return _get_p10(); }
		Vec2f get_p11() override { return _get_p11(); }
		Vec2f get_p01() override { return _get_p01(); }

		Vec3c get_fill_color() override { return _fill_color; }
		void set_fill_color(const Vec3c& c) override { _set_fill_color(c); }

		void on_entered_world() override { _on_entered_world(); }
	};
}

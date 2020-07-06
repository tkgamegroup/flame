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

		Vec4f _g = Vec4f(0.f);
		bool _dirty = true;

		//cElementPrivate();
		//~cElementPrivate();

		void _set_x(float x);
		void _set_y(float y);
		void _set_width(float w);
		void _set_height(float h);

		Vec4f _get_g();

		void _on_entered_world();

		static cElementPrivate* _create();

		float get_x() const override { return _x; }
		void set_x(float x) { _set_x(x); }
		float get_y() const override { return _y; }
		void set_y(float y) { _set_y(y); }
		float get_width() const override { return _width; }
		void set_width(float w) { _set_width(w); }
		float get_height() const override { return _height; }
		void set_height(float h) { _set_height(h); }

		//void calc_geometry();
		void _draw(graphics::Canvas* canvas);
		void on_entered_world() override { _on_entered_world(); }
	};
}

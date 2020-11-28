#pragma once

#include <flame/universe/components/element.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct sRendererPrivate;
	struct sLayoutSystemPrivate;

	struct cElementPrivate : cElement // R ~ on_*
	{
		vec2 pos = vec2(0.f);
		vec2 size = vec2(0.f);
		vec4 padding = vec4(0.f);
		vec2 content_size = vec2(0.f);
		vec2 pivot = vec2(0.f);
		vec2 scaling = vec2(1.f);
		float rotation = 0.f;
		float skewx = 0.f;
		float skewy = 0.f;

		bool transform_dirty = true;
		bool crooked = false;
		vec2 points[8];
		mat3 axes;
		mat3 transform;
		Rect aabb;

		cvec4 fill_color = cvec4(0);

		float border = 0.f;
		cvec4 border_color = cvec4(0, 0, 0, 255);

		bool clipping = false;

		vec4 boundaries;
		bool culled = false;

		std::vector<std::pair<Component*, void(*)(Component*, graphics::Canvas*)>> drawers[2];

		bool pending_sizing = false;
		std::vector<std::pair<Component*, void(*)(Component*, vec2&)>> measurables;

		sRendererPrivate* renderer = nullptr; // R ref
		sLayoutSystemPrivate* layout_system = nullptr; // R ref

		float get_x() const override { return pos.x; }
		void set_x(float x) override;

		float get_y() const override { return pos.y; }
		void set_y(float y) override;

		float get_width() const override { return size.x; }
		void set_width(float w) override;

		float get_height() const override { return size.y; }
		void set_height(float h) override;

		vec4 get_padding() const override { return padding; }
		void set_padding(const vec4& p) override;

		float get_pivotx() const override { return pivot.x; }
		void set_pivotx(float p) override;

		float get_pivoty() const override { return pivot.y; }
		void set_pivoty(float p) override;

		float get_scalex() const override { return scaling.x; }
		void set_scalex(float s) override;

		float get_scaley() const override { return scaling.y; }
		void set_scaley(float s) override;

		float get_rotation() const override { return rotation; }
		void set_rotation(float r) override;

		float get_skewx() const override { return skewx; }
		void set_skewx(float s) override;

		float get_skewy() const override { return skewy; }
		void set_skewy(float s) override;

		void update_transform();

		cvec4 get_fill_color() override { return fill_color; }
		void set_fill_color(const cvec4& c) override;

		float get_border() override { return border; }
		void set_border(float b) override;

		cvec4 get_border_color() override { return border_color; }
		void set_border_color(const cvec4& c) override;

		bool get_clipping() const override { return clipping; }
		void set_clipping(bool c) override;

		void mark_transform_dirty();
		void mark_drawing_dirty();
		void mark_size_dirty();

		void on_gain_renderer();
		void on_lost_renderer();
		void on_gain_layout_system();

		bool contains(const vec2& p) override;

		void on_local_message(Message msg, void* p) override;

		void draw(graphics::Canvas* canvas);
	};
}

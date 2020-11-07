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
		Vec2f pos = Vec2f(0.f);
		Vec2f size = Vec2f(0.f);
		Vec4f padding = Vec4f(0.f);
		Vec2f content_size = Vec2f(0.f);
		Vec2f pivot = Vec2f(0.f);
		Vec2f scale = Vec2f(1.f);
		float rotation = 0.f;
		Vec2f skew = Vec2f(0.f);

		bool transform_dirty = true;
		bool crooked = false;
		Vec2f global_points[8];
		Mat2f global_axes;
		Vec4f aabb;

		Vec4c fill_color = Vec4c(0);

		float border = 0.f;
		Vec4c border_color = Vec4c(0, 0, 0, 255);

		bool clipping = false;

		Vec4f boundaries;
		bool culled = false;

		std::vector<std::pair<Component*, void(*)(Component*, graphics::Canvas*)>> drawers[2];

		bool pending_sizing = false;
		std::vector<std::pair<Component*, void(*)(Component*, Vec2f&)>> measurables;

		sRendererPrivate* renderer = nullptr; // R ref
		sLayoutSystemPrivate* layout_system = nullptr; // R ref

		float get_x() const override { return pos.x(); }
		void set_x(float x) override;

		float get_y() const override { return pos.y(); }
		void set_y(float y) override;

		float get_width() const override { return size.x(); }
		void set_width(float w) override;

		float get_height() const override { return size.y(); }
		void set_height(float h) override;

		Vec4f get_padding() override { return padding; }
		void set_padding(const Vec4f& p) override;

		float get_pivotx() const override { return pivot.x(); }
		void set_pivotx(float p) override;

		float get_pivoty() const override { return pivot.y(); }
		void set_pivoty(float p) override;

		float get_scalex() const override { return scale.x(); }
		void set_scalex(float s) override;

		float get_scaley() const override { return scale.y(); }
		void set_scaley(float s) override;

		float get_rotation() const override { return rotation; }
		void set_rotation(float r) override;

		float get_skewx() const override { return skew.x(); }
		void set_skewx(float s) override;

		float get_skewy() const override { return skew.y(); }
		void set_skewy(float s) override;

		void update_transform();

		Vec4c get_fill_color() override { return fill_color; }
		void set_fill_color(const Vec4c& c) override;

		float get_border() override { return border; }
		void set_border(float b) override;

		Vec4c get_border_color() override { return border_color; }
		void set_border_color(const Vec4c& c) override;

		bool get_clipping() const override { return clipping; }
		void set_clipping(bool c) override;

		void mark_transform_dirty();
		void mark_drawing_dirty();
		void mark_size_dirty();

		void on_gain_renderer();
		void on_lost_renderer();
		void on_gain_layout_system();

		bool contains(const Vec2f& p) override;

		void on_local_message(Message msg, void* p) override;

		void draw(graphics::Canvas* canvas);
	};
}

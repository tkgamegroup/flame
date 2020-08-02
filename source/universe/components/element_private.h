#pragma once

#include <flame/universe/components/element.h>

namespace flame
{
	struct sElementRendererPrivate;

	struct cElementPrivate : cElement // R ~ on_*
	{
		sElementRendererPrivate* renderer = nullptr; // R ref

		float x = 0.f;
		float y = 0.f;
		float width = 0.f;
		float height = 0.f;
		Vec4f padding = Vec4f(0.f);
		float pivotx = 0.f;
		float pivoty = 0.f;
		float scalex = 1.f;
		float scaley = 1.f;
		float rotation = 0.f;
		float skewx = 0.f;
		float skewy = 0.f;

		Mat23f transform = Mat23f(1.f);
		bool transform_dirty = true;
		Vec2f points[8];

		Vec4c fill_color = Vec4c(255);

		float border = 0.f;
		Vec4c border_color = Vec4c(0, 0, 0, 255);

		bool clipping = false;
		bool culled = false;

		std::vector<Drawer*> drawers;

		float get_x() const override { return x; }
		void set_x(float x) override;

		float get_y() const override { return y; }
		void set_y(float y) override;

		float get_width() const override { return width; }
		void set_width(float w) override;

		float get_height() const override { return height; }
		void set_height(float h) override;

		Vec4f get_padding() override { return padding; }
		void set_padding(const Vec4f& p) override;

		float get_pivotx() const override { return pivotx; }
		void set_pivotx(float p) override;

		float get_pivoty() const override { return pivoty; }
		void set_pivoty(float p) override;

		float get_scalex() const override { return scalex; }
		void set_scalex(float s) override;

		float get_scaley() const override { return scaley; }
		void set_scaley(float s) override;

		float get_rotation() const override { return rotation; }
		void set_rotation(float r) override;

		float get_skewx() const override { return skewx; }
		void set_skewx(float s) override;

		float get_skewy() const override { return skewy; }
		void set_skewy(float s) override;

		void update_transform();
		const Mat23f& get_transform();
		Vec2f get_point(uint idx) override;

		Vec4c get_fill_color() override { return fill_color; }
		void set_fill_color(const Vec4c& c) override;

		float get_border() override { return border; }
		void set_border(float b) override;

		Vec4c get_border_color() override { return border_color; }
		void set_border_color(const Vec4c& c) override;

		bool get_clipping() const override { return clipping; }
		void set_clipping(bool c) override;

		bool contains(const Vec2f& p) override;

		void on_gain_renderer();
		void on_lost_renderer();

		void on_entity_visibility_changed() override;
		void on_entity_message(Message msg) override;
		void on_entity_position_changed() override;

		void draw_background(graphics::Canvas* canvas);
		void draw_content(graphics::Canvas* canvas);
	};
}

#pragma once

#include <flame/universe/components/element.h>

namespace flame
{
	struct sElementRendererPrivate;

	struct cElementPrivate : cElement
	{
		sElementRendererPrivate* renderer = nullptr;

		float x = 0.f;
		float y = 0.f;
		float width = 0.f;
		float height = 0.f;
		float pivotx = 0.f;
		float pivoty = 0.f;
		float scalex = 1.f;
		float scaley = 1.f;
		float rotation = 0.f;
		float skewx = 0.f;
		float skewy = 0.f;

		Mat<3, 2, float> transform = Mat<3, 2, float>(1.f);
		bool transform_dirty = true;
		Vec2f p00, p10, p11, p01;

		Vec3c fill_color = Vec3c(255);

		std::vector<Drawer*> drawers;

		float get_x() const override { return x; }
		void set_x(float x) override;

		float get_y() const override { return y; }
		void set_y(float y) override;

		float get_width() const override { return width; }
		void set_width(float w) override;

		float get_height() const override { return height; }
		void set_height(float h) override;

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
		const Mat<3, 2, float>& get_transform();
		Vec2f get_p00() override;
		Vec2f get_p10() override;
		Vec2f get_p11() override;
		Vec2f get_p01() override;

		Vec3c get_fill_color() override { return fill_color; }
		void set_fill_color(const Vec3c& c) override;

		void on_entered_world() override;
		void on_left_world() override;
		void on_entity_visibility_changed() override;
		void on_entity_position_changed() override;

		void mark_transform_dirty() override;
		void mark_drawing_dirty() override;

		void draw(graphics::Canvas* canvas);

		static cElementPrivate* create();
	};
}

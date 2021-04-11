#pragma once

#include <flame/universe/components/element.h>

namespace flame
{
	struct sRendererPrivate;
	struct sLayoutPrivate;

	struct cElementPrivate : cElement
	{
		vec2 pos = vec2(0.f);
		vec2 size = vec2(0.f);
		vec4 padding = vec4(0.f);
		vec2 padding_size = vec2(0.f);
		vec2 content_size = vec2(0.f);
		vec4 margin = vec4(0.f);
		vec2 margin_size = vec2(0.f);
		vec2 pivot = vec2(0.f);
		vec2 scl = vec2(1.f);
		float angle = 0.f;
		vec2 skew = vec2(0.f);

		bool align_in_layout = true;
		bool align_absolute = false;
		Align alignx = AlignNone;
		Align aligny = AlignNone;
		float width_factor = 1.f;
		float height_factor = 1.f;
		vec2 desired_size = vec2(0.f);

		bool need_layout = false;
		LayoutType layout_type = LayoutFree;
		float layout_gap = 0.f;
		uint layout_columns = 1;
		bool auto_width = true;
		bool auto_height = true;

		vec2 scroll = vec2(0.f);

		uint layer_policy = 0; // 0: no new layer, 1: new layer, 2: new layer and need populate

		cElementPrivate* pelement = nullptr;
		bool transform_dirty = true;
		bool crooked = false;
		vec2 points[8];
		mat2 axes;
		mat2 axes_inv;
		mat3 transform;
		Rect aabb;

		float alpha = 1.f;

		cvec4 fill_color = cvec4(0);

		float border = 0.f;
		cvec4 border_color = cvec4(0, 0, 0, 255);

		bool clipping = false;

		Rect parent_scissor;
		bool culled = false;

		std::vector<std::unique_ptr<Closure<uint(Capture&, uint, sRenderer*)>>> drawers;
		std::vector<std::unique_ptr<Closure<void(Capture&, vec2*)>>> measurers;
		bool pending_sizing = false;
		bool pending_layout = false;

		sRendererPrivate* renderer = nullptr;
		sLayoutPrivate* layout_system = nullptr;

		float get_x() const override { return pos.x; }
		void set_x(float x) override;

		float get_y() const override { return pos.y; }
		void set_y(float y) override;

		vec2 get_pos() const override { return pos; }
		void set_pos(const vec2& p) override;
		void add_pos(const vec2& p) override;

		float get_width() const override { return size.x; }
		void set_width(float w) override;

		float get_height() const override { return size.y; }
		void set_height(float h) override;

		vec2 get_size() const override { return size; }
		void set_size(const vec2& s) override;
		void add_size(const vec2& s) override;

		vec4 get_padding() const override { return padding; }
		void set_padding(const vec4& p) override;
		void add_padding(const vec4& p) override;

		vec4 get_margin() const override { return margin; }
		void set_margin(const vec4& m) override;

		float get_pivotx() const override { return pivot.x; }
		void set_pivotx(float p) override;

		float get_pivoty() const override { return pivot.y; }
		void set_pivoty(float p) override;

		float get_scalex() const override { return scl.x; }
		void set_scalex(float s) override;

		float get_scaley() const override { return scl.y; }
		void set_scaley(float s) override;

		vec2 get_scale() const override { return scl; }
		void set_scale(const vec2& s) override;

		float get_angle() const override { return angle; }
		void set_angle(float a) override;

		float get_skewx() const override { return skew.x; }
		void set_skewx(float s) override;

		float get_skewy() const override { return skew.y; }
		void set_skewy(float s) override;

		bool get_align_in_layout() const override { return align_in_layout; }
		void set_align_in_layout(bool v) override;

		bool get_align_absolute() const override { return align_absolute; }
		void set_align_absolute(bool a) override;

		Align get_alignx() const override { return alignx; }
		void set_alignx(Align a) override;
		Align get_aligny() const override { return aligny; }
		void set_aligny(Align a) override;

		float get_width_factor() const override { return width_factor; }
		void set_width_factor(float f) override;
		float get_height_factor() const override { return height_factor; }
		void set_height_factor(float f) override;

		LayoutType get_layout_type() const override { return layout_type; }
		void set_layout_type(LayoutType t) override;

		float get_layout_gap() const override { return layout_gap; }
		void set_layout_gap(float g) override;

		bool get_auto_width() const override { return auto_width; }
		void set_auto_width(bool a) override;
		bool get_auto_height() const override { return auto_height; }
		void set_auto_height(bool a) override;

		float get_scrollx() const override { return scroll.x; }
		void set_scrollx(float s) override;
		float get_scrolly() const override { return scroll.y; }
		void set_scrolly(float s) override;

		cvec4 get_fill_color() override { return fill_color; }
		void set_fill_color(const cvec4& c) override;

		float get_border() override { return border; }
		void set_border(float b) override;

		cvec4 get_border_color() override { return border_color; }
		void set_border_color(const cvec4& c) override;

		bool get_clipping() const override { return clipping; }
		void set_clipping(bool c) override;

		bool get_culled() const override { return culled; }

		vec2 get_point(uint idx) const override { return points[idx]; };

		void* add_drawer(uint (*drawer)(Capture&, uint, sRenderer*), const Capture& capture) override;
		void remove_drawer(void* drawer) override;
		void* add_measurer(void (*measurer)(Capture&, vec2*), const Capture& capture) override;
		void remove_measurer(void* measurer) override;

		void update_transform();

		void mark_transform_dirty() override;
		void mark_drawing_dirty() override;
		void mark_size_dirty() override;
		void mark_layout_dirty() override;
		void remove_from_sizing_list();
		void remove_from_layout_list();

		bool contains(const vec2& p) override;

		void on_self_added() override;
		void on_self_removed() override;
		void on_child_added(Entity* e) override;
		void on_child_removed(Entity* e) override;
		void on_entered_world() override;
		void on_left_world() override;
		void on_visibility_changed(bool v) override;
		void on_reposition(uint from, uint to) override;

		bool on_save_attribute(uint h) override;

		void draw(uint layer, sRenderer* renderer);
	};
}

#pragma once

#include "element.h"

namespace flame
{
	struct cElementPrivate : cElement
	{
		vec2 padding_size = vec2(0.f);
		vec2 content_size = vec2(0.f);
		vec2 margin_size = vec2(0.f);

		vec2 desired_size = vec2(0.f);

		bool need_layout = false;
		uint layout_columns = 1;

		uint layer_policy = 0; // 0: no new layer, 1: new layer, 2: new layer and need populate

		cElementPrivate* pelement = nullptr;
		bool transform_dirty = true;
		bool crooked = false;
		mat2 axes;
		mat2 axes_inv;
		mat3 transform;
		Rect bounds;

		Rect parent_scissor;

		sScenePrivate* s_scene = nullptr;
		sRendererPrivate* s_renderer = nullptr;
		bool pending_sizing = false;
		bool pending_layout = false;

		void set_pos(const vec2& p) override;
		void set_size(const vec2& s) override;
		void set_padding(const vec4& p) override;
		void set_margin(const vec4& m) override;
		void set_pivot(const vec2& p) override;
		void set_scale(const vec2& s) override;
		void set_angle(float a) override;
		void set_skew(const vec2& s) override;

		void set_align_in_layout(bool v) override;
		void set_align_absolute(bool a) override;

		void set_alignx(Align a) override;
		void set_aligny(Align a) override;

		void set_width_factor(float f) override;
		void set_height_factor(float f) override;

		void set_layout_type(LayoutType t) override;
		void set_layout_gap(float g) override;
		void set_layout_column(uint c) override;

		void set_auto_width(bool a) override;
		void set_auto_height(bool a) override;

		void set_scroll(const vec2& s) override;

		void set_alpha(float a) override;
		void set_fill_color(const cvec4& c) override;

		void set_border(float b) override;
		void set_border_color(const cvec4& c) override;
		void set_roundness(float r) override;

		void set_enable_clipping(bool c) override;

		void update_transform();

		void mark_transform_dirty() override;
		void mark_drawing_dirty() override;
		void mark_size_dirty() override;
		void mark_layout_dirty() override;
		void remove_from_sizing_list();
		void remove_from_layout_list();

		bool contains(const vec2& p) override;

		void on_component_added(Component* c) override;
		void on_component_removed(Component* c) override;
		void on_child_added(EntityPtr e) override;
		void on_child_removed(EntityPtr e) override;
		void on_entered_world() override;
		void on_left_world() override;
		void on_visibility_changed(bool v) override;
		void on_reposition(uint from, uint to) override;

		bool draw(uint layer);
	};
}

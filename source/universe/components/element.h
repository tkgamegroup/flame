#pragma once

#include "../component.h"

namespace flame
{
	struct cElement : Component
	{
		inline static auto type_name = "flame::cElement";
		inline static auto type_hash = ch(type_name);

		vec2 pos = vec2(0.f);
		vec2 size = vec2(0.f);
		vec4 padding = vec4(0.f);
		vec4 margin = vec4(0.f);
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

		LayoutType layout_type = LayoutFree;
		float layout_gap = 0.f;

		bool auto_width = true;
		bool auto_height = true;

		vec2 scroll = vec2(0.f);

		float alpha = 1.f;
		cvec4 fill_color = cvec4(0);
		float border = 0.f;
		cvec4 border_color = cvec4(255);

		bool enable_clipping = false;

		bool culled = false;
		vec2 points[8];

		Listeners<uint(uint, sRendererPtr)> drawers;
		Listeners<uint(vec2*)> measurers;

		cElement() :
			Component(type_name, type_hash)
		{
		}

		virtual void set_pos(const vec2& p) = 0;
		virtual void set_size(const vec2& s) = 0;
		virtual void set_padding(const vec4& p) = 0;
		virtual void set_margin(const vec4& m) = 0;
		virtual void set_pivot(const vec2& p) = 0;
		virtual void set_scale(const vec2& s) = 0;
		virtual void set_angle(float angle) = 0;
		virtual void set_skew(const vec2& s) = 0;

		virtual void set_align_in_layout(bool v) = 0;
		virtual void set_align_absolute(bool a) = 0;

		virtual void set_alignx(Align a) = 0;
		virtual void set_aligny(Align a) = 0;

		virtual void set_width_factor(float f) = 0;
		virtual void set_height_factor(float f) = 0;

		virtual void set_layout_type(LayoutType t) = 0;
		virtual void set_layout_gap(float g) = 0;
		virtual void set_layout_column(uint c) = 0;

		virtual void set_auto_width(bool a) = 0;
		virtual void set_auto_height(bool a) = 0;

		virtual void set_scroll(const vec2& s) = 0;

		virtual void set_alpha(float a) = 0;
		virtual void set_fill_color(const cvec4& c) = 0;

		virtual void set_border(float b) = 0;
		virtual void set_border_color(const cvec4& c) = 0;
		virtual void set_roundness(float r) = 0;

		virtual void set_enable_clipping(bool c) = 0;

		virtual bool contains(const vec2& p) = 0;

		virtual void mark_transform_dirty() = 0;
		virtual void mark_drawing_dirty() = 0;
		virtual void mark_size_dirty() = 0;
		virtual void mark_layout_dirty() = 0;

		FLAME_UNIVERSE_EXPORTS static cElement* create();
	};
}

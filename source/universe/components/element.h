#pragma once

#include "../component.h"

namespace flame
{
	struct ElementDrawer
	{
		virtual uint draw(uint, sRendererPtr) = 0;
	};

	struct ElementMeasurer
	{
		virtual bool measure(vec2*) = 0;
	};

	struct cElement : Component
	{
		inline static auto type_name = "flame::cElement";
		inline static auto type_hash = ch(type_name);

		cElement() :
			Component(type_name, type_hash)
		{
		}

		virtual float get_x() const = 0;
		virtual void set_x(float x) = 0;

		virtual float get_y() const = 0;
		virtual void set_y(float y) = 0;

		virtual vec2 get_pos() const = 0;
		virtual void set_pos(const vec2& p) = 0;
		virtual void add_pos(const vec2& p) = 0;

		virtual float get_width() const = 0;
		virtual void set_width(float w) = 0;

		virtual float get_height() const = 0;
		virtual void set_height(float h) = 0;

		virtual vec2 get_size() const = 0;
		virtual void set_size(const vec2& s) = 0;
		virtual void add_size(const vec2& s) = 0;

		virtual vec4 get_padding() const = 0;
		virtual void set_padding(const vec4& p) = 0;
		virtual void add_padding(const vec4& p) = 0;

		virtual vec4 get_margin() const = 0;
		virtual void set_margin(const vec4& m) = 0;

		virtual float get_pivotx() const = 0;
		virtual void set_pivotx(float p) = 0;

		virtual float get_pivoty() const = 0;
		virtual void set_pivoty(float p) = 0;

		virtual float get_scalex() const = 0;
		virtual void set_scalex(float s) = 0;

		virtual float get_scaley() const = 0;
		virtual void set_scaley(float s) = 0;

		virtual vec2 get_scale() const = 0;
		virtual void set_scale(const vec2& s) = 0;

		virtual float get_angle() const = 0;
		virtual void set_angle(float angle) = 0;

		virtual float get_skewx() const = 0;
		virtual void set_skewx(float angle) = 0;

		virtual float get_skewy() const = 0;
		virtual void set_skewy(float angle) = 0;

		virtual bool get_align_in_layout() const = 0;
		virtual void set_align_in_layout(bool v) = 0;

		virtual bool get_align_absolute() const = 0;
		virtual void set_align_absolute(bool a) = 0;

		virtual Align get_alignx() const = 0;
		virtual void set_alignx(Align a) = 0;
		virtual Align get_aligny() const = 0;
		virtual void set_aligny(Align a) = 0;

		virtual float get_width_factor() const = 0;
		virtual void set_width_factor(float f) = 0;
		virtual float get_height_factor() const = 0;
		virtual void set_height_factor(float f) = 0;

		virtual LayoutType get_layout_type() const = 0;
		virtual void set_layout_type(LayoutType t) = 0;

		virtual float get_layout_gap() const = 0;
		virtual void set_layout_gap(float g) = 0;

		virtual bool get_auto_width() const = 0;
		virtual void set_auto_width(bool a) = 0;
		virtual bool get_auto_height() const = 0;
		virtual void set_auto_height(bool a) = 0;

		virtual float get_scrollx() const = 0;
		virtual void set_scrollx(float s) = 0;
		virtual float get_scrolly() const = 0;
		virtual void set_scrolly(float s) = 0;

		//virtual uint get_layout_column() const = 0;
		//virtual void set_layout_column(uint c) = 0;

		//virtual float get_alpha() const = 0;
		//virtual void set_alpha(float a) = 0;

		virtual cvec4 get_fill_color() = 0;
		virtual void set_fill_color(const cvec4& c) = 0;

		virtual float get_border() = 0;
		virtual void set_border(float b) = 0;
		virtual cvec4 get_border_color() = 0;
		virtual void set_border_color(const cvec4& c) = 0;

		//virtual float get_roughness() const = 0;
		//virtual void set_roughness(float r) = 0;

		virtual bool get_clipping() const = 0;
		virtual void set_clipping(bool c) = 0;

		virtual void add_drawer(ElementDrawer* d) = 0;
		virtual void remove_drawer(ElementDrawer* d) = 0;
		virtual void add_measurer(ElementMeasurer* m) = 0;
		virtual void remove_measurer(ElementMeasurer* m) = 0;

		virtual bool get_culled() const = 0;

		virtual vec2 get_point(uint idx) const = 0;

		virtual bool contains(const vec2& p) = 0;

		virtual void mark_transform_dirty() = 0;
		virtual void mark_drawing_dirty() = 0;
		virtual void mark_size_dirty() = 0;
		virtual void mark_layout_dirty() = 0;

		FLAME_UNIVERSE_EXPORTS static cElement* create(void* parms = nullptr);
	};
}

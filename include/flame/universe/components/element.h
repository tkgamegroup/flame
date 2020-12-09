#pragma once

#include <flame/universe/component.h>

namespace flame
{
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

		virtual float get_width() const = 0;
		virtual void set_width(float w) = 0;

		virtual float get_height() const = 0;
		virtual void set_height(float h) = 0;

		virtual vec4 get_padding() const = 0;
		virtual void set_padding(const vec4& p) = 0;

		// 0 - 1
		virtual float get_pivotx() const = 0;
		// 0 - 1
		virtual void set_pivotx(float p) = 0;

		// 0 - 1
		virtual float get_pivoty() const = 0;
		// 0 - 1
		virtual void set_pivoty(float p) = 0;

		virtual float get_scalex() const = 0;
		virtual void set_scalex(float s) = 0;

		virtual float get_scaley() const = 0;
		virtual void set_scaley(float s) = 0;

		virtual float get_angle() const = 0;
		virtual void set_angle(float angle) = 0;

		virtual float get_skewx() const = 0;
		virtual void set_skewx(float angle) = 0;

		virtual float get_skewy() const = 0;
		virtual void set_skewy(float angle) = 0;

		//virtual float get_alpha() const = 0;
		//virtual void set_alpha(float a) = 0;

		virtual cvec4 get_fill_color() = 0;
		virtual void set_fill_color(const cvec4& c) = 0;

		virtual float get_border() = 0;
		virtual void set_border(float b) = 0;
		virtual cvec4 get_border_color() = 0;
		virtual void set_border_color(const cvec4& c) = 0;

		virtual bool get_clipping() const = 0;
		virtual void set_clipping(bool c) = 0;

		virtual bool contains(const vec2& p) = 0;

		//FLAME_RV(vec4, roundness);
		//FLAME_RV(uint, roundness_lod);

		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_roundness)(const vec4& r);

		virtual void mark_transform_dirty() = 0;
		virtual void mark_drawing_dirty() = 0;
		virtual void mark_size_dirty() = 0;

		FLAME_UNIVERSE_EXPORTS static cElement* create();
	};
}

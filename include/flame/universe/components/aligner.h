#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cAligner : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cAligner";
		inline static auto type_hash = ch(type_name);

		cAligner() :
			Component(type_name, type_hash)
		{
		}

		virtual Align get_alignx() const = 0;
		virtual void set_alignx(Align a) = 0;
		virtual Align get_aligny() const = 0;
		virtual void set_aligny(Align a) = 0;

		virtual float get_width_factor() const = 0;
		virtual void set_width_factor(float f) = 0;
		virtual float get_height_factor() const = 0;
		virtual void set_height_factor(float f) = 0;

		// no padding
		virtual bool get_absolute() const = 0;
		// no padding
		virtual void set_absolute(bool a) = 0;

		// L T R B
		virtual Vec4f get_margin() const = 0;
		// L T R B
		virtual void set_margin(const Vec4f& m) = 0;

		virtual bool get_include_in_layout() const = 0;
		virtual void set_include_in_layout(bool o) = 0;

		FLAME_UNIVERSE_EXPORTS static cAligner* create();
	};
}

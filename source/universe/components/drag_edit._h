#pragma once

#include "../../foundation/typeinfo.h"
#include "../component.h"

namespace flame
{
	struct cDragEdit : Component
	{
		inline static auto type_name = "flame::cDragEdit";
		inline static auto type_hash = ch(type_name);

		cDragEdit() : Component(type_name, type_hash)
		{
		}

		virtual BasicType get_type() const = 0;
		virtual void set_type(BasicType t) = 0;

		virtual int get_int() const = 0;
		virtual void set_int(int i) = 0;

		virtual float get_float() const = 0;
		virtual void set_float(float f) = 0;

		virtual float get_min() const = 0;
		virtual void set_min(float v) = 0;
		virtual float get_max() const = 0;
		virtual void set_max(float v) = 0;

		virtual float get_speed() const = 0;
		virtual void set_speed(float v) = 0;

		FLAME_UNIVERSE_EXPORTS static cDragEdit* create(void* parms = nullptr);
	};
}

#pragma once

#include "../../foundation/typeinfo.h"
#include "../driver.h"

namespace flame
{
	struct dDragEdit : Driver
	{
		inline static auto type_name = "flame::dDragEdit";
		inline static auto type_hash = ch(type_name);

		dDragEdit() :
			Driver(type_name, type_hash)
		{
		}

		virtual BasicType get_type() const = 0;
		virtual void set_type(BasicType t) = 0;

		virtual int get_int() const = 0;
		virtual void set_int(int i) = 0;

		virtual float get_float() const = 0;
		virtual void set_float(float f) = 0;

		FLAME_UNIVERSE_EXPORTS static dDragEdit* create(void* parms = nullptr);
	};
}

#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct FLAME_RU(cLayout : Component, all)
	{
		inline static auto type_name = "cLayout";
		inline static auto type_hash = ch(type_name);

		cLayout() :
			Component(type_name, type_hash, true, true, true, true)
		{
		}

		virtual LayoutType get_type() const = 0;
		virtual void set_type(LayoutType t) = 0;

		//FLAME_RV(uint, column);
		//FLAME_RV(float, item_padding);
		//FLAME_RV(bool, width_fit_children);
		//FLAME_RV(bool, height_fit_children);
		//FLAME_RV(int, fence);

		//Vec2f scroll_offset;
		//Vec2f content_size;

		virtual void mark_dirty() = 0;

		//FLAME_UNIVERSE_EXPORTS void set_x_scroll_offset(float x);
		//FLAME_UNIVERSE_EXPORTS void set_y_scroll_offset(float y);
		//FLAME_UNIVERSE_EXPORTS void set_column(uint c);

		FLAME_UNIVERSE_EXPORTS static cLayout* create();
	};
}

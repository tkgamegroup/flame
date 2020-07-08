#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct FLAME_R(cLayout : Component, all)
	{
		inline static auto type_name = "cLayout";
		inline static auto type_hash = ch(type_name);

		cLayout() :
			Component(type_name, type_hash)
		{
		}

		//sLayoutManagement* management;

		//cElement* element;
		//cAligner* aligner;

		//FLAME_RV(uint, column);
		//FLAME_RV(float, item_padding);
		//FLAME_RV(bool, width_fit_children);
		//FLAME_RV(bool, height_fit_children);
		//FLAME_RV(int, fence);

		//Vec2f scroll_offset;
		//Vec2f content_size;

		//void mark_dirty()
		//{
		//	if (management)
		//		management->add_to_update_list(this);
		//}

		//FLAME_UNIVERSE_EXPORTS void set_x_scroll_offset(float x);
		//FLAME_UNIVERSE_EXPORTS void set_y_scroll_offset(float y);
		//FLAME_UNIVERSE_EXPORTS void set_column(uint c);

		FLAME_UNIVERSE_EXPORTS static cLayout* create();
	};
}

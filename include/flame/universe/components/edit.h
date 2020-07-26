#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct FLAME_RU(cEdit : Component, all)
	{
		inline static auto type_name = "cEdit";
		inline static auto type_hash = ch(type_name);

		cEdit() :
			Component(type_name, type_hash, true)
		{
		}
//
//		bool select_all_on_dbclicked;
//		bool select_all_on_focus;
//		bool enter_to_throw_focus;
//		bool trigger_changed_on_lost_focus;
//
//		void set_select(uint start, int length = 0)
//		{
//			select_start = start;
//			select_end = start + length;
//		}

		FLAME_UNIVERSE_EXPORTS static cEdit* create();
	};
}

#pragma once

#include "../components/element_private.h"
#include <flame/universe/drivers/edit.h>

namespace flame
{
	struct cTextPrivate;
	struct cReceiverPrivate;

	struct dEditPrivate : dEdit
	{
		uint select_start = 0;
		uint select_end = 0;

		bool select_all_on_dbclicked = true;
		bool select_all_on_focus = true;
		bool enter_to_throw_focus = false;
		bool trigger_changed_on_lost_focus = false;

		cElementPrivate* element = nullptr;
		cReceiverPrivate* receiver = nullptr;
		cTextPrivate* text = nullptr;

		void* flash_event = nullptr;

		bool show_cursor = false;
		bool changed = false;

		float scroll = 0.f;

		void mark_changed();
		void flash_cursor(int mode);
		int locate_cursor(const vec2& mpos);
		
		void on_load_finished() override;
		
		void draw(graphics::Canvas* canvas);

		static dEditPrivate* create();
	};
}

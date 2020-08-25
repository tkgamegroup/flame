#pragma once

#include <flame/universe/components/edit.h>
#include "element_private.h"

namespace flame
{
	struct cTextPrivate;
	struct cEventReceiverPrivate;

	struct cEditPrivate : cEdit // R ~ on_*
	{
		uint select_start = 0;
		uint select_end = 0;

		bool select_all_on_dbclicked = true;
		bool select_all_on_focus = true;
		bool enter_to_throw_focus = false;
		bool trigger_changed_on_lost_focus = false;

		cElementPrivate* element = nullptr; // R ref
		cTextPrivate* text = nullptr; // R ref
		cEventReceiverPrivate* event_receiver = nullptr; // R ref

		void* key_down_listener = nullptr;
		void* char_listener = nullptr;
		void* mouse_down_listener = nullptr;
		void* mouse_move_listener = nullptr;
		void* mouse_dbclick_listener = nullptr;
		void* flash_event = nullptr;

		bool show_cursor = false;
		bool changed = false;

		float scroll = 0.f;

		void mark_changed();
		void flash_cursor(int mode);
		int locate_cursor(const Vec2f& mpos);
		
		void on_gain_event_receiver();
		void on_lost_event_receiver();

		void on_local_message(Message msg, void* p) override;

		void draw(graphics::Canvas* canvas); // R

		static cEditPrivate* create();
	};
}

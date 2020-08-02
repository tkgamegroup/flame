#pragma once

#include <flame/universe/components/edit.h>
#include "element_private.h"

namespace flame
{
	struct cTextPrivate;
	struct cEventReceiverPrivate;

	struct cEditPrivate : cEdit, cElement::Drawer // R ~ on_*
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

		void* key_listener = nullptr;
		void* mouse_listener = nullptr;
		void* focus_listener = nullptr;
		void* flash_event = nullptr;

		bool show_cursor = false;
		bool changed = false;

		float scroll = 0.f;

		void flash_cursor(int mode);
		int locate_cursor(const Vec2f& mpos);
		
		void on_gain_element();
		void on_lost_element();
		void on_gain_event_receiver();
		void on_lost_event_receiver();

		void on_entity_left_world() override;
		void on_entity_visibility_changed() override;
		void on_entity_state_changed() override;

		void draw(graphics::Canvas* canvas) override;

		static cEditPrivate* create();
	};
}

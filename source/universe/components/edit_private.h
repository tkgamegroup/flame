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

		cElementPrivate* element = nullptr;
		cTextPrivate* text = nullptr;
		cEventReceiverPrivate* event_receiver = nullptr;

		void* key_listener = nullptr;
		void* mouse_listener = nullptr;
		void* focus_listener = nullptr;
		void* flash_event = nullptr;

		bool show_cursor = false;
		bool changed = false;

		float scroll = 0.f;

		void flash_cursor(int mode);
		int locate_cursor(const Vec2f& mpos);
		
		void on_added() override;
		void on_removed() override;
		void on_entity_left_world() override;
		void on_entity_visibility_changed() override;
		void on_entity_state_changed() override;

		void draw(graphics::Canvas* canvas) override;

		static cEditPrivate* create();
	};
}

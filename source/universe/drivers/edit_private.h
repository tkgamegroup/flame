#pragma once

#include "../components/element_private.h"
#include "edit.h"

namespace flame
{
	struct dEditPrivate : dEdit, ElementDrawer
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

		uint get_select_start() const override { return select_start; }
		void set_select_start(uint s) override;
		uint get_select_end() const override { return select_end; }
		void set_select_end(uint s) override;

		bool get_select_all_on_dbclicked() const override { return select_all_on_dbclicked; }
		void set_select_all_on_dbclicked(bool b) override;
		bool get_select_all_on_focus() const override { return select_all_on_focus; }
		void set_select_all_on_focus(bool b) override;
		bool get_enter_to_throw_focus() const override { return enter_to_throw_focus; }
		void set_enter_to_throw_focus(bool b) override;
		bool get_trigger_changed_on_lost_focus() const override { return trigger_changed_on_lost_focus; }
		void set_trigger_changed_on_lost_focus(bool b) override;
		
		void on_load_finished() override;
		
		uint draw(uint layer, sRendererPtr s_renderer) override;

		static dEditPrivate* create();
	};
}

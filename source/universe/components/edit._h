#pragma once

#include "../component.h"

namespace flame
{
	struct cEdit : Component
	{
		inline static auto type_name = "flame::cEdit";
		inline static auto type_hash = ch(type_name);

		cEdit() : Component(type_name, type_hash)
		{
		}

		virtual uint get_select_start() const = 0;
		virtual void set_select_start(uint v) = 0;

		virtual uint get_select_end() const = 0;
		virtual void set_select_end(uint v) = 0;

		virtual bool get_select_all_on_dbclicked() const = 0;
		virtual void set_select_all_on_dbclicked(bool v) = 0;

		virtual bool get_select_all_on_focus() const = 0;
		virtual void set_select_all_on_focus(bool v) = 0;

		virtual bool get_enter_to_throw_focus() const = 0;
		virtual void set_enter_to_throw_focus(bool v) = 0;

		virtual bool get_trigger_changed_on_lost_focus() const = 0;
		virtual void set_trigger_changed_on_lost_focus(bool v) = 0;

		FLAME_UNIVERSE_EXPORTS static cEdit* create(void* parms = nullptr);
	};
}

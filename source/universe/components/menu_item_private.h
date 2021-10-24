#pragma once

#include "menu_item.h"

namespace flame
{
	struct cMenuItemPrivate : cMenuItem
	{
		bool checkable = false;
		bool checked = false;
		
		cElementPrivate* element;
		cReceiverPrivate* receiver;
		cTextPrivate* text;
		EntityPrivate* arrow;
		
		bool get_checkable() const override { return checkable; }
		void set_checkable(bool v) override;
		bool get_checked() const override { return checked; }
		void set_checked(bool v) override;
		void set_radio_checked() override;

		void on_entered_world() override;
	};
}

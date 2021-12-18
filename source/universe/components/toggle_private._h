#pragma once

#include "toggle.h"

namespace flame
{
	struct cTogglePrivate : cToggle
	{
		bool toggled = false;
		
		cReceiverPrivate* receiver;
		EntityPrivate* box;
		
		bool get_toggled() const override { return toggled; }
		void set_toggled(bool v) override;

		void on_load_finished() override;
	};
}

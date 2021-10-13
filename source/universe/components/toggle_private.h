#pragma once

#include "toggle.h"

namespace flame
{
	struct cTogglePrivate : cToggle
	{
		bool toggled = false;
		int test = 0;
		
		cReceiverPrivate* receiver;
		EntityPrivate* box;
		
		bool get_toggled() const override { return toggled; }
		void set_toggled(bool v) override;
	};
}

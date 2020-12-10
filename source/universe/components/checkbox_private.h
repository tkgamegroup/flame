#pragma once

#include "../entity_private.h"
#include <flame/universe/components/checkbox.h>

namespace flame
{
	struct cElementPrivate;
	struct cReceiverPrivate;

	struct cCheckboxPrivate : cCheckbox
	{
		cReceiverPrivate* receiver = nullptr;

		void* click_listener = nullptr;

		bool checked = false;

		bool get_checked() const override { return checked; }
		void set_checked(bool checked) override;

		void on_gain_receiver();
		void on_lost_receiver();
	};
}

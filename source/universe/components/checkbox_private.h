#pragma once

#include "../entity_private.h"
#include <flame/universe/components/checkbox.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;

	struct cCheckboxPrivate : cCheckbox
	{
		cEventReceiverPrivate* event_receiver = nullptr;

		void* click_listener = nullptr;

		bool checked = false;

		bool get_checked() const override { return checked; }
		void set_checked(bool checked) override;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
	};
}

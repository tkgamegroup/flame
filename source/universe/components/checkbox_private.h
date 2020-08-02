#pragma once

#include <flame/universe/components/checkbox.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;

	struct cCheckboxPrivate : cCheckbox  // R ~ on_*
	{
		cEventReceiverPrivate* event_receiver = nullptr; // R ref

		void* mouse_listener = nullptr;

		bool checked = false;

		bool get_checked() const override { return checked; }
		void set_checked(bool checked) override;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
	};
}

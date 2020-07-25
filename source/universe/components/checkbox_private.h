#pragma once

#include <flame/universe/components/checkbox.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;

	struct cCheckboxPrivate : cCheckbox
	{
		cEventReceiverPrivate* event_receiver = nullptr;

		void* mouse_listener = nullptr;

		bool checked = false;

		bool get_checked() const override { return checked; }
		void set_checked(bool checked) override;

		void on_added() override;
		void on_removed() override;
	};
}

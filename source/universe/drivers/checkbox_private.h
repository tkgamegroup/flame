#pragma once

#include "../entity_private.h"
#include <flame/universe/drivers/checkbox.h>

namespace flame
{
	struct cReceiverPrivate;

	struct cCheckboxPrivate : dCheckbox
	{
		cReceiverPrivate* receiver = nullptr;

		bool checked = false;

		bool get_checked() const override { return checked; }
		void set_checked(bool checked) override;

		void on_load_finished() override;
	};
}

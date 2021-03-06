#pragma once

#include "../entity_private.h"
#include "checkbox.h"

namespace flame
{
	struct dCheckboxPrivate : dCheckbox
	{
		cReceiverPrivate* receiver;
		EntityPrivate* box;

		bool checked = false;

		bool get_checked() const override { return checked; }
		void set_checked(bool checked) override;

		void on_load_finished() override;
	};
}

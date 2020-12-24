#pragma once

#include "../entity_private.h"
#include <flame/universe/drivers/combobox.h>

namespace flame
{
	struct cTextPrivate;
	struct cReceiverPrivate;
	struct dMenuPrivate;

	struct dComboboxPrivate : dCombobox
	{
		int index = -1;

		dMenuPrivate* menu = nullptr;
		cTextPrivate* text = nullptr;

		int get_index() const override { return index; }
		void set_index(int index) override;
	};

	struct dComboboxItemPrivate : dComboboxItem
	{
		dComboboxPrivate* combobox = nullptr;

		cReceiverPrivate* receiver = nullptr;

		void on_gain_receiver();
	};
}

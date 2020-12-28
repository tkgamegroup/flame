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

		void on_load_finished() override;
		bool on_child_added(Entity* e) override;
	};
}

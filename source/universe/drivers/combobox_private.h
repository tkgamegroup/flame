#pragma once

#include "../entity_private.h"
#include "combobox.h"

namespace flame
{
	struct dComboboxPrivate : dCombobox
	{
		int index = -1;

		dMenuPrivate* menu = nullptr;
		cTextPrivate* text = nullptr;

		int get_index() const override { return index; }
		void set_index(int index) override;

		void on_load_finished() override;
		bool on_child_added(EntityPtr e, uint& pos) override;
	};
}

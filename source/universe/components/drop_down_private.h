#pragma once

#include "drop_down.h"

namespace flame
{
	struct cDropDownPrivate : cDropDown
	{
		int index = -1;

		cMenuPrivate* menu = nullptr;
		cTextPrivate* text = nullptr;

		int get_index() const override { return index; }
		void set_index(int v) override;

		void on_load_finished() override;
		bool on_before_adding_child(EntityPtr e);
	};
}

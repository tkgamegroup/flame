#pragma once

#include <flame/foundation/typeinfo.h>

#include "utils.h"

namespace flame
{
	namespace ui
	{
		void create_enum_combobox(EnumInfo* info, float width)
		{
			ui::e_begin_combobox(120.f);
			for (auto i = 0; i < info->item_count(); i++)
				ui::e_combobox_item(s2w(info->item(i)->name()).c_str());
			ui::e_end_combobox();
		}

		void create_enum_checkboxs(EnumInfo* info)
		{
			for (auto i = 0; i < info->item_count(); i++)
				ui::e_checkbox(s2w(info->item(i)->name()).c_str());
		}
	}
}

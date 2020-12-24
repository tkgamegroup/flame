#include "../entity_private.h"
#include "../components/text_private.h"
#include "../components/receiver_private.h"
#include "combobox_private.h"

namespace flame
{
	void dComboboxPrivate::set_index(int i)
	{
		//if (!menu || index == i)
		//	return;
		//auto items = menu->items;
		if (index != -1)
		{
			//auto e = items->children[index].get();
			//e->set_state((StateFlags)(e->state & (~StateSelected)));
		}
		index = i;
		if (index < 0)
			text->set_text(L"");
		else
		{
			//auto e = items->children[index].get();
			//text->set_text(e->get_component_t<cTextPrivate>()->text.c_str());
			//e->set_state((StateFlags)(e->state | StateSelected));
		}
		//data_changed(S<"index"_h>);
	}

	dCombobox* dCombobox::create()
	{
		return f_new<dComboboxPrivate>();
	}

	void dComboboxItemPrivate::on_gain_receiver()
	{
		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<dComboboxItemPrivate>();
			thiz->combobox->set_index(thiz->entity->index);
		}, Capture().set_thiz(this));
	}

	dComboboxItem* dComboboxItem::create()
	{
		return f_new<dComboboxItemPrivate>();
	}
}

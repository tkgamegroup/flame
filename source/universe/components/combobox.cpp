#include "../entity_private.h"
#include "text_private.h"
#include "receiver_private.h"
#include "combobox_private.h"

namespace flame
{
	void cComboboxPrivate::set_index(int i)
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
		data_changed(S<"index"_h>);
	}

	cCombobox* cCombobox::create()
	{
		return f_new<cComboboxPrivate>();
	}

	void cComboboxItemPrivate::on_gain_receiver()
	{
		mouse_listener = receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<cComboboxItemPrivate>();
			if (thiz->staging_combobox)
				thiz->staging_combobox->set_index(thiz->entity->index);
		}, Capture().set_thiz(this));
	}

	void cComboboxItemPrivate::on_lost_receiver()
	{
		receiver->remove_mouse_left_down_listener(mouse_listener);
	}

	void cComboboxItemPrivate::on_lost_combobox()
	{
		//staging_combobox = combobox->menu->opened ? combobox : nullptr;
	}

	cComboboxItem* cComboboxItem::create()
	{
		return f_new<cComboboxItemPrivate>();
	}
}

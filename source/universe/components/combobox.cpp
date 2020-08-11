#include "../entity_private.h"
#include "text_private.h"
#include "event_receiver_private.h"
#include "menu_private.h"
#include "combobox_private.h"

namespace flame
{
	void cComboboxPrivate::set_index(int i)
	{
		if (!menu || index == i)
			return;
		auto items = menu->items.get();
		if (index != -1)
		{
			auto e = items->children[index].get();
			e->set_state((StateFlags)(e->state & (~StateSelected)));
		}
		index = i;
		if (index < 0)
			text->set_text(L"");
		else
		{
			auto e = items->children[index].get();
			text->set_text(((cTextPrivate*)e->get_component(cText::type_hash))->text.c_str());
			e->set_state((StateFlags)(e->state | StateSelected));
		}
		Entity::report_data_changed(this, S<ch("index")>::v);
	}

	void cComboboxPrivate::on_entity_component_data_changed(Component* c, uint64 data_name_hash)
	{
		if (c == menu)
		{
			if (data_name_hash == S<ch("items")>::v)
			{
				auto i = 0;
				for (auto& e : menu->items->children)
				{
					auto ci = (cComboboxItemPrivate*)e->get_component(cComboboxItem::type_hash);
					if (ci)
					{
						ci->combobox = this;
						ci->index = i;
						i++;
					}
				}
			}
		}
	}

	cCombobox* cCombobox::create()
	{
		return f_new<cComboboxPrivate>();
	}

	void cComboboxItemPrivate::on_gain_event_receiver()
	{
		mouse_listener = event_receiver->add_mouse_left_down_listener([](Capture& c, const Vec2i& pos) {
			auto thiz = c.thiz<cComboboxItemPrivate>();
			thiz->combobox->set_index(thiz->index);
		}, Capture().set_thiz(this));
	}

	void cComboboxItemPrivate::on_lost_event_receiver()
	{
		event_receiver->remove_mouse_left_down_listener(mouse_listener);
	}

	cComboboxItem* cComboboxItem::create()
	{
		return f_new<cComboboxItemPrivate>();
	}
}

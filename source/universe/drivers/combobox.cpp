#include "../entity_private.h"
#include "../components/text_private.h"
#include "../components/receiver_private.h"
#include "../components/menu_private.h"
#include "combobox_private.h"

namespace flame
{
	void dComboboxPrivate::set_index(int i)
	{
		if (!menu || index == i)
			return;
		auto items = menu->items;
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
			text->set_text(e->get_component_t<cTextPrivate>()->text.c_str());
			e->set_state((StateFlags)(e->state | StateSelected));
		}
		//data_changed(S<"index"_h>);
	}

	void dComboboxPrivate::on_load_finished()
	{
		menu = entity->get_component_t<cMenuPrivate>();
		fassert(menu);

		text = entity->get_component_t<cTextPrivate>();
		fassert(text);
	}

	bool dComboboxPrivate::on_child_added(EntityPtr e, uint& pos)
	{
		if (load_finished)
		{
			auto receiver = e->get_component_t<cReceiverPrivate>();
			fassert(receiver);
			receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
				auto thiz = c.thiz<dComboboxPrivate>();
				thiz->set_index(c.data<EntityPrivate*>()->index);
			}, Capture().set_thiz(this).set_data(&e));
		}
		return false;
	}

	dCombobox* dCombobox::create(void* parms)
	{
		return new dComboboxPrivate();
	}
}

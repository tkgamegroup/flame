#include "../entity_private.h"
#include "text_private.h"
#include "receiver_private.h"
#include "menu_private.h"
#include "drop_down_private.h"

namespace flame
{
	void cDropDownPrivate::set_index(int i)
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
		data_changed(S<"index"_h>);
	}

	void cDropDownPrivate::on_load_finished()
	{
		menu = entity->get_component_t<cMenuPrivate>();
		assert(menu);

		text = entity->get_component_t<cTextPrivate>();
		assert(text);
	}

	bool cDropDownPrivate::on_before_adding_child(EntityPtr e)
	{
		if (load_finished)
		{
			if (e != menu->items)
			{
				auto receiver = e->get_component_t<cReceiverPrivate>();
				assert(receiver);

				receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
					auto thiz = c.thiz<cDropDownPrivate>();
					thiz->set_index(c.data<EntityPrivate*>()->index);
					}, Capture().set_thiz(this).set_data(&e));
			}
		}
		return false;
	}

	cDropDown* cDropDown::create(void* parms)
	{
		return new cDropDownPrivate();
	}
}

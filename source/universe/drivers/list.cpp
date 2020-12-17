#include "../entity_private.h"
#include "../components/receiver_private.h"
#include "list_private.h"

namespace flame
{
	void dListPrivate::on_load_finished()
	{
		receiver = entity->get_component_t<cReceiverPrivate>();
		fassert(receiver);

		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<dListPrivate>();
			if (thiz->enable_deselect)
				thiz->set_selected(nullptr);
		}, Capture().set_thiz(this));
	}

	bool dListPrivate::on_child_added(Entity* _e)
	{
		if (load_finished)
		{
			auto e = (EntityPrivate*)_e;
			auto dli = e->get_driver_t<dListItemPrivate>();
			if (dli)
				dli->list = this;
		}
		return false;
	}

	void dListPrivate::set_selected(Entity* e)
	{
		if (selected == e)
			return;
		if (selected)
			selected->set_state((StateFlags)(selected->state & (~StateSelected)));
		if (e)
			e->set_state((StateFlags)(((EntityPrivate*)e)->state | StateSelected));
		selected = (EntityPrivate*)e;
		//data_changed(S<"selected"_h>);
	}

	dList* dList::create()
	{
		return f_new<dListPrivate>();
	}

	void dListItemPrivate::on_load_finished()
	{
		receiver = entity->get_component_t<cReceiverPrivate>();
		fassert(receiver);

		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<dListItemPrivate>();
			if (thiz->list)
				thiz->list->set_selected(thiz->entity);
		}, Capture().set_thiz(this));
	}

	dListItem* dListItem::create()
	{
		return f_new<dListItemPrivate>();
	}
}

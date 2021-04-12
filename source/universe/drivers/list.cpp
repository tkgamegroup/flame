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

	bool dListPrivate::on_child_added(EntityPtr e)
	{
		if (load_finished)
		{
			auto receiver = e->get_component_t<cReceiverPrivate>();
			fassert(receiver);
			receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
				auto thiz = c.thiz<dListPrivate>();
				thiz->set_selected(c.data<EntityPrivate*>());
			}, Capture().set_thiz(this).set_data(&e));
		}
		return false;
	}

	void dListPrivate::set_selected(EntityPtr e)
	{
		if (selected == e)
			return;
		if (selected)
			selected->set_state((StateFlags)(selected->state & (~StateSelected)));
		if (e)
			e->set_state((StateFlags)(e->state | StateSelected));
		selected = e;
		entity->driver_data_changed(this, S<"selected"_h>);
	}

	dList* dList::create(void* parms)
	{
		return f_new<dListPrivate>();
	}
}

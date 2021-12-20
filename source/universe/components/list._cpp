#include "../entity_private.h"
#include "receiver_private.h"
#include "list_private.h"

namespace flame
{
	void cListPrivate::set_selected(EntityPtr v)
	{
		if (selected == v)
			return;
		if (selected)
			selected->set_state((StateFlags)(selected->state & (~StateSelected)));
		if (v)
			v->set_state((StateFlags)(v->state | StateSelected));
		selected = v;
		data_changed(S<"selected"_h>);
	}

	void cListPrivate::on_load_finished()
	{
		receiver = entity->get_component_t<cReceiverPrivate>();
		assert(receiver);

		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<cListPrivate>();
			if (thiz->enable_deselect)
				thiz->set_selected(nullptr);
			}, Capture().set_thiz(this));
	}

	bool cListPrivate::on_before_adding_child(EntityPtr e)
	{
		if (load_finished)
		{
			auto receiver = e->get_component_t<cReceiverPrivate>();
			assert(receiver);

			receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
				auto thiz = c.thiz<cListPrivate>();
				thiz->set_selected(c.data<EntityPrivate*>());
			}, Capture().set_thiz(this).set_data(&e));
		}
		return false;
	}

	cList* cList::create(void* parms)
	{
		return new cListPrivate();
	}
}

#include "../entity_private.h"
#include "receiver_private.h"
#include "toggle_private.h"

namespace flame
{
	void cTogglePrivate::set_toggled(bool v)
	{
		if (toggled == v)
			return;
		toggled = v;
		entity->set_state(toggled ? (entity->state | StateSelected) :
			(StateFlags)(entity->state & ~StateSelected));
		entity->component_data_changed(this, S<"checked"_h>);
	}

	void cTogglePrivate::on_load_finished()
	{
		receiver = entity->get_component_t<cReceiverPrivate>();
		fassert(receiver);

		receiver->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<cTogglePrivate>();
			thiz->set_toggled(!thiz->toggled);
			}, Capture().set_thiz(this));

		box = entity->find_child("box");
		fassert(box);
	}
	
	cToggle* cToggle::create(void* parms)
	{
		return new cTogglePrivate();
	}
}

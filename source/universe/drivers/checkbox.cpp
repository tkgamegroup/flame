#include "../components/receiver_private.h"
#include "checkbox_private.h"

namespace flame
{
	void dCheckboxPrivate::set_checked(bool c)
	{
		if (checked == c)
			return;
		checked = c;
		entity->set_state(checked ? (entity->state | StateSelected) :
			(StateFlags)(entity->state & ~StateSelected));
		entity->driver_data_changed(this, S<"checked"_h>);
	}

	void dCheckboxPrivate::on_load_finished()
	{
		receiver = entity->get_component_t<cReceiverPrivate>();
		fassert(receiver);

		receiver->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<dCheckboxPrivate>();
			thiz->set_checked(!thiz->checked);
		}, Capture().set_thiz(this));

		box = entity->find_child("box");
		fassert(box);
	}

	dCheckbox* dCheckbox::create(void* parms)
	{
		return new dCheckboxPrivate();
	}
}

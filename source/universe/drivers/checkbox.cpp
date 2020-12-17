#include "../entity_private.h"
#include "../components/receiver_private.h"
#include "checkbox_private.h"

namespace flame
{
	void cCheckboxPrivate::set_checked(bool c)
	{
		if (checked == c)
			return;
		checked = c;
		auto& s = entity->state;
		if (checked)
			s = (StateFlags)(int)(s | StateSelected);
		else
			s = (StateFlags)(int)(s & ~StateSelected);
		//data_changed(S<"checked"_h>);
	}

	void cCheckboxPrivate::on_load_finished()
	{
		receiver = entity->get_component_t<cReceiverPrivate>();
		fassert(receiver);

		receiver->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<cCheckboxPrivate>();
			thiz->set_checked(!thiz->checked);
		}, Capture().set_thiz(this));
	}

	dCheckbox* dCheckbox::create()
	{
		return f_new<cCheckboxPrivate>();
	}
}

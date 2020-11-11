#include "../entity_private.h"
#include "event_receiver_private.h"
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
		Entity::report_data_changed(this, S<"checked"_h>);
	}

	void cCheckboxPrivate::on_gain_event_receiver()
	{
		click_listener = event_receiver->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<cCheckboxPrivate>();
			thiz->set_checked(!thiz->checked);
		}, Capture().set_thiz(this));
	}

	void cCheckboxPrivate::on_lost_event_receiver()
	{
		event_receiver->remove_mouse_click_listener(click_listener);
	}

	cCheckbox* cCheckbox::create()
	{
		return f_new<cCheckboxPrivate>();
	}
}

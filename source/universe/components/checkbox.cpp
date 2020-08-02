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
		auto& s = ((EntityPrivate*)entity)->state;
		if (checked)
			s = (StateFlags)(int)(s | StateSelected);
		else
			s = (StateFlags)(int)(s & ~StateSelected);
		Entity::report_data_changed(this, S<ch("checked")>::v);
	}

	void cCheckboxPrivate::on_gain_event_receiver()
	{
		mouse_listener = event_receiver->add_mouse_listener([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			if (is_mouse_clicked(action, key))
			{
				auto thiz = c.thiz<cCheckboxPrivate>();
				thiz->set_checked(!thiz->checked);
			}
			return true;
		}, Capture().set_thiz(this));
	}

	void cCheckboxPrivate::on_lost_event_receiver()
	{
		event_receiver->remove_mouse_listener(mouse_listener);
	}

	cCheckbox* cCheckbox::create()
	{
		return f_new<cCheckboxPrivate>();
	}
}

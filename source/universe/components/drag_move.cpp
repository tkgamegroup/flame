#include "element_private.h"
#include "event_receiver_private.h"
#include "drag_move_private.h"
#include "../systems/event_dispatcher_private.h"

namespace flame
{
	void cDragMovePrivate::on_gain_event_receiver()
	{
		mouse_listener = event_receiver->add_mouse_move_listener([](Capture& c, const Vec2i& disp, const Vec2i&) {
			auto thiz = c.thiz<cDragMovePrivate>();
			if (thiz->event_receiver->dispatcher->active == thiz->event_receiver)
			{
				thiz->element->set_x(thiz->element->x + disp.x() / thiz->element->scalex);
				thiz->element->set_y(thiz->element->y + disp.y() / thiz->element->scaley);
			}
		}, Capture().set_thiz(this));
	}

	void cDragMovePrivate::on_lost_event_receiver()
	{
		event_receiver->remove_mouse_move_listener(mouse_listener);
	}

	cDragMove* cDragMove::create()
	{
		return f_new<cDragMovePrivate>();
	}
}

#include "element_private.h"
#include "receiver_private.h"
#include "drag_move_private.h"
#include "../systems/event_dispatcher_private.h"

namespace flame
{
	void cDragMovePrivate::on_gain_receiver()
	{
		mouse_listener = receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2&) {
			auto thiz = c.thiz<cDragMovePrivate>();
			if (thiz->receiver->dispatcher->active == thiz->receiver)
			{
				thiz->element->set_x(thiz->element->pos.x + disp.x / thiz->element->scl.x);
				thiz->element->set_y(thiz->element->pos.y + disp.y / thiz->element->scl.y);
			}
		}, Capture().set_thiz(this));
	}

	void cDragMovePrivate::on_lost_receiver()
	{
		receiver->remove_mouse_move_listener(mouse_listener);
	}

	cDragMove* cDragMove::create()
	{
		return f_new<cDragMovePrivate>();
	}
}

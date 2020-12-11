#include "../entity_private.h"
#include "element_private.h"
#include "receiver_private.h"
#include "drag_resize_private.h"
#include "../systems/dispatcher_private.h"

namespace flame
{
	void cDragResizePrivate::on_gain_block_receiver()
	{
		block_mouse_listener = block_receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2&) {
			auto thiz = c.thiz<cDragResizePrivate>();
			if (thiz->block_receiver->dispatcher->active == thiz->block_receiver)
			{
				thiz->element->set_width(thiz->element->size.x + disp.x);
				thiz->element->set_height(thiz->element->size.y + disp.y);
			}
		}, Capture().set_thiz(this));

		//block_receiver->entity->add_local_message_listener([](Capture& c, Message msg, void*) {
		//	auto thiz = c.thiz<cDragResizePrivate>();
		//	switch (msg)
		//	{
		//	case MessageStateChanged:
		//		thiz->block_receiver->dispatcher->window->set_cursor(
		//			(state & StateHovering) ? CursorSizeNWSE : CursorArrow);
		//		break;
		//	}
		//}, Capture().set_thiz(this));
	}

	void cDragResizePrivate::on_lost_block_receiver()
	{
		block_receiver->remove_mouse_move_listener(block_mouse_listener);
	}

	cDragResize* cDragResize::create()
	{
		return f_new<cDragResizePrivate>();
	}
}

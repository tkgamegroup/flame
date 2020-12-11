#include "../entity_private.h"
#include "../components/element_private.h"
#include "../components/receiver_private.h"
#include "../systems/dispatcher_private.h"
#include "window_private.h"

namespace flame
{
	void dWindowPrivate::on_load_finished()
	{
		element = entity->get_component_t<cElementPrivate>();
		fassert(element);
		receiver = entity->get_component_t<cReceiverPrivate>();
		fassert(receiver);

		receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2&) {
			auto thiz = c.thiz<dWindowPrivate>();
			if (thiz->receiver->dispatcher->active == thiz->receiver)
			{
				thiz->element->set_x(thiz->element->pos.x + disp.x / thiz->element->scl.x);
				thiz->element->set_y(thiz->element->pos.y + disp.y / thiz->element->scl.y);
			}
		}, Capture().set_thiz(this));

		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto e = c.thiz<dWindowPrivate>()->entity;
			auto parent = e->parent;
			if (parent && parent->children.size() > 1)
				parent->reposition_child(parent->children.size() - 1, e->index);
		}, Capture().set_thiz(this));

		auto size_dragger = entity->find_child("size_dragger");
		if (size_dragger)
		{
			size_dragger_receiver = size_dragger->get_component_t<cReceiverPrivate>();
			fassert(size_dragger_receiver);

			receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2&) {
				auto thiz = c.thiz<dWindowPrivate>();
				if (thiz->size_dragger_receiver->dispatcher->active == thiz->size_dragger_receiver)
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
	}

	dWindow* dWindow::create()
	{
		return f_new<dWindowPrivate>();
	}
}

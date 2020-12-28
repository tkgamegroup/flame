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
			if (thiz->receiver->is_active())
				thiz->element->add_pos(vec2(disp) / thiz->element->scl);
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

			size_dragger_receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2&) {
				auto thiz = c.thiz<dWindowPrivate>();
				if (thiz->size_dragger_receiver->is_active())
					thiz->element->add_size(disp);
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

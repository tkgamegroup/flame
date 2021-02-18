#include "../entity_private.h"
#include "../components/element_private.h"
#include "../components/receiver_private.h"
#include "../components/text_private.h"
#include "../systems/dispatcher_private.h"
#include "window_private.h"

namespace flame
{
	const wchar_t* dWindowPrivate::get_title() const
	{
		return title.c_str();
	}

	void dWindowPrivate::set_title(const wchar_t* _title)
	{
		title = _title;
		if (load_finished)
			title_text->set_text(title.c_str());
	}

	void* dWindowPrivate::add_close_listener(void (*callback)(Capture& c), const Capture& capture)
	{

		if (load_finished)
			close_button->set_visible(!close_listeners.empty());
		return nullptr;
	}

	void dWindowPrivate::remove_close_listener(void* lis)
	{
		if (load_finished)
			close_button->set_visible(!close_listeners.empty());
	}

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

		auto etitle = entity->find_child("title");
		fassert(etitle);
		title_text = etitle->get_component_t<cTextPrivate>();
		fassert(title_text);
		if (!title.empty())
			title_text->set_text(title.c_str());

		close_button = entity->find_child("close_button");
		fassert(close_button);
		close_button->set_visible(!close_listeners.empty());

		content = entity->find_child("content");
		fassert(content);
	}

	bool dWindowPrivate::on_child_added(Entity* _e)
	{
		if (load_finished)
		{
			content->add_child((EntityPrivate*)_e);
			return true;
		}
		return false;
	}

	dWindow* dWindow::create()
	{
		return f_new<dWindowPrivate>();
	}
}

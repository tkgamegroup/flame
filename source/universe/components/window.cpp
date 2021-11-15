#include "../entity_private.h"
#include "element_private.h"
#include "receiver_private.h"
#include "text_private.h"
#include "../systems/dispatcher_private.h"
#include "window_private.h"

namespace flame
{
	void cWindowPrivate::set_title(const wchar_t* _title)
	{
		title = _title;
		if (load_finished)
			title_text->set_text(title.c_str());
	}

	void cWindowPrivate::set_nomove(bool v)
	{
		nomove = v;
	}

	void cWindowPrivate::set_noresize(bool v)
	{
		noresize = v;
		size_dragger->set_visible(!noresize);

		element->set_auto_width(noresize);
		element->set_auto_height(noresize);
		content_element->set_alignx(noresize ? AlignNone : AlignMinMax);
		content_element->set_aligny(noresize ? AlignNone : AlignMinMax);
	}

	void* cWindowPrivate::add_close_listener(void (*callback)(Capture& c), const Capture& capture)
	{
		close_button->set_visible(true);

		auto c = new Closure(callback, capture);
		close_listeners.emplace_back(c);
		return c;
	}

	void cWindowPrivate::remove_close_listener(void* lis)
	{
		std::erase_if(close_listeners, [&](const auto& i) {
			return i.get() == (decltype(i.get()))lis;
		});

		if (load_finished)
			close_button->set_visible(!close_listeners.empty());
	}

	void cWindowPrivate::on_load_finished()
	{
		element = entity->get_component_i<cElementPrivate>(0);
		assert(element);
		receiver = entity->get_component_t<cReceiverPrivate>();
		assert(receiver);

		receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2&) {
			auto thiz = c.thiz<cWindowPrivate>();
			if (!thiz->nomove && thiz->receiver->is_active())
				thiz->element->add_pos(vec2(disp) / thiz->element->scl);
			}, Capture().set_thiz(this));

		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto e = c.thiz<cWindowPrivate>()->entity;
			auto parent = e->parent;
			if (parent && parent->children.size() > 1)
				parent->reposition_child(parent->children.size() - 1, e->index);
			}, Capture().set_thiz(this));

		size_dragger = entity->find_child("size_dragger");
		assert(size_dragger);
		size_dragger_receiver = size_dragger->get_component_t<cReceiverPrivate>();
		assert(size_dragger_receiver);

		size_dragger_receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2&) {
			auto thiz = c.thiz<cWindowPrivate>();
			if (thiz->size_dragger_receiver->is_active())
				thiz->element->add_size(disp);
			}, Capture().set_thiz(this));

		//block_receiver->entity->add_local_message_listener([](Capture& c, Message msg, void*) {
		//	auto thiz = c.thiz<cDragResizePrivate>();
		//	switch (msg)
		//	{
		//	case MessageStateChanged:
		//		thiz->block_receiver->dispatcher->window->set_cursor(
		//			(state & StateHovering) ? CursorS
		// izeNWSE : CursorArrow);
		//		break;
		//	}
		//}, Capture().set_thiz(this));

		auto etitle = entity->find_child("title");
		assert(etitle);
		title_text = etitle->get_component_t<cTextPrivate>();
		assert(title_text);
		if (!title.empty())
			title_text->set_text(title.c_str());

		close_button = entity->find_child("close_button");
		assert(close_button);
		close_button->set_visible(!close_listeners.empty());
		close_button->get_component_t<cReceiverPrivate>()->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<cWindowPrivate>();
			for (auto& l : thiz->close_listeners)
				l->call();
			auto e = thiz->entity;
			e->get_parent()->remove_child(e);
			}, Capture().set_thiz(this));

		content = entity->find_child("content");
		assert(content);
		content_element = content->get_component_t<cElementPrivate>();
		assert(content_element);
	}

	bool cWindowPrivate::on_before_adding_child(EntityPtr e)
	{
		if (load_finished)
		{
			content->add_child(e);
			return true;
		}
		return false;
	}

	cWindow* cWindow::create(void* parms)
	{
		return new cWindowPrivate();
	}
}

#include "../entity_private.h"
#include "element_private.h"
#include "event_receiver_private.h"
#include "layout_private.h"
#include "scroller_private.h"

namespace flame
{
	void cScrollerPrivate::scroll(float v)
	{
		//auto target_element = target_layout->element;
		//if (type == ScrollbarVertical)
		//{
		//	auto content_size = target_layout->content_size.y() + 20.f;
		//	if (target_element->size.y() > 0.f)
		//	{
		//		if (content_size > target_element->size.y())
		//			element->set_height(target_element->size.y() / content_size * scrollbar->element->size.y());
		//		else
		//			element->set_height(0.f);
		//	}
		//	else
		//		element->set_height(0.f);
		//	v += element->pos.y();
		//	element->set_y(element->size.y() > 0.f ? clamp(v, 0.f, scrollbar->element->size.y() - element->size.y()) : 0.f);
		//	target_layout->set_scrolly(-int(element->pos.y() / scrollbar->element->size.y() * content_size / step) * step);
		//}
		//else
		//{
		//	auto content_size = target_layout->content_size.x() + 20.f;
		//	if (target_element->size.x() > 0.f)
		//	{
		//		if (content_size > target_element->size.x())
		//			element->set_width(target_element->size.x() / content_size * scrollbar->element->size.x());
		//		else
		//			element->set_width(0.f);
		//	}
		//	else
		//		element->set_width(0.f);
		//	v += element->pos.x();
		//	element->set_x(element->size.x() > 0.f ? clamp(v, 0.f, scrollbar->element->size.x() - element->size.x()) : 0.f);
		//	target_layout->set_scrollx(-int(element->pos.x() / scrollbar->element->size.x() * content_size / step) * step);
		//}
	}

	void cScrollerPrivate::on_gain_event_receiver()
	{
		mouse_scroll_listener = event_receiver->add_mouse_scroll_listener([](Capture& c, int v) {
			c.thiz<cScrollerPrivate>()->scroll(-v * 20.f);
		}, Capture().set_thiz(this));
	}

	void cScrollerPrivate::on_lost_event_receiver()
	{
		event_receiver->remove_mouse_scroll_listener(mouse_scroll_listener);
	}

	void cScrollerPrivate::on_entity_added_child(Entity* e)
	{
		if (!view)
		{
			auto cs = e->get_component(cScrollView::type_hash);
			if (cs)
			{
				auto cl = e->get_component(cLayout::type_hash);
				if (cl)
				{
					view = e;
					view_layout = (cLayoutPrivate*)cl;
				}
			}
		}
	}

	void cScrollerPrivate::on_entity_removed_child(Entity* e)
	{
		if (view == e)
		{
			view = nullptr;
			view_layout = nullptr;
		}
	}

	cScroller* cScroller::create()
	{
		return f_new<cScrollerPrivate>();
	}

	void cScrollViewPrivate::on_gain_scroller()
	{
		scroller->target_element = (cElementPrivate*)target->get_component(cElement::type_hash);
		scroller->scroll(0.f);
	}

	void cScrollViewPrivate::on_entity_added_child(Entity* e)
	{
		if (!target)
		{
			auto ce = e->get_component(cElement::type_hash);
			if (ce)
				target = e;
		}
	}

	void cScrollViewPrivate::on_entity_removed_child(Entity* e)
	{
		if (target == e)
		{
			target = nullptr;
			scroller->target_element = nullptr;
		}
	}

	cScrollView* cScrollView::create()
	{
		return f_new<cScrollViewPrivate>();
	}

//	struct cScrollbarThumbPrivate : cScrollbarThumb
//	{
//		cScrollbarThumbPrivate(ScrollbarType _type)
//		{
//			step = 1.f;
//		}
//
//		void on_event(EntityEvent e, void* t) override
//		{
//			switch (e)
//			{
//			case EntityComponentAdded:
//			{
//				if (t == this)
//				{
//					mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
//						auto thiz = c.thiz<cScrollbarThumbPrivate>();
//						if (thiz->event_receiver->is_active() && is_mouse_move(action, key))
//						{
//							if (thiz->type == ScrollbarVertical)
//								thiz->update(pos.y());
//							else
//								thiz->update(pos.x());
//						}
//						return true;
//					}, Capture().set_thiz(this));
//
//					auto parent = entity->parent;
//					parent_element = parent->get_component(cElement);
//					parent_element_listener = parent_element->data_changed_listeners.add([](Capture& c, uint hash, void*) {
//						if (hash == FLAME_CHASH("size"))
//							c.thiz<cScrollbarThumbPrivate>()->update(0.f);
//						return true;
//					}, Capture().set_thiz(this));
//					scrollbar = parent->get_component(cScrollbar);
//					target_layout = parent->parent->children[0]->get_component(cLayout);
//					target_element_listener = target_layout->element->data_changed_listeners.add([](Capture& c, uint hash, void*) {
//						if (hash == FLAME_CHASH("size"))
//							c.thiz<cScrollbarThumbPrivate>()->update(0.f);
//						return true;
//					}, Capture().set_thiz(this));
//					target_layout_listener = target_layout->data_changed_listeners.add([](Capture& c, uint hash, void*) {
//						if (hash == FLAME_CHASH("content_size"))
//							c.thiz<cScrollbarThumbPrivate>()->update(0.f);
//						return true;
//					}, Capture().set_thiz(this));
//					update(0.f);
//				}
//			}
//				break;
//			}
//		}
//	};
}

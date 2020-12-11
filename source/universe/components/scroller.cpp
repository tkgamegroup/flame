#include "../entity_private.h"
#include "element_private.h"
#include "receiver_private.h"
#include "layout_private.h"
#include "scroller_private.h"
#include "../systems/dispatcher_private.h"

namespace flame
{
	void cScrollerPrivate::scroll(const vec2& v)
	{
		if (view_layout && target_element)
		{
			if (htrack_element)
			{
				auto target_size = target_element->size.x;
				if (target_size > view_element->size.x)
					hthumb_element->set_width(view_element->size.x / target_size * htrack_element->size.x);
				else
					hthumb_element->set_width(0.f);
				auto x = v.x + hthumb_element->pos.x;
				hthumb_element->set_x(hthumb_element->size.x > 0.f ? clamp(x, 0.f, htrack_element->size.x - hthumb_element->size.x) : 0.f);
				view_layout->set_scrollx(-int(hthumb_element->pos.x / htrack_element->size.x * target_size / step) * step);
			}
			if (vtrack_element)
			{
				auto target_size = target_element->size.y;
				if (target_size > view_element->size.y)
					vthumb_element->set_height(view_element->size.y / target_size * vtrack_element->size.y);
				else
					vthumb_element->set_height(0.f);
				auto y = v.y + vthumb_element->pos.y;
				vthumb_element->set_y(vthumb_element->size.y > 0.f ? clamp(y, 0.f, vtrack_element->size.y - vthumb_element->size.y) : 0.f);
				view_layout->set_scrolly(-int(vthumb_element->pos.y / vtrack_element->size.y * target_size / step) * step);
			}
		}
	}

	void cScrollerPrivate::on_gain_receiver()
	{
		mouse_scroll_listener = receiver->add_mouse_scroll_listener([](Capture& c, int v) {
			c.thiz<cScrollerPrivate>()->scroll(vec2(0.f, -v * 20.f));
		}, Capture().set_thiz(this));
	}

	void cScrollerPrivate::on_lost_receiver()
	{
		receiver->remove_mouse_scroll_listener(mouse_scroll_listener);
	}

	void cScrollerPrivate::on_gain_htrack_element()
	{
		//htrack_element_listener = htrack_element->entity->add_local_data_changed_listener([](Capture& c, Component* t, uint64 h) {
		//	auto thiz = c.thiz<cScrollerPrivate>();
		//	if (t == thiz->htrack_element)
		//	{
		//		if (h == S<"width"_h>)
		//			thiz->scroll(vec2(0.f));
		//	}
		//}, Capture().set_thiz(this));
	}

	void cScrollerPrivate::on_lost_htrack_element()
	{
		//htrack_element->entity->remove_local_data_changed_listener(htrack_element_listener);
	}

	void cScrollerPrivate::on_gain_hthumb_receiver()
	{
		hthumb_mouse_listener = hthumb_receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
			auto thiz = c.thiz<cScrollerPrivate>();
			if (thiz->receiver->dispatcher->active == thiz->hthumb_receiver)
				thiz->scroll(vec2(disp.x, 0.f));
		}, Capture().set_thiz(this));
	}

	void cScrollerPrivate::on_lost_hthumb_receiver()
	{
		hthumb_receiver->remove_mouse_move_listener(hthumb_mouse_listener);
	}

	void cScrollerPrivate::on_gain_vtrack_element()
	{
		//vtrack_element_listener = vtrack_element->entity->add_local_data_changed_listener([](Capture& c, Component* t, uint64 h) {
		//	auto thiz = c.thiz<cScrollerPrivate>();
		//	if (t == thiz->vtrack_element)
		//	{
		//		if (h == S<"height"_h>)
		//			thiz->scroll(vec2(0.f));
		//	}
		//}, Capture().set_thiz(this));
	}

	void cScrollerPrivate::on_lost_vtrack_element()
	{
		//vtrack_element->entity->remove_local_data_changed_listener(vtrack_element_listener);
	}

	void cScrollerPrivate::on_gain_vthumb_receiver()
	{
		vthumb_mouse_listener = vthumb_receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
			auto thiz = c.thiz<cScrollerPrivate>();
			if (thiz->receiver->dispatcher->active == thiz->vthumb_receiver)
				thiz->scroll(vec2(0.f, disp.y));
		}, Capture().set_thiz(this));
	}

	void cScrollerPrivate::on_lost_vthumb_receiver()
	{
		vthumb_receiver->remove_mouse_move_listener(vthumb_mouse_listener);
	}

	void cScrollerPrivate::on_gain_view_element()
	{
		//view_element->entity->add_child_message_listener([](Capture& c, Message msg, void* p) {
		//	auto thiz = c.thiz<cScrollerPrivate>();
		//	switch (msg)
		//	{
		//	case MessageAdded:
		//		if (!thiz->target_element)
		//		{
		//			auto e = (EntityPrivate*)p;
		//			auto ce = e->get_component_t<cElementPrivate>();
		//			if (ce)
		//			{
		//				thiz->target_element = ce;

		//				thiz->scroll(vec2(0.f));

		//				e->add_local_data_changed_listener([](Capture& c, Component* t, uint64 h) {
		//					auto thiz = c.thiz<cScrollerPrivate>();
		//					if (t == thiz->target_element)
		//					{
		//						if (h == S<"width"_h> || h == S<"height"_h>)
		//							thiz->scroll(vec2(0.f));
		//					}
		//				}, Capture().set_thiz(thiz));
		//			}
		//		}
		//		break;
		//	case MessageRemoved:
		//		if (thiz->target_element && thiz->target_element == ((EntityPrivate*)p)->get_component_t<cElementPrivate>())
		//			thiz->target_element = nullptr;
		//		break;
		//	}
		//}, Capture().set_thiz(this));

		//view_element->entity->add_local_data_changed_listener([](Capture& c, Component* t, uint64 h) {
		//	auto thiz = c.thiz<cScrollerPrivate>();
		//	if (t == thiz->view_element)
		//	{
		//		if (h == S<"width"_h> || h == S<"height"_h>)
		//			thiz->scroll(vec2(0.f));
		//	}
		//}, Capture().set_thiz(this));
	}

	cScroller* cScroller::create()
	{
		return f_new<cScrollerPrivate>();
	}
}

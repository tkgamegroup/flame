#include "../entity_private.h"
#include "element_private.h"
#include "event_receiver_private.h"
#include "layout_private.h"
#include "scroller_private.h"
#include "../systems/event_dispatcher_private.h"

namespace flame
{
	void cScrollerPrivate::scroll(float v)
	{
		if (view_layout && target_element)
		{
			if (type == ScrollerHorizontal)
			{
				auto target_size = target_element->width;
				if (target_size > target_element->width)
					thumb_element->set_width(target_element->width / target_size * track_element->width);
				else
					thumb_element->set_width(0.f);
				v += thumb_element->x;
				thumb_element->set_x(thumb_element->width > 0.f ? clamp(v, 0.f, track_element->width - thumb_element->width) : 0.f);
				view_layout->set_scrollx(-int(thumb_element->x / track_element->width * target_size / step) * step);
			}
			else
			{
				auto target_size = target_element->height;
				if (target_size > view_element->height)
					thumb_element->set_height(view_element->height / target_size * track_element->height);
				else
					thumb_element->set_height(0.f);
				v += thumb_element->y;
				thumb_element->set_y(thumb_element->height > 0.f ? clamp(v, 0.f, track_element->height - thumb_element->height) : 0.f);
				view_layout->set_scrolly(-int(thumb_element->y / track_element->height * target_size / step) * step);
			}
		}
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

	void cScrollerPrivate::on_gain_track_element()
	{
		track_element_listener = track_element->entity->add_data_changed_listener([](Capture& c, Component* t, uint64 h) {
			auto thiz = c.thiz<cScrollerPrivate>();
			if (t == thiz->track_element)
			{
				if (h == S<ch("width")>::v && thiz->type == ScrollerHorizontal ||
					h == S<ch("height")>::v && thiz->type == ScrollerVertical)
					thiz->scroll(0.f);
			}
		}, Capture().set_thiz(this));
	}

	void cScrollerPrivate::on_lost_track_element()
	{
		track_element->entity->remove_data_changed_listener(track_element_listener);
	}

	void cScrollerPrivate::on_gain_thumb_event_receiver()
	{
		thumb_mouse_listener = thumb_event_receiver->add_mouse_move_listener([](Capture& c, const Vec2i& disp, const Vec2i& pos) {
			auto thiz = c.thiz<cScrollerPrivate>();
			if (thiz->thumb_event_receiver->dispatcher->active == thiz->thumb_event_receiver)
			{
				if (thiz->type == ScrollerHorizontal)
					thiz->scroll(disp.x());
				else
					thiz->scroll(disp.y());
			}
		}, Capture().set_thiz(this));
	}

	void cScrollerPrivate::on_lost_thumb_event_receiver()
	{
		thumb_event_receiver->remove_mouse_move_listener(thumb_mouse_listener);
	}

	void cScrollerPrivate::on_child_message(Message msg, void* p)
	{
		switch (msg)
		{
		case MessageAdded:
			if (!view)
			{
				auto e = (Entity*)p;
				auto cv = e->get_component_t<cScrollViewPrivate>();
				if (cv)
				{
					auto ce = e->get_component_t<cElementPrivate>();
					auto cl = e->get_component_t<cLayoutPrivate>();
					if (ce && cl)
					{
						view = e;
						view_element = ce;
						view_layout = cl;
					}

					view->add_data_changed_listener([](Capture& c, Component* t, uint64 h) {
						auto thiz = c.thiz<cScrollerPrivate>();
						if (t == thiz->view_element)
						{
							if (h == S<ch("width")>::v && thiz->type == ScrollerHorizontal ||
								h == S<ch("height")>::v && thiz->type == ScrollerVertical)
								thiz->scroll(0.f);
						}
					}, Capture().set_thiz(this));
				}
			}
			break;
		case MessageRemoved:
			if (view == p)
			{
				view = nullptr;
				view_layout = nullptr;
			}
			break;
		}
	}

	cScroller* cScroller::create()
	{
		return f_new<cScrollerPrivate>();
	}

	void cScrollViewPrivate::on_gain_scroller()
	{
		if (target_element)
		{
			scroller->target_element = target_element;
			scroller->scroll(0.f);

			target->add_data_changed_listener([](Capture& c, Component* t, uint64 h) {
				auto thiz = c.thiz<cScrollerPrivate>();
				if (t == thiz->target_element)
				{
					if (h == S<ch("width")>::v && thiz->type == ScrollerHorizontal ||
						h == S<ch("height")>::v && thiz->type == ScrollerVertical)
						thiz->scroll(0.f);
				}
			}, Capture().set_thiz(scroller));
		}
	}

	void cScrollViewPrivate::on_lost_scroller()
	{
		scroller->target_element = nullptr;
	}

	void cScrollViewPrivate::on_child_message(Message msg, void* p)
	{
		switch (msg)
		{
		case MessageAdded:
			if (!target)
			{
				auto e = (Entity*)p;
				auto ce = e->get_component_t<cElementPrivate>();
				if (ce)
				{
					target = e;
					target_element = ce;

					if (scroller)
						on_gain_scroller();
				}
			}
			break;
		case MessageRemoved:
			if (target == p)
			{
				target = nullptr;
				target_element = nullptr;
				scroller->target_element = nullptr;

				if (scroller)
					scroller->target_element = nullptr;
			}
			break;
		}
	}

	cScrollView* cScrollView::create()
	{
		return f_new<cScrollViewPrivate>();
	}
}

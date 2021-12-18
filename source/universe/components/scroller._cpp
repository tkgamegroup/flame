#include "../entity_private.h"
#include "element_private.h"
#include "receiver_private.h"
#include "scroller_private.h"
#include "../systems/dispatcher_private.h"

namespace flame
{
	void cScrollerPrivate::set_type(ScrollType _type)
	{
		type = _type;
		if (load_finished)
		{
			switch (type)
			{
			case ScrollHorizontal:
				break;
			case ScrollVertical:
				element->set_layout_type(LayoutHorizontal);
				track_element->set_alignx(AlignMax);
				track_element->set_aligny(AlignMinMax);
				break;
			case ScrollBoth:
				break;
			}
		}
	}

	void cScrollerPrivate::on_load_finished()
	{
		element = entity->get_component_i<cElementPrivate>(0);
		assert(element);

		receiver = entity->get_component_t<cReceiverPrivate>();
		assert(receiver);

		receiver->add_mouse_scroll_listener([](Capture& c, int v) {
			c.thiz<cScrollerPrivate>()->scroll(vec2(0.f, -v * 20.f));
			}, Capture().set_thiz(this));

		track = entity->find_child("track");
		assert(track);
		track_element = track->get_component_i<cElementPrivate>(0);
		assert(track_element);

		thumb = entity->find_child("thumb");
		assert(thumb);
		thumb_element = thumb->get_component_i<cElementPrivate>(0);
		assert(thumb_element);
		thumb_receiver = thumb->get_component_t<cReceiverPrivate>();
		assert(thumb_receiver);

		view = entity->find_child("view");
		assert(view);
		view_element = view->get_component_i<cElementPrivate>(0);
		assert(view_element);

		thumb_receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
			auto thiz = c.thiz<cScrollerPrivate>();
			if (thiz->receiver->dispatcher->active == thiz->thumb_receiver)
			{
				switch (thiz->type)
				{
				case ScrollHorizontal:
					thiz->scroll(vec2(disp.x, 0.f));
					break;
				case ScrollVertical:
					thiz->scroll(vec2(0.f, disp.y));
					break;
				case ScrollBoth:
					thiz->scroll(vec2(disp));
					break;
				}
			}
			}, Capture().set_thiz(this));

		view->add_component_data_listener([](Capture& c, uint h) {
			auto thiz = c.thiz<cScrollerPrivate>();
			switch (h)
			{
			case S<"width"_h>:
				if (thiz->type != ScrollVertical)
					thiz->scroll(vec2(0.f));
				break;
			case S<"height"_h>:
				if (thiz->type != ScrollHorizontal)
					thiz->scroll(vec2(0.f));
				break;
			}
			}, Capture().set_thiz(this), view_element);

		set_type(type);
	}

	bool cScrollerPrivate::on_before_adding_child(EntityPtr e)
	{
		if (load_finished && !target)
		{
			target = e;
			target_element = target->get_component_i<cElementPrivate>(0);
			assert(target_element);
			target->add_component_data_listener([](Capture& c, uint h) {
				auto thiz = c.thiz<cScrollerPrivate>();
				switch (h)
				{
				case S<"width"_h>:
					if (thiz->type != ScrollVertical)
						thiz->scroll(vec2(0.f));
					break;
				case S<"height"_h>:
					if (thiz->type != ScrollHorizontal)
						thiz->scroll(vec2(0.f));
					break;
				}
				}, Capture().set_thiz(this), target_element);

			view->add_child(target);
			return true;
		}
		return false;
	}

	void cScrollerPrivate::scroll(const vec2& v)
	{
		if (target)
		{
			auto target_size = target_element->size.y;
			if (target_size > view_element->size.y)
				thumb_element->set_height(view_element->size.y / target_size * track_element->size.y);
			else
				thumb_element->set_height(0.f);
			auto y = v.y + thumb_element->pos.y;
			thumb_element->set_y(thumb_element->size.y > 0.f ? clamp(y, 0.f, track_element->size.y - thumb_element->size.y) : 0.f);
			target_element->set_scrolly(-int(thumb_element->pos.y / track_element->size.y * target_size / step) * step);
		}
	}

	cScroller* cScroller::create(void* parms)
	{
		return new cScrollerPrivate();
	}
}

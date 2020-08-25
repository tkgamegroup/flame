#include "../entity_private.h"
#include "element_private.h"
#include "event_receiver_private.h"
#include "slider_private.h"
#include "../systems/event_dispatcher_private.h"

namespace flame
{
	void cSliderPrivate::on_gain_bar_element()
	{
		bar_element_listener = bar_element->entity->add_local_data_changed_listener([](Capture& c, Component* t, uint64 h) {
			auto thiz = c.thiz<cSliderPrivate>();
			if (t == thiz->bar_element)
			{
				switch (h)
				{
				case ch("width"):
					thiz->thumb_element->set_x(thiz->v * thiz->bar_element->width);
					break;
				case ch("height"):
					thiz->thumb_element->set_y(thiz->bar_element->height * 0.5f);
					break;
				}
			}
		}, Capture().set_thiz(this));
	}

	void cSliderPrivate::on_lost_bar_element()
	{
		bar_element->entity->remove_local_data_changed_listener(bar_element_listener);
	}

	void cSliderPrivate::on_gain_thumb_event_receiver()
	{
		thumb_mouse_listener = thumb_event_receiver->add_mouse_move_listener([](Capture& c, const Vec2i& disp, const Vec2i& pos) {
			auto thiz = c.thiz<cSliderPrivate>();
			if (thiz->thumb_event_receiver->dispatcher->active == thiz->thumb_event_receiver)
			{
				auto x = clamp(thiz->thumb_element->x + disp.x(), 0.f, thiz->bar_element->width);
				thiz->v = x / thiz->bar_element->width;
				thiz->thumb_element->set_x(x);
				thiz->thumb_element->set_y(thiz->bar_element->height * 0.5f);
			}
		}, Capture().set_thiz(this));
	}

	void cSliderPrivate::on_lost_thumb_event_receiver()
	{
		thumb_event_receiver->remove_mouse_move_listener(thumb_mouse_listener);
	}

	cSlider* cSlider::create()
	{
		return f_new<cSliderPrivate>();
	}
}

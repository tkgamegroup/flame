#include "../entity_private.h"
#include "element_private.h"
#include "text_private.h"
#include "receiver_private.h"
#include "slider_private.h"
#include "../systems/dispatcher_private.h"

namespace flame
{
	void cSliderPrivate::set_value(float v)
	{
		value = clamp(v, value_min, value_max);
		proportion = (v - value_min) / value_max * bar_element->size.x;
		thumb_element->set_x(proportion * bar_element->size.x);
		text->set_text(std::to_wstring(value));
		data_changed(S<"value"_h>);
	}

	void cSliderPrivate::set_value_min(float v)
	{
		value_min = v;
		value = proportion * (value_max -value_min) + value_min;
		text->set_text(std::to_wstring(value));
		data_changed(S<"value"_h>);
	}

	void cSliderPrivate::set_value_max(float v)
	{
		value_max = v;
		value = proportion * (value_max - value_min) + value_min;
		text->set_text(std::to_wstring(value));
		data_changed(S<"value"_h>);
	}

	void cSliderPrivate::on_gain_bar_element()
	{
		//bar_element_listener = bar_element->entity->add_local_data_changed_listener([](Capture& c, Component* t, uint64 h) {
		//	auto thiz = c.thiz<cSliderPrivate>();
		//	if (t == thiz->bar_element)
		//	{
		//		switch (h)
		//		{
		//		case ch("width"):
		//			thiz->thumb_element->set_x(thiz->proportion * thiz->bar_element->size.x);
		//			break;
		//		case ch("height"):
		//			thiz->thumb_element->set_y(thiz->bar_element->size.y * 0.5f);
		//			break;
		//		}
		//	}
		//}, Capture().set_thiz(this));
	}

	void cSliderPrivate::on_lost_bar_element()
	{
		//bar_element->entity->remove_local_data_changed_listener(bar_element_listener);
	}

	void cSliderPrivate::on_gain_thumb_receiver()
	{
		thumb_mouse_listener = thumb_receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
			auto thiz = c.thiz<cSliderPrivate>();
			if (thiz->thumb_receiver->dispatcher->active == thiz->thumb_receiver)
			{
				auto x = clamp(thiz->thumb_element->pos.x + disp.x, 0.f, thiz->bar_element->size.x);
				thiz->proportion = x / thiz->bar_element->size.x;
				thiz->value = thiz->proportion * (thiz->value_max - thiz->value_min) + thiz->value_min;
				thiz->thumb_element->set_x(x);
				thiz->thumb_element->set_y(thiz->bar_element->size.y * 0.5f);
				thiz->text->set_text(std::to_wstring(thiz->value));
				thiz->data_changed(S<"value"_h>);
			}
		}, Capture().set_thiz(this));
	}

	void cSliderPrivate::on_lost_thumb_receiver()
	{
		thumb_receiver->remove_mouse_move_listener(thumb_mouse_listener);
	}

	cSlider* cSlider::create()
	{
		return f_new<cSliderPrivate>();
	}
}

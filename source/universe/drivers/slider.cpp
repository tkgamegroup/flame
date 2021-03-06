#include "../entity_private.h"
#include "../components/element_private.h"
#include "../components/text_private.h"
#include "../components/receiver_private.h"
#include "slider_private.h"
#include "../systems/dispatcher_private.h"

namespace flame
{
	void dSliderPrivate::set_value(float v)
	{
		value = clamp(v, value_min, value_max);
		proportion = (v - value_min) / value_max * bar_element->size.x;
		thumb_element->set_x(proportion * bar_element->size.x);
		text->set_text(std::to_wstring(value));
		if (entity)
			entity->component_data_changed(this, S <"value"_h>);
	}

	void dSliderPrivate::set_value_min(float v)
	{
		value_min = v;
		value = proportion * (value_max - value_min) + value_min;
		text->set_text(std::to_wstring(value));
		if (entity)
			entity->component_data_changed(this, S <"value_min"_h>);
	}

	void dSliderPrivate::set_value_max(float v)
	{
		value_max = v;
		value = proportion * (value_max - value_min) + value_min;
		text->set_text(std::to_wstring(value));
		if (entity)
			entity->component_data_changed(this, S <"value_max"_h>);
	}

	void dSliderPrivate::on_gain_bar_element()
	{
		//bar_element_listener = bar_element->entity->add_local_data_changed_listener([](Capture& c, Component* t, uint64 h) {
		//	auto thiz = c.thiz<dSliderPrivate>();
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

	void dSliderPrivate::on_lost_bar_element()
	{
		//bar_element->entity->remove_local_data_changed_listener(bar_element_listener);
	}

	void dSliderPrivate::on_gain_thumb_receiver()
	{
		thumb_mouse_listener = thumb_receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
			auto thiz = c.thiz<dSliderPrivate>();
			if (thiz->thumb_receiver->dispatcher->active == thiz->thumb_receiver)
			{
				auto x = clamp(thiz->thumb_element->pos.x + disp.x, 0.f, thiz->bar_element->size.x);
				thiz->proportion = x / thiz->bar_element->size.x;
				thiz->value = thiz->proportion * (thiz->value_max - thiz->value_min) + thiz->value_min;
				thiz->thumb_element->set_x(x);
				thiz->thumb_element->set_y(thiz->bar_element->size.y * 0.5f);
				thiz->text->set_text(std::to_wstring(thiz->value));
				thiz->entity->component_data_changed(thiz, S <"value"_h>);
			}
		}, Capture().set_thiz(this));
	}

	void dSliderPrivate::on_lost_thumb_receiver()
	{
		thumb_receiver->remove_mouse_move_listener(thumb_mouse_listener);
	}

	dSlider* dSlider::create(void* parms)
	{
		return f_new<dSliderPrivate>();
	}
}

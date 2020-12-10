#pragma once

#include "../entity_private.h"
#include <flame/universe/components/slider.h>

namespace flame
{
	struct cElementPrivate;
	struct cReceiverPrivate;
	struct cTextPrivate;

	struct cSliderPrivate : cSlider
	{
		float proportion = 0.f;
		float value = 0.f;
		float value_min = 0.f;
		float value_max = 0.f;

		cElementPrivate* bar_element = nullptr;
		cElementPrivate* thumb_element = nullptr;
		cReceiverPrivate* thumb_receiver = nullptr;
		cTextPrivate* text = nullptr;

		void* bar_element_listener = nullptr;
		void* thumb_mouse_listener = nullptr;

		float get_value() const override { return value; }
		void set_value(float v) override;
		float get_value_min() const override { return value_min; }
		void set_value_min(float v) override;
		float get_value_max() const override { return value_max; }
		void set_value_max(float v) override;

		void on_gain_bar_element();
		void on_lost_bar_element();

		void on_gain_thumb_receiver();
		void on_lost_thumb_receiver();
	};
}

#include <flame/universe/components/slider.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;
	struct cTextPrivate;

	struct cSliderPrivate : cSlider // R ~ on_*
	{
		float proportion = 0.f;
		float value = 0.f;
		float value_min = 0.f;
		float value_max = 0.f;

		cElementPrivate* bar_element = nullptr; // R ref place=bar
		cElementPrivate* thumb_element = nullptr; // R ref place=thumb
		cEventReceiverPrivate* thumb_event_receiver = nullptr; // R ref place=thumb
		cTextPrivate* text = nullptr; // R ref place=text

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

		void on_gain_thumb_event_receiver();
		void on_lost_thumb_event_receiver();
	};
}

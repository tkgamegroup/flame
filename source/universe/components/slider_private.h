#include <flame/universe/components/slider.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;

	struct cSliderPrivate : cSlider // R ~ on_*
	{
		float v = 0.f;

		cElementPrivate* bar_element = nullptr; // R ref place=bar
		cElementPrivate* thumb_element = nullptr; // R ref place=thumb
		cEventReceiverPrivate* thumb_event_receiver = nullptr; // R ref place=thumb

		void* bar_element_listener = nullptr;
		void* thumb_mouse_listener = nullptr;

		void on_gain_bar_element();
		void on_lost_bar_element();

		void on_gain_thumb_event_receiver();
		void on_lost_thumb_event_receiver();
	};
}

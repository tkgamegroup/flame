#pragma once

#include "../entity_private.h"
#include <flame/universe/components/scroller.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;
	struct cLayoutPrivate;

	struct cScrollerPrivate : cScroller
	{
		float step = 1.f;

		cEventReceiverPrivate* event_receiver = nullptr;

		cElementPrivate* htrack_element = nullptr;
		cElementPrivate* hthumb_element = nullptr;
		cEventReceiverPrivate* hthumb_event_receiver = nullptr;

		cElementPrivate* vtrack_element = nullptr;
		cElementPrivate* vthumb_element = nullptr;
		cEventReceiverPrivate* vthumb_event_receiver = nullptr;

		cElementPrivate* view_element = nullptr;
		cLayoutPrivate* view_layout = nullptr;

		cElementPrivate* target_element = nullptr;

		void* mouse_scroll_listener = nullptr;

		void* htrack_element_listener = nullptr;
		void* hthumb_mouse_listener = nullptr;
		void* vtrack_element_listener = nullptr;
		void* vthumb_mouse_listener = nullptr;

		void scroll(const vec2& v) override;

		void on_gain_event_receiver();
		void on_lost_event_receiver();

		void on_gain_htrack_element();
		void on_lost_htrack_element();
		void on_gain_hthumb_event_receiver();
		void on_lost_hthumb_event_receiver();

		void on_gain_vtrack_element();
		void on_lost_vtrack_element();
		void on_gain_vthumb_event_receiver();
		void on_lost_vthumb_event_receiver();

		void on_gain_view_element();
	};
}

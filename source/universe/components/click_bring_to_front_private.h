#pragma once

#include <flame/universe/components/click_bring_to_front.h>

namespace flame
{
	struct cEventReceiverPrivate;

	struct cClickBringToFrontPrivate : cClickBringToFront // R ~ on_*
	{
		cEventReceiverPrivate* event_receiver = nullptr; // R ref

		void* mouse_listener = nullptr;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
	};
}

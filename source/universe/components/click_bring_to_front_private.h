#pragma once

#include "../entity_private.h"
#include <flame/universe/components/click_bring_to_front.h>

namespace flame
{
	struct cEventReceiverPrivate;

	struct cClickBringToFrontPrivate : cClickBringToFront
	{
		cEventReceiverPrivate* event_receiver = nullptr;

		void* mouse_listener = nullptr;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
	};
}

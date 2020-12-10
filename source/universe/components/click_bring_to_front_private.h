#pragma once

#include "../entity_private.h"
#include <flame/universe/components/click_bring_to_front.h>

namespace flame
{
	struct cReceiverPrivate;

	struct cClickBringToFrontPrivate : cClickBringToFront
	{
		cReceiverPrivate* receiver = nullptr;

		void* mouse_listener = nullptr;

		void on_gain_receiver();
		void on_lost_receiver();
	};
}

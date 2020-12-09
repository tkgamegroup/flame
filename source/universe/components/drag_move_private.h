#pragma once

#include "../entity_private.h"
#include <flame/universe/components/drag_move.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;

	struct cDragMovePrivate : cDragMove
	{
		cElementPrivate* element = nullptr;
		cEventReceiverPrivate* event_receiver = nullptr;

		void* mouse_listener = nullptr;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
	};
}

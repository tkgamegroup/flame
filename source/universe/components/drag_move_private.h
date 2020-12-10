#pragma once

#include "../entity_private.h"
#include <flame/universe/components/drag_move.h>

namespace flame
{
	struct cElementPrivate;
	struct cReceiverPrivate;

	struct cDragMovePrivate : cDragMove
	{
		cElementPrivate* element = nullptr;
		cReceiverPrivate* receiver = nullptr;

		void* mouse_listener = nullptr;

		void on_gain_receiver();
		void on_lost_receiver();
	};
}

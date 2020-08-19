#pragma once

#include <flame/universe/components/drag_move.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;

	struct cDragMovePrivate : cDragMove // R ~ on_*
	{
		cElementPrivate* element = nullptr; // R ref
		cEventReceiverPrivate* event_receiver = nullptr; // R ref

		void* mouse_listener = nullptr;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
	};
}

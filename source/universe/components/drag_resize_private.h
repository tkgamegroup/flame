#pragma once

#include "../entity_private.h"
#include <flame/universe/components/drag_resize.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;

	struct cDragResizePrivate : cDragResize
	{
		cElementPrivate* element = nullptr;
		cEventReceiverPrivate* block_event_receiver = nullptr;

		void* block_mouse_listener = nullptr;

		void on_gain_block_event_receiver();
		void on_lost_block_event_receiver();
	};
}

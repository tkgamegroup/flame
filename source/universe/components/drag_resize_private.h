#pragma once

#include "../entity_private.h"
#include <flame/universe/components/drag_resize.h>

namespace flame
{
	struct cElementPrivate;
	struct cReceiverPrivate;

	struct cDragResizePrivate : cDragResize
	{
		cElementPrivate* element = nullptr;
		cReceiverPrivate* block_receiver = nullptr;

		void* block_mouse_listener = nullptr;

		void on_gain_block_receiver();
		void on_lost_block_receiver();
	};
}

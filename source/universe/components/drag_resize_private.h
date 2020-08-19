#pragma once

#include <flame/universe/components/drag_resize.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;

	struct cDragResizePrivate : cDragResize // R ~ on_*
	{
		cElementPrivate* element = nullptr; // R ref
		cEventReceiverPrivate* block_event_receiver = nullptr; // R ref place=drag_resize_block

		void* block_mouse_listener = nullptr;

		void on_gain_block_event_receiver();
		void on_lost_block_event_receiver();
	};
}

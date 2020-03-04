#pragma once

#include <flame/universe/components/event_receiver.h>
#include <flame/universe/systems/event_dispatcher.h>

namespace flame
{
	inline bool is_hovering(cEventReceiver* er)
	{
		return er->dispatcher->hovering == er;
	}

	inline bool is_focusing(cEventReceiver* er)
	{
		return er->dispatcher->focusing == er;
	}

	inline bool is_active(cEventReceiver* er)
	{
		auto dp = er->dispatcher;
		return dp->focusing == er && dp->focusing_state == FocusingAndActive;
	}

	inline bool is_dragging(cEventReceiver* er)
	{
		auto dp = er->dispatcher;
		return dp->focusing == er && dp->focusing_state == FocusingAndDragging;
	}
}

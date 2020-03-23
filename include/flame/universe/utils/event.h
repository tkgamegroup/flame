#pragma once

#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/components/event_receiver.h>

namespace flame
{
	namespace utils
	{
		inline bool is_hovering(cEventReceiver* er)
		{
			return er->dispatcher->hovering == er;
		}

		inline bool is_focusing(cEventReceiver* er)
		{
			return er->dispatcher->focusing == er;
		}

		inline bool is_focusing_and_not_normal(cEventReceiver* er)
		{
			auto dp = er->dispatcher;
			if (dp->focusing != er)
				return false;
			return dp->focusing_state != FocusingNormal;
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
}

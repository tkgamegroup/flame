#pragma once

#include "toggle.h"

namespace flame
{
	struct cTogglePrivate : cToggle
	{
		cReceiverPrivate* receiver;
		EntityPrivate* box;
	};
}

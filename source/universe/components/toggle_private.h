#pragma once

#include "toggle.h"

namespace flame
{
	struct cTogglePrivate : cToggle
	{
		int a = 1;
		
		cReceiverPrivate* receiver;
		EntityPrivate* box;
	};
}


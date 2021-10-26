#pragma once

#include "grid.h"

namespace flame
{
	struct cGridPrivate : cGrid
	{
		cElementPrivate* element;
		cReceiverPrivate* receiver;
		EntityPrivate* anchor;
		cElementPrivate* anchor_element;
	};
}

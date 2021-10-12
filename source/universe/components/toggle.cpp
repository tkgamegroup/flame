#include "toggle_private.h"

namespace flame
{
	cToggle* cToggle::create(void* parms)
	{
		return new cTogglePrivate();
	}
}



#include "scroller_private.h"

namespace flame
{
	cScroller* cScroller::create(void* parms)
	{
		return new cScrollerPrivate();
	}
}

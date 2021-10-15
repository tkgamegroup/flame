#include "window_private.h"

namespace flame
{
	cWindow* cWindow::create(void* parms)
	{
		return new cWindowPrivate();
	}
}

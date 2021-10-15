#include "menu_bar_private.h"

namespace flame
{
	cMenuBar* cMenuBar::create(void* parms)
	{
		return new cMenuBarPrivate();
	}
}

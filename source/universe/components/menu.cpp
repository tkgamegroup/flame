#include "menu_private.h"

namespace flame
{
	cMenu* cMenu::create(void* parms)
	{
		return new cMenuPrivate();
	}
}

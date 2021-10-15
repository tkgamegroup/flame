#include "menu_item_private.h"

namespace flame
{
	cMenuItem* cMenuItem::create(void* parms)
	{
		return new cMenuItemPrivate();
	}
}

#include "toggle_private.h"

namespace flame
{
	void cTogglePrivate::set_toggled(bool v)
	{
		toggled = v;
	}
	
	cToggle* cToggle::create(void* parms)
	{
		return new cTogglePrivate();
	}
}




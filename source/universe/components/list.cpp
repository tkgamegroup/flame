#include "list_private.h"

namespace flame
{
	cList* cList::create(void* parms)
	{
		return new cListPrivate();
	}
}

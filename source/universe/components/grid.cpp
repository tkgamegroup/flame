#include "grid_private.h"

namespace flame
{
	cGrid* cGrid::create(void* parms)
	{
		return new cGridPrivate();
	}
}

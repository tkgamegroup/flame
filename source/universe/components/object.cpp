#include "object_private.h"

namespace flame
{
	cObject* cObject::create()
	{
		return f_new<cObjectPrivate>();
	}
}

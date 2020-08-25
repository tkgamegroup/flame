#pragma once

#include <flame/universe/components/object.h>

namespace flame
{
	struct cNodePrivate;

	struct cObjectPrivate : cObject // R ~ on_*
	{
		cNodePrivate* node = nullptr; // R ref
	};
}

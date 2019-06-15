#pragma once

#include "shape.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct ShapePrivate
		{
			PxShape *v;
		};
	}
}


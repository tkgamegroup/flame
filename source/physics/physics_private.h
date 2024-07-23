#pragma once

#include <box2d/box2d.h>
#include "physics.h"

namespace flame
{
	namespace physics
	{
		inline b2BodyType to_backend(BodyType t)
		{
			switch (t)
			{
			case BodyStatic:
				return b2_staticBody;
			case BodyKinematic:
				return b2_kinematicBody;
			case BodyDynamic:
				return b2_dynamicBody;
			}
			return b2_dynamicBody;
		}
	}
}

#pragma once

#include "../foundation/foundation.h"

#ifdef FLAME_PHYSICS_MODULE

#define FLAME_PHYSICS_API __declspec(dllexport)

#define FLAME_PHYSICS_TYPE(name) FLAME_TYPE_PRIVATE(name)

#else

#define FLAME_PHYSICS_API __declspec(dllimport)

#define FLAME_PHYSICS_TYPE(name) FLAME_TYPE(name)

#endif

namespace flame
{
	namespace physics
	{
		FLAME_PHYSICS_TYPE(World2d);
		FLAME_PHYSICS_TYPE(Body2d);

		enum BodyType
		{
			BodyStatic,
			BodyKinematic,
			BodyDynamic
		};

		enum ShapeType
		{
			ShapeBox,
			ShapeCircle
		};
	}
}

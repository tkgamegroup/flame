#pragma once

#ifdef FLAME_PHYSICS_MODULE
#define FLAME_PHYSICS_EXPORTS __declspec(dllexport)
template<class T, class U>
struct FlamePhysicsTypeSelector
{
	typedef U result;
};
#else
#define FLAME_PHYSICS_EXPORTS __declspec(dllimport)
template<class T, class U>
struct FlamePhysicsTypeSelector
{
	typedef T result;
};
#endif

#define FLAME_PHYSICS_TYPE(name) struct name; struct name##Private; \
	typedef FlamePhysicsTypeSelector<name*, name##Private*>::result name##Ptr;

#include "../foundation/foundation.h"
#include "../graphics/graphics.h"

namespace flame
{
	namespace physics
	{
		FLAME_PHYSICS_TYPE(Device);
		FLAME_PHYSICS_TYPE(Rigid);
		FLAME_PHYSICS_TYPE(Shape);
		FLAME_PHYSICS_TYPE(Controller);
		FLAME_PHYSICS_TYPE(Material);
		FLAME_PHYSICS_TYPE(Scene);

		enum TouchType
		{
			TouchFound,
			TouchLost
		};

		enum ShapeType
		{
			ShapeBox,
			ShapeSphere,
			ShapeCapsule,
			ShapeTriangleMesh,
			ShapeHeightField
		};
	}
}


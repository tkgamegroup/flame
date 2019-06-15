#pragma once

#ifdef FLAME_PHYSICS_MODULE
#define FLAME_PHYSICS_EXPORTS __declspec(dllexport)
#else
#define FLAME_PHYSICS_EXPORTS __declspec(dllimport)
#endif

namespace flame
{
	namespace physics
	{
		enum TouchType
		{
			TouchFound,
			TouchLost
		};
	}
}


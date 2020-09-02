#pragma once

#include <flame/physics/rigid.h>
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct RigidPrivate : Rigid
		{
#ifdef USE_PHYSX
			PxRigidActor* v;
#endif

			RigidPrivate();
			~RigidPrivate();

			void release() override { delete this; }
		};
	}
}


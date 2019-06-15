#pragma once

#include "scene.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct pxCallback : PxSimulationEventCallback
		{
			Scene *s;

			virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override;
			virtual void onWake(PxActor** actors, PxU32 count) override;
			virtual void onSleep(PxActor** actors, PxU32 count) override;
			virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override;
			virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) override;
			virtual void onAdvance(const PxRigidBody*const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override;
		};

		struct ScenePrivate
		{
			PxScene *v;
			pxCallback callback;
			TriggerCallback trigger_callback;
		};
	}
}


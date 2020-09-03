#pragma once

#include <flame/physics/scene.h>
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct DevicePrivate;
		struct RigidPrivate;

		struct SceneBridge : Scene
		{
			void add_rigid(Rigid* r) override;
			void remove_rigid(Rigid* r) override;
		};

		struct ScenePrivate : SceneBridge
		{
#ifdef USE_PHYSX
			struct Callback : PxSimulationEventCallback
			{
				ScenePrivate* thiz;

				void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override;
				void onWake(PxActor** actors, PxU32 count) override;
				void onSleep(PxActor** actors, PxU32 count) override;
				void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override;
				void onTrigger(PxTriggerPair* pairs, PxU32 count) override;
				void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override;
			};

			PxScene* px_scene;
			Callback px_callback;
#endif

			std::vector<std::unique_ptr<Closure<void(Capture&, TouchType type, Shape* trigger_shape, Shape* other_shape)>>> trigger_listeners;

			ScenePrivate(float gravity, uint thread_count);

			void release() override { delete this; }

			void add_rigid(RigidPrivate* r);
			void remove_rigid(RigidPrivate* r);
			void update(float disp) override;
			void* add_trigger_listener(void (*callback)(Capture& c, TouchType type, Shape* trigger_shape, Shape* other_shape), const Capture& capture) override;
			void remove_trigger_listener(void* lis) override;
		};

		inline void SceneBridge::add_rigid(Rigid* s)
		{
			((ScenePrivate*)this)->add_rigid((RigidPrivate*)s);
		}

		inline void SceneBridge::remove_rigid(Rigid* s)
		{
			((ScenePrivate*)this)->remove_rigid((RigidPrivate*)s);
		}
	}
}


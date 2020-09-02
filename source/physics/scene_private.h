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
			struct Callback : physx::PxSimulationEventCallback
			{
				ScenePrivate* thiz;

				void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
				void onWake(physx::PxActor** actors, physx::PxU32 count) override;
				void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
				void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
				void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
				void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;
			};

			physx::PxScene* px_scene;
			Callback px_callback;
#endif

			std::vector<std::unique_ptr<Closure<void(Capture&, TouchType type, Shape* trigger_shape, Shape* other_shape)>>> trigger_listeners;

			ScenePrivate(DevicePrivate* d, float gravity, uint thread_count);

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


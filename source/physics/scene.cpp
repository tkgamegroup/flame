#include "scene_private.h"
#include "device_private.h"
#include "rigid_private.h"

namespace flame
{
	namespace physics
	{
#ifdef USE_PHYSX
		void ScenePrivate::Callback::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
		{
		}

		void ScenePrivate::Callback::onWake(physx::PxActor** actors, physx::PxU32 count)
		{
		}

		void ScenePrivate::Callback::onSleep(physx::PxActor** actors, physx::PxU32 count)
		{
		}

		void ScenePrivate::Callback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
		{
		}

		void ScenePrivate::Callback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
		{
			for (auto i = 0; i < count; i++)
			{
				auto type = pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND ? TouchFound : TouchLost;
				auto trigger_shape = (Shape*)pairs[i].triggerShape->userData;
				auto other_shape = (Shape*)pairs[i].otherShape->userData;
				for (auto& l : thiz->trigger_listeners)
					l->call(type, trigger_shape, other_shape);
			}
		}

		void ScenePrivate::Callback::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
		{
		}
#endif

		ScenePrivate::ScenePrivate(DevicePrivate* d, float gravity, uint thread_count)
		{
#ifdef USE_PHYSX
			physx::PxSceneDesc desc(d->px_instance->getTolerancesScale());
			desc.gravity = physx::PxVec3(0.0f, gravity, 0.0f);
			desc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(thread_count);
			desc.filterShader = physx::PxDefaultSimulationFilterShader;
			px_scene = d->px_instance->createScene(desc);
			px_callback.thiz = this;
			px_scene->setSimulationEventCallback(&px_callback);
#endif
		}

		void ScenePrivate::add_rigid(RigidPrivate* r)
		{
#ifdef USE_PHYSX
			px_scene->addActor(*r->px_rigid);
#endif
		}

		void ScenePrivate::remove_rigid(RigidPrivate* r)
		{
#ifdef USE_PHYSX
			px_scene->removeActor(*r->px_rigid);
#endif
		}

		void ScenePrivate::update(float disp)
		{
#ifdef USE_PHYSX
			px_scene->simulate(disp);
			px_scene->fetchResults(true);
#endif
		}

		void* ScenePrivate::add_trigger_listener(void (*callback)(Capture& c, TouchType type, Shape* trigger_shape, Shape* other_shape), const Capture& capture)
		{
			auto c = new Closure(callback, capture);
			trigger_listeners.emplace_back(c);
			return c;
		}

		void ScenePrivate::remove_trigger_listener(void* lis)
		{
			std::erase_if(trigger_listeners, [&](const auto& i) {
				return i == (decltype(i))lis;
			});
		}

		Scene* Scene::create(Device* d, float gravity, uint threads_count)
		{
			return new ScenePrivate((DevicePrivate*)d, gravity, threads_count);
		}
	}
}


#include "scene_private.h"
#include "device_private.h"
#include "rigid_private.h"

namespace flame
{
	namespace physics
	{
#ifdef USE_PHYSX
		void ScenePrivate::Callback::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
		{
		}

		void ScenePrivate::Callback::onWake(PxActor** actors, PxU32 count)
		{
		}

		void ScenePrivate::Callback::onSleep(PxActor** actors, PxU32 count)
		{
		}

		void ScenePrivate::Callback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
		{
		}

		void ScenePrivate::Callback::onTrigger(PxTriggerPair* pairs, PxU32 count)
		{
			if (thiz->trigger_callback)
			{
				for (auto i = 0; i < count; i++)
				{
					auto type = pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_FOUND ? TouchFound : TouchLost;
					auto trigger_shape = (Shape*)pairs[i].triggerShape->userData;
					auto other_shape = (Shape*)pairs[i].otherShape->userData;
					thiz->trigger_callback->call(type, trigger_shape, other_shape);
				}
			}
		}

		void ScenePrivate::Callback::onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)
		{
		}
#endif

		ScenePrivate::ScenePrivate(float gravity, uint thread_count)
		{
#ifdef USE_PHYSX
			PxSceneDesc desc(DevicePrivate::get()->px_instance->getTolerancesScale());
			desc.gravity = PxVec3(0.0f, gravity, 0.0f);
			desc.cpuDispatcher = PxDefaultCpuDispatcherCreate(thread_count);
			desc.filterShader = PxDefaultSimulationFilterShader;
			px_scene = DevicePrivate::get()->px_instance->createScene(desc);
			px_callback.thiz = this;
			px_scene->setSimulationEventCallback(&px_callback);

			px_controller_manager = PxCreateControllerManager(*px_scene);
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
			px_scene->simulate(max(disp, 0.001f));
			px_scene->fetchResults(true);
#endif
		}

		void ScenePrivate::set_trigger_callback(void (*callback)(Capture& c, TouchType type, Shape* trigger_shape, Shape* other_shape), const Capture& capture)
		{
			trigger_callback.reset(new Closure(callback, capture));
		}

		Scene* Scene::create(float gravity, uint threads_count)
		{
			return new ScenePrivate(gravity, threads_count);
		}
	}
}


#include "scene_private.h"
#include "device_private.h"
#include "rigid_private.h"
#include "controller_private.h"

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
					auto trigger_shape = (ShapePtr)pairs[i].triggerShape->userData;
					auto other_shape = (ShapePtr)pairs[i].otherShape->userData;
					thiz->trigger_callback->call(type, trigger_shape, other_shape);
				}
			}
		}

		void ScenePrivate::Callback::onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)
		{
		}
#endif

		ScenePrivate::ScenePrivate(DevicePrivate* _device, float gravity, uint thread_count) :
			device(_device)
		{
			if (!device)
				device = default_device;

#ifdef USE_PHYSX
			PxSceneDesc desc(device->px_instance->getTolerancesScale());
			desc.gravity = PxVec3(0.0f, gravity, 0.0f);
			desc.cpuDispatcher = PxDefaultCpuDispatcherCreate(thread_count);
			desc.filterShader = PxDefaultSimulationFilterShader;
			px_scene = device->px_instance->createScene(desc);
			px_callback.thiz = this;
			px_scene->setSimulationEventCallback(&px_callback);

			px_controller_manager = PxCreateControllerManager(*px_scene);
#endif
		}

		void ScenePrivate::add_rigid(RigidPtr r)
		{
#ifdef USE_PHYSX
			px_scene->addActor(*r->px_rigid);
#endif
		}

		void ScenePrivate::remove_rigid(RigidPtr r)
		{
#ifdef USE_PHYSX
			px_scene->removeActor(*r->px_rigid);
#endif
		}

		vec3 ScenePrivate::raycast(const vec3& origin, const vec3& dir, float max_distance, void** out_user_data)
		{
#ifdef USE_PHYSX
			PxRaycastBuffer hit;
			px_scene->raycast(cvt(origin), cvt(dir), max_distance, hit, PxHitFlag::ePOSITION);
			if (out_user_data)
			{
				auto actor = hit.block.actor;
				if (!actor)
					*out_user_data = nullptr;
				else
				{
					auto name = std::string(actor->getName());
					if (name == "Rigid")
						*out_user_data = ((RigidPtr)actor->userData)->user_data;
					else if (name == "Controller")
						*out_user_data = ((ControllerPtr)actor->userData)->user_data;
					else
						fassert(0);
				}
			}
			return cvt(hit.block.position);
#endif
			return vec3(0.f);
		}

		void ScenePrivate::update(float disp)
		{
#ifdef USE_PHYSX
			px_scene->simulate(max(disp, 0.001f));
			px_scene->fetchResults(true);
#endif
		}

		void ScenePrivate::set_trigger_callback(void (*callback)(Capture& c, TouchType type, ShapePtr trigger_shape, ShapePtr other_shape), const Capture& capture)
		{
			trigger_callback.reset(new Closure(callback, capture));
		}

		void ScenePrivate::set_visualization(bool v)
		{
#ifdef USE_PHYSX
			if (v)
			{
				px_scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.f);
				px_scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.f);
				px_scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_AABBS, 1.f);
			}
			else 
			{
				px_scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, -1.f);
				px_scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, -1.f);
				px_scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_AABBS, -1.f);
			}
#endif
		}

		void ScenePrivate::get_visualization_data(uint* lines_count, graphics::Line** lines)
		{
#ifdef USE_PHYSX
			auto& rb = px_scene->getRenderBuffer();
			*lines_count = rb.getNbLines();
			*lines = (graphics::Line*)rb.getLines();
#endif
		}

		Scene* Scene::create(Device* device, float gravity, uint threads_count)
		{
			return new ScenePrivate((DevicePrivate*)device, gravity, threads_count);
		}
	}
}


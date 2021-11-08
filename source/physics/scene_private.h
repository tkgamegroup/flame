#pragma once

#include "scene.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct ScenePrivate : Scene
		{
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
			PxControllerManager* px_controller_manager;

			DevicePrivate* device;

			std::unique_ptr<Closure<void(Capture&, TouchType type, ShapePtr trigger_shape, ShapePtr other_shape)>> trigger_callback;

			ScenePrivate(DevicePrivate* device, float gravity, uint thread_count);

			void release() override { delete this; }

			void add_rigid(RigidPtr r) override;
			void remove_rigid(RigidPtr r) override;
			vec3 raycast(const vec3& origin, const vec3& dir, float max_distance, void** out_user_data) override;
			void update(float disp) override;
			void set_trigger_callback(void (*callback)(Capture& c, TouchType type, ShapePtr trigger_shape, ShapePtr other_shape), const Capture& capture) override;
			void set_visualization(bool v) override;
			void get_visualization_data(uint* lines_count, graphics::Line** lines) override;
		};
	}
}


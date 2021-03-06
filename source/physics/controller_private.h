#pragma once

#include "controller.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct ControllerPrivate : Controller
		{
#ifdef USE_PHYSX
			UniPtr<PxController> px_controller;
#endif

			ControllerPrivate(ScenePrivate* scene, MaterialPrivate* material, float radius, float height);

			void release() override { delete this; }

			vec3 get_position() const override;
			void set_position(const vec3& pos) override;
			void move(const vec3& disp, float delta_time) override;
		};
	}
}

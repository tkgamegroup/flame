#pragma once

#include <flame/physics/controller.h>
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct ScenePrivate;
		struct MaterialPrivate;

		struct ControllerPrivate : Controller
		{
#ifdef USE_PHYSX
			PxController* px_controller;
#endif

			ControllerPrivate(ScenePrivate* scene, MaterialPrivate* material, float radius, float height);
			~ControllerPrivate();

			void release() override { delete this; }

			Vec3f get_position() const override;
			void set_position(const Vec3f& pos) override;
			void move(const Vec3f& disp, float delta_time) override;
		};
	}
}

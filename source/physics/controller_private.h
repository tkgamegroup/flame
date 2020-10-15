#pragma once

#include <flame/physics/controller.h>
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct ScenePrivate;

		struct ControllerPrivate : Controller
		{
			PxController* px_controller;

			ControllerPrivate(ScenePrivate* scene, float radius, float height);
			~ControllerPrivate();

			void release() override { delete this; }

			Vec3f get_position() const override;
			void set_position(const Vec3f& pos) override;
			void move(const Vec3f& disp, float delta_time) override;
		};
	}
}

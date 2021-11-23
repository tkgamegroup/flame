#pragma once

#include "controller.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct ControllerPrivate : Controller
		{
			std::unique_ptr<PxController> px_controller;

			ControllerPrivate(ScenePrivate* scene, MaterialPrivate* material, float radius, float height);

			void release() override { delete this; }

			vec3 get_position() const override;
			void set_position(const vec3& pos) override;
			bool move(const vec3& disp, float delta_time) override;
		};
	}
}

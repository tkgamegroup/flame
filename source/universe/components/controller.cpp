#include <flame/physics/device.h>
#include <flame/physics/material.h>
#include <flame/physics/controller.h>
#include "node_private.h"
#include "controller_private.h"
#include "../systems/physics_world_private.h"

namespace flame
{
	static physics::Material* material = nullptr;
	static physics::Material* get_material()
	{
		if (!material)
			material = physics::Material::create(physics::Device::get(), 0.2f, 0.2f, 0.3f);
		return material;
	}

	void cControllerPrivate::on_gain_physics_world()
	{
		auto size = Vec3f(1.f, 2.f, 1.f);
		phy_controller = physics::Controller::create(physics_world->phy_scene, get_material(), max(size.x(), size.z()), size.y());
		node->update_transform();
		phy_controller->set_position(node->global_pos);
		physics_world->controllers.push_back(this);
	}

	void cControllerPrivate::on_lost_physics_world()
	{
		if (phy_controller)
		{
			std::erase_if(physics_world->controllers, [&](const auto& i) {
				return i == this;
			});
			phy_controller->release();
			phy_controller = nullptr;
		}
	}

	cController* cController::create()
	{
		return f_new<cControllerPrivate>();
	}
}

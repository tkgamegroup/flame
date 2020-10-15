#include <flame/graphics/model.h>
#include <flame/physics/controller.h>
#include "mesh_private.h"
#include "controller_private.h"
#include "../systems/physics_world_private.h"

namespace flame
{
	void cControllerPrivate::on_gain_physics_world()
	{
		if (mesh->mesh)
		{
			auto m = mesh->mesh;
			auto size = m->get_upper_bound() - m->get_lower_bound();
			phy_controller = physics::Controller::create(physics_world->phy_scene, max(size.x(), size.z()), size.y());
			physics_world->controllers.push_back(this);
		}
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
}

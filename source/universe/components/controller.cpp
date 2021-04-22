#include "../../physics/controller.h"
#include "../world_private.h"
#include "node_private.h"
#include "controller_private.h"
#include "../systems/physics_private.h"

namespace flame
{
	void cControllerPrivate::set_radius(float r)
	{
		radius = r;
	}

	void cControllerPrivate::set_height(float h)
	{
		height = h;
	}

	void cControllerPrivate::move(const vec3& _disp)
	{ 
		disp = _disp; 
	}

	void cControllerPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);
		node->set_auto_update_qut();
	}

	void cControllerPrivate::on_removed()
	{
		node = nullptr;
	}

	void cControllerPrivate::on_entered_world()
	{
		physics = entity->world->get_system_t<sPhysicsPrivate>();
		fassert(physics);

		phy_controller = physics::Controller::create(physics->physics_scene.get(), nullptr, radius, height);
		node->update_transform();
		phy_controller->set_position(node->g_pos);
		physics->controllers.push_back(this);
	}

	void cControllerPrivate::on_left_world()
	{
		if (phy_controller)
		{
			std::erase_if(physics->controllers, [&](const auto& i) {
				return i == this;
			});
			phy_controller->release();
			phy_controller = nullptr;
		}

		physics = nullptr;
	}

	cController* cController::create(void* parms)
	{
		return f_new<cControllerPrivate>();
	}
}

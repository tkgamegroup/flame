#include "../../physics/device.h"
#include "../../physics/controller.h"
#include "../../physics/material.h"
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

	void cControllerPrivate::set_static_friction(float v)
	{
		static_friction = v;
	}

	void cControllerPrivate::set_dynamic_friction(float v)
	{
		dynamic_friction = v;
	}

	void cControllerPrivate::set_restitution(float v)
	{
		restitution = v;
	}

	void cControllerPrivate::move(const vec3& _disp)
	{ 
		disp = _disp; 
	}

	void cControllerPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);
	}

	void cControllerPrivate::on_removed()
	{
		node = nullptr;
	}

	void cControllerPrivate::on_entered_world()
	{
		phy_scene = entity->world->get_system_t<sPhysicsPrivate>();
		fassert(phy_scene);

		auto device = physics::Device::get_default();
		phy_controller = physics::Controller::create(phy_scene->physics_scene.get(), 
			physics::Material::get(device, static_friction, dynamic_friction, restitution), radius, height);
		phy_controller->user_data = entity;
		node->update_transform();
		phy_controller->set_position(node->pos);
		phy_scene->controllers.push_back(this);
	}

	void cControllerPrivate::on_left_world()
	{
		if (phy_controller)
		{
			std::erase_if(phy_scene->controllers, [&](const auto& i) {
				return i == this;
			});
			phy_controller->release();
			phy_controller = nullptr;
		}

		phy_scene = nullptr;
	}

	cController* cController::create(void* parms)
	{
		return new cControllerPrivate();
	}
}

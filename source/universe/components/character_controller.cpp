#ifdef USE_PHYSICS_MODULE

#include "../../physics/device.h"
#include "../../physics/controller.h"
#include "../../physics/material.h"
#include "../world_private.h"
#include "node_private.h"
#include "character_controller_private.h"
#include "../systems/physics_private.h"

namespace flame
{
	void cCharacterControllerPrivate::set_radius(float r)
	{
		radius = r;
	}

	void cCharacterControllerPrivate::set_height(float h)
	{
		height = h;
	}

	void cCharacterControllerPrivate::set_static_friction(float v)
	{
		static_friction = v;
	}

	void cCharacterControllerPrivate::set_dynamic_friction(float v)
	{
		dynamic_friction = v;
	}

	void cCharacterControllerPrivate::set_restitution(float v)
	{
		restitution = v;
	}

	void cCharacterControllerPrivate::move(const vec3& _disp)
	{ 
		disp = _disp; 
	}

	void cCharacterControllerPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		assert(node);
	}

	void cCharacterControllerPrivate::on_removed()
	{
		node = nullptr;
	}

	void cCharacterControllerPrivate::on_entered_world()
	{
		phy_scene = entity->world->get_system_t<sPhysicsPrivate>();
		assert(phy_scene);

		phy_controller = physics::Controller::create(phy_scene->physics_scene.get(), 
			physics::Material::get(nullptr, static_friction, dynamic_friction, restitution), radius, height);
		phy_controller->user_data = entity;
		node->update_transform();
		phy_controller->set_position(node->pos);
		phy_scene->controllers.push_back(this);
	}

	void cCharacterControllerPrivate::on_left_world()
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

	cCharacterController* cCharacterController::create()
	{
		return new cCharacterControllerPrivate();
	}
}

#endif

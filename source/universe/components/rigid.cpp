#include "../../physics/device.h"
#include "../../physics/scene.h"
#include "../../physics/rigid.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "rigid_private.h"
#include "../systems/physics_private.h"

namespace flame
{
	cRigidPrivate::~cRigidPrivate()
	{
		destroy();
	}

	void cRigidPrivate::set_dynamic(bool v)
	{
		if (dynamic == v)
			return;
		fassert(!phy_rigid);
		dynamic = v;
	}

	bool cRigidPrivate::is_sleeping() const
	{
		if (phy_rigid)
			return phy_rigid->is_sleeping();
		return false;
	}

	void cRigidPrivate::add_impulse(const vec3& v)
	{
		if (dynamic)
		{
			if (!phy_rigid)
				staging_impulse += v;
			else
				phy_rigid->add_impulse(v);
		}
	}

	void* cRigidPrivate::add_trigger_listener(void (*callback)(Capture& c, physics::TouchType type, EntityPtr trigger_shape, EntityPtr other_shape), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		trigger_listeners.emplace_back(c);
		return c;
	}

	void cRigidPrivate::remove_trigger_listener(void* lis)
	{
		std::erase_if(trigger_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void cRigidPrivate::create()
	{
		phy_rigid = physics::Rigid::create(physics::Device::get_default(), dynamic);
		phy_rigid->user_data = entity;
		phy_scene->rigids.push_back(this);
		phy_scene->physics_scene->add_rigid(phy_rigid);
		node->update_transform();
		phy_rigid->set_pose(node->pos, node->qut);
	}

	void cRigidPrivate::destroy()
	{
		if (!phy_rigid)
			return;
		std::erase_if(phy_scene->rigids, [&](const auto& i) {
			return i == this;
		});
		phy_scene->physics_scene->remove_rigid(phy_rigid);
		phy_rigid->release();
	}

	void cRigidPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);
	}

	void cRigidPrivate::on_removed()
	{
		node = nullptr;
	}

	void cRigidPrivate::on_entered_world()
	{
		phy_scene = entity->world->get_system_t<sPhysicsPrivate>();
		fassert(phy_scene);

		create();
		phy_rigid->add_impulse(staging_impulse);
		staging_impulse = vec3(0.f);
	}

	void cRigidPrivate::on_left_world()
	{
		destroy();
		phy_rigid = nullptr;
	}

	cRigid* cRigid::create(void* parms)
	{
		return new cRigidPrivate();
	}
}

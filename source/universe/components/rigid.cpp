#include <flame/physics/device.h>
#include <flame/physics/scene.h>
#include <flame/physics/rigid.h>
#include <flame/script/script.h>
#include "../entity_private.h"
#include "node_private.h"
#include "rigid_private.h"
#include "../systems/physics_world_private.h"

namespace flame
{
	cRigidPrivate::~cRigidPrivate()
	{
		on_lost_physics_world();

		for (auto s : trigger_listeners_s)
			script::Instance::get_default()->release_slot(s);
	}

	void cRigidPrivate::set_dynamic(bool v)
	{
		if (dynamic != v)
		{
			dynamic = v;
			if (phy_rigid)
			{
				for (auto s : phy_shapes)
					phy_rigid->remove_shape(s);
				on_lost_physics_world();
				on_gain_physics_world();
				for (auto s : phy_shapes)
					phy_rigid->add_shape(s);
			}
		}
	}

	void cRigidPrivate::add_impulse(const Vec3f& v)
	{
		if (dynamic)
		{
			if (!phy_rigid)
				staging_impulse += v;
			else
				phy_rigid->add_impulse(v);
		}
	}

	void* cRigidPrivate::add_trigger_listener(void (*callback)(Capture& c, physics::TouchType type, cShape* trigger_shape, cShape* other_shape), const Capture& capture)
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

	void cRigidPrivate::add_trigger_listener_s(uint slot)
	{
		trigger_listeners_s.push_back(slot);
	}

	void cRigidPrivate::remove_trigger_listener_s(uint slot)
	{
		for (auto it = trigger_listeners_s.begin(); it != trigger_listeners_s.end(); it++)
		{
			if (*it == slot)
			{
				trigger_listeners_s.erase(it);
				script::Instance::get_default()->release_slot(slot);
			}
		}
	}

	void cRigidPrivate::on_trigger_event(physics::TouchType type, cShape* trigger_shape, cShape* other_shape)
	{
		for (auto& l : trigger_listeners)
			l->call(type, trigger_shape, other_shape);
		script::Parameter ps[3];
		ps[0].type = script::ScriptTypeInt;
		ps[0].data.i[0] = type;
		ps[1].type = script::ScriptTypePointer;
		ps[1].data.p = trigger_shape;
		ps[2].type = script::ScriptTypePointer;
		ps[2].data.p = other_shape;
		for (auto s : trigger_listeners_s)
			script::Instance::get_default()->call_slot(s, size(ps), ps);
	}

	void cRigidPrivate::on_gain_physics_world()
	{
		phy_rigid = physics::Rigid::create(physics::Device::get_default(), dynamic);
		phy_rigid->user_data = this;
		physics_world->rigids.push_back(this);
		physics_world->phy_scene->add_rigid(phy_rigid);
		node->update_transform();
		phy_rigid->set_pose(node->global_pos, node->global_quat);
		phy_rigid->add_impulse(staging_impulse);
		staging_impulse = 0.f;
	}

	void cRigidPrivate::on_lost_physics_world()
	{
		std::erase_if(physics_world->rigids, [&](const auto& i) {
			return i == this;
		});
		physics_world->phy_scene->remove_rigid(phy_rigid);
		phy_rigid->release();
		phy_rigid = nullptr;
	}

	cRigid* cRigid::create()
	{
		return new cRigidPrivate();
	}
}

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
			script::Instance::get()->release_slot(s);
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
				on_lost_node();
				on_gain_node();
				on_gain_physics_world();
				for (auto s : phy_shapes)
					phy_rigid->add_shape(s);
			}
		}
	}

	void cRigidPrivate::add_impulse(const Vec3f& v)
	{
		if (dynamic && phy_rigid)
		{
			if (!physics_world)
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
				script::Instance::get()->release_slot(slot);
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
			script::Instance::get()->call_slot(s, size(ps), ps);
	}

	void cRigidPrivate::on_gain_node()
	{
		phy_rigid = physics::Rigid::create(dynamic);
		phy_rigid->user_data = this;
	}

	void cRigidPrivate::on_lost_node()
	{
		phy_rigid->release();
		phy_rigid = nullptr;
	}

	void cRigidPrivate::on_gain_physics_world()
	{
		if (!physics_world)
			return;
		physics_world->rigids.push_back(this);
		physics_world->scene->add_rigid(phy_rigid);
		phy_rigid->add_impulse(staging_impulse);
		staging_impulse = 0.f;
	}

	void cRigidPrivate::on_lost_physics_world()
	{
		if (!physics_world)
			return;
		std::erase_if(physics_world->rigids, [&](const auto& i) {
			return i == this;
		});
		physics_world->scene->remove_rigid(phy_rigid);
	}

	void cRigidPrivate::set_pose()
	{
		node->update_transform();
		if (!dynamic || distance(node->global_pos, curr_coord) > 0.01f || distance(node->global_quat, curr_quat) > 0.01f)
			phy_rigid->set_pose(node->global_pos, node->global_quat);
	}

	void cRigidPrivate::get_pose()
	{
		if (!dynamic)
			return;
		phy_rigid->get_pose(curr_coord, curr_quat);
		auto pn = entity->get_parent_component_t<cNodePrivate>();
		if (pn)
		{
			auto q_inv = pn->global_quat;
			q_inv = Vec4f(-q_inv.x(), -q_inv.y(), -q_inv.z(), q_inv.w());
			node->set_pos(quat_mul(q_inv, curr_coord - pn->global_pos) / pn->global_scale);
			node->set_quat(quat_mul(q_inv, curr_quat));
		}
		else
		{
			node->set_pos(curr_coord);
			node->set_quat(curr_quat);
		}
	}

	cRigid* cRigid::create()
	{
		return new cRigidPrivate();
	}
}

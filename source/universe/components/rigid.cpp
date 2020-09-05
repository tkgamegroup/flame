#include <flame/physics/scene.h>
#include "../entity_private.h"
#include "node_private.h"
#include "rigid_private.h"
#include "../systems/physics_world_private.h"

namespace flame
{
	void cRigidPrivate::set_dynamic(bool v)
	{
		dynamic = v;
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

	void cRigidPrivate::on_gain_node()
	{
		phy_rigid = physics::Rigid::create(dynamic);
	}

	void cRigidPrivate::on_lost_node()
	{
		phy_rigid->release();
		phy_rigid = nullptr;
	}

	void cRigidPrivate::on_gain_physics_world()
	{
		physics_world->rigids.push_back(this);
		physics_world->scene->add_rigid(phy_rigid);
		phy_rigid->add_impulse(staging_impulse);
		staging_impulse = 0.f;
	}

	void cRigidPrivate::on_lost_physics_world()
	{
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

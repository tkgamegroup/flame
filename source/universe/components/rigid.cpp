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

	void cRigidPrivate::on_gain_node()
	{
		rigid = physics::Rigid::create(dynamic);
		rigid->set_pose(node->pos, node->quat);
	}

	void cRigidPrivate::on_lost_node()
	{
		rigid->release();
		rigid = nullptr;
	}

	void cRigidPrivate::on_gain_physics_world()
	{
		physics_world->rigids.push_back(this);
		physics_world->scene->add_rigid(rigid);
	}

	void cRigidPrivate::on_lost_physics_world()
	{
		std::erase_if(physics_world->rigids, [&](const auto& i) {
			return i == this;
		});
		physics_world->scene->remove_rigid(rigid);
	}

	void cRigidPrivate::on_local_data_changed(Component* t, uint64 h)
	{
		if (t == node && !retrieving)
		{
			switch (h)
			{
			case ch("pos"):
			case ch("quat"):
				rigid->set_pose(node->pos, node->quat);
				break;
			}
		}
	}

	cRigid* cRigid::create()
	{
		return new cRigidPrivate();
	}
}

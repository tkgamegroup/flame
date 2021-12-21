#ifdef USE_PHYSICS_MODULE

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
	void cRigidPrivate::set_dynamic(bool v)
	{
		if (dynamic == v)
			return;
		assert(!phy_rigid);
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

	void cRigidPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		assert(node);
	}

	void cRigidPrivate::on_removed()
	{
		node = nullptr;
	}

	void cRigidPrivate::on_entered_world()
	{
		phy_scene = entity->world->get_system_t<sPhysicsPrivate>();
		assert(phy_scene);

		phy_rigid = physics::Rigid::create(nullptr, dynamic);
		phy_rigid->user_data = entity;
		phy_scene->add_rigid(this);
		node->update_transform();
		phy_rigid->set_pose(node->g_pos, node->get_quat());
		phy_rigid->add_impulse(staging_impulse);
		staging_impulse = vec3(0.f);

		if (!dynamic)
		{
			node_lis = entity->add_component_data_listener([](Capture& c, uint h) {
				if (h == S<"transform"_h>)
				{
					auto thiz = c.thiz<cRigidPrivate>();
					thiz->phy_rigid->set_pose(thiz->node->g_pos, thiz->node->get_quat());
				}
			}, Capture().set_thiz(this), node);
		}
	}

	void cRigidPrivate::on_left_world()
	{
		if (!phy_rigid)
			return;

		phy_scene->remove_rigid(this);
		phy_rigid->release();
		phy_rigid = nullptr;

		if (!dynamic)
		{
			entity->remove_component_data_listener(node_lis, node);
			node_lis = nullptr;
		}
	}

	cRigid* cRigid::create()
	{
		return new cRigidPrivate();
	}
}

#endif

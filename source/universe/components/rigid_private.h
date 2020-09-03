#pragma once

#include <flame/physics/rigid.h>
#include <flame/universe/components/rigid.h>

namespace flame
{
	struct cNodePrivate;
	struct sPhysicsWorldPrivate;

	struct cRigidPrivate : cRigid // R ~ on_*
	{
		bool dynamic = true;
		physics::Rigid* rigid = nullptr;
		bool retrieving = false;

		cNodePrivate* node = nullptr; // R ref
		sPhysicsWorldPrivate* physics_world = nullptr; // R ref

		bool get_dynamic() const override { return dynamic; }
		void set_dynamic(bool v) override;

		void on_gain_node();
		void on_lost_node();
		void on_gain_physics_world();
		void on_lost_physics_world();

		void on_local_data_changed(Component* t, uint64 h) override;
	};
}

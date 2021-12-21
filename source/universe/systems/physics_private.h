#pragma once
#ifdef USE_PHYSICS_MODULE

#include "physics.h"
#include "../components/element_private.h"

namespace flame
{
	struct sPhysicsPrivate : sPhysics
	{
		struct Visualizer : ElementDrawer
		{
			physics::Scene* scene;

			uint draw(uint layer, sRendererPtr s_renderer) override;
		};

		std::vector<cRigidPrivate*> rigids;
		std::vector<cCharacterControllerPrivate*> controllers;
		cElementPrivate* visualization_layer = nullptr;
		Visualizer visualizer;

		std::unique_ptr<physics::Scene> physics_scene;

		void add_rigid(cRigidPrivate* r);
		void remove_rigid(cRigidPrivate* r);

		vec3 raycast(const vec3& origin, const vec3& dir, EntityPtr* out_e) override;
		void set_visualization(bool v) override;

		void on_added() override;
		void on_removed() override;
		void update() override;
	};
}

#endif

#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Scene
		{
			virtual void release() = 0;

			virtual void add_rigid(RigidPtr r) = 0;
			virtual void remove_rigid(RigidPtr r) = 0;
			virtual vec3 raycast(const vec3& origin, const vec3& dir, float max_distance, void** out_user_data = nullptr) = 0;
			virtual void update(float disp) = 0;
			virtual void set_trigger_callback(void (*callback)(Capture& c, TouchType type, ShapePtr trigger_shape, ShapePtr other_shape), const Capture& capture) = 0;
			virtual void set_visualization(bool v) = 0;
			virtual void get_visualization_data(uint* lines_count, graphics::Line** lines) = 0;

			FLAME_PHYSICS_EXPORTS static Scene* create(Device* device, float gravity, uint threads_count);
		};
	}
}


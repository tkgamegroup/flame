#pragma once

#include <flame/foundation/foundation.h>
#include <flame/physics/physics.h>

namespace flame
{
	namespace physics
	{
		struct Device;
		struct Rigid;
		struct Shape;

		struct Scene
		{
			virtual void release() = 0;

			virtual void add_rigid(Rigid* r) = 0;
			virtual void remove_rigid(Rigid* r) = 0;
			virtual void update(float disp) = 0;
			virtual void set_trigger_callback(void (*callback)(Capture& c, TouchType type, Shape* trigger_shape, Shape* other_shape), const Capture& capture) = 0;

			FLAME_PHYSICS_EXPORTS static Scene* create(Device* device, float gravity, uint threads_count);
		};
	}
}


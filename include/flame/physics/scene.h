#pragma once

#include "physics.h"

#include <functional>

namespace flame
{
	namespace physics
	{
		struct Device;
		struct Rigid;
		struct Shape;

		struct ScenePrivate;

		typedef std::function<void(Rigid *trigger_rigid, Shape *trigger_shape, Rigid *other_rigid, 
			Shape *other_shape, TouchType tt)> TriggerCallback;

		struct Scene
		{
			ScenePrivate *_priv;

			FLAME_PHYSICS_EXPORTS void add_rigid(Rigid *r);
			FLAME_PHYSICS_EXPORTS void remove_rigid(Rigid *r);
			FLAME_PHYSICS_EXPORTS void update(float disp);
			FLAME_PHYSICS_EXPORTS void enable_callback();
			FLAME_PHYSICS_EXPORTS void disable_callback();
			FLAME_PHYSICS_EXPORTS void set_trigger_callback(const TriggerCallback &callback);
		};

		FLAME_PHYSICS_EXPORTS Scene *create_scene(Device *d, float gravity, int thread_count);
		FLAME_PHYSICS_EXPORTS void destroy_scene(Scene *s);
	}
}


//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

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


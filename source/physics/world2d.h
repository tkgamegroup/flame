#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct World2d
		{
			virtual ~World2d() {}

			virtual void step() = 0;
			virtual void query(const vec2& lt, const vec2& rb, const std::function<void(Body2dPtr body)>& callback) = 0;
			virtual void set_contact_listener(const std::function<void(Body2dPtr bodyA, Body2dPtr bodyB)>& listener) = 0;

			struct Create
			{
				virtual World2dPtr operator()() = 0;
			};
			FLAME_PHYSICS_API static Create& create;
		};
	}
}

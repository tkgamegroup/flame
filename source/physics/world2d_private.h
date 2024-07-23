#pragma once

#include "physics_private.h"
#include "world2d.h"

namespace flame
{
	namespace physics
	{
		struct ContactListener : b2ContactListener
		{
			std::function<void(Body2dPtr bodyA, Body2dPtr bodyB)> func;

			void BeginContact(b2Contact* contact) override;
		};

		struct World2dPrivate : World2d
		{
			b2World b2_world;
			std::vector<Body2dPtr> bodies;
			ContactListener contact_listener;

			World2dPrivate() :
				b2_world(b2Vec2(0.f, 0.f))
			{
			}

			void step() override;
			void query(const vec2& lt, const vec2& rb, const std::function<void(Body2dPtr body)>& callback) override;
			void set_contact_listener(const std::function<void(Body2dPtr bodyA, Body2dPtr bodyB)>& listener) override;
		};
	}
}

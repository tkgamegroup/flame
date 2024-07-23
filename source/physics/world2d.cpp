#include "world2d_private.h"
#include "body2d_private.h"

namespace flame
{
	namespace physics
	{
		void World2dPrivate::step()
		{
			b2_world.Step(delta_time, 6, 2);

			for (auto body : bodies)
			{
				auto pos = body->b2_body->GetPosition();
				auto angle = body->b2_body->GetAngle();
				body->pos = vec2(pos.x, pos.y);
				body->angle = angle;
			}
		}
		
		struct QueryCallback : b2QueryCallback 
		{
			std::function<void(Body2dPtr body)> callback;

			bool ReportFixture(b2Fixture* fixture) 
			{
				auto b2_body = fixture->GetBody();
				callback((Body2dPtr)(b2_body->GetUserData().pointer));
				return true;
			}
		};

		void World2dPrivate::query(const vec2& lt, const vec2& rb, const std::function<void(Body2dPtr body)>& callback)
		{
			b2AABB aabb;
			aabb.lowerBound.Set(lt.x, lt.y);
			aabb.upperBound.Set(rb.x, rb.y);
			QueryCallback cb;
			cb.callback = callback;
			b2_world.QueryAABB(&cb, aabb);
		}

		struct World2dCreate : World2d::Create
		{
			World2dPtr operator()() override
			{
				auto ret = new World2dPrivate;
				return ret;
			}
		}World2d_create;
		World2d::Create& World2d::create = World2d_create;
	}
}

#include "body2d_private.h"

namespace flame
{
	namespace physics
	{
		Body2dPrivate::Body2dPrivate(World2dPtr _world) :
			world(_world)
		{
			world->bodies.push_back(this);
		}

		Body2dPrivate::~Body2dPrivate()
		{
			if (b2_body)
			{
				world->b2_world.DestroyBody(b2_body);
				std::erase_if(world->bodies, [&](const auto i) {
					return i == this;
				});
			}
		}

		float Body2dPrivate::get_mass()
		{
			if (b2_body)
				return b2_body->GetMass();
			return 0.f;
		}

		vec2 Body2dPrivate::get_velocity()
		{
			if (b2_body)
			{
				auto v = b2_body->GetLinearVelocity();
				return vec2(v.x, v.y);
			}
			return vec2(0.f);
		}

		void Body2dPrivate::set_pos(const vec2& pos)
		{
			if (b2_body)
				b2_body->SetTransform(b2Vec2(pos.x, pos.y), b2_body->GetAngle());
		}

		void Body2dPrivate::apply_force(const vec2& force)
		{
			if (b2_body)
				b2_body->ApplyForceToCenter(b2Vec2(force.x, force.y), true);
		}

		struct Body2dCreateBox : Body2d::CreateBox
		{
			Body2dPtr operator()(World2dPtr world, BodyType type, const vec2& pos, const vec2& hf_ext, float density, float friction) override
			{
				auto ret = new Body2dPrivate(world);
				b2BodyDef body_def;
				body_def.type = to_backend(type);
				body_def.position.Set(pos.x, pos.y);
				ret->b2_body = world->b2_world.CreateBody(&body_def);
				ret->b2_body->GetUserData().pointer = (uint64)ret;
				b2PolygonShape shape;
				shape.SetAsBox(hf_ext.x, hf_ext.y);
				b2FixtureDef fixture_def;
				fixture_def.shape = &shape;
				fixture_def.density = density;
				fixture_def.friction = friction;
				ret->b2_body->CreateFixture(&fixture_def);
				ret->pos = vec2(body_def.position.x, body_def.position.y);
				ret->angle = body_def.angle;
				return ret;
			}
		}Body2d_create_box;
		Body2d::CreateBox& Body2d::create_box = Body2d_create_box;

		struct Body2dCreateCircle : Body2d::CreateCircle
		{
			Body2dPtr operator()(World2dPtr world, BodyType type, const vec2& pos, float radius, float density, float friction) override
			{
				auto ret = new Body2dPrivate(world);
				b2BodyDef body_def;
				body_def.type = to_backend(type);
				body_def.position.Set(pos.x, pos.y);
				ret->b2_body = world->b2_world.CreateBody(&body_def);
				ret->b2_body->GetUserData().pointer = (uint64)ret;
				b2CircleShape shape;
				shape.m_radius = radius;
				b2FixtureDef fixture_def;
				fixture_def.shape = &shape;
				fixture_def.density = density;
				fixture_def.friction = friction;
				ret->b2_body->CreateFixture(&fixture_def);
				ret->pos = vec2(body_def.position.x, body_def.position.y);
				ret->angle = body_def.angle;
				return ret;
			}
		}Body2d_create_circle;
		Body2d::CreateCircle& Body2d::create_circle = Body2d_create_circle;
	}
}

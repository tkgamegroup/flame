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

		void Body2dPrivate::add_shape(const Shape2d& shape, float density, float friction, ushort collide_bit, ushort collide_mask)
		{
			b2PolygonShape box_shape;
			b2CircleShape circle_shape;
			b2FixtureDef fixture_def;
			switch (shape.type)
			{
			case ShapeBox:
				box_shape.SetAsBox(shape.data.box.hf_ext.x, shape.data.box.hf_ext.y, b2Vec2(shape.data.box.center.x, shape.data.box.center.y), 0.f);
				fixture_def.shape = &box_shape;
				break;
			case ShapeCircle:
				circle_shape.m_p.Set(shape.data.circle.center.x, shape.data.circle.center.y);
				circle_shape.m_radius = shape.data.circle.radius;
				fixture_def.shape = &circle_shape;
				break;
			}
			fixture_def.density = density;
			fixture_def.friction = friction;
			fixture_def.filter.categoryBits = collide_bit;
			fixture_def.filter.maskBits = collide_mask;
			b2_body->CreateFixture(&fixture_def);
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

		void Body2dPrivate::set_velocity(const vec2& vel)
		{
			if (b2_body)
				b2_body->SetLinearVelocity(b2Vec2(vel.x, vel.y));
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

		struct Body2dCreate : Body2d::Create
		{
			Body2dPtr operator()(World2dPtr world, BodyType type, const vec2& pos) override
			{
				auto ret = new Body2dPrivate(world);
				b2BodyDef body_def;
				body_def.type = to_backend(type);
				body_def.position.Set(pos.x, pos.y);
				ret->b2_body = world->b2_world.CreateBody(&body_def);
				ret->b2_body->GetUserData().pointer = (uint64)ret;
				ret->pos = pos;
				ret->angle = body_def.angle;
				return ret;
			}
		}Body2d_create;
		Body2d::Create& Body2d::create = Body2d_create;
	}
}

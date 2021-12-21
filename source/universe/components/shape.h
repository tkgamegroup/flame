#pragma once
#ifdef USE_PHYSICS_MODULE

#include "../component.h"

namespace flame
{
	struct cShape : Component
	{
		inline static auto type_name = "flame::cShape";
		inline static auto type_hash = ch(type_name);

		cShape() :
			Component(type_name, type_hash)
		{
		}

		virtual physics::ShapeType get_type() const = 0;
		virtual void set_type(physics::ShapeType t) = 0;

		// Cube size.x - hf ext.x, size.y - hf ext.y, size.z - hf ext.z
		// Sphere size.x - radius, size.y - no use, size.z - no use
		// Capsule size.x - radius, size.y - hf height, size.z - no use
		// TriangleMesh size.x - no use, size.y - no use, size.z - no use
		// HeightField size.x - no use, size.y - no use, size.z - no use
		virtual vec3 get_size() const = 0;
		virtual void set_size(const vec3& s) = 0;

		virtual float get_static_friction() const = 0;
		virtual void set_static_friction(float v) = 0;

		virtual float get_dynamic_friction() const = 0;
		virtual void set_dynamic_friction(float v) = 0;

		virtual float get_restitution() const = 0;
		virtual void set_restitution(float v) = 0;

		virtual bool get_trigger() const = 0;
		virtual void set_trigger(bool v) = 0;

		FLAME_UNIVERSE_EXPORTS static cShape* create();
	};
}

#endif

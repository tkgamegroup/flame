#pragma once

#include <flame/physics/physics.h>
#include <flame/universe/component.h>

namespace flame
{
	struct cShape : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cShape";
		inline static auto type_hash = ch(type_name);

		cShape() :
			Component(type_name, type_hash)
		{
		}

		virtual physics::ShapeType get_type() const = 0;
		virtual void set_type(physics::ShapeType t) = 0;

		FLAME_UNIVERSE_EXPORTS static cShape* create();
	};
}

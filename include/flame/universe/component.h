#pragma once

#include <flame/universe/entity.h>

namespace flame
{
	struct Entity;

	struct Component
	{
		const char* type_name;
		const uint type_hash;

		Entity* entity;

		Component(const char* name) :
			type_name(name),
			type_hash(H(name))
		{
		}

		virtual ~Component() {};
		virtual void on_added() {}
		virtual void on_other_added(Component* c) {}
		virtual void update() {};
	};

	// each component type should has a type for serialization
	// the type name is component type name + 'A$'
	// the type contains the data for serialization
	// the type should has a static function called 'create' that returns the component
	// the type should has a static function called 'save' that takes the component for parameter to save it
}

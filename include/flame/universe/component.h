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

		bool first_update;

		Component(const char* name) :
			type_name(name),
			type_hash(H(name)),
			first_update(true)
		{
		}

		virtual ~Component() {};

		virtual void on_entity_added_to_parent() {}

		virtual void start() {}
		virtual void update() {}
		virtual Component* copy() { return nullptr; }
	};

	// component type may has a type for serialization
	// the type name is 'Component' + component type name + '$'
	// the type contains the data for serialization
}

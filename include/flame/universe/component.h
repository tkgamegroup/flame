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
		virtual void on_component_added(Component* c) {}
		virtual void on_child_component_added(Component* c) {}
		virtual void update() {}
		virtual Component* copy() { return nullptr; }
	};

	// component type may has a type for serialization
	// the type name is 'Component' + component type name + '$'
	// the type contains the data for serialization
}

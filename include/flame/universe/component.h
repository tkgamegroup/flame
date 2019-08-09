#pragma once

#include <flame/type.h>
#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;

	struct Component
	{
		Entity* entity;

		virtual ~Component() {};

		virtual const char* type_name() const = 0;
		virtual uint type_hash() const = 0;

		virtual void on_attach() {}

		virtual void update() = 0;
	};

	// each component type should has a type for serialization
	// the type name is component type name + 'A$'
	// the type contains the data for serialization
	// the type should has a static function called 'create' that returns the component
	// the type should has a static function called 'save' that takes the component for parameter to save it
}

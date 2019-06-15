#pragma once

#include <flame/type.h>
#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;

	struct Component // pod
	{
		Entity* entity;

		virtual ~Component() {};

		virtual const char* type_name() const = 0;
		virtual uint type_hash() const = 0;

		virtual void on_attach() {}

		virtual void update(float delta_time) = 0;

		virtual void serialize(void* data) {}
	};

	/*
		 a derived component should have one and only one
		 static void function call 'create' that takes one 
		 void* parameter

		 for example:
		 FLAME_UNIVERSE_EXPORTS static cText$* create(void*);

		 *NOTE: the void* contents the data of type '*Archive$' which will be described below

		 if a component wants serialization (well, most of the time), 
		 you need to add a '$' at the end of the component name 
		 and make a struct call '*Archive$' (while * is the name of 
		 component).
		 in the '*Archive$', define the members you want to serialize, 
		 the member should end with a '$', and its type should be one of 
		 the CommonData's union types). 
		 the '*Archive$' will be passed to from/to_archive when you do 
		 serialize, and you don't need to allocate/free the data.

		 for example:
		 struct cShootingMachine$ : Component
		 {
				...
				float speed;
				...
		 };
		 struct cShootingMachineArchive$
		 {
				float speed$;
		 };

		 *NOTE: the 'c' at the begin of name is the cue for component, 
				don't add it if you don't like it
		 *NOTE: the value that 'type_name' and 'type_hash' returned shouldn't 
				  have the 'c' and the '$'
	*/
}

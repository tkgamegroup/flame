#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;
	struct System;

	struct World
	{
		virtual void release() = 0;

		virtual void register_object(void* o, const char* name) = 0;
		virtual void* find_object(const char* name) const = 0;

		virtual System* get_system(uint name_hash) const = 0;
		virtual void add_system(System* s) = 0;
		virtual void remove_system(System* s) = 0;

		virtual Entity* get_root() const = 0;

		virtual void update() = 0;

		FLAME_UNIVERSE_EXPORTS static World* create();
	};
}

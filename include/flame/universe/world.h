#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;
	struct System;

	struct World
	{
		virtual void release() = 0;

		//virtual Object* find_object(uint name_hash, uint id) const = 0;

		virtual System* get_system_plain(uint name_hash) const = 0;
#define get_system(T) (T*)get_system_plain(FLAME_CHASH(#T))
		virtual void add_system(System* s) = 0;
		virtual void remove_system(System* s) = 0;

		virtual void update() = 0;

		FLAME_UNIVERSE_EXPORTS static World* create();
	};
}

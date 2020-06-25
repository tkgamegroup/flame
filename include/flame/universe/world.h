#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;
	struct System;

	struct World
	{
		virtual void release() = 0;

		Object* find_object(uint name_hash, uint id) const
		{
			for (auto o : objects)
			{
				if (o->name_hash == name_hash)
				{
					if (!id || o->id == id)
						return o;
				}
			}
			return nullptr;
		}

		System* get_system_plain(uint name_hash) const
		{
			for (auto s : systems)
			{
				if (s->name_hash == name_hash)
					return s;
			}
			return nullptr;
		}

		template <class T>
		T* get_system_t(uint name_hash) const
		{
			return (T*)get_system_plain(name_hash);
		}

#define get_system(T) get_system_t<T>(FLAME_CHASH(#T))

		virtual void add_system(System* s) = 0;
		virtual void remove_system(System* s) = 0;

		virtual void update();

		FLAME_UNIVERSE_EXPORTS static World* create();
	};
}

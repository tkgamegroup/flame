#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct TypeInfoDatabase;

	struct Entity;
	struct System;

	struct World
	{
		Array<Object*> objects;

		Array<System*> systems;
		Entity* root;

		FLAME_UNIVERSE_EXPORTS World();
		FLAME_UNIVERSE_EXPORTS ~World();

		Object* find_object(uint name_hash, uint id) const
		{
			const auto& objects = ((WorldPrivate*)this)->objects;
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

		FLAME_UNIVERSE_EXPORTS System* get_system_plain(uint name_hash) const;

		template <class T>
		T* get_system_t(uint name_hash) const
		{
			return (T*)get_system_plain(name_hash);
		}

#define get_system(T) get_system_t<T>(FLAME_CHASH(#T))

		FLAME_UNIVERSE_EXPORTS void add_system(System* s);

		FLAME_UNIVERSE_EXPORTS void update();

		FLAME_UNIVERSE_EXPORTS static World* create();
		FLAME_UNIVERSE_EXPORTS static void destroy(World* w);
	};
}

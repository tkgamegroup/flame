#include "universe_private.h"

namespace flame
{
	void* universe_alloc(uint size)
	{
		return malloc(size);
	}

	void UniversePrivate::update()
	{
		for (auto& w : worlds)
		{
			w->root->update_visibility();
			for (auto& s : w->systems)
				s->update(w->root.get());
		}
	}

	void Universe::add_object(Object* o)
	{
		((UniversePrivate*)this)->objects.push_back(o);
	}

	Object* Universe::find_object(uint name_hash, uint id)
	{
		const auto& objects = ((UniversePrivate*)this)->objects;
		for (auto& o : objects)
		{
			if (o->name_hash == name_hash)
			{
				if (!id || o->id == id)
					return o;
			}
		}
		return nullptr;
	}

	void Universe::update()
	{
		((UniversePrivate*)this)->update();
	}

	Universe* Universe::create()
	{
		return new UniversePrivate;
	}

	void Universe::destroy(Universe* u)
	{
		delete (UniversePrivate*)u;
	}
}

#include <flame/foundation/foundation.h>
#include "universe_private.h"

namespace flame
{
	void* universe_alloc(uint size)
	{
		return malloc(size);
	}

	void* add_listener_plain(void* hub, void(*pf)(void* c), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c)>;
		c->function = pf;
		c->capture = capture;
		((ListenerHub*)hub)->listeners.emplace_back(c);
		return c;
	}

	void remove_listener_plain(void* hub, void* c)
	{
		auto& listeners = ((ListenerHub*)hub)->listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == c)
			{
				listeners.erase(it);
				return;
			}
		}
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

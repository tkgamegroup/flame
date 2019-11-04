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

	void Universe::add_object(Object* o, const std::string& id)
	{
		((UniversePrivate*)this)->objects.emplace_back(o, id);
	}

	Object* Universe::find_object(uint name_hash, const std::string& id)
	{
		const auto& objects = ((UniversePrivate*)this)->objects;
		for (auto& o : objects)
		{
			if (o.first->name_hash == name_hash)
			{
				if (id.empty() || o.second == id)
					return o.first;
			}
		}
		return nullptr;
	}

	const std::string* Universe::find_id(Object* _o)
	{
		const auto& objects = ((UniversePrivate*)this)->objects;
		for (auto& o : objects)
		{
			if (o.first == _o)
				return &o.second;
		}
		return nullptr;
	}

	void Universe::add_world(World* _w)
	{
		auto w = (WorldPrivate*)_w;
		w->universe_ = this;
		((UniversePrivate*)this)->worlds.emplace_back((WorldPrivate*)w);
		for (auto& s : w->systems)
			s->on_added();
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

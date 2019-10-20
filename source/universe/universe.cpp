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

	void Universe::add_world(World* w)
	{
		w->universe_ = this;
		((UniversePrivate*)this)->worlds.emplace_back((WorldPrivate*)w);
	}

	void Universe::update()
	{
		((UniversePrivate*)this)->update();
	}

	void Universe::clear_bank()
	{
		((UniversePrivate*)this)->bank.clear();
	}

	void Universe::bank_save(const std::string& key, void* v)
	{
		((UniversePrivate*)this)->bank[key] = v;
	}

	void* Universe::bank_get(const std::string& key)
	{
		return ((UniversePrivate*)this)->bank[key];
	}

	const std::string& Universe::bank_find(void* v)
	{
		const auto& bank = ((UniversePrivate*)this)->bank;
		for (auto it = bank.begin(); it != bank.end(); it++)
		{
			if (it->second == v)
				return it->first;
		}
		return "";
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

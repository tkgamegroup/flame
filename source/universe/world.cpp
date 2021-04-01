#include <flame/foundation/typeinfo.h>
#include <flame/script/script.h>
#include "entity_private.h"
#include "components/element_private.h"
#include "components/receiver_private.h"
#include "components/node_private.h"
#include "world_private.h"

namespace flame
{
	void WorldPrivate::register_object(void* o, const std::string& name)
	{
		objects.emplace_back(o, name);
	}

	void* WorldPrivate::find_object(const std::string& name) const
	{
		for (auto& o : objects)
		{
			if (o.second == name)
				return o.first;
		}
		return nullptr;
	}

	System* WorldPrivate::get_system(uint type_hash) const
	{
		for (auto& s : systems)
		{
			if (s->type_hash == type_hash)
				return s.get();
		}
		return nullptr;
	}

	System* WorldPrivate::find_system(const std::string& _name) const
	{
		System* ret = nullptr;
		auto name = _name;
		for (auto& s : systems)
		{
			if (s->type_name == name)
			{
				ret = s.get();
				break;
			}
		}
		name = "flame::" + _name;
		for (auto& s : systems)
		{
			if (s->type_name == name)
			{
				ret = s.get();
				break;
			}
		}
		if (ret)
		{
			auto script = script::Instance::get_default();
			script->push_string(name.c_str());
			script->set_global_name("__type__");
		}
		return ret;
	}

	void WorldPrivate::add_system(System* s)
	{
		fassert(!s->world);
		s->world = this;
		systems.emplace_back(s);
		s->on_added();
	}

	void WorldPrivate::remove_system(System* s)
	{
		for (auto it = systems.begin(); it != systems.end(); it++)
		{
			if ((*it)->type_hash == s->type_hash)
			{
				systems.erase(it);
				return;
			}
		}
	}

	Entity* WorldPrivate::get_element_root()
	{
		if (!element_root)
		{
			element_root.reset(f_new<EntityPrivate>());
			element_root->global_visibility = true;
			element_root->add_component(cElement::create());
			auto cer = cReceiver::create();
			cer->set_ignore_occluders(true);
			element_root->add_component(cer);
			element_root->on_entered_world(this);
		}
		return element_root.get();
	}

	Entity* WorldPrivate::get_node_root()
	{
		if (!node_root)
		{
			node_root.reset(f_new<EntityPrivate>());
			node_root->global_visibility = true;
			node_root->add_component(cNode::create());
			node_root->on_entered_world(this);
		}
		return node_root.get();
	}

	void WorldPrivate::update()
	{
		for (auto& s : systems)
		{
			for (auto& l : update_listeners)
				l->call(s.get(), true);
			s->update();
			for (auto& l : update_listeners)
				l->call(s.get(), false);
		}
	}

	void* WorldPrivate::add_update_listener(void (*callback)(Capture& c, System* system, bool before), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		update_listeners.emplace_back(c);
		return c;
	}

	void WorldPrivate::remove_update_listener(void* ret)
	{
		std::erase_if(update_listeners, [&](const auto& i) {
			return i == (decltype(i))ret;
		});
	}

	World* World::create()
	{
		return new WorldPrivate;
	}
}

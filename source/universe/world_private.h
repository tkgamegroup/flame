#pragma once

#include <flame/universe/world.h>
#include <flame/universe/system.h>
#include "universe_private.h"
#include "entity_private.h"

namespace flame
{
	struct WorldPrivate : World
	{
		std::vector<Object*> _objects;

		std::vector<std::unique_ptr<System, Delecter>> _systems;
		std::unique_ptr<EntityPrivate, Delecter> _root;

		WorldPrivate();

		void _register_object(Object* o);
		Object* _find_object(uint name_hash) const;

		System* _get_system(uint name_hash) const;
		void _add_system(System* s);
		void _remove_system(System* s);

		void _update();

		void release() override { delete this; }

		void register_object(Object* o) override { _register_object(o); }
		Object* find_object(uint name_hash) const override { return _find_object(name_hash); }

		System* get_system(uint name_hash) const override { return _get_system(name_hash); }
		void add_system(System* s) override { _add_system(s); }
		void remove_system(System* s) override { _remove_system(s); }

		Entity* get_root() const override { return _root.get(); }

		void update() override { _update(); }
	};
}

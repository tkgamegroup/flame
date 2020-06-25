#pragma once

#include <flame/universe/entity.h>
#include "universe_private.h"

namespace flame
{
	struct EntityPrivate : Entity
	{
		std::string _name;
		uint _name_hash;

		bool _visible;
		bool _global_visibility;

		void* _gene;

		World* _world;

		Entity* _parent;
		std::unordered_map<uint, std::unique_ptr<Component, Delecter>> _components;
		std::vector<std::unique_ptr<EntityPrivate, Delecter>> _children;

		std::vector<std::unique_ptr<Closure<void(Capture& c, EntityEvent e, void* t)>>> _event_listeners;
		ListenerHub<bool(Capture& c, uint hash, void* sender)> _local_data_changed_listeners;
		ListenerHub<bool(Capture& c, uint hash, void* sender)> _child_data_changed_listeners;

		uint _depth;
		uint _index;
		int _created_frame;
		std::vector<void*> _created_stack;
		bool _dying;

		void mark_dying()
		{
			_dying = true;
			for (auto& c : _children)
				c->mark_dying();
		}
	};
}

#pragma once

#include <flame/universe/entity.h>
#include "universe_private.h"

namespace flame
{
	struct EntityPrivate : Entity
	{
		std::string _name;
		uint _name_hash;

		bool _visible = true;
		bool _global_visibility = false;

		void* _gene = nullptr;

		World* _world = nullptr;

		Entity* _parent = nullptr;
		std::unordered_map<uint, std::unique_ptr<Component, Delecter>> _components;
		std::vector<std::unique_ptr<EntityPrivate, Delecter>> _children;

		std::vector<std::unique_ptr<Closure<void(Capture& c, EntityEvent e, void* t)>>> _event_listeners;
		ListenerHub<bool(Capture& c, uint hash, void* sender)> _local_data_changed_listeners;
		ListenerHub<bool(Capture& c, uint hash, void* sender)> _child_data_changed_listeners;

		uint _depth = 0;
		uint _index = 0;
		int _created_frame;
		std::vector<void*> _created_stack;
		bool _dying = false;

		EntityPrivate();
		~EntityPrivate();

		void mark_dying();

		void update_visibility();

		void data_changed(Component* c, uint hash, void* sender) override;
	};
}

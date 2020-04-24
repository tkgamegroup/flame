#pragma once

#include <flame/universe/utils/entity.h>

namespace flame
{
	namespace utils
	{
		static Entity* _current_root;
		static Entity* _current_entity;
		static std::stack<Entity*> _parents;

		Entity* current_root()
		{
			return _current_root;
		}

		void set_current_root(Entity* e)
		{
			_current_root = e;
		}

		Entity* current_entity()
		{
			return _current_entity;
		}

		void set_current_entity(Entity* e)
		{
			_current_entity = e;
		}

		Entity* current_parent()
		{
			return _parents.empty() ? nullptr : _parents.top();
		}

		void push_parent(Entity* parent)
		{
			_parents.push(parent);
		}

		void pop_parent()
		{
			_parents.pop();
		}

		Entity* next_entity = nullptr;
		uint next_component_id = 0;
	}
}

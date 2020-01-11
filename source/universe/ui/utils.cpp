#include <flame/universe/ui/utils.h>

namespace flame
{
	namespace ui
	{
		static std::stack<Entity*> _current_parents;
		static Entity* _current_entity;

		Entity* current_parent()
		{
			return _current_parents.top();
		}

		void push_parent(Entity* parent)
		{
			_current_parents.push(parent);
		}

		void pop_parent()
		{
			_current_parents.pop();
		}
		
		Entity* current_entity()
		{
			return _current_entity;
		}

		void set_current_entity(Entity* e)
		{
			_current_entity = e;
		}
	}
}

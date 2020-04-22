#pragma once

#include <flame/universe/components/timer.h>
#include <flame/universe/components/data_keeper.h>

namespace flame
{
	namespace utils
	{
		Entity* current_root();
		void set_current_root(Entity* e);
		Entity* current_entity();
		void set_current_entity(Entity* e);
		Entity* current_parent();
		void push_parent(Entity* parent);
		void pop_parent();

		extern Entity* next_entity;
		extern uint next_component_id;

		inline cTimer* c_timer()
		{
			auto c = cTimer::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cDataKeeper* c_data_keeper()
		{
			auto c = cDataKeeper::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}
	}
}

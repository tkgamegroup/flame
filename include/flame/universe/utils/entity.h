#pragma once

#include <flame/universe/components/timer.h>
#include <flame/universe/components/data_keeper.h>

namespace flame
{
	namespace utils
	{
		FLAME_UNIVERSE_EXPORTS Entity* current_root();
		FLAME_UNIVERSE_EXPORTS void set_current_root(Entity* e);
		FLAME_UNIVERSE_EXPORTS Entity* current_entity();
		FLAME_UNIVERSE_EXPORTS void set_current_entity(Entity* e);
		FLAME_UNIVERSE_EXPORTS Entity* current_parent();
		FLAME_UNIVERSE_EXPORTS void push_parent(Entity* parent);
		FLAME_UNIVERSE_EXPORTS void pop_parent();

		FLAME_UNIVERSE_EXPORTS extern Entity* next_entity;
		FLAME_UNIVERSE_EXPORTS extern uint next_component_id;

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

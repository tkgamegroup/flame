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

		inline cTimer* c_timer(float interval)
		{
			auto c = cTimer::create();
			c->interval = interval;
			current_entity()->add_component(c);
			return c;
		}

		inline cDataKeeper* c_data_keeper()
		{
			auto c = cDataKeeper::create();
			current_entity()->add_component(c);
			return c;
		}
	}
}

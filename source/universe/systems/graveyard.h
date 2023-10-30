#pragma once

#include "../universe.h"

namespace flame
{
	struct Graveyard
	{
		float duration = 5.f; // seconds
		virtual void set_duration(float v) = 0;

		virtual void add(EntityPtr e) = 0;
		virtual void clear() = 0;

		struct Instance
		{
			virtual GraveyardPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;
	};
}

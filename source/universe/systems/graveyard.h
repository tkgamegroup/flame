#pragma once

#include "../system.h"

namespace flame
{
	struct sGraveyard : System
	{
		float duration = 5.f; // seconds
		virtual void set_duration(float v) = 0;

		virtual void add(EntityPtr e) = 0;
		virtual void clear() = 0;

		struct Instance
		{
			virtual sGraveyardPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sGraveyardPtr operator()(WorldPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}

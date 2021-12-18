#pragma once

#include "world.h"
#include "system.h"
#include "entity_private.h"

namespace flame
{
	struct WorldPrivate : World
	{
		WorldPrivate();

		void add_system(System* s) override;
		void remove_system(System* s, bool destroy) override;

		void update() override;
	};
}

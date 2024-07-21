#pragma once

#include "world.h"
#include "system.h"
#include "entity_private.h"

namespace flame
{
	struct WorldPrivate : World
	{
		WorldPrivate();

		System* add_system(uint hash, int pos = -1) override;
		void add_system_p(System* system, int pos = -1) override;
		void remove_system(uint hash, bool destroy) override;

		void update() override;

		void bordcast(EntityPtr root, uint msg, void* data1, void* data2) override;
	};
}

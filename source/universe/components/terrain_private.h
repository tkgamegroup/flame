#pragma once

#include "terrain.h"
#include "node_private.h"

namespace flame
{
	struct cTerrainPrivate : cTerrain
	{
		~cTerrainPrivate();
		void on_init() override;

		int instance_id = -1;

		void on_active() override;
		void on_inactive() override;
	};
}

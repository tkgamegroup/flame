#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "terrain_private.h"
#include "../systems/node_renderer_private.h"

namespace flame
{
	struct cTerrainCreatePrivate : cTerrain::Create
	{
		cTerrainPtr operator()(EntityPtr e) override
		{
			return new cTerrainPrivate();
		}
	}cTerrain_create_private;
	cTerrain::Create& cTerrain::create = cTerrain_create_private;
}

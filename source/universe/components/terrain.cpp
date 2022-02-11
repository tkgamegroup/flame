#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "terrain_private.h"
#include "../systems/node_renderer_private.h"

namespace flame
{
	cTerrainPrivate::~cTerrainPrivate()
	{

	}

	void cTerrainPrivate::on_init()
	{

	}

	void cTerrainPrivate::on_active()
	{
		instance_id = sNodeRenderer::instance()->register_terrain_instance(-1);

		node->mark_transform_dirty();
	}

	void cTerrainPrivate::on_inactive()
	{
		sNodeRenderer::instance()->register_terrain_instance(instance_id);
		instance_id = -1;
	}

	struct cTerrainCreatePrivate : cTerrain::Create
	{
		cTerrainPtr operator()(EntityPtr e) override
		{
			return new cTerrainPrivate();
		}
	}cTerrain_create_private;
	cTerrain::Create& cTerrain::create = cTerrain_create_private;
}

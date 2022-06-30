#pragma once

#include "scene.h"
#include "../components/node_private.h"

#ifdef USE_RECASTNAV
#include <DetourCrowd.h>
#include <DetourTileCache.h>
#include <Recast.h>
extern dtTileCache* dt_tile_cache;
extern dtNavMesh* dt_nav_mesh;
extern dtNavMeshQuery* dt_nav_query;
extern dtQueryFilter dt_filter;
extern dtCrowd* dt_crowd;

dtPolyRef dt_nearest_poly(const vec3& pos);
#endif

namespace flame
{
	struct sScenePrivate : sScene
	{
	#ifdef USE_RECASTNAV
	#endif

		sScenePrivate();
		~sScenePrivate();

		void update_transform(EntityPtr e, bool mark_dirty);

		void generate_nav_mesh() override;
		std::vector<vec3> query_nav_path(const vec3& start, const vec3& end) override;
		void get_debug_draw(DrawData& draw_data) override;

		void update() override;
	};
}

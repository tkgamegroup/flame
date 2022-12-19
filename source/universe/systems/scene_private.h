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

dtPolyRef dt_nearest_poly(const vec3& pos, const vec3& ext, vec3* pt = nullptr);
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

		void generate_navmesh(float agent_radius, float agent_height, float walkable_climb, float walkable_slope_angle) override;
		std::vector<vec3> query_navmesh_path(const vec3& start, const vec3& end, uint max_smooth) override;
		bool navmesh_nearest_point(const vec3& check, const vec3& ext, vec3& res) override;
		void get_debug_draw(DrawData& draw_data) override;

		void update() override;
	};
}

#pragma once

#include "scene.h"
#include "../components/node_private.h"

#ifdef USE_RECASTNAV
struct rcHeightfield;
struct rcCompactHeightfield;
struct rcContourSet;
struct rcPolyMesh;
struct rcPolyMeshDetail;

struct dtNavMesh;
struct dtNavMeshQuery;
struct dtCrowd;
#endif

namespace flame
{
	struct sScenePrivate : sScene
	{
	#ifdef USE_RECASTNAV
		rcHeightfield* rc_height_field = nullptr;
		rcCompactHeightfield* rc_c_height_field = nullptr;
		rcContourSet* rc_contour_set = nullptr;
		rcPolyMesh* rc_poly_mesh = nullptr;
		rcPolyMeshDetail* rc_poly_mesh_d = nullptr;

		dtNavMesh* dt_nav_mesh = nullptr;
		dtNavMeshQuery* dt_nav_query = nullptr;
		dtCrowd* dt_crowd = nullptr;
	#endif

		sScenePrivate();

		void update_transform(EntityPtr e, bool mark_dirty);

		void generate_navmesh(const std::filesystem::path& output) override;
		std::vector<vec3> navmesh_calc_path(const vec3& start, const vec3& end) override;

		void update() override;
	};
}

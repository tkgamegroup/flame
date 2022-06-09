#pragma once

#include "scene.h"
#include "../components/node_private.h"

#ifdef USE_RECASTNAV
#include <DetourCrowd.h>
#include <Recast.h>
extern rcHeightfield* rc_height_field;
extern rcCompactHeightfield* rc_c_height_field;
extern rcContourSet* rc_contour_set;
extern rcPolyMesh* rc_poly_mesh;
extern rcPolyMeshDetail* rc_poly_mesh_d;
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
		std::vector<vec3> calc_nav_path(const vec3& start, const vec3& end) override;

		void update() override;
	};
}

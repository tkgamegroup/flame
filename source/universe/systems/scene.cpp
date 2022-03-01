#include "../../graphics/image.h"
#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "../components/node_private.h"
#include "../components/mesh_private.h"
#include "../components/terrain_private.h"
#include "../octree.h"
#include "scene_private.h"

#ifdef USE_RECASTNAV
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>
#include <DetourCrowd.h>
#include <Recast.h>
rcContext rc_ctx;
#endif

namespace flame
{
#ifdef USE_RECASTNAV
	static void init_dt_crowd(sScenePrivate* scene)
	{
		scene->dt_crowd->init(128, 2.f/*max agent radius*/, scene->dt_nav_mesh);
	}
#endif

	sScenePrivate::sScenePrivate()
	{
		octree = new OctNode(999999999.f, vec3(0.f));

#ifdef USE_RECASTNAV
		dt_nav_mesh = dtAllocNavMesh();
		dtNavMeshParams parms;
		memset(&parms, 0, sizeof(dtNavMeshParams));
		parms.maxPolys = 1;
		parms.maxTiles = 1;
		parms.tileWidth = 0.1f;
		parms.tileHeight = 0.1f;
		dt_nav_mesh->init(&parms);
		dt_nav_query = dtAllocNavMeshQuery();
		dt_filter = new dtQueryFilter;
		dt_crowd = dtAllocCrowd();
		init_dt_crowd(this);
#endif
	}

	sScenePrivate::~sScenePrivate()
	{
#ifdef USE_RECASTNAV
		dtFreeNavMesh(dt_nav_mesh);
		dtFreeNavMeshQuery(dt_nav_query);
		dtFreeCrowd(dt_crowd);
#endif
	}

	void sScenePrivate::update_transform(EntityPtr e, bool mark_dirty)
	{
		if (!e->global_enable)
			return;

		auto is_static = false;
		if (auto node = e->get_component_i<cNodeT>(0); node)
		{
			is_static = (int)node->is_static == 2;
			if (!is_static)
			{
				if (node->is_static)
					node->is_static = 2;
				if (mark_dirty)
					node->mark_transform_dirty();
				if (node->update_transform())
				{
					if (!node->measurers.list.empty())
					{
						node->bounds.reset();
						for (auto m : node->measurers.list)
						{
							AABB b;
							if (m.first(&b))
								node->bounds.expand(b);
						}
					}
					else if (!node->drawers.list.empty())
						node->bounds = AABB(AABB(vec3(0.f), 10000.f).get_points(node->transform));
					if (node->bounds.invalid())
					{
						if (node->octnode)
							node->octnode->remove(node);
					}
					else
					{
						if (node->octnode)
							node->octnode->add(node);
						else
							octree->add(node);
					}

					mark_dirty = true;
				}
			}
		}

		if (!is_static)
		{
			for (auto& c : e->children)
				update_transform(c.get(), mark_dirty);
		}
	}

	void sScenePrivate::generate_nav_mesh()
	{
		std::vector<vec3> positions;
		std::vector<uint> indices;
		AABB bounds;

		std::function<void(EntityPtr e)> get_meshes;
		get_meshes = [&](EntityPtr e) {
			if (!e->global_enable)
				return;

			if (auto node = e->get_component_i<cNode>(0); node)
			{
				if (auto mesh = e->get_component_t<cMesh>(); mesh)
				{

				}
				if (auto terrain = e->get_component_t<cTerrain>(); terrain)
				{
					auto node = terrain->node;
					auto g_pos = node->g_pos;
					auto g_scl = node->g_scl;

					auto& textures = terrain->textures;
					if (textures)
					{
						auto blocks = terrain->blocks;
						auto tess_level = terrain->tess_level;
						auto cx = blocks.x * tess_level;
						auto cz = blocks.y * tess_level;
						auto extent = terrain->extent;
						extent.x /= cx;
						extent.z /= cz;

						auto n0 = positions.size();
						positions.resize(n0 + (cx + 1) * (cz + 1));
						for (auto z = 0; z < cz + 1; z++)
						{
							for (auto x = 0; x < cx + 1; x++)
							{
								positions[n0 + z * (cx + 1) + x] = vec3(x * extent.x * g_scl.x,
									textures->linear_sample(vec2((float)x / cx, (float)z / cz)).x * extent.y * g_scl.y,
									z * extent.z * g_scl.z) + g_pos;
							}
						}
						auto n1 = indices.size();
						indices.resize(n1 + cx * cz * 6);
						for (auto z = 0; z < cz; z++)
						{
							for (auto x = 0; x < cx; x++)
							{
								auto s1 = x % tess_level < tess_level / 2 ? 1 : -1;
								auto s2 = z % tess_level < tess_level / 2 ? 1 : -1;
								auto dst = &indices[n1 + (z * cx + x) * 6];
								if (s1 * s2 > 0)
								{
									dst[0] = z * (cx + 1) + x;
									dst[1] = (z + 1) * (cx + 1) + x;
									dst[2] = (z + 1) * (cx + 1) + x + 1;

									dst[3] = z * (cx + 1) + x;
									dst[4] = (z + 1) * (cx + 1) + x + 1;
									dst[5] = z * (cx + 1) + x + 1;
								}
								else
								{
									dst[0] = z * (cx + 1) + x;
									dst[1] = (z + 1) * (cx + 1) + x;
									dst[2] = z * (cx + 1) + x + 1;

									dst[3] = z * (cx + 1) + x + 1;
									dst[4] = (z + 1) * (cx + 1) + x;
									dst[5] = (z + 1) * (cx + 1) + x + 1;
								}
							}
						}
					}
				}

				for (auto& c : e->children)
					get_meshes(c.get());
			}
		};

		rcFreeHeightField(rc_height_field); rc_height_field = nullptr;
		rcFreeCompactHeightfield(rc_c_height_field); rc_c_height_field = nullptr;
		rcFreeContourSet(rc_contour_set); rc_contour_set = nullptr;
		rcFreePolyMesh(rc_poly_mesh); rc_poly_mesh = nullptr;
		rcFreePolyMeshDetail(rc_poly_mesh_d); rc_poly_mesh_d = nullptr;

		get_meshes(world->root.get());
		for (auto& p : positions) bounds.expand(p);

		auto agnent_height = 2.f;
		auto agnet_riadius = 0.6f;
		auto agnet_max_climb = 0.9f;

		rcConfig rc_cfg;
		memset(&rc_cfg, 0, sizeof(rcConfig));
		rc_cfg.cs = 0.3f;
		rc_cfg.ch = 0.2f;
		rc_cfg.walkableSlopeAngle = 45.f;
 		rc_cfg.walkableHeight = (int)ceil(agnent_height / rc_cfg.ch);
		rc_cfg.walkableClimb = (int)ceil(agnet_max_climb / rc_cfg.ch);
		rc_cfg.walkableRadius = (int)ceil(agnet_riadius / rc_cfg.cs);
		rc_cfg.maxEdgeLen = (int)(/*edge max len*/12.f / rc_cfg.cs);
		rc_cfg.maxSimplificationError = /*edge max error*/1.3f;
		rc_cfg.minRegionArea = (int)square(/*region min size*/8.f);
		rc_cfg.mergeRegionArea = (int)square(/*region merge size*/20.f);
		rc_cfg.maxVertsPerPoly = /*verts per poly*/6;
		rc_cfg.detailSampleDist = (int)/*detail sample dist*/6.f;
		if (rc_cfg.detailSampleDist < 0.9f) rc_cfg.detailSampleDist *= rc_cfg.cs;
		rc_cfg.detailSampleMaxError = rc_cfg.ch * /*detail sample max error*/1.f;
		memcpy(&rc_cfg.bmin, &bounds.a, sizeof(vec3));
		memcpy(&rc_cfg.bmax, &bounds.b, sizeof(vec3));

		rcCalcGridSize(rc_cfg.bmin, rc_cfg.bmax, rc_cfg.cs, &rc_cfg.width, &rc_cfg.height);

		rc_height_field = rcAllocHeightfield();
		if (!rcCreateHeightfield(&rc_ctx, *rc_height_field, rc_cfg.width, rc_cfg.height, rc_cfg.bmin, rc_cfg.bmax, rc_cfg.cs, rc_cfg.ch))
		{
			printf("generate navmesh: Could not create solid heightfield.\n");
			return;
		}

		std::vector<uchar> triareas(indices.size() / 3);
		for (auto i = 0; i < triareas.size(); i++) triareas[i] = 0;
		rcMarkWalkableTriangles(&rc_ctx, rc_cfg.walkableSlopeAngle, (float*)positions.data(), positions.size(), 
			(int*)indices.data(), triareas.size(), triareas.data());
		if (!rcRasterizeTriangles(&rc_ctx, (float*)positions.data(), positions.size(), (int*)indices.data(), triareas.data(), triareas.size(), *rc_height_field, rc_cfg.walkableClimb))
		{
			printf("generate navmesh: Could not rasterize triangles.\n");
			return;
		}

		rcFilterLowHangingWalkableObstacles(&rc_ctx, rc_cfg.walkableClimb, *rc_height_field);
		rcFilterLedgeSpans(&rc_ctx, rc_cfg.walkableHeight, rc_cfg.walkableClimb, *rc_height_field);
		rcFilterWalkableLowHeightSpans(&rc_ctx, rc_cfg.walkableHeight, *rc_height_field);

		rc_c_height_field = rcAllocCompactHeightfield();
		if (!rcBuildCompactHeightfield(&rc_ctx, rc_cfg.walkableHeight, rc_cfg.walkableClimb, *rc_height_field, *rc_c_height_field))
		{
			printf("generate navmesh: Could not build compact data.\n");
			return;
		}

		if (!rcErodeWalkableArea(&rc_ctx, rc_cfg.walkableRadius, *rc_c_height_field))
		{
			printf("generate navmesh: Could not erode.\n");
			return;
		}

		if (!rcBuildDistanceField(&rc_ctx, *rc_c_height_field))
		{
			printf("generate navmesh: Could not build distance field.\n");
			return;
		}

		if (!rcBuildRegions(&rc_ctx, *rc_c_height_field, 0, rc_cfg.minRegionArea, rc_cfg.mergeRegionArea))
		{
			printf("generate navmesh: Could not build watershed regions.\n");
			return;
		}

		rc_contour_set = rcAllocContourSet();
		if (!rcBuildContours(&rc_ctx, *rc_c_height_field, rc_cfg.maxSimplificationError, rc_cfg.maxEdgeLen, *rc_contour_set))
		{
			printf("generate navmesh: Could not create contours.\n");
			return;
		}

		rc_poly_mesh = rcAllocPolyMesh();
		if (!rcBuildPolyMesh(&rc_ctx, *rc_contour_set, rc_cfg.maxVertsPerPoly, *rc_poly_mesh))
		{
			printf("generate navmesh: Could not triangulate contours.\n");
			return;
		}

		rc_poly_mesh_d = rcAllocPolyMeshDetail();
		if (!rcBuildPolyMeshDetail(&rc_ctx, *rc_poly_mesh, *rc_c_height_field, rc_cfg.detailSampleDist, rc_cfg.detailSampleMaxError, *rc_poly_mesh_d))
		{
			printf("generate navmesh: Could not build detail mesh.\n");
			return;
		}

		if (rc_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
		{
			for (auto i = 0; i < rc_poly_mesh->npolys; ++i)
			{
				rc_poly_mesh->flags[i] = 1;
			//	if (rc_poly_mesh->areas[i] == RC_WALKABLE_AREA)
			//		rc_poly_mesh->areas[i] = POLYAREA_GROUND;

			//	if (rc_poly_mesh->areas[i] == POLYAREA_GROUND ||
			//		rc_poly_mesh->areas[i] == POLYAREA_GRASS ||
			//		rc_poly_mesh->areas[i] == POLYAREA_ROAD)
			//	{
			//		rc_poly_mesh->flags[i] = POLYFLAGS_WALK;
			//	}
			//	else if (rc_poly_mesh->areas[i] == POLYAREA_WATER)
			//	{
			//		rc_poly_mesh->flags[i] = POLYFLAGS_SWIM;
			//	}
			//	else if (rc_poly_mesh->areas[i] == POLYAREA_DOOR)
			//	{
			//		rc_poly_mesh->flags[i] = POLYFLAGS_WALK | POLYFLAGS_DOOR;
			//	}
			}

			dtNavMeshCreateParams parms;
			memset(&parms, 0, sizeof(parms));
			parms.verts = rc_poly_mesh->verts;
			parms.vertCount = rc_poly_mesh->nverts;
			parms.polys = rc_poly_mesh->polys;
			parms.polyAreas = rc_poly_mesh->areas;
			parms.polyFlags = rc_poly_mesh->flags;
			parms.polyCount = rc_poly_mesh->npolys;
			parms.nvp = rc_poly_mesh->nvp;
			parms.detailMeshes = rc_poly_mesh_d->meshes;
			parms.detailVerts = rc_poly_mesh_d->verts;
			parms.detailVertsCount = rc_poly_mesh_d->nverts;
			parms.detailTris = rc_poly_mesh_d->tris;
			parms.detailTriCount = rc_poly_mesh_d->ntris;
			parms.walkableHeight = agnent_height;
			parms.walkableRadius = agnet_riadius;
			parms.walkableClimb = agnet_max_climb;
			memcpy(parms.bmin, rc_poly_mesh->bmin, sizeof(vec3));
			memcpy(parms.bmax, rc_poly_mesh->bmax, sizeof(vec3));
			parms.cs = rc_cfg.cs;
			parms.ch = rc_cfg.ch;
			parms.buildBvTree = true;

			uchar* navData = nullptr;
			auto navDataSize = 0;
			if (!dtCreateNavMeshData(&parms, &navData, &navDataSize))
			{
				printf("generate navmesh: Could not build Detour navmesh data.\n");
				return;
			}

			dtFreeNavMesh(dt_nav_mesh);
			dt_nav_mesh = dtAllocNavMesh();
			init_dt_crowd(this);

			dtStatus status;

			status = dt_nav_mesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
			if (dtStatusFailed(status))
			{
				dtFree(navData);
				printf("generate navmesh: Could not init Detour navmesh.\n");
				return;
			}

			status = dt_nav_query->init(dt_nav_mesh, 2048);
			if (dtStatusFailed(status))
			{
				printf("generate navmesh: Could not init Detour navmesh query.\n");
				return;
			}
		}
	}

	inline bool in_range(const vec3& v1, const vec3& v2, const float r, const float h)
	{
		auto d = v2 - v1;
		return (d.x * d.x + d.z * d.z) < r * r && fabsf(d.y) < h;
	}

	static int fixup_corridor(dtPolyRef* path, int npath, int max_path,
		const dtPolyRef* visited, int nvisited)
	{
		auto furthest_path = -1;
		auto furthest_visited = -1;

		for (auto i = npath - 1; i >= 0; --i)
		{
			auto found = false;
			for (auto j = nvisited - 1; j >= 0; --j)
			{
				if (path[i] == visited[j])
				{
					furthest_path = i;
					furthest_visited = j;
					found = true;
				}
			}
			if (found)
				break;
		}

		if (furthest_path == -1 || furthest_visited == -1)
			return npath;

		auto req = nvisited - furthest_visited;
		auto orig = rcMin(furthest_path + 1, npath);
		auto size = rcMax(0, npath - orig);
		if (req + size > max_path)
			size = max_path - req;
		if (size)
			memmove(path + req, path + orig, size * sizeof(dtPolyRef));

		for (auto i = 0; i < req; ++i)
			path[i] = visited[(nvisited - 1) - i];

		return req + size;
	}

	static int fixup_shortcuts(dtPolyRef* path, int npath, dtNavMesh* nav_mesh)
	{
		if (npath < 3)
			return npath;

		const auto MaxNeis = 16;
		dtPolyRef neis[MaxNeis];
		auto nneis = 0;

		const dtMeshTile* tile = 0;
		const dtPoly* poly = 0;
		if (dtStatusFailed(nav_mesh->getTileAndPolyByRef(path[0], &tile, &poly)))
			return npath;

		for (auto k = poly->firstLink; k != DT_NULL_LINK; k = tile->links[k].next)
		{
			const dtLink* link = &tile->links[k];
			if (link->ref != 0)
			{
				if (nneis < MaxNeis)
					neis[nneis++] = link->ref;
			}
		}

		const auto MaxLookAhead = 6;
		auto cut = 0;
		for (auto i = min(MaxLookAhead, npath) - 1; i > 1 && cut == 0; i--) {
			for (auto j = 0; j < nneis; j++)
			{
				if (path[i] == neis[j]) {
					cut = i;
					break;
				}
			}
		}
		if (cut > 1)
		{
			auto offset = cut - 1;
			npath -= offset;
			for (auto i = 1; i < npath; i++)
				path[i] = path[i + offset];
		}

		return npath;
	}

	static bool get_steer_target(dtNavMeshQuery* nav_query, const vec3& start_pos, const vec3& end_pos,
		float min_target_dist, const dtPolyRef* path, int path_size,
		vec3& steer_pos, uchar& steer_pos_flag, dtPolyRef& steer_pos_ref, std::vector<vec3>* out_points = nullptr)
	{
		const auto MAX_STEER_POINTS = 3;
		vec3 steer_path[MAX_STEER_POINTS];
		uchar steer_path_flags[MAX_STEER_POINTS];
		dtPolyRef steer_path_polys[MAX_STEER_POINTS];
		auto n_steer_ath = 0;
		nav_query->findStraightPath(&start_pos[0], &end_pos[0], path, path_size, 
			&steer_path[0][0], steer_path_flags, steer_path_polys, &n_steer_ath, MAX_STEER_POINTS);
		if (!n_steer_ath)
			return false;

		if (out_points)
		{
			out_points->resize(n_steer_ath);
			for (auto i = 0; i < n_steer_ath; ++i)
				(*out_points)[i] = steer_path[i];
		}

		auto ns = 0;
		while (ns < n_steer_ath)
		{
			// Stop at Off-Mesh link or when point is further than slop away.
			if ((steer_path_flags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ||
				!in_range(steer_path[ns], start_pos, min_target_dist, 1000.0f))
				break;
			ns++;
		}
		if (ns >= n_steer_ath)
			return false;

		steer_pos = steer_path[ns];
		steer_pos.y = start_pos.y;
		steer_pos_flag = steer_path_flags[ns];
		steer_pos_ref = steer_path_polys[ns];

		return true;
	}

	uint sScenePrivate::nav_mesh_nearest_poly(const vec3& pos)
	{
		dtPolyRef ret = 0;
		const auto poly_pick_ext = vec3(2.f, 4.f, 2.f);
		dt_nav_query->findNearestPoly(&pos[0], &poly_pick_ext[0], dt_filter, &ret, nullptr);
		return ret;
	}

	std::vector<vec3> sScenePrivate::calc_nav_path(const vec3& start, const vec3& end)
	{
		std::vector<vec3> ret;

		dtPolyRef start_ref = nav_mesh_nearest_poly(start);
		dtPolyRef end_ref = nav_mesh_nearest_poly(end);

		if (!start_ref || !end_ref)
			return ret;

		const auto MaxPolys = 256;
		dtPolyRef polys[MaxPolys];
		auto n_polys = 0;
		dt_nav_query->findPath(start_ref, end_ref, &start[0], &end[0], dt_filter, polys, &n_polys, MaxPolys);
		if (!n_polys)
			return ret;

		vec3 iter_pos, target_pos;
		dt_nav_query->closestPointOnPoly(start_ref, &start[0], &iter_pos[0], 0);
		dt_nav_query->closestPointOnPoly(polys[n_polys - 1], &end[0], &target_pos[0], 0);

		const auto StepSize = 0.5f;
		const auto Slop = 0.01f;

		ret.push_back(iter_pos);

		const auto MaxSmooth = 2048;
		while (n_polys && ret.size() < MaxSmooth)
		{
			vec3 steer_pos;
			uchar steer_pos_flag;
			dtPolyRef steer_pos_ref;

			if (!get_steer_target(dt_nav_query, iter_pos, target_pos, Slop,
				polys, n_polys, steer_pos, steer_pos_flag, steer_pos_ref))
				break;

			bool end_of_path = (steer_pos_flag & DT_STRAIGHTPATH_END) ? true : false;
			bool off_mesh_connection = (steer_pos_flag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ? true : false;

			auto delta = steer_pos - iter_pos;
			auto len = sqrtf(dot(delta, delta));
			if ((end_of_path || off_mesh_connection) && len < StepSize)
				len = 1;
			else
				len = StepSize / len;
			auto moveTgt = iter_pos + delta * len;

			vec3 result;
			dtPolyRef visited[16];
			auto nvisited = 0;
			dt_nav_query->moveAlongSurface(polys[0], &iter_pos[0], &moveTgt[0], dt_filter,
				&result[0], visited, &nvisited, 16);

			n_polys = fixup_corridor(polys, n_polys, MaxPolys, visited, nvisited);
			n_polys = fixup_shortcuts(polys, n_polys, dt_nav_mesh);

			float h = 0;
			dt_nav_query->getPolyHeight(polys[0], &result[0], &h);
			result[1] = h;
			iter_pos = result;

			if (end_of_path && in_range(iter_pos, steer_pos, Slop, 1.0f))
			{
				iter_pos = target_pos;
				if (ret.size() < MaxSmooth)
					ret.push_back(iter_pos);
				break;
			}
			else if (off_mesh_connection && in_range(iter_pos, steer_pos, Slop, 1.0f))
			{
				vec3 startPos, endPos;

				dtPolyRef prevRef = 0, polyRef = polys[0];
				auto npos = 0;
				while (npos < n_polys && polyRef != steer_pos_ref)
				{
					prevRef = polyRef;
					polyRef = polys[npos];
					npos++;
				}
				for (auto i = npos; i < n_polys; ++i)
					polys[i - npos] = polys[i];
				n_polys -= npos;

				auto status = dt_nav_mesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, &startPos[0], &endPos[0]);
				if (dtStatusSucceed(status))
				{
					if (ret.size() < MaxSmooth)
					{
						ret.push_back(startPos);
						if (ret.size() % 2 == 1)
							ret.push_back(startPos);
					}
					iter_pos = endPos;
					float eh = 0.0f;
					dt_nav_query->getPolyHeight(polys[0], &iter_pos[0], &eh);
					iter_pos[1] = eh;
				}
			}

			if (ret.size() < MaxSmooth)
				ret.push_back(iter_pos);
		}

		return ret;
	}

	void sScenePrivate::update()
	{
		update_transform(world->root.get(), false);

#ifdef USE_RECASTNAV
		if (dt_nav_mesh)
			dt_crowd->update(delta_time, nullptr);
#endif
	}

	static sScenePtr _instance = nullptr;

	struct sSceneInstance : sScene::Instance
	{
		sScenePtr operator()() override
		{
			return _instance;
		}
	}sScene_instance_private;
	sScene::Instance& sScene::instance = sScene_instance_private;

	struct sSceneCreate : sScene::Create
	{
		sScenePtr operator()(WorldPtr w) override
		{
			if (!w)
				return nullptr;

			assert(!_instance);
			_instance = new sScenePrivate();
			return _instance;
		}
	}sScene_create_private;
	sScene::Create& sScene::create = sScene_create_private;
}

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
	sScenePrivate::sScenePrivate()
	{
		octree = new OctNode(999999999.f, vec3(0.f));

#ifdef USE_RECASTNAV
		dt_nav_query = dtAllocNavMeshQuery();
		dt_crowd = dtAllocCrowd();
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

	void sScenePrivate::generate_navmesh(const std::filesystem::path& output)
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
								positions[n0 + z * (cx + 1) + x] = vec3(x * extent.x,
									textures->linear_sample(vec2((float)x / cx, (float)z / cz)).x * extent.y,
									z * extent.z);
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
		dtFreeNavMesh(dt_nav_mesh); dt_nav_mesh = nullptr;

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
			unsigned char* navData = 0;
			int navDataSize = 0;

			// Update poly flags from areas. TODO
			//for (int i = 0; i < rc_poly_mesh->npolys; ++i)
			//{
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
			//}

			dtNavMeshCreateParams params;
			memset(&params, 0, sizeof(params));
			params.verts = rc_poly_mesh->verts;
			params.vertCount = rc_poly_mesh->nverts;
			params.polys = rc_poly_mesh->polys;
			params.polyAreas = rc_poly_mesh->areas;
			params.polyFlags = rc_poly_mesh->flags;
			params.polyCount = rc_poly_mesh->npolys;
			params.nvp = rc_poly_mesh->nvp;
			params.detailMeshes = rc_poly_mesh_d->meshes;
			params.detailVerts = rc_poly_mesh_d->verts;
			params.detailVertsCount = rc_poly_mesh_d->nverts;
			params.detailTris = rc_poly_mesh_d->tris;
			params.detailTriCount = rc_poly_mesh_d->ntris;
			params.walkableHeight = agnent_height;
			params.walkableRadius = agnet_riadius;
			params.walkableClimb = agnet_max_climb;
			memcpy(params.bmin, rc_poly_mesh->bmin, sizeof(vec3));
			memcpy(params.bmax, rc_poly_mesh->bmax, sizeof(vec3));
			params.cs = rc_cfg.cs;
			params.ch = rc_cfg.ch;
			params.buildBvTree = true;

			if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
			{
				printf("generate navmesh: Could not build Detour navmesh.\n");
				return;
			}

			dt_nav_mesh = dtAllocNavMesh();
			if (!dt_nav_mesh)
			{
				dtFree(navData);
				printf("generate navmesh: Could not create Detour navmesh.\n");
				return;
			}

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

	void sScenePrivate::update()
	{
		update_transform(world->root.get(), false);
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

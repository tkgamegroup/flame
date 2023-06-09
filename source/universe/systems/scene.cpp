#include "../../graphics/image.h"
#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "../components/node_private.h"
#include "../components/element_private.h"
#include "../components/layout_private.h"
#include "../components/mesh_private.h"
#include "../components/terrain_private.h"
#include "../components/volume_private.h"
#include "../components/nav_agent_private.h"
#include "../components/nav_obstacle_private.h"
#include "../octree.h"
#include "../draw_data.h"
#include "scene_private.h"
#include "renderer_private.h"

#ifdef USE_RECASTNAV

#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>
#include <DetourCrowd.h>
#include <DetourTileCache.h>
#include <DetourTileCacheBuilder.h>
rcContext rc_ctx;
dtTileCache* dt_tile_cache = nullptr;
dtNavMesh* dt_nav_mesh = nullptr;
dtNavMeshQuery* dt_nav_query = nullptr;
dtQueryFilter dt_filter;
dtCrowd* dt_crowd = nullptr;

#define EXPECTED_LAYERS_PER_TILE 4
#define MAX_LAYERS 32

dtPolyRef dt_nearest_poly(const vec3& pos, const vec3& ext, vec3* pt)
{
	dtPolyRef ret = 0;
	dt_nav_query->findNearestPoly(&pos[0], &ext[0], &dt_filter, &ret, &pt->x);
	return ret;
}

#endif

namespace flame
{
	sScenePrivate::sScenePrivate()
	{
		octree = new OctNode(999999999.f, vec3(0.f));
	}

	sScenePrivate::~sScenePrivate()
	{
#ifdef USE_RECASTNAV
		if (dt_nav_mesh)
			dtFreeNavMesh(dt_nav_mesh);
		if (dt_nav_query)
			dtFreeNavMeshQuery(dt_nav_query);
		if (dt_crowd)
			dtFreeCrowd(dt_crowd);
#endif
	}

	static void update_node_transform(OctNode* octree, EntityPtr e, bool mark_dirty)
	{
		if (!e->global_enable)
			return;

		auto is_static = false;
		if (auto node = e->node(); node)
		{
			is_static = node->static_state == Static;
			if (!is_static)
			{
				if (node->static_state == StaticButDirty)
					node->static_state = Static;
				if (mark_dirty)
					node->mark_transform_dirty();
				if (node->update_transform())
				{
					if (node->measurers)
					{
						node->bounds.reset();
						node->measurers.call<AABB&>(node->bounds);
					}
					else if (node->drawers)
						node->bounds = AABB(AABB(vec3(0.f), 10000.f).get_points(node->transform));
					if (node->octnode)
						node->octnode->remove(node);
					if (!node->bounds.invalid())
						octree->add(node);

					mark_dirty = true;
				}
			}
		}

		if (!is_static)
		{
			for (auto& c : e->children)
				update_node_transform(octree, c.get(), mark_dirty);
		}
	}

	static void update_alignment(cElementPtr element, const vec2& parent_ext, const vec4& padding)
	{
		switch (element->horizontal_alignment)
		{
		case ElementAlignCenter:
			element->set_x((parent_ext.x - (padding.x + padding.z) - element->ext.x) * 0.5f + element->alignment_offset.x);
			break;
		case ElementAlignEnd0:
			element->set_x(padding.x + element->alignment_offset.x);
			break;
		case ElementAlignEnd1:
			element->set_x(parent_ext.x - padding.z - element->ext.x - element->alignment_offset.x);
			break;
		case ElementAlignFill:
			element->set_x(element->alignment_offset.x);
			element->set_w(parent_ext.x - padding.x + padding.z - element->alignment_offset.x * 2.f);
			break;
		}

		switch (element->vertical_alignment)
		{
		case ElementAlignCenter:
			element->set_y((parent_ext.y - (padding.y + padding.w) - element->ext.y) * 0.5f + element->alignment_offset.y);
			break;
		case ElementAlignEnd0:
			element->set_y(padding.y + element->alignment_offset.y);
			break;
		case ElementAlignEnd1:
			element->set_y(parent_ext.y - padding.w - element->ext.y - element->alignment_offset.y);
			break;
		case ElementAlignFill:
			element->set_y(element->alignment_offset.y);
			element->set_h(parent_ext.y - (padding.y + padding.w) - element->alignment_offset.y * 2.f);
			break;
		}
	}

	static void update_element_transform(cLayoutPtr playout, EntityPtr e, bool mark_dirty)
	{
		if (!e->global_enable)
			return;

		auto is_static = false;
		if (auto element = e->element(); element)
		{
			is_static = element->static_state == Static;
			if (!is_static)
			{
				if (element->static_state == StaticButDirty)
					element->static_state = Static;

				if (mark_dirty)
					element->mark_transform_dirty();

				if (element->transform_dirty)
				{
					if (playout)
					{
						if (element->horizontal_alignment == ElementAlignNone || element->vertical_alignment == ElementAlignNone)
							playout->update_layout();
					}

					if (auto pelement = element->entity->get_parent_component_i<cElementT>(0); pelement)
						update_alignment(element, pelement->ext, playout ? playout->padding : vec4(0.f));
				}

				if (element->update_transform())
					mark_dirty = true;
			}
		}

		if (!is_static)
		{
			auto layout = e->get_component_t<cLayoutT>();
			for (auto& c : e->children)
				update_element_transform(layout, c.get(), mark_dirty);
		}
	}

	namespace navmesh_gen_detail
	{
		struct MyTileCacheAllocator : dtTileCacheAlloc
		{
			uchar* buffer = nullptr;
			size_t capacity = 0;
			size_t top = 0;
			size_t high = 0;

			MyTileCacheAllocator(uint cap)
			{
				resize(cap);
			}

			~MyTileCacheAllocator()
			{
				dtFree(buffer);
			}

			void resize(uint cap)
			{
				if (buffer) dtFree(buffer);
				buffer = (uchar*)dtAlloc(cap, DT_ALLOC_PERM);
				capacity = cap;
			}

			void reset() override
			{
				high = max(high, top);
				top = 0;
			}

			void* alloc(const size_t size) override
			{
				if (!buffer)
					return nullptr;
				if (top + size > capacity)
					return nullptr;
				uchar* mem = &buffer[top];
				top += size;
				return mem;
			}

			void free(void*) override
			{
			}
		};

		auto my_title_cache_allocator = new MyTileCacheAllocator(32000);

		struct MyTileCacheCompressor : public dtTileCacheCompressor
		{
			int maxCompressedSize(const int bufferSize) override
			{
				return bufferSize;
			}

			dtStatus compress(const uchar* buffer, const int bufferSize,
				uchar* compressed, const int /*maxCompressedSize*/, int* compressedSize) override
			{
				memcpy(compressed, buffer, bufferSize);
				*compressedSize = bufferSize;
				return DT_SUCCESS;
			}

			dtStatus decompress(const uchar* compressed, const int compressedSize,
				uchar* buffer, const int maxBufferSize, int* bufferSize) override
			{
				memcpy(buffer, compressed, compressedSize);
				*bufferSize = compressedSize;
				return DT_SUCCESS;
			}
		};

		auto my_tile_cache_compressor = new MyTileCacheCompressor;

		struct MyTileCacheMeshProcess : public dtTileCacheMeshProcess
		{
			void process(struct dtNavMeshCreateParams* params, uchar* polyAreas, ushort* polyFlags) override
			{
				for (int i = 0; i < params->polyCount; ++i)
				{
					if (polyAreas[i] == DT_TILECACHE_WALKABLE_AREA)
						polyAreas[i] = 0;

					polyFlags[i] = 1;
				}
			}
		};

		auto my_tile_cache_mesh_process = new MyTileCacheMeshProcess;

		struct ChunkyTriMeshNode
		{
			float bmin[2];
			float bmax[2];
			int i;
			int n;
		};

		struct ChunkyTriMesh
		{
			ChunkyTriMeshNode* nodes = nullptr;
			int nnodes = 0;
			int* tris = nullptr;
			int ntris = 0;
			int max_tris_per_chunk = 0;

			~ChunkyTriMesh()
			{
				delete[] nodes;
				delete[] tris;
			}
		};

		bool create_chunky_tri_mesh(const float* verts, const int* tris, int ntris, int tris_per_chunk, ChunkyTriMesh* cm)
		{
			struct BoundsItem
			{
				float bmin[2];
				float bmax[2];
				int i;
			};

			auto compareItemX = [](const void* va, const void* vb) {
				const BoundsItem* a = (const BoundsItem*)va;
				const BoundsItem* b = (const BoundsItem*)vb;
				if (a->bmin[0] < b->bmin[0])
					return -1;
				if (a->bmin[0] > b->bmin[0])
					return 1;
				return 0;
			};

			auto compareItemY = [](const void* va, const void* vb) {
				const BoundsItem* a = (const BoundsItem*)va;
				const BoundsItem* b = (const BoundsItem*)vb;
				if (a->bmin[1] < b->bmin[1])
					return -1;
				if (a->bmin[1] > b->bmin[1])
					return 1;
				return 0;
			};

			auto calcExtends = [](const BoundsItem* items, const int, const int imin, const int imax, float* bmin, float* bmax) {
				bmin[0] = items[imin].bmin[0];
				bmin[1] = items[imin].bmin[1];

				bmax[0] = items[imin].bmax[0];
				bmax[1] = items[imin].bmax[1];

				for (int i = imin + 1; i < imax; ++i)
				{
					const BoundsItem& it = items[i];
					if (it.bmin[0] < bmin[0]) bmin[0] = it.bmin[0];
					if (it.bmin[1] < bmin[1]) bmin[1] = it.bmin[1];

					if (it.bmax[0] > bmax[0]) bmax[0] = it.bmax[0];
					if (it.bmax[1] > bmax[1]) bmax[1] = it.bmax[1];
				}
			};

			auto longestAxis = [](float x, float y) {
				return y > x ? 1 : 0;
			};

			std::function<void(BoundsItem*, int, int, int, int, int&, ChunkyTriMeshNode*, const int, int&, int*, const int*)> subdivide;
			subdivide = [&](BoundsItem* items, int nitems, int imin, int imax, int trisPerChunk,
				int& curNode, ChunkyTriMeshNode* nodes, const int maxNodes, int& curTri, int* outTris, const int* inTris) {
					int inum = imax - imin;
					int icur = curNode;

					if (curNode >= maxNodes)
						return;

					ChunkyTriMeshNode& node = nodes[curNode++];

					if (inum <= trisPerChunk)
					{
						calcExtends(items, nitems, imin, imax, node.bmin, node.bmax);

						node.i = curTri;
						node.n = inum;

						for (int i = imin; i < imax; ++i)
						{
							const int* src = &inTris[items[i].i * 3];
							int* dst = &outTris[curTri * 3];
							curTri++;
							dst[0] = src[0];
							dst[1] = src[1];
							dst[2] = src[2];
						}
					}
					else
					{
						calcExtends(items, nitems, imin, imax, node.bmin, node.bmax);

						int	axis = longestAxis(node.bmax[0] - node.bmin[0],
							node.bmax[1] - node.bmin[1]);

						if (axis == 0)
							qsort(items + imin, static_cast<size_t>(inum), sizeof(BoundsItem), compareItemX);
						else if (axis == 1)
							qsort(items + imin, static_cast<size_t>(inum), sizeof(BoundsItem), compareItemY);

						int isplit = imin + inum / 2;

						subdivide(items, nitems, imin, isplit, trisPerChunk, curNode, nodes, maxNodes, curTri, outTris, inTris);
						subdivide(items, nitems, isplit, imax, trisPerChunk, curNode, nodes, maxNodes, curTri, outTris, inTris);

						int iescape = curNode - icur;
						node.i = -iescape;
					}
			};

			int nchunks = (ntris + tris_per_chunk - 1) / tris_per_chunk;

			cm->nodes = new ChunkyTriMeshNode[nchunks * 4];
			if (!cm->nodes)
				return false;

			cm->tris = new int[ntris * 3];
			if (!cm->tris)
				return false;

			cm->ntris = ntris;

			auto items = new BoundsItem[ntris];
			if (!items)
				return false;

			for (int i = 0; i < ntris; i++)
			{
				const int* t = &tris[i * 3];
				BoundsItem& it = items[i];
				it.i = i;
				it.bmin[0] = it.bmax[0] = verts[t[0] * 3 + 0];
				it.bmin[1] = it.bmax[1] = verts[t[0] * 3 + 2];
				for (int j = 1; j < 3; ++j)
				{
					const float* v = &verts[t[j] * 3];
					if (v[0] < it.bmin[0]) it.bmin[0] = v[0];
					if (v[2] < it.bmin[1]) it.bmin[1] = v[2];

					if (v[0] > it.bmax[0]) it.bmax[0] = v[0];
					if (v[2] > it.bmax[1]) it.bmax[1] = v[2];
				}
			}

			int curTri = 0;
			int curNode = 0;
			subdivide(items, ntris, 0, ntris, tris_per_chunk, curNode, cm->nodes, nchunks * 4, curTri, cm->tris, tris);

			delete[] items;

			cm->nnodes = curNode;

			cm->max_tris_per_chunk = 0;
			for (int i = 0; i < cm->nnodes; ++i)
			{
				ChunkyTriMeshNode& node = cm->nodes[i];
				const bool isLeaf = node.i >= 0;
				if (!isLeaf) continue;
				if (node.n > cm->max_tris_per_chunk)
					cm->max_tris_per_chunk = node.n;
			}

			return true;
		}

		int get_chunks_overlapping_rect(const ChunkyTriMesh* cm, float bmin[2], float bmax[2], int* ids, const int max_ids)
		{
			int i = 0;
			int n = 0;
			while (i < cm->nnodes)
			{
				auto check_overlap_rect = [](const float amin[2], const float amax[2], const float bmin[2], const float bmax[2]) {
					bool overlap = true;
					overlap = (amin[0] > bmax[0] || amax[0] < bmin[0]) ? false : overlap;
					overlap = (amin[1] > bmax[1] || amax[1] < bmin[1]) ? false : overlap;
					return overlap;
				};

				auto node = &cm->nodes[i];
				auto overlap = check_overlap_rect(bmin, bmax, node->bmin, node->bmax);
				auto is_leaf = node->i >= 0;

				if (is_leaf && overlap)
				{
					if (n < max_ids)
					{
						ids[n] = i;
						n++;
					}
				}

				if (overlap || is_leaf)
					i++;
				else
				{
					const int escapeIndex = -node->i;
					i += escapeIndex;
				}
			}

			return n;
		}
	}

	void sScenePrivate::generate_navmesh(float agent_radius, float agent_height, float walkable_climb, float walkable_slope_angle)
	{
#ifdef USE_RECASTNAV
		std::vector<vec3> positions;
		std::vector<uint> indices;

		std::function<void(EntityPtr e)> form_mesh;
		form_mesh = [&](EntityPtr e) {
			if (!e->global_enable)
				return;

			if (auto node = e->node(); node)
			{
				auto& mat = node->transform;

				if (e->tag & TagMarkNavMesh)
				{
					if (auto cmesh = e->get_component_t<cMesh>(); cmesh)
					{
						auto mesh = cmesh->mesh;

						auto pos_off = positions.size();
						positions.resize(positions.size() + mesh->positions.size());
						for (auto i = 0; i < mesh->positions.size(); i++)
							positions[pos_off + i] = mat * vec4(mesh->positions[i], 1.f);
						auto idx_off = indices.size();
						indices.resize(indices.size() + mesh->indices.size());
						for (auto i = 0; i < mesh->indices.size(); i++)
							indices[idx_off + i] = pos_off + mesh->indices[i];
					}
					if (auto terrain = e->get_component_t<cTerrain>(); terrain)
					{
						if (auto height_map = terrain->height_map; height_map)
						{
							auto blocks = terrain->blocks;
							auto tess_level = terrain->tess_level;
							auto cx = blocks.x * tess_level;
							auto cz = blocks.y * tess_level;
							auto extent = terrain->extent;
							extent.x /= cx;
							extent.z /= cz;

							auto pos_off = positions.size();
							positions.resize(pos_off + (cx + 1) * (cz + 1));
							for (auto z = 0; z < cz + 1; z++)
							{
								for (auto x = 0; x < cx + 1; x++)
								{
									positions[pos_off + z * (cx + 1) + x] = mat * vec4(x * extent.x,
										height_map->linear_sample(vec2((float)x / cx, (float)z / cz)).x * extent.y,
										z * extent.z, 1.f);
								}
							}
							auto idx_off = indices.size();
							indices.resize(idx_off + cx * cz * 6);
							for (auto z = 0; z < cz; z++)
							{
								for (auto x = 0; x < cx; x++)
								{
									auto s1 = x % tess_level < tess_level / 2 ? 1 : -1;
									auto s2 = z % tess_level < tess_level / 2 ? 1 : -1;
									auto dst = &indices[idx_off + (z * cx + x) * 6];
									if (s1 * s2 > 0)
									{
										dst[0] = pos_off + z * (cx + 1) + x;
										dst[1] = pos_off + (z + 1) * (cx + 1) + x;
										dst[2] = pos_off + (z + 1) * (cx + 1) + x + 1;

										dst[3] = pos_off + z * (cx + 1) + x;
										dst[4] = pos_off + (z + 1) * (cx + 1) + x + 1;
										dst[5] = pos_off + z * (cx + 1) + x + 1;
									}
									else
									{
										dst[0] = pos_off + z * (cx + 1) + x;
										dst[1] = pos_off + (z + 1) * (cx + 1) + x;
										dst[2] = pos_off + z * (cx + 1) + x + 1;

										dst[3] = pos_off + z * (cx + 1) + x + 1;
										dst[4] = pos_off + (z + 1) * (cx + 1) + x;
										dst[5] = pos_off + (z + 1) * (cx + 1) + x + 1;
									}
								}
							}
						}
					}
					if (auto volume = e->get_component_t<cVolume>(); volume && volume->marching_cubes)
					{
						graphics::Queue::get()->wait_idle();

						auto volume_vretices = sRenderer::instance()->transform_feedback(e->node());
						auto pos_off = positions.size();
						positions.resize(positions.size() + volume_vretices.size());
						for (auto i = 0; i < volume_vretices.size(); i++)
							positions[pos_off + i] = mat * vec4(volume_vretices[i], 1.f);
						auto idx_off = indices.size();
						indices.resize(indices.size() + volume_vretices.size());
						for (auto i = 0; i < volume_vretices.size(); i++)
							indices[idx_off + i] = pos_off + i;
					}
				}

				for (auto& c : e->children)
					form_mesh(c.get());
			}
		};

		form_mesh(world->root.get());
		if (positions.empty())
		{
			printf("generate navmesh: no vertices.\n");
			return;
		}

		AABB bounds;
		for (auto& p : positions)
			bounds.expand(p);

		auto cell_size = 0.3f;
		auto cell_height = 0.2f;
		auto tile_size = 48.f;

		auto edge_max_error = 1.3f;

		int gw = 0, gh = 0;
		rcCalcGridSize(&bounds.a[0], &bounds.b[0], cell_size, &gw, &gh);
		const int ts = (int)tile_size;
		const int tw = (gw + ts - 1) / ts;
		const int th = (gh + ts - 1) / ts;

		rcConfig cfg;
		memset(&cfg, 0, sizeof(rcConfig));
		cfg.cs = cell_size;
		cfg.ch = cell_height;
		cfg.walkableSlopeAngle = walkable_slope_angle;
		cfg.walkableHeight = (int)ceil(agent_height / cfg.ch);
		cfg.walkableClimb = (int)ceil(walkable_climb / cfg.ch);
		cfg.walkableRadius = (int)ceil(agent_radius / cfg.cs);
		cfg.maxEdgeLen = (int)(/*edge max len*/12.f / cfg.cs);
		cfg.maxSimplificationError = edge_max_error;
		cfg.minRegionArea = (int)square(/*region min size*/8.f);
		cfg.mergeRegionArea = (int)square(/*region merge size*/20.f);
		cfg.maxVertsPerPoly = /*verts per poly*/6;
		cfg.tileSize = (int)tile_size;
		cfg.borderSize = cfg.walkableRadius + 3;
		cfg.width = cfg.tileSize + cfg.borderSize * 2;
		cfg.height = cfg.tileSize + cfg.borderSize * 2;
		cfg.detailSampleDist = /*detail sample dist*/6.f;
		cfg.detailSampleDist = cfg.detailSampleDist < 0.9f ? 0 : cfg.detailSampleDist * cfg.cs;
		cfg.detailSampleMaxError = cfg.ch * /*detail sample max error*/1.f;
		memcpy(&cfg.bmin, &bounds.a, sizeof(vec3));
		memcpy(&cfg.bmax, &bounds.b, sizeof(vec3));

		dtFreeTileCache(dt_tile_cache);
		dt_tile_cache = dtAllocTileCache();
		dtTileCacheParams tcparams;
		memset(&tcparams, 0, sizeof(dtTileCacheParams));
		rcVcopy(tcparams.orig, &bounds.a[0]);
		tcparams.cs = cell_size;
		tcparams.ch = cell_height;
		tcparams.width = (int)tile_size;
		tcparams.height = (int)tile_size;
		tcparams.walkableHeight = agent_height;
		tcparams.walkableRadius = agent_radius;
		tcparams.walkableClimb = walkable_climb;
		tcparams.maxSimplificationError = edge_max_error;
		tcparams.maxTiles = tw * th * EXPECTED_LAYERS_PER_TILE;
		tcparams.maxObstacles = 128;
		if (auto status = dt_tile_cache->init(&tcparams, 
			navmesh_gen_detail::my_title_cache_allocator, navmesh_gen_detail::my_tile_cache_compressor, navmesh_gen_detail::my_tile_cache_mesh_process); 
			dtStatusFailed(status))
		{
			printf("generate navmesh: Could not init tile cache.\n");
			return;
		}

		dtFreeNavMesh(dt_nav_mesh);
		dt_nav_mesh = dtAllocNavMesh();
		dtNavMeshParams params;
		memset(&params, 0, sizeof(params));
		rcVcopy(params.orig, &bounds.a[0]);
		params.tileWidth = tile_size * cell_size;
		params.tileHeight = tile_size * cell_size;
		auto tile_bits = 10;
		auto poly_bits = 22 - tile_bits;
		params.maxTiles = 1 << tile_bits;
		params.maxPolys = 1 << poly_bits;
		if (dtStatusFailed(dt_nav_mesh->init(&params)))
		{
			printf("generate navmesh: Could not init navmesh.\n");
			return;
		}

		auto chunky_mesh = new navmesh_gen_detail::ChunkyTriMesh;
		navmesh_gen_detail::create_chunky_tri_mesh((float*)positions.data(), (int*)indices.data(), indices.size() / 3, 256, chunky_mesh);

		for (auto y = 0; y < th; y++)
		{
			for (auto x = 0; x < tw; x++)
			{
				struct TileCacheData
				{
					uchar* data;
					int size;
				};

				TileCacheData tiles[MAX_LAYERS];
				memset(tiles, 0, sizeof(tiles));

				auto tcs = tile_size * cell_size;

				rcConfig tcfg;
				memcpy(&tcfg, &cfg, sizeof(tcfg));

				tcfg.bmin[0] = cfg.bmin[0] + x * tcs;
				tcfg.bmin[2] = cfg.bmin[2] + y * tcs;
				tcfg.bmax[0] = cfg.bmin[0] + (x + 1) * tcs;
				tcfg.bmax[1] = cfg.bmax[1];
				tcfg.bmax[2] = cfg.bmin[2] + (y + 1) * tcs;
				tcfg.bmin[0] -= tcfg.borderSize * tcfg.cs;
				tcfg.bmin[2] -= tcfg.borderSize * tcfg.cs;
				tcfg.bmax[0] += tcfg.borderSize * tcfg.cs;
				tcfg.bmax[2] += tcfg.borderSize * tcfg.cs;

				auto solid = rcAllocHeightfield();
				if (!rcCreateHeightfield(&rc_ctx, *solid, tcfg.width, tcfg.height, tcfg.bmin, tcfg.bmax, tcfg.cs, tcfg.ch))
				{
					printf("generate navmesh: Could not create solid heightfield.\n");
					return;
				}

				auto triareas = new uchar[chunky_mesh->max_tris_per_chunk];

				float tbmin[2], tbmax[2];
				tbmin[0] = tcfg.bmin[0];
				tbmin[1] = tcfg.bmin[2];
				tbmax[0] = tcfg.bmax[0];
				tbmax[1] = tcfg.bmax[2];
				int cid[512];
				const int ncid = get_chunks_overlapping_rect(chunky_mesh, tbmin, tbmax, cid, 512);
				if (ncid)
				{
					for (int i = 0; i < ncid; ++i)
					{
						auto& node = chunky_mesh->nodes[cid[i]];
						auto tris = &chunky_mesh->tris[node.i * 3];
						auto ntris = node.n;

						memset(triareas, 0, ntris * sizeof(uchar));
						rcMarkWalkableTriangles(&rc_ctx, tcfg.walkableSlopeAngle,
							(float*)positions.data(), positions.size(), tris, ntris, triareas);

						if (!rcRasterizeTriangles(&rc_ctx, (float*)positions.data(), positions.size(), tris, triareas, ntris, *solid, tcfg.walkableClimb))
						{
							printf("generate navmesh: Could not rasterize triangles.\n");
							return;
						}
					}

					rcFilterLowHangingWalkableObstacles(&rc_ctx, tcfg.walkableClimb, *solid);
					rcFilterLedgeSpans(&rc_ctx, tcfg.walkableHeight, tcfg.walkableClimb, *solid);
					rcFilterWalkableLowHeightSpans(&rc_ctx, tcfg.walkableHeight, *solid);

					auto chf = rcAllocCompactHeightfield();
					if (!rcBuildCompactHeightfield(&rc_ctx, tcfg.walkableHeight, tcfg.walkableClimb, *solid, *chf))
					{
						printf("generate navmesh: Could not build compact data.\n");
						return;
					}
					if (!rcErodeWalkableArea(&rc_ctx, tcfg.walkableRadius, *chf))
					{
						printf("generate navmesh: Could not erode.\n");
						return;
					}

					auto lset = rcAllocHeightfieldLayerSet();
					if (!rcBuildHeightfieldLayers(&rc_ctx, *chf, tcfg.borderSize, tcfg.walkableHeight, *lset))
					{
						printf("generate navmesh: Could not build heighfield layers.\n");
						return;
					}

					auto ntiles = 0;
					for (int i = 0; i < min(lset->nlayers, MAX_LAYERS); i++)
					{
						auto tile = &tiles[ntiles++];
						auto layer = &lset->layers[i];

						dtTileCacheLayerHeader header;
						header.magic = DT_TILECACHE_MAGIC;
						header.version = DT_TILECACHE_VERSION;

						header.tx = x;
						header.ty = y;
						header.tlayer = i;
						memcpy(header.bmin, layer->bmin, sizeof(float) * 3);
						memcpy(header.bmax, layer->bmax, sizeof(float) * 3);

						header.width = (uchar)layer->width;
						header.height = (uchar)layer->height;
						header.minx = (uchar)layer->minx;
						header.maxx = (uchar)layer->maxx;
						header.miny = (uchar)layer->miny;
						header.maxy = (uchar)layer->maxy;
						header.hmin = (ushort)layer->hmin;
						header.hmax = (ushort)layer->hmax;

						if (dtStatusFailed(dtBuildTileCacheLayer(navmesh_gen_detail::my_tile_cache_compressor, 
							&header, layer->heights, layer->areas, layer->cons,
							&tile->data, &tile->size)))
						{
							printf("generate navmesh: Could not build tile cache layer.\n");
							return;
						}
					}

					for (int i = 0; i < ntiles; i++)
					{
						auto tile = &tiles[i];
						if (dtStatusFailed(dt_tile_cache->addTile(tile->data, tile->size, DT_COMPRESSEDTILE_FREE_DATA, 0)))
						{
							dtFree(tile->data);
							tile->data = 0;
							continue;
						}
					}
				}
			}
		}

		delete chunky_mesh;

		for (int y = 0; y < th; y++)
		{
			for (int x = 0; x < tw; x++)
				dt_tile_cache->buildNavMeshTilesAt(x, y, dt_nav_mesh);
		}

		if (!dt_nav_query)
			dt_nav_query = dtAllocNavMeshQuery();
		dt_nav_query->init(dt_nav_mesh, 2048);

		if (!dt_crowd)
			dt_crowd = dtAllocCrowd();
		for (auto ag : nav_agents)
			ag->dt_id = -1;
		for (auto ob : nav_obstacles)
			ob->dt_id = -1;
		dt_crowd->init(128, 2.f/*max agent radius*/, dt_nav_mesh);
		dtObstacleAvoidanceParams avoid_params;
		memcpy(&avoid_params, dt_crowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams));
		avoid_params.velBias = 0.5f;
		avoid_params.adaptiveDivs = 7;
		avoid_params.adaptiveRings = 2;
		avoid_params.adaptiveDepth = 3;
		dt_crowd->setObstacleAvoidanceParams(0, &avoid_params);
#endif
	}

	bool sScenePrivate::navmesh_nearest_point(const vec3& center, const vec3& ext, vec3& res)
	{
		if (!dt_nav_query)
			return false;

		dtPolyRef poly = 0;
		if (auto status = dt_nav_query->findNearestPoly(&center[0], &ext[0], &dt_filter, &poly, &res[0]); dtStatusFailed(status))
		{
			printf("navmesh nearest point: failed\n");
			return false;
		}
		if (!poly)
			return false;
		return true;
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
		auto orig = min(furthest_path + 1, npath);
		auto size = max(0, npath - orig);
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

	std::vector<vec3> sScenePrivate::query_navmesh_path(const vec3& start, const vec3& end, uint max_smooth)
	{
		std::vector<vec3> ret;
		if (!dt_nav_query)
			return ret;

		dtPolyRef start_ref = dt_nearest_poly(start, vec3(2.f, 4.f, 2.f));
		dtPolyRef end_ref = dt_nearest_poly(end, vec3(2.f, 4.f, 2.f));

		if (!start_ref || !end_ref)
			return ret;

		const auto MaxPolys = 256;
		dtPolyRef polys[MaxPolys];
		auto n_polys = 0;
		dt_nav_query->findPath(start_ref, end_ref, &start[0], &end[0], &dt_filter, polys, &n_polys, MaxPolys);
		if (!n_polys)
			return ret;

		vec3 iter_pos, target_pos;
		dt_nav_query->closestPointOnPoly(start_ref, &start[0], &iter_pos[0], 0);
		dt_nav_query->closestPointOnPoly(polys[n_polys - 1], &end[0], &target_pos[0], 0);

		const auto StepSize = 0.5f;
		const auto Slop = 0.01f;

		ret.push_back(iter_pos);
		if (max_smooth <= 2)
		{
			ret.push_back(target_pos);
			return ret;
		}

		while (n_polys && ret.size() < max_smooth)
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
			dt_nav_query->moveAlongSurface(polys[0], &iter_pos[0], &moveTgt[0], &dt_filter,
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
				if (ret.size() < max_smooth)
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
					if (ret.size() < max_smooth)
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

			if (ret.size() < max_smooth)
				ret.push_back(iter_pos);
		}

		return ret;
	}

	bool sScenePrivate::navmesh_check_agents_and_obstacles(const vec3& pos, float radius)
	{
		for (auto ag : nav_agents)
		{
			if (distance(ag->node->pos.xz(), pos.xz()) < ag->radius + radius)
				return false;	
		}
		for (auto ob : nav_obstacles)
		{
			if (distance(ob->node->pos.xz(), pos.xz()) < ob->radius + radius)
				return false;
		}
		return true;
	}

	void sScenePrivate::draw_debug_primitives()
	{
		if (auto renderer = sRenderer::instance(); renderer)
		{
			if (dt_nav_mesh)
			{
				std::vector<vec3> points;
				auto& navmesh = (const dtNavMesh&)*dt_nav_mesh;
				auto ntiles = navmesh.getMaxTiles();
				for (auto i = 0; i < ntiles; i++)
				{
					auto tile = navmesh.getTile(i);
					if (tile->header)
					{
						auto npolys = tile->header->polyCount;
						for (auto n = 0; n < npolys; n++)
						{
							auto& p = tile->polys[n];
							if (p.getType() != DT_POLYTYPE_OFFMESH_CONNECTION)
							{
								auto& pd = tile->detailMeshes[n];
								for (auto j = 0; j < pd.triCount; j++)
								{
									auto t = &tile->detailTris[(pd.triBase + j) * 4];
									for (int k = 0; k < 3; ++k)
									{
										if (t[k] < p.vertCount)
											points.push_back(*(vec3*)&tile->verts[p.verts[t[k]] * 3]);
										else
											points.push_back(*(vec3*)&tile->detailVerts[(pd.vertBase + t[k] - p.vertCount) * 3]);
									}
								}
							}
						}
					}
				}
				renderer->draw_primitives("TriangleList"_h, points.data(), points.size(), cvec4(0, 127, 255, 127), false);
			}
		}
	}

	void sScenePrivate::update()
	{
		first_node = nullptr;
		first_element = nullptr;

		world->root->traversal_bfs([&](EntityPtr e, int depth) {
			if (!first_node)
			{
				if (auto node = e->node(); node)
					first_node = e;
			}
			if (!first_element)
			{
				if (auto element = e->element(); element)
					first_element = e;
			}
			if ((first_node && first_element) || depth > 2)
				return false;
			return true;
		});

		if (first_node)
			update_node_transform(octree, first_node, false);

		static auto last_target_extent = vec2(0.f);
		if (first_element)
		{
			auto target_extent = sRenderer::instance()->target_extent();
			update_alignment(first_element->element(), target_extent, vec4(0.f));
			update_element_transform(first_element->get_component_t<cLayoutT>(), first_element, false);
		}

#ifdef USE_RECASTNAV
		if (dt_crowd)
		{
			for (auto i = (int)nav_agents.size() - 1; i >= 0; i--)
			{
				auto ag = nav_agents[i];
				if (ag->dt_id != -1)
					break;
				dtCrowdAgentParams parms;
				memset(&parms, 0, sizeof(dtCrowdAgentParams));
				parms.radius = ag->radius;
				parms.height = ag->height;
				parms.maxAcceleration = 600.f;
				parms.maxSpeed = ag->speed * ag->speed_scale;
				parms.collisionQueryRange = parms.radius * 12.0f;
				parms.pathOptimizationRange = parms.radius * 30.0f;
				parms.separationGroup = ag->separation_group;
				parms.separationWeight = 2.f;
				parms.updateFlags = DT_CROWD_ANTICIPATE_TURNS | DT_CROWD_OPTIMIZE_VIS | DT_CROWD_OPTIMIZE_TOPO |
					DT_CROWD_OBSTACLE_AVOIDANCE | DT_CROWD_SEPARATION;
				parms.userData = ag;
				auto pos = ag->node->pos;
				ag->npos = pos;
				ag->dt_id = dt_crowd->addAgent(&pos[0], &parms);
				if (ag->dt_id == -1)
					printf("dt crowd add agent failed: -1 is returned\n");
			}

			dt_crowd->update(delta_time, nullptr);
		}
		if (dt_tile_cache)
		{
			int count = nav_obstacles.size();
			for (auto i = count - 1; i >= 0; i--)
			{
				auto ob = nav_obstacles[i];
				if (ob->dt_id != -1)
					break;
				if (dt_tile_cache->addObstacle(&ob->node->pos[0], ob->radius, ob->height, (uint*)&ob->dt_id) != DT_SUCCESS)
				{
					auto n = count - i - 1;
					if (n > 0)
					{
						auto temp = nav_obstacles;
						for (auto j = 0; j < count; j++)
						{
							if (j < n)
								nav_obstacles[j] = temp[count - j - 1];
							else
								nav_obstacles[j] = temp[j - n];
						}
					}
					break;
				}
			}

			dt_tile_cache->update(delta_time, dt_nav_mesh);
		}
#endif
	}

	static sScenePtr _instance = nullptr;

	struct sSceneInstance : sScene::Instance
	{
		sScenePtr operator()() override
		{
			return _instance;
		}
	}sScene_instance;
	sScene::Instance& sScene::instance = sScene_instance;

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
	}sScene_create;
	sScene::Create& sScene::create = sScene_create;
}

#include "../../graphics/material.h"
#include "../../graphics/model.h"
#include "tile_map_private.h"
#include "node_private.h"
#include "mesh_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cTileMapPrivate::set_extent(const vec3& _extent)
	{
		if (extent == _extent)
			return;
		extent = _extent;

		update_tiles();

		node->mark_transform_dirty();
		data_changed("extent"_h);
	}

	void cTileMapPrivate::set_blocks(const uvec3& _blocks)
	{
		if (blocks == _blocks)
			return;
		blocks = _blocks;

		update_tiles();

		node->mark_transform_dirty();
		data_changed("blocks"_h);
	}

	static uint get_mesh_id(int a, int b, int c, int d)
	{
		return a + b * 3 + c * 3 * 3 + d * 3 * 3 * 3;
	}

	static void parse_mesh_id(uint id, int& a, int& b, int& c, int& d)
	{
		a = id % 3; id /= 3;
		b = id % 3; id /= 3;
		c = id % 3; id /= 3;
		d = id % 3;
	}

	void cTileMapPrivate::set_tiles_path(const std::filesystem::path& _path)
	{
		if (tiles_path == _path)
			return;
		tiles_path = _path;

		meshes.clear();
		meshes.resize(81);

		auto path = Path::get(tiles_path);
		if (std::filesystem::exists(path))
		{
			for (auto& p : std::filesystem::directory_iterator(path))
			{
				if (p.path().extension() != L".prefab")
					continue;
				auto name = p.path().filename().stem().string();
				if (name.size() != 4)
					continue;
				auto a = name[0]; if (a == '0') a = 0; else if (a == '1') a = 2; else if (a == 'H') a = 1; else continue;
				auto b = name[1]; if (b == '0') b = 0; else if (b == '1') b = 2; else if (b == 'H') b = 1; else continue;
				auto c = name[2]; if (c == '0') c = 0; else if (c == '1') c = 2; else if (c == 'H') c = 1; else continue;
				auto d = name[3]; if (d == '0') d = 0; else if (d == '1') d = 2; else if (d == 'H') d = 1; else continue;

				meshes[a + b * 3 + c * 3 * 3 + d * 3 * 3 * 3] = std::make_pair(p.path(), 0U);
			}
			for (auto i = 0; i < 81; i++)
			{
				if (!meshes[i].first.empty())
				{
					int a, b, c, d;
					parse_mesh_id(i, a, b, c, d);

					auto r90 = get_mesh_id(b, d, a, c);
					if (meshes[r90].first.empty())
						meshes[r90] = std::make_pair(meshes[i].first, 1);
					auto r180 = get_mesh_id(d, c, b, a);
					if (meshes[r180].first.empty())
						meshes[r180] = std::make_pair(meshes[i].first, 2);
					auto r270 = get_mesh_id(c, a, d, b);
					if (meshes[r270].first.empty())
						meshes[r270] = std::make_pair(meshes[i].first, 3);
				}
			}
		}

		update_tiles();

		data_changed("tiles_path"_h);
	}

	void cTileMapPrivate::set_samples(const std::vector<uint>& _samples)
	{
		samples = _samples;
		update_tiles();
	}

	void cTileMapPrivate::set_sample(uint idx, uint v)
	{
		if (idx < samples.size())
		{
			samples[idx] = v;
			update_tiles();
		}
	}

	void cTileMapPrivate::update_tiles()
	{
		if (meshes.empty())
			return;

		samples.resize((blocks.x + 1) * (blocks.z + 1));
		if (entity->children.size() != blocks.x * blocks.z)
		{
			entity->remove_all_children();
			for (auto i = 0; i < blocks.x * blocks.z; i++)
			{
				auto e = Entity::create();
				e->name = std::format("{}, {}", i % blocks.x, i / blocks.x);
				e->tag = e->tag | TagNotSerialized;
				e->add_component_t<cNode>();
				entity->add_child(e);
			}
		}
		auto gap_x = extent.x / blocks.x;
		auto gap_y = extent.y / blocks.y;
		auto gap_z = extent.z / blocks.z;
		for (auto i = 0; i < blocks.x; i++)
		{
			for (auto j = 0; j < blocks.z; j++)
			{
				auto a = samples[i + j * (blocks.x + 1)];
				auto b = samples[i + 1 + j * (blocks.x + 1)];
				auto c = samples[i + (j + 1) * (blocks.x + 1)];
				auto d = samples[i + 1 + (j + 1) * (blocks.x + 1)];
				auto base_lv = min(min(a / 2, b / 2), min(c / 2, d / 2));
				a -= base_lv * 2; b -= base_lv * 2; c -= base_lv * 2; d -= base_lv * 2;

				auto id = get_mesh_id(a, b, c, d);
				auto dst = entity->children[i + j * blocks.x].get();
				auto node = dst->node();
				if (!meshes[id].first.empty())
				{
					dst->remove_all_children();
					dst->add_child(Entity::create(meshes[id].first));
					dst->forward_traversal([](EntityPtr e) { 
						if (e->get_component_t<cMesh>())
							e->tag = e->tag | TagMarkNavMesh;
					});
					switch (meshes[id].second)
					{
					case 0:
						node->set_qut(quat(1.f, 0.f, 0.f, 0.f));
						break;
					case 1:
						node->set_qut(angleAxis(radians(90.f), vec3(0.f, 1.f, 0.f)));
						break;
					case 2:
						node->set_qut(angleAxis(radians(180.f), vec3(0.f, 1.f, 0.f)));
						break;
					case 3:
						node->set_qut(angleAxis(radians(270.f), vec3(0.f, 1.f, 0.f)));
						break;
					}
				}
				node->set_pos(vec3((i + 0.5f) * gap_x , base_lv * gap_y, (j + 0.5f) * gap_z));
			}
		}
	}

	struct cTileMapCreate : cTileMap::Create
	{
		cTileMapPtr operator()(EntityPtr e) override
		{
			return new cTileMapPrivate();
		}
	}cTileMap_create;
	cTileMap::Create& cTileMap::create = cTileMap_create;
}

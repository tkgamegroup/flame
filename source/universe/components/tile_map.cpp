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

	void cTileMapPrivate::set_tiles_path(const std::filesystem::path& _path)
	{
		if (tiles_path == _path)
			return;
		tiles_path = _path;

		meshes.clear();

		auto path = Path::get(tiles_path);
		if (std::filesystem::exists(path))
		{
			for (auto& p : std::filesystem::directory_iterator(path))
			{
				if (p.path().extension() != L".prefab")
					continue;
				auto name = p.path().filename().stem().string();
				auto sp = SUS::split(name, '|');
				if (sp.size() != 4)
					continue;
				Mesh mesh;
				mesh.path = p.path();
				meshes[name] = mesh;
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

	void cTileMapPrivate::set_samples(const std::vector<Sample>& _samples)
	{
		samples = _samples;
		update_tiles();
	}

	void cTileMapPrivate::set_sample(uint idx, const Sample& v)
	{
		if (idx < samples.size())
		{
			samples[idx] = v;
			update_tiles();
		}
	}

	void cTileMapPrivate::on_active()
	{
		update_tiles();
	}

	void cTileMapPrivate::on_inactive()
	{
		entity->remove_all_children();
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
				if (a > 2 || b > 2 || c > 2 || d > 2)
					continue;

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
					auto r = meshes[id].second;
					if (id == 0)
						r = linearRand(0, 3);
					switch (r)
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

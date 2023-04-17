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

		for (auto& m : meshes)
		{
			if (m)
			{
				delete m;
				m = nullptr;
			}
		}
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
				auto a = name[0]; if (a == '0') a = 0; else if (a == '1') a = 1; else if (a == 'H') a = 2; else continue;
				auto b = name[1]; if (b == '0') b = 0; else if (b == '1') b = 1; else if (b == 'H') b = 2; else continue;
				auto c = name[2]; if (c == '0') c = 0; else if (c == '1') c = 1; else if (c == 'H') c = 2; else continue;
				auto d = name[3]; if (d == '0') d = 0; else if (d == '1') d = 1; else if (d == 'H') d = 2; else continue;

				auto e = Entity::create();
				e->load(p.path(), false);
				meshes[a + b * 3 + c * 3 * 3 + d * 3 * 3 * 3] = e;
			}
			for (auto i = 0; i < 81; i++)
			{
				if (meshes[i])
				{
					int a, b, c, d;
					parse_mesh_id(i, a, b, c, d);

					auto r90 = get_mesh_id(b, d, a, c);
					if (!meshes[r90])
					{
						auto e = meshes[i]->copy();
						e->children[0]->node()->set_qut(angleAxis(radians(90.f), vec3(0.f, 1.f, 0.f)));
						meshes[r90] = e;
					}
					auto r180 = get_mesh_id(d, c, b, a);
					if (!meshes[r180])
					{
						auto e = meshes[i]->copy();
						e->children[0]->node()->set_qut(angleAxis(radians(180.f), vec3(0.f, 1.f, 0.f)));
						meshes[r90] = e;
					}
					auto r270 = get_mesh_id(c, a, d, b);
					if (!meshes[r270])
					{
						auto e = meshes[i]->copy();
						e->children[0]->node()->set_qut(angleAxis(radians(270.f), vec3(0.f, 1.f, 0.f)));
						meshes[r90] = e;
					}
				}
			}
		}

		update_tiles();

		data_changed("tiles_path"_h);
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
				e->add_component_t<cNode>();
				entity->add_child(e);
			}
		}
		for (auto i = 0; i < blocks.x; i++)
		{
			for (auto j = 0; j < blocks.z; j++)
			{
				auto a = samples[i + j * (blocks.x + 1)];
				auto b = samples[i + 1 + j * (blocks.x + 1)];
				auto c = samples[i + (j + 1) * (blocks.x + 1)];
				auto d = samples[i + 1 + (j + 1) * (blocks.x + 1)];
				auto id = get_mesh_id(a, b, c, d);
				auto dst = entity->children[i + j * blocks.x].get();
				if (meshes[id])
					meshes[id]->copy(dst);
				dst->node()->set_pos(vec3(i * 2.f, 0.f, j * 2.f));
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

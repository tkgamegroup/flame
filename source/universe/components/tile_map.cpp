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

		dirty = true;
		update_tiles();

		node->mark_transform_dirty();
		data_changed("extent"_h);
	}

	void cTileMapPrivate::set_blocks(const uvec3& _blocks)
	{
		if (blocks == _blocks)
			return;
		blocks = _blocks;

		dirty = true;
		update_tiles();

		node->mark_transform_dirty();
		data_changed("blocks"_h);
	}

	static std::string rotate_name(const std::string& name)
	{
		auto sp = SUS::split(name, '_');
		std::rotate(sp.rbegin(), sp.rbegin() + 1, sp.rend());
		for (auto& t : sp)
		{
			if (t.size() >= 2)
			{
				if (t.back() == 'V')
					t.back() = 'H';
				else if (t.back() == 'H')
					t.back() = 'V';
			}
		}
		std::string ret;
		for (auto& t : sp)
		{
			if (!ret.empty())
				ret += '_';
			ret += t;
		}
		return ret;
	}

	static std::string form_name(cTileMap::Sample a, cTileMap::Sample b, cTileMap::Sample c, cTileMap::Sample d)
	{
		auto get_name = [](cTileMap::Sample sp)->std::string {
			switch (sp.slope)
			{
			case cTileMap::SlopeNone:
				return sp.height == 0 ? "0" : "1";
			case cTileMap::SlopeHorizontal:
				return "HH";
			case cTileMap::SlopeVertical:
				return "HV";

			}
		};
		return get_name(a) + '_' + get_name(b) + '_' + get_name(c) + '_' + get_name(d);
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
				auto sp = SUS::split(name, '_');
				if (sp.size() != 4)
					continue;
				Mesh mesh;
				mesh.path = p.path();
				meshes[name] = mesh;
			}

			std::vector<std::pair<std::string, Mesh>> rotated_meshes;
			for (auto& mesh : meshes)
			{
				if (mesh.second.rotation != 0)
					continue;
				auto r90 = rotate_name(mesh.first);
				if (meshes.find(r90) == meshes.end())
					rotated_meshes.emplace_back(r90, Mesh(mesh.second.path, 1));
				auto r180 = rotate_name(r90);
				if (meshes.find(r180) == meshes.end())
					rotated_meshes.emplace_back(r180, Mesh(mesh.second.path, 2));
				auto r270 = rotate_name(r180);
				if (meshes.find(r270) == meshes.end())
					rotated_meshes.emplace_back(r270, Mesh(mesh.second.path, 3));
			}
			for (auto& i : rotated_meshes)
				meshes[i.first] = i.second;
		}

		dirty = true;
		update_tiles();

		data_changed("tiles_path"_h);
	}

	void cTileMapPrivate::set_samples(const std::vector<Sample>& _samples)
	{
		samples = _samples;

		dirty = true;
		update_tiles();
	}

	void cTileMapPrivate::set_sample(uint idx, const Sample& v)
	{
		if (idx < samples.size())
		{
			samples[idx] = v;

			if (!dirty)
			{
				dirty = true;
				auto x = idx % (blocks.x + 1);
				auto y = idx / (blocks.x + 1);
				if (x > 0 && y > 0)
					dirty_tiles.push_back(uvec2(x - 1, y - 1));
				if (y > 0)
					dirty_tiles.push_back(uvec2(x, y - 1));
				if (x > 0)
					dirty_tiles.push_back(uvec2(x - 1, y));
				dirty_tiles.push_back(uvec2(x, y));

			}
			update_tiles();
		}
	}

	void cTileMapPrivate::on_active()
	{
		update_tiles();
	}

	void cTileMapPrivate::on_inactive()
	{
		dirty = true;
		entity->remove_all_children();
	}

	void cTileMapPrivate::update_tiles()
	{
		if (!dirty)
			return;
		dirty = false;

		if (dirty_tiles.empty())
		{
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
		}

		auto gap_x = extent.x / blocks.x;
		auto gap_y = extent.y / blocks.y;
		auto gap_z = extent.z / blocks.z;
		auto update_tile = [&](int i, int j) {
			auto a = samples[i + j * (blocks.x + 1)];
			auto b = samples[i + (j + 1) * (blocks.x + 1)];
			auto c = samples[i + 1 + (j + 1) * (blocks.x + 1)];
			auto d = samples[i + 1 + j * (blocks.x + 1)];
			auto base_lv = min(min(a.height, b.height), min(c.height, d.height));
			a.height -= base_lv; b.height -= base_lv; c.height -= base_lv; d.height -= base_lv;
			if (a.height > 1 || b.height > 1 || c.height > 1 || d.height > 1)
				return;

			auto name = form_name(a, b, c, d);
			auto dst = entity->children[i + j * blocks.x].get();
			auto node = dst->node();
			if (auto it = meshes.find(name); it != meshes.end())
			{
				dst->remove_all_children();
				dst->add_child(Entity::create(it->second.path));
				dst->forward_traversal([](EntityPtr e) {
					if (e->get_component_t<cMesh>())
						e->tag = e->tag | TagMarkNavMesh;
				});

				switch (name == "0_0_0_0" ? linearRand(0, 3) : it->second.rotation)
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
			node->set_pos(vec3((i + 0.5f) * gap_x, base_lv * gap_y, (j + 0.5f) * gap_z));
		};
		if (dirty_tiles.empty())
		{
			for (auto i = 0; i < blocks.x; i++)
			{
				for (auto j = 0; j < blocks.z; j++)
					update_tile(i, j);
			}
		}
		else
		{
			for (auto& ij : dirty_tiles)
				update_tile(ij.x, ij.y);
			dirty_tiles.clear();
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

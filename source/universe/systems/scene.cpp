#include "../../graphics/image.h"
#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "../components/node_private.h"
#include "../components/mesh_private.h"
#include "../components/terrain_private.h"
#include "../octree.h"
#include "scene_private.h"

namespace flame
{
	sScenePrivate::sScenePrivate()
	{
		octree = new OctNode(999999999.f, vec3(0.f));
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

						auto model = graphics::Model::create();
						auto& m = model->meshes.emplace_back();
						m.model = model;

						m.positions.resize((cx + 1) * (cz + 1));
						m.normals.resize((cx + 1) * (cz + 1));
						for (auto z = 0; z < cz + 1; z++)
						{
							for (auto x = 0; x < cx + 1; x++)
							{
								m.positions[z * (cx + 1) + x] = vec3(x * extent.x,
									textures->linear_sample(vec2((float)x / cx, (float)z / cz)).x * extent.y,
									z * extent.z);
								m.normals[z * (cx + 1) + x] = textures->linear_sample(vec2((float)x / cx, (float)z / cz), 0, 1).xyz;
							}
						}
						m.indices.resize(cx * cz * 6);
						for (auto z = 0; z < cz; z++)
						{
							for (auto x = 0; x < cx; x++)
							{
								auto s1 = x % tess_level < tess_level / 2 ? 1 : -1;
								auto s2 = z % tess_level < tess_level / 2 ? 1 : -1;
								auto dst = &m.indices[z * cx + x];
								if (s1 * s2 > 0)
								{
									dst[0] = z * cx + x;
									dst[1] = (z + 1) * cx + x;
									dst[2] = (z + 1) * cx + x + 1;

									dst[3] = z * cx + x;
									dst[4] = (z + 1) * cx + x + 1;
									dst[5] = z * cx + x + 1;
								}
								else
								{
									dst[0] = z * cx + x;
									dst[1] = (z + 1) * cx + x;
									dst[2] = z * cx + x + 1;

									dst[3] = z * cx + x + 1;
									dst[4] = (z + 1) * cx + x;
									dst[5] = (z + 1) * cx + x + 1;
								}
							}
						}

						model->save(L"D:\\1.fmod");
					}
				}

				for (auto& c : e->children)
					get_meshes(c.get());
			}
		};

		get_meshes(world->root.get());
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

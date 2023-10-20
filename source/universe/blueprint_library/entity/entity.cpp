#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../entity_private.h"
#include "../../components/node_private.h"
#include "../../components/camera_private.h"
#include "../../components/mesh_private.h"
#include "../../components/nav_agent_private.h"
#include "../../components/bp_instance_private.h"
#include "../../systems/input_private.h"
#include "../../systems/scene_private.h"
#include "../../systems/renderer_private.h"
#include "../../octree.h"

namespace flame
{
	void add_entity_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Get Name", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
				{
					.name = "Name",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				*(std::string*)outputs[0].data = entity ? entity->name : "";
			}
		);

		library->add_template("Set Name", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Name",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
					entity->name = *(std::string*)inputs[1].data;
			}
		);

		library->add_template("Get Tag", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
				{
					.name = "Tag",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				*(uint*)outputs[0].data = entity ? entity->tag : 0;
			}
		);

		library->add_template("Set Tag", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Tag",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
					entity->tag = (TagFlags)*(uint*)inputs[1].data;
			}
		);

		library->add_template("Add Tag", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Tag",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
					entity->tag = entity->tag | (TagFlags)*(uint*)inputs[1].data;
			}
		);

		library->add_template("Check Tag", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Tag",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Result",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
					*(bool*)outputs[0].data = (entity->tag & (*(uint*)inputs[1].data)) != 0;
				else
					*(bool*)outputs[0].data = false;
			}
		);

		library->add_template("Get Parent", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
					*(EntityPtr*)outputs[0].data = entity->parent;
				else
					*(EntityPtr*)outputs[0].data = nullptr;
			}
		);

		library->add_template("Get Child", "",
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Index",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Child",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto parent = *(EntityPtr*)inputs[0].data;
				if (parent)
				{
					auto index = *(uint*)inputs[1].data;
					if (index < parent->children.size())
						*(EntityPtr*)outputs[0].data = parent->children[index].get();
					else
						*(EntityPtr*)outputs[0].data = nullptr;
				}
				else
					*(EntityPtr*)outputs[0].data = nullptr;
			}
		);

		library->add_template("Get Pos", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
				{
					.name = "Pos",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto node = entity->get_component<cNode>();
					*(vec3*)outputs[0].data = node ? node->global_pos() : vec3(0.f);
				}
			}
		);

		library->add_template("Update Transform", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto node = entity->get_component<cNode>();
					if (node)
						node->update_transform_from_root();
				}
			}
		);

		library->add_template("World To Screen", "",
			{
				{
					.name = "In",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<vec2>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto world_pos = *(vec3*)inputs[0].data;
				*(vec2*)outputs[0].data = sRenderer::instance()->render_tasks.front()->camera->world_to_screen(world_pos);
			}
		);

		library->add_template("Spawn Prefab", "",
			{
				{
					.name = "Path",
					.allowed_types = { TypeInfo::get<std::filesystem::path>() }
				},
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Position",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Snap NavMesh",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				EntityPtr e = nullptr;

				auto& path = *(std::filesystem::path*)inputs[0].data;
				if (!path.empty())
				{
					path = Path::get(path);
					if (std::filesystem::exists(path))
					{
						if (auto parent = *(EntityPtr*)inputs[1].data; parent)
						{
							e = Entity::create();
							e->load(path);
							if (auto node = e->get_component<cNode>(); node)
							{
								auto pos = *(vec3*)inputs[2].data;
								auto snap_navmesh = *(bool*)inputs[3].data;
								if (snap_navmesh)
								{
									auto parent_pos = parent->get_component<cNode>()->global_pos();
									pos += parent_pos;
									auto scene = sScene::instance();
									float radius = 0.f;
									if (auto nav_agent = e->get_component<cNavAgent>(); nav_agent)
										radius = nav_agent->radius;
									auto times = 20;
									while (times > 0)
									{
										bool ok = true;
										ok = scene->navmesh_nearest_point(pos, vec3(2.f, 4.f, 2.f), pos);
										if (ok)
											ok = scene->navmesh_check_free_space(pos, radius);
										if (ok)
											break;
										pos.xz = pos.xz() + circularRand(4.f);
										times--;
									}
									if (times == 0)
									{
										delete e;
										e = nullptr;
									}
									else
										pos -= parent_pos;
								}
								if (e)
									node->set_pos(pos);
							}
							if (e)
								parent->add_child(e);
						}
					}
				}

				*(EntityPtr*)outputs[0].data = e;
			}
		);

		library->add_template("Add Blueprint", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Path",
					.allowed_types = { TypeInfo::get<std::filesystem::path>() }
				}
			},
			{
				{
					.name = "Instance",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto& path = *(std::filesystem::path*)inputs[1].data;
					if (!path.empty())
					{
						path = Path::get(path);
						if (std::filesystem::exists(path))
						{
							if (auto ins = entity->get_component<cBpInstance>(); ins)
							{
								ins->set_bp_name(path);
								*(BlueprintInstancePtr*)outputs[0].data = ins->bp_ins;
							}
							else
							{
								ins = entity->add_component<cBpInstance>();
								ins->set_bp_name(path);
								*(BlueprintInstancePtr*)outputs[0].data = ins->bp_ins;
							}
						}
						else
							*(BlueprintInstancePtr*)outputs[0].data = nullptr;
					}
					else
						*(BlueprintInstancePtr*)outputs[0].data = nullptr;
				}
				else
					*(BlueprintInstancePtr*)outputs[0].data = nullptr;
			}
		);

		library->add_template("Get Blueprint Instance", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "Instance",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto name = *(uint*)inputs[1].data;
					if (auto ins = entity->get_component<cBpInstance>(); ins)
					{
						if (ins->bp->name_hash == name)
							*(BlueprintInstancePtr*)outputs[0].data = ins->bp_ins;
						else
							*(BlueprintInstancePtr*)outputs[0].data = nullptr;
					}
					else
						*(BlueprintInstancePtr*)outputs[0].data = nullptr;
				}
				else
					*(BlueprintInstancePtr*)outputs[0].data = nullptr;
			}
		);

		library->add_template("Get Nearest Entity", "",
			{
				{
					.name = "Location",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Radius",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "5"
				},
				{
					.name = "Any Filter",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "4294967295"
				},
				{
					.name = "All Filter",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "0"
				},
				{
					.name = "Parent Search Times",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "999"
				}
			},
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto location = *(vec3*)inputs[0].data;
				auto radius = *(float*)inputs[1].data;
				auto any_filter = *(uint*)inputs[2].data;
				auto all_filter = *(uint*)inputs[3].data;
				auto parent_search_times = *(uint*)inputs[4].data;
				std::vector<std::pair<EntityPtr, cNodePtr>> res;
				sScene::instance()->octree->get_colliding(location, radius, res, any_filter, all_filter, parent_search_times);

				if (res.empty())
					*(EntityPtr*)outputs[0].data = nullptr;
				else if (res.size() == 1)
					*(EntityPtr*)outputs[0].data = res[0].first;
				else
				{
					std::vector<std::pair<float, EntityPtr>> nodes_with_distance(res.size());
					for (auto i = 0; i < res.size(); i++)
						nodes_with_distance[i] = std::make_pair(distance(res[i].second->global_pos(), location), res[i].first);
					std::sort(nodes_with_distance.begin(), nodes_with_distance.end(), [](const auto& a, const auto& b) {
						return a.first < b.first;
					});
					*(EntityPtr*)outputs[0].data = nodes_with_distance[0].second;
				}
			}
		);

		library->add_template("Get Mouse Hovering", "",
			{
				{
					.name = "Tag",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Parent Search Times",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "999"
				}
			},
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Pos",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto input = sInput::instance();
				if (input->mouse_used)
				{
					*(EntityPtr*)outputs[0].data = nullptr;
					return;
				}

				vec3 pos;
				auto node = sRenderer::instance()->pick_up(input->mpos, &pos, nullptr);
				if (!node)
				{
					*(EntityPtr*)outputs[0].data = nullptr;
					return;
				}

				auto e = node->entity;
				auto tag = *(uint*)inputs[0].data;
				auto parent_search_times = *(uint*)inputs[1].data;
				while (e)
				{
					if (e->tag & tag)
						break;
					e = e->parent;
					parent_search_times--;
					if (parent_search_times == 0)
					{
						e = nullptr;
						break;
					}
				}
				*(EntityPtr*)outputs[0].data = e;
				*(vec3*)outputs[1].data = e ? pos : vec3(-1000.f);
			}
		);

		library->add_template("Set Default Material", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default");
				}
			}
		);

		library->add_template("Set Default Red Material", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default_red");
				}
			}
		);

		library->add_template("Set Default Green Material", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default_green");
				}
			}
		);

		library->add_template("Set Default Blue Material", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default_blue");
				}
			}
		);

		library->add_template("Set Default Yellow Material", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default_yellow");
				}
			}
		);

		library->add_template("Set Default Purple Material", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default_purple");
				}
			}
		);

		library->add_template("Set Default Cyan Material", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default_cyan");
				}
			}
		);
	}
}

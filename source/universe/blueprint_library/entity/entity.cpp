#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../entity_private.h"
#include "../../world_private.h"
#include "../../components/node_private.h"
#include "../../components/mesh_private.h"
#include "../../components/nav_agent_private.h"
#include "../../components/bp_instance_private.h"
#include "../../systems/input_private.h"
#include "../../systems/scene_private.h"
#include "../../systems/renderer_private.h"
#include "../../systems/graveyard_private.h"
#include "../../octree.h"

namespace flame
{
	void add_entity_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Get Name", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				*(std::string*)outputs[0].data = entity ? entity->name : "";
			}
		);

		library->add_template("Set Name", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
					entity->name = *(std::string*)inputs[1].data;
			}
		);

		library->add_template("Get Tag", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				*(uint*)outputs[0].data = entity ? entity->tag : 0;
			}
		);

		library->add_template("Set Tag", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
					entity->tag = (TagFlags)*(uint*)inputs[1].data;
			}
		);

		library->add_template("Add Tag", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
					entity->tag = (TagFlags)(entity->tag | *(uint*)inputs[1].data);
			}
		);

		library->add_template("Remove Tag", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
					entity->tag = (TagFlags)(entity->tag & ~*(uint*)inputs[1].data);
			}
		);

		library->add_template("Check Tag", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
					*(bool*)outputs[0].data = (entity->tag & (*(uint*)inputs[1].data)) != 0;
				else
					*(bool*)outputs[0].data = false;
			}
		);

		library->add_template("Set Enable", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
					entity->set_enable(*(bool*)inputs[1].data);
			}
		);

		library->add_template("Create Entity", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto e = Entity::create();
				*(EntityPtr*)outputs[0].data = e;
				auto parent = *(EntityPtr*)inputs[0].data;
				if (parent)
					parent->add_child(e);
				else
					printf("A free entity is created! Please remember to destroy it\n");
			}
		);

		library->add_template("Remove Entity", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Immediately",
					.allowed_types = { TypeInfo::get<bool>() },
					.default_value = "false"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					if (*(bool*)inputs[1].data)
					{
						if (entity->parent)
							entity->remove_from_parent();
						else
							delete entity;
					}
					else
						Graveyard::instance()->add(entity);
				}
			}
		);

		library->add_template("Get Parent", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
					*(EntityPtr*)outputs[0].data = entity->parent;
				else
					*(EntityPtr*)outputs[0].data = nullptr;
			}
		);

		library->add_template("Get Child", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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

		library->add_template("Add Child", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Child",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Position",
					.allowed_types = { TypeInfo::get<int>() },
					.default_value = "-1"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto parent = *(EntityPtr*)inputs[0].data;
				if (parent)
				{
					auto child = *(EntityPtr*)inputs[1].data;
					auto position = *(int*)inputs[2].data;
					if (child)
						parent->add_child(child, position);
				}
			}
		);

		library->add_template("Add Component", "", BlueprintNodeFlagNone,
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
					.name = "Component",
					.allowed_types = { TypeInfo::get<Component*>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				auto hash = *(uint*)inputs[1].data;
				if (entity && hash)
					*(Component**)outputs[0].data = entity->add_component_h(hash);
			}
		);

		library->add_template("Find Entity", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Name",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto name = *(std::string*)inputs[0].data;
				if (!name.empty())
					*(EntityPtr*)outputs[0].data = World::instance()->root->find_child_recursively(name);
				else
					*(EntityPtr*)outputs[0].data = nullptr;
			}
		);

		library->add_template("Get Pos", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto node = entity->get_component<cNode>();
					*(vec3*)outputs[0].data = node ? node->global_pos() : vec3(0.f);
				}
				else
					*(vec3*)outputs[0].data = vec3(0.f);
			}
		);

		library->add_template("Set Pos", "", BlueprintNodeFlagNone,
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
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto node = entity->get_component<cNode>();
					node->set_pos(*(vec3*)inputs[1].data);
				}
			}
		);

		library->add_template("Add Pos", "", BlueprintNodeFlagNone,
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
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					if (auto node = entity->get_component<cNode>(); node)
						node->add_pos(*(vec3*)inputs[1].data);
				}
			}
		);

		library->add_template("Get Eul", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
				{
					.name = "Eul",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto node = entity->get_component<cNode>();
					*(vec3*)outputs[0].data = node ? node->get_eul() : vec3(0.f);
				}
				else
					*(vec3*)outputs[0].data = vec3(0.f);
			}
		);

		library->add_template("Set Eul", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Eul",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto node = entity->get_component<cNode>();
					node->set_eul(*(vec3*)inputs[1].data);
				}
			}
		);

		library->add_template("Get Scl", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
				{
					.name = "Scl",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto node = entity->get_component<cNode>();
					*(vec3*)outputs[0].data = node ? node->scl : vec3(0.f);
				}
				else
					*(vec3*)outputs[0].data = vec3(0.f);
			}
		);

		library->add_template("Set Scl", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Scl",
					.allowed_types = { TypeInfo::get<vec3>(), TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto node = entity->get_component<cNode>();
					if (inputs[1].type == TypeInfo::get<float>())
						node->set_scl(vec3(*(float*)inputs[1].data));
					else
						node->set_scl(*(vec3*)inputs[1].data);
				}
			}
		);

		library->add_template("Look At", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Target",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				auto target = *(EntityPtr*)inputs[1].data;
				if (entity && target)
				{
					if (auto node = entity->get_component<cNode>(); node)
					{
						if (auto target_node = target->get_component<cNode>(); target_node)
							node->look_at(target_node->global_pos());
					}
				}
			}
		);


		library->add_template("Update Transform", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto node = entity->get_component<cNode>();
					if (node)
						node->update_transform_from_root();
				}
			}
		);

		library->add_template("Spawn Cube", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Position",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				EntityPtr e = nullptr;

				if (auto parent = *(EntityPtr*)inputs[0].data; parent)
				{
					auto pos = *(vec3*)inputs[1].data;

					e = Entity::create();

					auto node = e->add_component<cNode>();
					node->set_pos(pos);

					auto mesh = e->add_component<cMesh>();
					mesh->set_mesh_and_material(L"standard_cube", L"default");

					parent->add_child(e);
				}
				*(EntityPtr*)outputs[0].data = e;
			}
		);

		library->add_template("Spawn Sphere", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Position",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				EntityPtr e = nullptr;

				if (auto parent = *(EntityPtr*)inputs[1].data; parent)
				{
					auto pos = *(vec3*)inputs[2].data;

					e = Entity::create();

					auto node = e->add_component<cNode>();
					node->set_pos(pos);

					auto mesh = e->add_component<cMesh>();
					mesh->set_mesh_and_material(L"standard_sphere", L"default");

					parent->add_child(e);
				}
				*(EntityPtr*)outputs[0].data = e;
			}
		);

		library->add_template("Spawn Prefab", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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
					else
						wprintf(L"Spawn Prefab Node: cannot find %s\n", path.c_str());
				}

				*(EntityPtr*)outputs[0].data = e;
			}
		);

		library->add_template("Add Blueprint", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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

		library->add_template("Get Blueprint Instance", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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

		library->add_template("Select Blueprint Instance2", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Name1_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Name2_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "Instance",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto name1 = *(uint*)inputs[1].data;
					auto name2 = *(uint*)inputs[2].data;
					if (auto ins = entity->get_component<cBpInstance>(); ins)
					{
						if (ins->bp->name_hash == name1)
							*(BlueprintInstancePtr*)outputs[0].data = ins->bp_ins;
						else
						{
							if (ins->bp->name_hash == name2)
								*(BlueprintInstancePtr*)outputs[0].data = ins->bp_ins;
							else
								*(BlueprintInstancePtr*)outputs[0].data = nullptr;
						}
					}
					else
						*(BlueprintInstancePtr*)outputs[0].data = nullptr;
				}
				else
					*(BlueprintInstancePtr*)outputs[0].data = nullptr;
			}
		);

		library->add_template("Start Coroutine", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Delay",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto name = *(uint*)inputs[1].data;
					if (auto ins = entity->get_component<cBpInstance>(); ins)
					{
						if (auto g = ins->bp_ins->find_group(name); g)
							ins->start_coroutine(g, *(float*)inputs[2].data);
					}
				}
			}
		);

		library->add_template("Entity Equal", "", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = *(EntityPtr*)inputs[0].data == *(EntityPtr*)inputs[1].data;
			}
		);
		
		library->add_template("Loop Var Entity", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto block_node = execution.block->node;
				auto vec_idx = execution.block->loop_vector_index;
				if (vec_idx != -1)
				{
					BlueprintAttribute vec_arg = { nullptr, nullptr };
					if (vec_idx < block_node->inputs.size())
						vec_arg = block_node->inputs[vec_idx];
					else
					{
						vec_idx -= block_node->inputs.size();
						if (vec_idx < block_node->outputs.size())
							vec_arg = block_node->outputs[vec_idx];
					}
					if (vec_arg.data && vec_arg.type)
					{
						auto i = execution.block->executed_times;
						auto item_type = vec_arg.type->get_wrapped();
						if (item_type == TypeInfo::get<EntityPtr>())
						{
							auto& vec = *(std::vector<char>*)vec_arg.data;
							auto length = vec.size() / item_type->size;
							if (i < length)
								*(EntityPtr*)outputs[0].data = *(EntityPtr*)(vec.data() + i * item_type->size);
							else
								*(EntityPtr*)outputs[0].data = nullptr;
						}
						else
							*(EntityPtr*)outputs[0].data = nullptr;
					}
					else
						*(EntityPtr*)outputs[0].data = nullptr;
				}
				else
					*(EntityPtr*)outputs[0].data = nullptr;
			}
		);

		library->add_template("Get BP Entity", "", BlueprintNodeFlagNone, 
		{
		{
			.name = "Instance", 
			.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
		}, 
		{
			.name = "Name_hash", 
			.allowed_types = { TypeInfo::get<std::string>() }
		}
		}, 
		{
		{
			.name = "V", 
			.allowed_types = { TypeInfo::get<EntityPtr>() }
		}
		}, 
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
			auto instance = *(BlueprintInstancePtr*)inputs[0].data; 
			auto name = *(uint*)inputs[1].data; 
			if (instance)
			{
				auto it = instance->variables.find(name); 
				if (it != instance->variables.end())
				{
					if (it->second.type == TypeInfo::get<EntityPtr>())
						* (EntityPtr*)outputs[0].data = *(EntityPtr*)it->second.data; 
					else
						* (EntityPtr*)outputs[0].data = nullptr; 
				}
				else
					* (EntityPtr*)outputs[0].data = nullptr; 
			}
			else
				* (EntityPtr*)outputs[0].data = nullptr; 
			}
		);

		library->add_template("Set BP Entity", "", BlueprintNodeFlagNone, 
		{
		{
			.name = "Instance", 
			.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
		}, 
		{
			.name = "Name_hash", 
			.allowed_types = { TypeInfo::get<std::string>() }
		}, 
		{
			.name = "V", 
			.allowed_types = { TypeInfo::get<EntityPtr>() }
		}
		}, 
		{
		}, 
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
			auto instance = *(BlueprintInstancePtr*)inputs[0].data; 
			auto name = *(uint*)inputs[1].data; 
			if (instance)
			{
				auto it = instance->variables.find(name); 
				if (it != instance->variables.end())
				{
					if (it->second.type == TypeInfo::get<EntityPtr>())
						* (EntityPtr*)it->second.data = *(EntityPtr*)inputs[2].data;
				}
			}
			}
		);

		library->add_template("Get Nearest Entity", "", BlueprintNodeFlagNone,
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
					.default_value = "3"
				}
			},
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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

		library->add_template("For Each Entity", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "temp_array",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<std::vector<EntityPtr>>() }
				}
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto& temp_array = *(std::vector<EntityPtr>*)outputs[1].data;
				temp_array.clear();
				World::instance()->root->forward_traversal([&](EntityPtr e) {
					temp_array.push_back(e);
				});

				execution.block->max_execute_times = temp_array.size();
				execution.block->loop_vector_index = 2;
			}
		);

		library->add_template("Foreach Surrounding Entity", "", BlueprintNodeFlagNone,
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
					.default_value = "3"
				}
			},
			{
				{
					.name = "Nearest Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "temp_array",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<std::vector<EntityPtr>>() }
				},
				{
					.name = "ok",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<bool>() },
					.default_value = "false"
				}
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto location = *(vec3*)inputs[1].data;
				auto radius = *(float*)inputs[2].data;
				auto any_filter = *(uint*)inputs[3].data;
				auto all_filter = *(uint*)inputs[4].data;
				auto parent_search_times = *(uint*)inputs[5].data;
				std::vector<std::pair<EntityPtr, cNodePtr>> res;
				sScene::instance()->octree->get_colliding(location, radius, res, any_filter, all_filter, parent_search_times);

				std::vector<std::pair<float, EntityPtr>> nodes_with_distance(res.size());
				for (auto i = 0; i < res.size(); i++)
					nodes_with_distance[i] = std::make_pair(distance(res[i].second->global_pos(), location), res[i].first);
				std::sort(nodes_with_distance.begin(), nodes_with_distance.end(), [](const auto& a, const auto& b) {
					return a.first < b.first;
				});

				auto& temp_array = *(std::vector<EntityPtr>*)outputs[2].data;
				temp_array.resize(res.size());
				for (auto i = 0; i < res.size(); i++)
					temp_array[i] = (nodes_with_distance[i].second);
				*(EntityPtr*)outputs[1].data = temp_array.empty() ? nullptr : temp_array.front();
				*(bool*)outputs[3].data = false;
				execution.block->max_execute_times = temp_array.size();
				execution.block->loop_vector_index = 8;
				execution.block->block_output_index = 9;
			},
			nullptr,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto ok = *(bool*)outputs[3].data;
				if (ok)
				{
					auto& temp_array = *(std::vector<EntityPtr>*)outputs[2].data;
					*(EntityPtr*)outputs[1].data = temp_array[execution.block->executed_times];
					execution.block->_break();
				}
			}
		);

		library->add_template("Get Mouse Hovering", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Tag",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Parent Search Times",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "3"
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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

		library->add_template("Set Object Color", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Color",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_color(*(cvec4*)inputs[1].data);
				}
			}
		);

		library->add_template("Set Default Material", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default");
				}
			}
		);

		library->add_template("Set Default Red Material", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default_red");
				}
			}
		);

		library->add_template("Set Default Green Material", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default_green");
				}
			}
		);

		library->add_template("Set Default Blue Material", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default_blue");
				}
			}
		);

		library->add_template("Set Default Yellow Material", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default_yellow");
				}
			}
		);

		library->add_template("Set Default Purple Material", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto mesh = entity->get_component<cMesh>();
					if (mesh)
						mesh->set_material_name(L"default_purple");
				}
			}
		);

		library->add_template("Set Default Cyan Material", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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

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
				},
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
				auto e = Entity::create();
				*(EntityPtr*)outputs[0].data = e;
				auto parent = *(EntityPtr*)inputs[0].data;
				if (parent)
					parent->add_child(e);
				else
					printf("A free entity is created! Please remember to destroy it\n");
				e->name = *(std::string*)inputs[1].data;
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

		library->add_template("Get First Child", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
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
					if (!parent->children.empty())
						*(EntityPtr*)outputs[0].data = parent->children.front().get();
					else
						*(EntityPtr*)outputs[0].data = nullptr;
				}
				else
					*(EntityPtr*)outputs[0].data = nullptr;
			}
		);

		library->add_template("Get Last Child", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
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
					if (!parent->children.empty())
						*(EntityPtr*)outputs[0].data = parent->children.back().get();
					else
						*(EntityPtr*)outputs[0].data = nullptr;
				}
				else
					*(EntityPtr*)outputs[0].data = nullptr;
			}
		);

		library->add_template("Foreach Child", "", BlueprintNodeFlagNone,
			{ {
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
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
				auto parent = *(EntityPtr*)inputs[0].data;
				auto& temp_array = *(std::vector<EntityPtr>*)outputs[0].data;
				temp_array.clear();
				if (parent)
				{
					for (auto& c : parent->children)
						temp_array.push_back(c.get());
				}

				execution.block->max_execute_times = temp_array.size();
				execution.block->loop_vector_index = 1;
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& temp_array = *(std::vector<EntityPtr>*)outputs[0].data;
				temp_array.clear();
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
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
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
				auto parent = *(EntityPtr*)inputs[0].data;
				auto name = *(std::string*)inputs[1].data;
				if (!name.empty())
				{
					if (parent)
						*(EntityPtr*)outputs[0].data = parent->find_child_recursively(name);
					else
						*(EntityPtr*)outputs[0].data = World::instance()->root->find_child_recursively(name);
				}
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
					if (auto node = entity->get_component<cNode>(); node)
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
					if (auto node = entity->get_component<cNode>(); node)
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
					if (auto node = entity->get_component<cNode>(); node)
					{
						if (inputs[1].type == TypeInfo::get<float>())
							node->set_scl(vec3(*(float*)inputs[1].data));
						else
							node->set_scl(*(vec3*)inputs[1].data);
					}
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

		library->add_template("Set BP", "", BlueprintNodeFlagNone,
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
					if (auto ins = entity->get_component<cBpInstance>(); ins && ins->bp_ins)
					{
						if (ins->bp->name_hash == name || name == "*"_h)
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

		library->add_template("EGet V", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Name0_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "V0",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto ins = entity->get_component<cBpInstance>(); ins && ins->bp_ins)
					{
						auto instance = ins->bp_ins;
						for (auto i = 0; i < outputs_count; i++)
						{
							auto type = outputs[i].type;
							if (auto it = instance->variables.find(*(uint*)inputs[i + 1].data); it != instance->variables.end())
							{
								auto& arg = it->second;
								if (arg.type == type ||
									(type == TypeInfo::get<uint>() && arg.type == TypeInfo::get<int>()) ||
									(type == TypeInfo::get<int>() && arg.type == TypeInfo::get<uint>()))
									type->copy(outputs[i].data, arg.data);
								else if (type == TypeInfo::get<voidptr>())
									*(voidptr*)outputs[i].data = arg.data;
								else
									type->create(outputs[i].data);
							}
							else
								type->create(outputs[i].data);
						}
					}
					else
					{
						for (auto i = 0; i < outputs_count; i++)
							outputs[i].type->create(outputs[i].data);
					}
				}
				else
				{
					for (auto i = 0; i < outputs_count; i++)
						outputs[i].type->create(outputs[i].data);
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					std::vector<TypeInfo*> types;
					for (auto t : SUS::split(info.template_string, ','))
					{
						auto type = blueprint_type_from_template_str(t);
						if (type)
							types.push_back(type);
					}

					if (types.empty())
						types.push_back(TypeInfo::get<float>());

					info.new_inputs.resize(types.size() + 1);
					info.new_inputs[0] = {
						.name = "Entity",
						.allowed_types = { TypeInfo::get<EntityPtr>() }
					};
					for (auto i = 0; i < types.size(); i++)
					{
						info.new_inputs[i + 1] = {
							.name = "Name" + str(i) + "_hash",
							.allowed_types = { TypeInfo::get<std::string>() }
						};
					}
					info.new_outputs.resize(types.size());
					for (auto i = 0; i < types.size(); i++)
					{
						info.new_outputs[i] = {
							.name = "V" + str(i),
							.allowed_types = { types[i] }
						};
					}
					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("ESet V", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Name0_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "V0",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto ins = entity->get_component<cBpInstance>(); ins && ins->bp_ins)
					{
						auto instance = ins->bp_ins;
						for (auto i = 1; i < inputs_count; i += 2)
						{
							if (auto it = instance->variables.find(*(uint*)inputs[i].data); it != instance->variables.end())
							{
								auto type = inputs[i + 1].type;
								auto& arg = it->second;
								if (it->second.type == type ||
									(type == TypeInfo::get<uint>() && arg.type == TypeInfo::get<int>()) ||
									(type == TypeInfo::get<int>() && arg.type == TypeInfo::get<uint>()))
									type->copy(arg.data, inputs[i + 1].data);
							}
						}
					}
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					std::vector<TypeInfo*> types;
					for (auto t : SUS::split(info.template_string, ','))
					{
						auto type = blueprint_type_from_template_str(t);
						if (type)
							types.push_back(type);
					}

					if (types.empty())
						types.push_back(TypeInfo::get<float>());

					info.new_inputs.resize(types.size() * 2 + 1);
					info.new_inputs[0] = {
						.name = "Entity",
						.allowed_types = { TypeInfo::get<EntityPtr>() }
					};
					for (auto i = 0; i < types.size(); i++)
					{
						info.new_inputs[i * 2 + 1] = {
							.name = "Name" + str(i) + "_hash",
							.allowed_types = { TypeInfo::get<std::string>() }
						};
						info.new_inputs[i * 2 + 2] = {
							.name = "V" + str(i),
							.allowed_types = { types[i] }
						};
					}

					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("EUnserialize", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Name",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Value",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto ins = entity->get_component<cBpInstance>(); ins && ins->bp_ins)
					{
						auto instance = ins->bp_ins;
						if (auto& name = *(std::string*)inputs[1].data; !name.empty())
						{
							if (auto it = instance->variables.find(sh(name.c_str())); it != instance->variables.end())
								it->second.type->unserialize(*(std::string*)inputs[2].data, it->second.data);
							else
								printf("EUnserialize: cannot find variable: %s\n", name.c_str());
						}
					}
				}
			}
		);

		library->add_template("EArray Clear", "", BlueprintNodeFlagNone,
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
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto ins = entity->get_component<cBpInstance>(); ins && ins->bp_ins)
					{
						auto instance = ins->bp_ins;
						auto it = instance->variables.find(*(uint*)inputs[1].data);
						if (it != instance->variables.end())
						{
							auto& arg = it->second;
							if (is_array(arg.type->tag))
								resize_vector(arg.data, arg.type->get_wrapped(), 0);
						}
					}
				}
			}
		);

		library->add_template("EArray Get Item", "", BlueprintNodeFlagEnableTemplate,
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
					.name = "Index",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto out_type = outputs[0].type;
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto ins = entity->get_component<cBpInstance>(); ins && ins->bp_ins)
					{
						auto instance = ins->bp_ins;
						auto it = instance->variables.find(*(uint*)inputs[1].data);
						if (it != instance->variables.end())
						{
							auto& arg = it->second;
							if (is_vector(arg.type->tag))
							{
								auto item_type = arg.type->get_wrapped();
								if (item_type == out_type ||
									(out_type == TypeInfo::get<uint>() && item_type == TypeInfo::get<int>()) ||
									(out_type == TypeInfo::get<int>() && item_type == TypeInfo::get<uint>()))
								{
									auto& array = *(std::vector<char>*)arg.data;
									auto array_size = array.size() / item_type->size;
									auto index = *(uint*)inputs[2].data;
									if (index < array_size)
										out_type->copy(outputs[0].data, array.data() + index * item_type->size);
									else
										out_type->create(outputs[0].data);
								}
								else
									out_type->create(outputs[0].data);
							}
							else
								out_type->create(outputs[0].data);
						}
						else
							out_type->create(outputs[0].data);
					}
					else
						out_type->create(outputs[0].data);
				}
				else
					out_type->create(outputs[0].data);
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					auto type = info.template_string.empty() ? TypeInfo::get<float>() : blueprint_type_from_template_str(info.template_string);
					if (!type)
						return false;

					info.new_inputs.resize(3);
					info.new_inputs[0] = {
						.name = "Entity",
						.allowed_types = { TypeInfo::get<EntityPtr>() }
					};
					info.new_inputs[1] = {
						.name = "Name_hash",
						.allowed_types = { TypeInfo::get<std::string>() }
					};
					info.new_inputs[2] = {
						.name = "Index",
						.allowed_types = { TypeInfo::get<uint>() }
					};
					info.new_outputs.resize(1);
					info.new_outputs[0] = {
						.name = "V",
						.allowed_types = { type }
					};

					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("EArray Set Item", "", BlueprintNodeFlagEnableTemplate,
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
					.name = "Index",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto in_type = inputs[3].type;
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto ins = entity->get_component<cBpInstance>(); ins && ins->bp_ins)
					{
						auto instance = ins->bp_ins;
						auto it = instance->variables.find(*(uint*)inputs[1].data);
						if (it != instance->variables.end())
						{
							auto& arg = it->second;
							if (is_vector(arg.type->tag))
							{
								auto item_type = arg.type->get_wrapped();
								if (item_type == in_type ||
									(in_type == TypeInfo::get<uint>() && item_type == TypeInfo::get<int>()) ||
									(in_type == TypeInfo::get<int>() && item_type == TypeInfo::get<uint>()))
								{
									auto& array = *(std::vector<char>*)arg.data;
									auto array_size = array.size() / item_type->size;
									auto index = *(uint*)inputs[2].data;
									if (index < array_size)
										in_type->copy(array.data() + index * item_type->size, inputs[3].data);
								}
							}
						}
					}
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					auto type = info.template_string.empty() ? TypeInfo::get<float>() : blueprint_type_from_template_str(info.template_string);
					if (!type)
						return false;

					info.new_inputs.resize(4);
					info.new_inputs[0] = {
						.name = "Entity",
						.allowed_types = { TypeInfo::get<EntityPtr>() }
					};
					info.new_inputs[1] = {
						.name = "Name_hash",
						.allowed_types = { TypeInfo::get<std::string>() }
					};
					info.new_inputs[2] = {
						.name = "Index",
						.allowed_types = { TypeInfo::get<uint>() }
					};
					info.new_inputs[3] = {
						.name = "V",
						.allowed_types = { type }
					};

					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("EArray Add Item", "", BlueprintNodeFlagEnableTemplate,
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
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto in_type = inputs[2].type;
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto ins = entity->get_component<cBpInstance>(); ins && ins->bp_ins)
					{
						auto instance = ins->bp_ins;
						auto it = instance->variables.find(*(uint*)inputs[1].data);
						if (it != instance->variables.end())
						{
							auto& arg = it->second;
							if (is_vector(arg.type->tag))
							{
								auto item_type = arg.type->get_wrapped();
								if (item_type == in_type ||
									(in_type == TypeInfo::get<uint>() && item_type == TypeInfo::get<int>()) ||
									(in_type == TypeInfo::get<int>() && item_type == TypeInfo::get<uint>()))
								{
									auto& array = *(std::vector<char>*)arg.data;
									auto array_size = array.size() / item_type->size;
									resize_vector(arg.data, item_type, array_size + 1);
									in_type->copy(array.data() + array_size * item_type->size, inputs[2].data);
								}
							}
						}
					}
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					auto type = info.template_string.empty() ? TypeInfo::get<float>() : blueprint_type_from_template_str(info.template_string);
					if (!type)
						return false;

					info.new_inputs.resize(3);
					info.new_inputs[0] = {
						.name = "Entity",
						.allowed_types = { TypeInfo::get<EntityPtr>() }
					};
					info.new_inputs[1] = {
						.name = "Name_hash",
						.allowed_types = { TypeInfo::get<std::string>() }
					};
					info.new_inputs[2] = {
						.name = "V",
						.allowed_types = { type }
					};

					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("ECall", "", BlueprintNodeFlagEnableTemplate,
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
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				auto name = *(uint*)inputs[1].data;
				if (entity)
				{
					if (auto ins = entity->get_component<cBpInstance>(); ins && ins->bp_ins)
					{
						auto instance = ins->bp_ins;
						if (auto g = instance->find_group(name); g)
						{
							std::vector<voidptr> input_args;
							std::vector<voidptr> output_args;
							for (auto i = 2; i < inputs_count; i++)
								input_args.push_back(inputs[i].data);
							for (auto i = 0; i < outputs_count; i++)
								output_args.push_back(outputs[i].data);
							instance->call(g, input_args.data(), output_args.data());
						}
					}
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					std::vector<TypeInfo*> input_types;
					std::vector<TypeInfo*> output_types;
					if (!info.template_string.empty())
					{
						auto sp = SUS::split(info.template_string, '|');
						if (sp.size() == 2)
						{
							for (auto t : SUS::split(sp[0], ','))
							{
								auto type = blueprint_type_from_template_str(t);
								if (type && type != TypeInfo::void_type)
									input_types.push_back(type);
							}
							for (auto t : SUS::split(sp[1], ','))
							{
								auto type = blueprint_type_from_template_str(t);
								if (type && type != TypeInfo::void_type)
									output_types.push_back(type);
							}
						}

						info.new_inputs.resize(input_types.size() + 2);
						info.new_inputs[0] = {
							.name = "Entity",
							.allowed_types = { TypeInfo::get<EntityPtr>() }
						};
						info.new_inputs[1] = {
							.name = "Name_hash",
							.allowed_types = { TypeInfo::get<std::string>() }
						};
						for (auto i = 0; i < input_types.size(); i++)
						{
							info.new_inputs[i + 2] = {
								.name = "Input " + str(i + 1),
								.allowed_types = { input_types[i] }
							};
						}
						for (auto i = 0; i < output_types.size(); i++)
						{
							info.new_outputs[i] = {
								.name = "Output " + str(i + 1),
								.allowed_types = { output_types[i] }
							};
						}
					}
					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
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
					if (auto ins = entity->get_component<cBpInstance>(); ins && ins->bp_ins)
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

		library->add_template("Find Nearest Entity", "", BlueprintNodeFlagEnableTemplate,
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

				*(EntityPtr*)outputs[0].data = nullptr;

				std::vector<std::pair<float, EntityPtr>> nodes_with_distance(res.size());
				for (auto i = 0; i < res.size(); i++)
					nodes_with_distance[i] = std::make_pair(distance(res[i].second->global_pos(), location), res[i].first);
				std::sort(nodes_with_distance.begin(), nodes_with_distance.end(), [](const auto& a, const auto& b) {
					return a.first < b.first;
				});
				if (inputs_count <= 5)
				{
					if (!nodes_with_distance.empty())
						*(EntityPtr*)outputs[0].data = nodes_with_distance.front().second;
				}
				else
				{
					for (auto& pair : nodes_with_distance)
					{
						auto ok = true;
						if (auto ins = pair.second->get_component<cBpInstance>(); ins && ins->bp_ins)
						{
							auto instance = ins->bp_ins;
							for (auto i = 5; i < inputs_count; i += 3)
							{
								if (auto it = instance->variables.find(*(uint*)inputs[i].data); it != instance->variables.end())
								{
									auto type = inputs[i + 1].type;
									auto& arg = it->second;
									if (it->second.type == type ||
										(type == TypeInfo::get<uint>() && arg.type == TypeInfo::get<int>()) ||
										(type == TypeInfo::get<int>() && arg.type == TypeInfo::get<uint>()))
									{
										switch (*(uint*)inputs[i + 2].data)
										{
										case "equal"_h:
											if (!type->compare(inputs[i + 1].data, arg.data))
											{
												ok = false;
												break;
											}
											break;
										case "any_flag"_h:
											if (type == TypeInfo::get<uint>() || type == TypeInfo::get<int>())
											{
												if ((*(uint*)inputs[i + 1].data & *(uint*)arg.data) == 0)
												{
													ok = false;
													break;
												}
											}
											break;
										case "all_flags"_h:
											if (type == TypeInfo::get<uint>() || type == TypeInfo::get<int>())
											{
												if ((*(uint*)inputs[i + 1].data & *(uint*)arg.data) != *(uint*)inputs[i + 1].data)
												{
													ok = false;
													break;
												}
											}
											break;
										}
									}
								}
							}
						}
						if (ok)
						{
							*(EntityPtr*)outputs[0].data = pair.second;
							break;
						}
					}
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					std::vector<TypeInfo*> types;
					for (auto t : SUS::split(info.template_string, ','))
					{
						auto type = blueprint_type_from_template_str(t);
						if (type)
							types.push_back(type);
					}

					if (types.empty())
						types.push_back(TypeInfo::get<float>());

					info.new_inputs.resize(types.size() * 3 + 5);
					info.new_inputs[0] = {
						.name = "Location",
						.allowed_types = { TypeInfo::get<vec3>() }
					};
					info.new_inputs[1] = {
						.name = "Radius",
						.allowed_types = { TypeInfo::get<float>() }
					};
					info.new_inputs[2] = {
						.name = "Any Filter",
						.allowed_types = { TypeInfo::get<uint>() }
					};
					info.new_inputs[3] = {
						.name = "All Filter",
						.allowed_types = { TypeInfo::get<uint>() }
					};
					info.new_inputs[4] = {
						.name = "Parent Search Times",
						.allowed_types = { TypeInfo::get<uint>() }
					};
					for (auto i = 0; i < types.size(); i++)
					{
						info.new_inputs[i * 3 + 5] = {
							.name = "Name" + str(i) + "_hash",
							.allowed_types = { TypeInfo::get<std::string>() }
						};
						info.new_inputs[i * 3 + 6] = {
							.name = "V" + str(i),
							.allowed_types = { types[i] }
						};
						info.new_inputs[i * 3 + 7] = {
							.name = "OP" + str(i) + "_hash",
							.allowed_types = { TypeInfo::get<std::string>() },
							.default_value = "equal"
						};
					}
					info.new_outputs.resize(1);
					info.new_outputs[0] = {
						.name = "Entity",
						.allowed_types = { TypeInfo::get<EntityPtr>() }
					};

					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("Find Random Entity", "", BlueprintNodeFlagEnableTemplate,
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

				*(EntityPtr*)outputs[0].data = nullptr;

				if (inputs_count <= 5)
				{
					if (!res.empty())
						*(EntityPtr*)outputs[0].data = res.front().first;
				}
				else
				{
					std::vector<EntityPtr> candidates;
					for (auto i = 0; i < res.size(); i++)
					{
						auto ok = true;
						if (auto ins = res[i].first->get_component<cBpInstance>(); ins && ins->bp_ins)
						{
							auto instance = ins->bp_ins;
							for (auto i = 5; i < inputs_count; i += 3)
							{
								if (auto it = instance->variables.find(*(uint*)inputs[i].data); it != instance->variables.end())
								{
									auto type = inputs[i + 1].type;
									auto& arg = it->second;
									if (it->second.type == type ||
										(type == TypeInfo::get<uint>() && arg.type == TypeInfo::get<int>()) ||
										(type == TypeInfo::get<int>() && arg.type == TypeInfo::get<uint>()))
									{
										switch (*(uint*)inputs[i + 2].data)
										{
										case "equal"_h:
											if (!type->compare(inputs[i + 1].data, arg.data))
											{
												ok = false;
												break;
											}
											break;
										case "any_flag"_h:
											if (type == TypeInfo::get<uint>() || type == TypeInfo::get<int>())
											{
												if ((*(uint*)inputs[i + 1].data & *(uint*)arg.data) == 0)
												{
													ok = false;
													break;
												}
											}
											break;
										case "all_flags"_h:
											if (type == TypeInfo::get<uint>() || type == TypeInfo::get<int>())
											{
												if ((*(uint*)inputs[i + 1].data & *(uint*)arg.data) != *(uint*)inputs[i + 1].data)
												{
													ok = false;
													break;
												}
											}
											break;
										}
									}
								}
							}
						}
						if (ok)
							candidates.push_back(res[i].first);
					}

					if (!candidates.empty())
						*(EntityPtr*)outputs[0].data = candidates[linearRand(0, (int)candidates.size() - 1)];
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					std::vector<TypeInfo*> types;
					for (auto t : SUS::split(info.template_string, ','))
					{
						auto type = blueprint_type_from_template_str(t);
						if (type)
							types.push_back(type);
					}

					if (types.empty())
						types.push_back(TypeInfo::get<float>());

					info.new_inputs.resize(types.size() * 3 + 5);
					info.new_inputs[0] = {
						.name = "Location",
						.allowed_types = { TypeInfo::get<vec3>() }
					};
					info.new_inputs[1] = {
						.name = "Radius",
						.allowed_types = { TypeInfo::get<float>() }
					};
					info.new_inputs[2] = {
						.name = "Any Filter",
						.allowed_types = { TypeInfo::get<uint>() }
					};
					info.new_inputs[3] = {
						.name = "All Filter",
						.allowed_types = { TypeInfo::get<uint>() }
					};
					info.new_inputs[4] = {
						.name = "Parent Search Times",
						.allowed_types = { TypeInfo::get<uint>() }
					};
					for (auto i = 0; i < types.size(); i++)
					{
						info.new_inputs[i * 3 + 5] = {
							.name = "Name" + str(i) + "_hash",
							.allowed_types = { TypeInfo::get<std::string>() }
						};
						info.new_inputs[i * 3 + 6] = {
							.name = "V" + str(i),
							.allowed_types = { types[i] }
						};
						info.new_inputs[i * 3 + 7] = {
							.name = "OP" + str(i) + "_hash",
							.allowed_types = { TypeInfo::get<std::string>() },
							.default_value = "equal"
						};
					}
					info.new_outputs.resize(1);
					info.new_outputs[0] = {
						.name = "Entity",
						.allowed_types = { TypeInfo::get<EntityPtr>() }
					};

					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("Foreach Entity", "", BlueprintNodeFlagNone,
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
				auto& temp_array = *(std::vector<EntityPtr>*)outputs[0].data;
				temp_array.clear();
				World::instance()->root->forward_traversal([&](EntityPtr e) {
					temp_array.push_back(e);
				});

				execution.block->max_execute_times = temp_array.size();
				execution.block->loop_vector_index = 0;
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& temp_array = *(std::vector<EntityPtr>*)outputs[0].data;
				temp_array.clear();
			}
		);

		library->add_template("Foreach Surrounding Entity", "", BlueprintNodeFlagReturnTarget,
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
				auto location = *(vec3*)inputs[0].data;
				auto radius = *(float*)inputs[1].data;
				auto any_filter = *(uint*)inputs[2].data;
				auto all_filter = *(uint*)inputs[3].data;
				auto parent_search_times = *(uint*)inputs[4].data;
				std::vector<std::pair<EntityPtr, cNodePtr>> res;
				sScene::instance()->octree->get_colliding(location, radius, res, any_filter, all_filter, parent_search_times);

				std::vector<std::pair<float, EntityPtr>> nodes_with_distance(res.size());
				for (auto i = 0; i < res.size(); i++)
					nodes_with_distance[i] = std::make_pair(distance(res[i].second->global_pos(), location), res[i].first);
				std::sort(nodes_with_distance.begin(), nodes_with_distance.end(), [](const auto& a, const auto& b) {
					return a.first < b.first;
				});

				auto& temp_array = *(std::vector<EntityPtr>*)outputs[1].data;
				temp_array.resize(res.size());
				for (auto i = 0; i < res.size(); i++)
					temp_array[i] = (nodes_with_distance[i].second);
				*(EntityPtr*)outputs[0].data = temp_array.empty() ? nullptr : temp_array.front();
				*(bool*)outputs[2].data = false;
				execution.block->max_execute_times = temp_array.size();
				execution.block->loop_vector_index = 6;
				execution.block->block_output_index = 7;
			},
			nullptr,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto ok = *(bool*)outputs[2].data;
				if (ok)
				{
					auto& temp_array = *(std::vector<EntityPtr>*)outputs[1].data;
					*(EntityPtr*)outputs[0].data = temp_array[execution.block->executed_times];
					execution.block->_break();
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& temp_array = *(std::vector<EntityPtr>*)outputs[1].data;
				temp_array.clear();
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

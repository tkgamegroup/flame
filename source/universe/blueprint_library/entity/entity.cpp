#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../entity_private.h"
#include "../../components/node_private.h"
#include "../../components/camera_private.h"
#include "../../components/bp_instance_private.h"
#include "../../systems/input_private.h"
#include "../../systems/renderer_private.h"

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
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
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
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
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
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
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
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
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
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
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
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
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
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
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
					.name = "Postiion",
					.allowed_types = { TypeInfo::get<vec3>() }
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
						auto parent = *(EntityPtr*)inputs[1].data;
						if (parent)
						{
							e = Entity::create();
							e->load(path);
							if (auto node = e->get_component<cNode>(); node)
								node->set_pos(*(vec3*)inputs[2].data);
							parent->add_child(e);
						}
					}
				}

				*(EntityPtr*)outputs[0].data = e;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Get Nearby Entities", "",
			{
				{
					.name = "Location",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Radius",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "5"
				}
			},
			{
				{
					.name = "Entities",
					.allowed_types = { TypeInfo::get<std::vector<EntityPtr>>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {

			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
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
				auto times = *(uint*)inputs[1].data;
				while (e)
				{
					if (e->tag & tag)
						break;
					e = e->parent;
					times--;
					if (times == 0)
					{
						e = nullptr;
						break;
					}
				}
				*(EntityPtr*)outputs[0].data = e;
				*(vec3*)outputs[1].data = e ? pos : vec3(-1000.f);
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
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
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto e = *(EntityPtr*)inputs[0].data;
				if (e)
				{
					auto& path = *(std::filesystem::path*)inputs[1].data;
					if (!path.empty())
					{
						path = Path::get(path);
						if (std::filesystem::exists(path))
						{
							if (auto ins = e->get_component<cBpInstance>(); ins)
								ins->set_bp_name(path);
							else
							{
								ins = e->add_component<cBpInstance>();
								ins->set_bp_name(path);
							}
						}
					}
				}
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
	}
}

#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../entity_private.h"
#include "../../components/node_private.h"

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
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				*(std::string*)outputs[0].data = entity ? entity->name : "";
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
					.name = "Out",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				auto node = entity->get_component<cNode>();
				*(vec3*)outputs[0].data = node ? node->pos : vec3(0.f);
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
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				auto& path = *(std::filesystem::path*)inputs[0].data;
				if (!path.empty())
				{
					path = Path::get(path);
					if (std::filesystem::exists(path))
					{
						auto parent = *(EntityPtr*)inputs[1].data;
						if (parent)
						{
							auto e = Entity::create();
							e->load(path);
							if (auto node = e->get_component<cNode>(); node)
								node->set_pos(*(vec3*)inputs[2].data);
							parent->add_child(e);
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

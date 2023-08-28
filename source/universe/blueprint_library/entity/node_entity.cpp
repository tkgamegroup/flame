#include "node_entity.h"
#include "../library.h"
#include "../../../foundation/typeinfo.h"
#include "../../entity_private.h"
#include "../../components/node_private.h"

namespace flame
{
	void add_node_templates_entity(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Self",
			{
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				*(EntityPtr*)outputs[0].data = bp_self;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
		library->add_template("Get Name",
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
		library->add_template("Get Pos",
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
		library->add_template("Spawn Prefabs",
			{
				{
					.name = "Execute",
					.allowed_types = { TypeInfo::get<Signal>() },
					.default_value = "true"
				},
				{
					.name = "Path",
					.allowed_types = { TypeInfo::get<std::filesystem::path>() }
				},
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Number",
					.allowed_types = { TypeInfo::get<uvec3>() },
					.default_value = "1,1,1"
				},
				{
					.name = "Spacing",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Odd Offset",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "Execute",
					.allowed_types = { TypeInfo::get<Signal>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				auto& in_signal = *(Signal*)inputs[0].data;
				auto& out_signal = *(Signal*)outputs[0].data;
				if (!in_signal.v)
				{
					out_signal.v = false;
					return;
				}
				else
					out_signal.v = true;

				auto& path = *(std::filesystem::path*)inputs[1].data;
				if (!path.empty() && std::filesystem::exists(path))
				{
					auto parent = *(EntityPtr*)inputs[2].data;
					if (parent)
					{
						auto number = *(uvec3*)inputs[3].data;
						if (number.x > 0 && number.y > 0 && number.z > 0)
						{
							auto prefab = Entity::create();
							prefab->load(path);

							auto spacing = *(vec3*)inputs[4].data;
							auto odd_offset = *(float*)inputs[5].data;

							for (auto x = 0; x < number.x; x++)
							{
								for (auto y = 0; y < number.y; y++)
								{
									for (auto z = 0; z < number.z; z++)
									{
										auto e = prefab->duplicate();
										if (auto node = e->get_component<cNode>(); node)
											node->set_pos(vec3(x, y, z) * spacing);
										parent->add_child(e);
									}
								}
							}

							delete prefab;
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

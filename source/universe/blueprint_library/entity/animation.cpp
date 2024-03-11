#include "../../../foundation/blueprint.h"
#include "../../components/armature_private.h"

namespace flame
{
	void add_animation_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Play Animation", "", BlueprintNodeFlagNone,
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
					.name = "Speed",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				},
				{
					.name = "Loop",
					.allowed_types = { TypeInfo::get<bool>() },
					.default_value = "true"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					cArmaturePtr armature = nullptr;
					armature = entity->get_component<cArmatureT>();
					if (!armature)
					{
						if (!entity->children.empty())
							armature = entity->children[0]->get_component<cArmatureT>();
					}
					if (armature)
					{
						auto name = *(uint*)inputs[1].data;
						auto speed = *(float*)inputs[2].data;
						auto loop = *(bool*)inputs[3].data;
						if (armature->playing_name != name)
						{
							armature->play(name);
							armature->speed = speed;
							armature->loop = loop;
						}
					}
				}
			}
		);

		library->add_template("Stop Animation", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					cArmaturePtr armature = nullptr;
					armature = entity->get_component<cArmatureT>();
					if (!armature)
					{
						if (!entity->children.empty())
							armature = entity->children[0]->get_component<cArmatureT>();
					}
					if (armature)
						armature->stop();
				}
			}
		);

		library->add_template("Set Animation Callback", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "BPI",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
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
					cArmaturePtr armature = nullptr;
					armature = entity->get_component<cArmatureT>();
					if (!armature)
					{
						if (!entity->children.empty())
							armature = entity->children[0]->get_component<cArmatureT>();
					}
					if (armature)
					{
						auto bpi = *(BlueprintInstancePtr*)inputs[1].data;
						auto name = *(uint*)inputs[2].data;
						auto group = bpi->find_group(name);
						if (group)
							armature->bp_callbacks.push_back(group);
					}
				}
			}
		);
	}
}

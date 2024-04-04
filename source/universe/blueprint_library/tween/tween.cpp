#include "../../../foundation/blueprint.h"
#include "../../entity_private.h"
#include "../../components/bp_instance_private.h"
#include "../../systems/tween_private.h"

namespace flame
{
	void add_tween_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Tween Begin", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = sTween::instance()->begin();
				*(uint*)outputs[0].data = id;
			}
		);

		library->add_template("Tween Begin Gui", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Renderer Host",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Group_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Target Count",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "1"
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto parent = *(EntityPtr*)inputs[0].data; parent)
				{
					if (auto renderer_host = *(EntityPtr*)inputs[1].data; renderer_host)
					{
						if (auto ins = renderer_host->get_component<cBpInstance>(); ins && ins->bp_ins)
						{
							auto instance = ins->bp_ins;
							if (auto group = instance->find_group(*(uint*)inputs[2].data); group)
							{
								auto count = *(uint*)inputs[3].data;
								auto id = sTween::instance()->begin(parent, group, count);
								*(uint*)outputs[0].data = id;
							}
							else
								*(uint*)outputs[0].data = 0;
						}
						else
							*(uint*)outputs[0].data = 0;
					}
					else
						*(uint*)outputs[0].data = 0;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Set Target Entity", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					auto entity = *(EntityPtr*)inputs[1].data;
					if (entity)
						sTween::instance()->set_target(id, entity);
					else
						*(uint*)outputs[0].data = 0;

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Set Target Idx", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Idx",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					auto idx = *(uint*)inputs[1].data;
					sTween::instance()->set_target(id, idx);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);
		library->add_template("Tween Set Custom Data", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					if (inputs[1].data)
						sTween::instance()->set_custom_data(id, inputs[1].type, inputs[1].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					auto type = info.template_string.empty() ? TypeInfo::get<float>() : blueprint_type_from_template_str(info.template_string);
					if (!type)
						return false;

					info.new_inputs.resize(2);
					info.new_inputs[0] = {
						.name = "ID",
						.allowed_types = { TypeInfo::get<uint>() }
					};
					info.new_inputs[1] = {
						.name = "V",
						.allowed_types = { type }
					};
					info.new_outputs.resize(1);
					info.new_outputs[0] = {
						.name = "ID",
						.allowed_types = { TypeInfo::get<uint>() }
					};

					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("Tween End", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
					sTween::instance()->end(id);
			}
		);

		library->add_template("Tween Newline", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->newline(id);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Wait", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->wait(id, *(float*)inputs[1].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Move To", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Pos",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->move_to(id, *(vec3*)inputs[1].data, *(float*)inputs[2].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Move From", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Pos",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->move_from(id, *(vec3*)inputs[1].data, *(float*)inputs[2].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Rotate To", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Eul",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->rotate_to(id, *(vec3*)inputs[1].data, *(float*)inputs[2].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Rotate From", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Eul",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->rotate_from(id, *(vec3*)inputs[1].data, *(float*)inputs[2].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Scale To", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Scl",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec3>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					vec3 scl;
					if (inputs[1].type == TypeInfo::get<float>())
						scl = vec3(*(float*)inputs[1].data);
					else
						scl = *(vec3*)inputs[1].data;
					sTween::instance()->scale_to(id, scl, *(float*)inputs[2].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Scale From", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Scl",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec3>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					vec3 scl;
					if (inputs[1].type == TypeInfo::get<float>())
						scl = vec3(*(float*)inputs[1].data);
					else
						scl = *(vec3*)inputs[1].data;
					sTween::instance()->scale_from(id, scl, *(float*)inputs[2].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Object Color To", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->object_color_to(id, *(cvec4*)inputs[1].data, *(float*)inputs[2].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Object Color From", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->object_color_from(id, *(cvec4*)inputs[1].data, *(float*)inputs[2].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Light Color To", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<vec4>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->light_color_to(id, *(vec4*)inputs[1].data, *(float*)inputs[2].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Light Color From", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<vec4>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->light_color_from(id, *(vec4*)inputs[1].data, *(float*)inputs[2].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Alpha To", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Alpha",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->alpha_to(id, *(float*)inputs[1].data, *(float*)inputs[2].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Alpha From", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Alpha",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->alpha_from(id, *(float*)inputs[1].data, *(float*)inputs[2].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Enable", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->enable(id);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Disable", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->disable(id);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Play Animation", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->play_animation(id, *(uint*)inputs[1].data);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Kill", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					sTween::instance()->kill(id);

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween Set Callback", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Group_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					if (auto entity = *(EntityPtr*)inputs[1].data; entity)
					{
						auto ins = entity->get_component<cBpInstance>();
						if (ins && ins->bp_ins)
						{
							auto instance = ins->bp_ins;
							if (auto group = instance->find_group(*(uint*)inputs[2].data); group)
							{
								sTween::instance()->set_callback(id, group);
								*(uint*)outputs[0].data = id;
							}
							else
								*(uint*)outputs[0].data = 0;
						}
						else
							*(uint*)outputs[0].data = 0;
					}
					else
						*(uint*)outputs[0].data = 0;

					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);
	}
}

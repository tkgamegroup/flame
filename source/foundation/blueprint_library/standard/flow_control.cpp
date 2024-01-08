#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_flow_control_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Join Flow", "", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				(*(BlueprintSignal*)outputs[0].data).v = (*(uint*)inputs[0].data == 1 || *(uint*)inputs[1].data == 1)
					? 1 : 0;
			}
		);

		library->add_template("If", "", BlueprintNodeFlagHorizontalOutputs,
			{
				{
					.name = "Condition",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				}
			},
			{
				{
					.name = "True",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				},
				{
					.name = "False",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				bool ok;
				if (inputs[0].type == TypeInfo::get<bool>())
					ok = *(bool*)inputs[0].data;
				else
					ok = (*(voidptr*)inputs[0].data) != nullptr;
				(*(BlueprintSignal*)outputs[0].data).v = ok ? 1 : 0;
				(*(BlueprintSignal*)outputs[1].data).v = ok ? 0 : 1;
			}
		);

		library->add_template("If True", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Condition",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				bool ok;
				if (inputs[1].type == TypeInfo::get<bool>())
					ok = *(bool*)inputs[1].data;
				else
					ok = (*(voidptr*)inputs[1].data) != nullptr;
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If False", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Condition",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				bool ok;
				if (inputs[1].type == TypeInfo::get<bool>())
					ok = *(bool*)inputs[1].data;
				else
					ok = (*(voidptr*)inputs[1].data) != nullptr;
				execution.block->max_execute_times = ok ? 0 : 1;
			}
		);

		library->add_template("If Less", "", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto in0_ti = (TypeInfo_Data*)inputs[1].type;
				auto in1_ti = (TypeInfo_Data*)inputs[2].type;
				auto in0_p = (char*)inputs[1].data;
				auto in1_p = (char*)inputs[2].data;
				auto ok = in0_ti->as_float(in0_p) < in1_ti->as_float(in1_p);
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If Greater", "", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto in0_ti = (TypeInfo_Data*)inputs[1].type;
				auto in1_ti = (TypeInfo_Data*)inputs[2].type;
				auto in0_p = (char*)inputs[1].data;
				auto in1_p = (char*)inputs[2].data;
				auto ok = in0_ti->as_float(in0_p) > in1_ti->as_float(in1_p);
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If Equal", "", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto in0_ti = (TypeInfo_Data*)inputs[1].type;
				auto in1_ti = (TypeInfo_Data*)inputs[2].type;
				auto in0_p = (char*)inputs[1].data;
				auto in1_p = (char*)inputs[2].data;
				auto ok = in0_ti->as_float(in0_p) == in1_ti->as_float(in1_p);
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If Not Equal", "", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto in0_ti = (TypeInfo_Data*)inputs[1].type;
				auto in1_ti = (TypeInfo_Data*)inputs[2].type;
				auto in0_p = (char*)inputs[1].data;
				auto in1_p = (char*)inputs[2].data;
				auto ok = in0_ti->as_float(in0_p) != in1_ti->as_float(in1_p);
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If Less Or Equal", "", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto in0_ti = (TypeInfo_Data*)inputs[1].type;
				auto in1_ti = (TypeInfo_Data*)inputs[2].type;
				auto in0_p = (char*)inputs[1].data;
				auto in1_p = (char*)inputs[2].data;
				auto ok = in0_ti->as_float(in0_p) <= in1_ti->as_float(in1_p);
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If Greater Or Equal", "", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto in0_ti = (TypeInfo_Data*)inputs[1].type;
				auto in1_ti = (TypeInfo_Data*)inputs[2].type;
				auto in0_p = (char*)inputs[1].data;
				auto in1_p = (char*)inputs[2].data;
				auto ok = in0_ti->as_float(in0_p) >= in1_ti->as_float(in1_p);
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("Loop", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Times",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "1"
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				execution.block->max_execute_times = *(uint*)inputs[1].data;
			}
		);

		library->add_template("Loop Index", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				*(uint*)outputs[0].data = execution.block->executed_times;
			}
		);

		library->add_template("Foreach", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Vector",
					.allowed_types = { TypeInfo::get<voidptr>() }
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto vec_type = inputs[1].type;
				auto vec = (std::vector<char>*)inputs[1].data;
				if (vec && is_vector(vec_type->tag))
				{
					execution.block->loop_vector_index = 1;
					execution.block->max_execute_times = vec->size() / vec_type->get_wrapped()->size;
				}
				else
					execution.block->max_execute_times = 0;
			}
		);

		library->add_template("Foreach File", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Folder",
					.allowed_types = { TypeInfo::get<std::filesystem::path>() }
				}
			},
			{
				{
					.name = "temp_array",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<std::vector<std::filesystem::path>>() }
				}
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto& temp_array = *(std::vector<std::filesystem::path>*)outputs[1].data;
				temp_array.clear();
				auto& folder = *(std::filesystem::path*)inputs[1].data;
				if (!folder.empty())
				{
					for (auto& it : std::filesystem::directory_iterator(Path::get(folder)))
					{
						if (it.is_regular_file())
							temp_array.push_back(it.path());
					}
				}
				execution.block->loop_vector_index = 3;
				execution.block->max_execute_times = temp_array.size();
			}
		);

		library->add_template("Loop Var", "", BlueprintNodeFlagEnableTemplate,
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto block_node = execution.block->node;
				auto vec_idx = execution.block->loop_vector_index;
				auto type = outputs[0].type;
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
						if (item_type == type)
						{
							auto& vec = *(std::vector<char>*)vec_arg.data;
							auto length = vec.size() / item_type->size;
							if (i < length)
								type->copy(outputs[0].data, vec.data() + i * item_type->size);
							else
								type->create(outputs[0].data);
						}
						else
							type->create(outputs[0].data);
					}
					else
						type->create(outputs[0].data);
				}
				else
					type->create(outputs[0].data);
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					auto type = info.template_string.empty() ? TypeInfo::get<float>() : blueprint_type_from_template_str(info.template_string);
					if (!type)
						return false;

					info.new_inputs.resize(0);
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

		library->add_template("Return", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto target = execution.block;
				auto ok = false;
				while (!ok)
				{
					if (target->node->original->flags & BlueprintNodeFlagReturnTarget)
					{
						ok = true;
						break;
					}
					target = target->parent;
					if (!target)
						break;
					if (!target->node->original)
						break;
				}
				if (ok)
				{
					auto block_node = target->node;
					auto out_idx = target->block_output_index;
					if (out_idx != -1)
					{
						BlueprintAttribute out_arg = { nullptr, nullptr };
						if (out_idx < block_node->inputs.size())
							out_arg = block_node->inputs[out_idx];
						else
						{
							out_idx -= block_node->inputs.size();
							if (out_idx < block_node->outputs.size())
								out_arg = block_node->outputs[out_idx];
						}
						if (out_arg.data && out_arg.type)
						{
							auto type = inputs[0].type;
							if (out_arg.type == type)
								type->copy(out_arg.data, inputs[0].data);
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

					info.new_inputs.resize(1);
					info.new_inputs[0] = {
						.name = "V",
						.allowed_types = { type }
					};
					info.new_outputs.resize(0);
					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("Break", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Levels",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "1"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto levels = *(uint*)inputs[0].data;
				auto target = execution.block;
				while (levels)
				{
					if (target->node->original->flags & BlueprintNodeFlagBreakTarget)
						levels--;
					target = target->parent;
					if (!target)
						break;
					if (!target->node->original)
						break;
				}
				if (target)
				{
					auto block = execution.block;
					while (block != target)
					{
						block->_break();
						block = block->parent;
					}
				}
			}
		);

		library->add_template("Semaphore", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Semaphore",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto semaphore = (bool*)inputs[1].data;
				if (*semaphore)
				{
					*semaphore = false;
					execution.block->max_execute_times = 1;
				}
				else
					execution.block->max_execute_times = 0;
			}
		);

		library->add_template("Branch", "", BlueprintNodeFlagEnableTemplate | BlueprintNodeFlagHorizontalInputs | BlueprintNodeFlagHorizontalOutputs,
			{
				{
					.name = "Case 1",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			{
				{
					.name = "Branch 1",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				},
				{
					.name = "Else",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				for (auto i = 0; i < outputs_count; i++)
					(*(BlueprintSignal*)outputs[i].data).v = 0; 
				auto n = outputs_count - 1;
				for (auto i = 0; i < n; i++) 
				{
					if (*(bool*)inputs[i].data)
					{
						(*(BlueprintSignal*)outputs[i].data).v = 1; 
						return; 
					}
				}
				(*(BlueprintSignal*)outputs[n].data).v = 1;
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					auto n = info.template_string.empty() ? 1 : s2t<uint>(info.template_string);
					if (!n)
						return false;

					info.new_inputs.resize(n);
					info.new_outputs.resize(n + 1);
					for (auto i = 0; i < n; i++)
					{
						info.new_inputs[i] = {
							.name = "Case " + str(i + 1),
							.allowed_types = { TypeInfo::get<bool>() }
						};
						info.new_outputs[i] = {
							.name = "Branch " + str(i + 1),
							.allowed_types = { TypeInfo::get<BlueprintSignal>() }
						};
					}
					info.new_outputs[n] = {
						.name = "Else",
						.allowed_types = { TypeInfo::get<BlueprintSignal>() }
					};
					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("Ramp Branch", "", BlueprintNodeFlagEnableTemplate | BlueprintNodeFlagHorizontalInputs | BlueprintNodeFlagHorizontalOutputs,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Stop 1",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "Branch 1",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				},
				{
					.name = "Else",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto v = *(float*)inputs[0].data;
				for (auto i = 0; i < outputs_count; i++)
					(*(BlueprintSignal*)outputs[i].data).v = 0;
				auto n = outputs_count - 1;
				for (auto i = 0; i < n; i++) 
				{
					if (v < *(float*)inputs[i + 1].data)
					{
						(*(BlueprintSignal*)outputs[i].data).v = 1; 
						return; 
					}
				}
				(*(BlueprintSignal*)outputs[n].data).v = 1; 
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					auto n = info.template_string.empty() ? 1 : s2t<uint>(info.template_string);
					if (!n)
						return false;

					info.new_inputs.resize(n + 1);
					info.new_outputs.resize(n + 1);
					info.new_inputs[0] = {
						.name = "V",
						.allowed_types = { TypeInfo::get<float>() }
					};
					for (auto i = 0; i < n; i++)
					{
						info.new_inputs[i + 1] = {
							.name = "Stop " + str(i + 1),
							.allowed_types = { TypeInfo::get<float>() }
						};
						info.new_outputs[i] = {
							.name = "Branch " + str(i + 1),
							.allowed_types = { TypeInfo::get<BlueprintSignal>() }
						};
					}
					info.new_outputs[n] = {
						.name = "Else",
						.allowed_types = { TypeInfo::get<BlueprintSignal>() }
					};
					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("Timer", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Interval",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				},
				{
					.name = "t",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "0"
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto interval = *(float*)inputs[1].data;
				auto& t = *(float*)inputs[2].data;

				t += delta_time;
				if (t >= interval)
				{
					*(float*)inputs[1].data = 0.f;
					execution.block->max_execute_times = 1;
				}
				else
					execution.block->max_execute_times = 0;
			}
		);

		library->add_template("Co Wait", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Time",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				execution.group->wait_time = *(float*)inputs[0].data;
			}
		);

		library->add_template("Co Loop", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Time",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				},
				{
					.name = "t",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "0"
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				execution.block->max_execute_times = std::numeric_limits<int>::max();
			},
			nullptr,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto time = *(float*)inputs[1].data;
				auto& t = *(float*)inputs[2].data;

				t += delta_time;
				if (t < time)
					execution.group->wait_time = -1;
				else
				{
					t = 0.f;
					execution.block->_break();
				}
			}
		);
	}
}

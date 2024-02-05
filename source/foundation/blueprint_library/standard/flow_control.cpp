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
					.name = "V",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				(*(BlueprintSignal*)outputs[0].data).v = (*(uint*)inputs[0].data == 1 || *(uint*)inputs[1].data == 1)
					? 1 : 0;
			}
		);

		library->add_template("If", "", BlueprintNodeFlagEnableTemplate | BlueprintNodeFlagHorizontalOutputs,
			{
				{
					.name = "Cond",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				bool ok;
				if (inputs[0].type == TypeInfo::get<bool>())
					ok = *(bool*)inputs[0].data;
				else
					ok = (*(voidptr*)inputs[0].data) != nullptr;
				execution.block->max_execute_times = ok ? 1 : 0;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					std::string type_str = "T";
					std::string else_str = "";
					auto sp = SUS::split(info.template_string, ',');
					if (sp.size() == 1)
					{
						auto tk = std::string(sp[0]);
						SUS::to_upper(tk);
						if (tk == "EL")
							else_str = "EL";
						else
							type_str = tk;
					}
					else if (sp.size() == 2)
					{
						type_str = std::string(sp[0]);
						SUS::to_upper(type_str);

						else_str = std::string(sp[1]);
						SUS::to_upper(else_str);
					}

					if (type_str == "T" || type_str == "Y" || type_str == "TRUE" || type_str == "YES" || type_str == "F" || type_str == "N" || type_str == "FALSE" || type_str == "NOT")
					{
						info.new_inputs.resize(1);
						info.new_inputs[0] = {
							.name = "Cond",
							.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
						};
					}
					else if (type_str == "E" || type_str == "NE" || type_str == "L" || type_str == "G" || type_str == "LE" || type_str == "GE")
					{
						info.new_inputs.resize(2);
						info.new_inputs[0] = {
							.name = "A",
							.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
						};
						info.new_inputs[1] = {
							.name = "B",
							.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
						};
					}
					else if (type_str == "DE" || type_str == "NDE")
					{
						info.new_inputs.resize(2);
						info.new_inputs[0] = {
							.name = "A",
							.allowed_types = { TypeInfo::get<uint>() }
						};
						info.new_inputs[1] = {
							.name = "B",
							.allowed_types = { TypeInfo::get<uint>() }
						};
					}
					else
						return false;
					if (else_str == "EL")
					{
						info.new_outputs.resize(1);
						info.new_outputs[0] = {
							.name = "Else",
							.allowed_types = { TypeInfo::get<BlueprintSignal>() }
						};
					}

					if (type_str == "T" || type_str == "Y" || type_str == "TRUE" || type_str == "YES")
					{
						if (else_str.empty())
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								bool b;
								if (inputs[0].type == TypeInfo::get<bool>())
									b = *(bool*)inputs[0].data;
								else
									b = (*(voidptr*)inputs[0].data) != nullptr;
								execution.block->max_execute_times = b ? 1 : 0;
							};
						}
						else if (else_str == "EL")
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								bool b;
								if (inputs[0].type == TypeInfo::get<bool>())
									b = *(bool*)inputs[0].data;
								else
									b = (*(voidptr*)inputs[0].data) != nullptr;
								execution.block->max_execute_times = b ? 1 : 0;

								(*(BlueprintSignal*)outputs[0].data).v = b ? 0 : 1;
							};
						}
					}
					else if (type_str == "F" || type_str == "N" || type_str == "FALSE" || type_str == "NOT")
					{
						if (else_str.empty())
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								bool b;
								if (inputs[0].type == TypeInfo::get<bool>())
									b = !*(bool*)inputs[0].data;
								else
									b = (*(voidptr*)inputs[0].data) == nullptr;
								execution.block->max_execute_times = b ? 1 : 0;
							};
						}
						else if (else_str == "EL")
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								bool b;
								if (inputs[0].type == TypeInfo::get<bool>())
									b = !*(bool*)inputs[0].data;
								else
									b = (*(voidptr*)inputs[0].data) == nullptr;
								execution.block->max_execute_times = b ? 1 : 0;

								(*(BlueprintSignal*)outputs[0].data).v = b ? 0 : 1;
							};
						}
					}
					else if (type_str == "E")
					{
						if (else_str.empty())
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto in0_ti = (TypeInfo_Data*)inputs[0].type;
								auto in1_ti = (TypeInfo_Data*)inputs[1].type;
								auto in0_p = (char*)inputs[0].data;
								auto in1_p = (char*)inputs[1].data;
								auto b = in0_ti->as_float(in0_p) == in1_ti->as_float(in1_p);
								execution.block->max_execute_times = b ? 1 : 0;
							};
						}
						else if (else_str == "EL")
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto in0_ti = (TypeInfo_Data*)inputs[0].type;
								auto in1_ti = (TypeInfo_Data*)inputs[1].type;
								auto in0_p = (char*)inputs[0].data;
								auto in1_p = (char*)inputs[1].data;
								auto b = in0_ti->as_float(in0_p) == in1_ti->as_float(in1_p);
								execution.block->max_execute_times = b ? 1 : 0;

								(*(BlueprintSignal*)outputs[0].data).v = b ? 0 : 1;
							};
						}
					}
					else if (type_str == "NE")
					{
						if (else_str.empty())
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto in0_ti = (TypeInfo_Data*)inputs[0].type;
								auto in1_ti = (TypeInfo_Data*)inputs[1].type;
								auto in0_p = (char*)inputs[0].data;
								auto in1_p = (char*)inputs[1].data;
								auto b = in0_ti->as_float(in0_p) != in1_ti->as_float(in1_p);
								execution.block->max_execute_times = b ? 1 : 0;
							};
						}
						else if (else_str == "EL")
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto in0_ti = (TypeInfo_Data*)inputs[0].type;
								auto in1_ti = (TypeInfo_Data*)inputs[1].type;
								auto in0_p = (char*)inputs[0].data;
								auto in1_p = (char*)inputs[1].data;
								auto b = in0_ti->as_float(in0_p) != in1_ti->as_float(in1_p);
								execution.block->max_execute_times = b ? 1 : 0;

								(*(BlueprintSignal*)outputs[0].data).v = b ? 0 : 1;
							};
						}
					}
					else if (type_str == "L")
					{
						if (else_str.empty())
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto in0_ti = (TypeInfo_Data*)inputs[0].type;
								auto in1_ti = (TypeInfo_Data*)inputs[1].type;
								auto in0_p = (char*)inputs[0].data;
								auto in1_p = (char*)inputs[1].data;
								auto b = in0_ti->as_float(in0_p) < in1_ti->as_float(in1_p);
								execution.block->max_execute_times = b ? 1 : 0;
							};
						}
						else if (else_str == "EL")
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto in0_ti = (TypeInfo_Data*)inputs[0].type;
								auto in1_ti = (TypeInfo_Data*)inputs[1].type;
								auto in0_p = (char*)inputs[0].data;
								auto in1_p = (char*)inputs[1].data;
								auto b = in0_ti->as_float(in0_p) < in1_ti->as_float(in1_p);
								execution.block->max_execute_times = b ? 1 : 0;

								(*(BlueprintSignal*)outputs[0].data).v = b ? 0 : 1;
							};
						}

					}
					else if (type_str == "G")
					{
						if (else_str.empty())
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto in0_ti = (TypeInfo_Data*)inputs[0].type;
								auto in1_ti = (TypeInfo_Data*)inputs[1].type;
								auto in0_p = (char*)inputs[0].data;
								auto in1_p = (char*)inputs[1].data;
								auto b = in0_ti->as_float(in0_p) > in1_ti->as_float(in1_p);
								execution.block->max_execute_times = b ? 1 : 0;
							};
						}
						else if (else_str == "EL")
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto in0_ti = (TypeInfo_Data*)inputs[0].type;
								auto in1_ti = (TypeInfo_Data*)inputs[1].type;
								auto in0_p = (char*)inputs[0].data;
								auto in1_p = (char*)inputs[1].data;
								auto b = in0_ti->as_float(in0_p) > in1_ti->as_float(in1_p);
								execution.block->max_execute_times = b ? 1 : 0;

								(*(BlueprintSignal*)outputs[0].data).v = b ? 0 : 1;
							};
						}
					}
					else if (type_str == "LE")
					{
						if (else_str.empty())
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto in0_ti = (TypeInfo_Data*)inputs[0].type;
								auto in1_ti = (TypeInfo_Data*)inputs[1].type;
								auto in0_p = (char*)inputs[0].data;
								auto in1_p = (char*)inputs[1].data;
								auto b = in0_ti->as_float(in0_p) <= in1_ti->as_float(in1_p);
								execution.block->max_execute_times = b ? 1 : 0;
							};
						}
						else if (else_str == "EL")
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto in0_ti = (TypeInfo_Data*)inputs[0].type;
								auto in1_ti = (TypeInfo_Data*)inputs[1].type;
								auto in0_p = (char*)inputs[0].data;
								auto in1_p = (char*)inputs[1].data;
								auto b = in0_ti->as_float(in0_p) <= in1_ti->as_float(in1_p);
								execution.block->max_execute_times = b ? 1 : 0;

								(*(BlueprintSignal*)outputs[0].data).v = b ? 0 : 1;
							};
						}
					}
					else if (type_str == "GE")
					{
						if (else_str.empty())
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto in0_ti = (TypeInfo_Data*)inputs[0].type;
								auto in1_ti = (TypeInfo_Data*)inputs[1].type;
								auto in0_p = (char*)inputs[0].data;
								auto in1_p = (char*)inputs[1].data;
								auto b = in0_ti->as_float(in0_p) >= in1_ti->as_float(in1_p);
								execution.block->max_execute_times = b ? 1 : 0;
							};
						}
						else if (else_str == "EL")
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto in0_ti = (TypeInfo_Data*)inputs[0].type;
								auto in1_ti = (TypeInfo_Data*)inputs[1].type;
								auto in0_p = (char*)inputs[0].data;
								auto in1_p = (char*)inputs[1].data;
								auto b = in0_ti->as_float(in0_p) >= in1_ti->as_float(in1_p);
								execution.block->max_execute_times = b ? 1 : 0;

								(*(BlueprintSignal*)outputs[0].data).v = b ? 0 : 1;
							};
						}
					}
					else if (type_str == "DE")
					{
						if (else_str.empty())
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto b = *(uint*)inputs[0].data % *(uint*)inputs[1].data == 0;
								execution.block->max_execute_times = b ? 1 : 0;
							};
						}
						else if (else_str == "EL")
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto b = *(uint*)inputs[0].data % *(uint*)inputs[1].data == 0;
								execution.block->max_execute_times = b ? 1 : 0;

								(*(BlueprintSignal*)outputs[0].data).v = b ? 0 : 1;
							};
						}
					}
					else if (type_str == "NDE")
					{
						if (else_str.empty())
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto b = *(uint*)inputs[0].data % *(uint*)inputs[1].data != 0;
								execution.block->max_execute_times = b ? 1 : 0;
							};
						}
						else if (else_str == "EL")
						{
							info.new_begin_block_function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
								auto b = *(uint*)inputs[0].data % *(uint*)inputs[1].data != 0;
								execution.block->max_execute_times = b ? 1 : 0;

								(*(BlueprintSignal*)outputs[0].data).v = b ? 0 : 1;
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

		library->add_template("While", "", BlueprintNodeFlagBreakTarget,
			{
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				execution.block->max_execute_times = 10000;
			}
		);

		library->add_template("Loop", "", BlueprintNodeFlagBreakTarget,
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
				execution.block->max_execute_times = *(uint*)inputs[0].data;
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

		library->add_template("Foreach", "", BlueprintNodeFlagBreakTarget,
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
				auto vec_type = inputs[0].type;
				auto vec = (std::vector<char>*)inputs[0].data;
				if (vec && is_vector(vec_type->tag))
				{
					execution.block->loop_vector_index = 0;
					execution.block->max_execute_times = vec->size() / vec_type->get_wrapped()->size;
				}
				else
					execution.block->max_execute_times = 0;
			}
		);

		library->add_template("Foreach File", "", BlueprintNodeFlagBreakTarget,
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
				auto& temp_array = *(std::vector<std::filesystem::path>*)outputs[0].data;
				temp_array.clear();
				auto& folder = *(std::filesystem::path*)inputs[0].data;
				if (!folder.empty())
				{
					for (auto& it : std::filesystem::directory_iterator(Path::get(folder)))
					{
						if (it.is_regular_file())
							temp_array.push_back(it.path());
					}
				}
				execution.block->loop_vector_index = 1;
				execution.block->max_execute_times = temp_array.size();
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& temp_array = *(std::vector<std::filesystem::path>*)outputs[0].data;
				temp_array.clear();
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
					if (!target || !target->node->original)
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
				while (true)
				{
					if (target->node->original->flags & BlueprintNodeFlagBreakTarget)
						levels--;
					if (levels == 0)
						break;
					target = target->parent;
					if (!target || !target->node->original)
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
					target->_break();
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
				auto& semaphore = *(bool*)inputs[0].data;
				if (semaphore)
				{
					semaphore = false;
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
					.flags = BlueprintSlotFlagBeginWidget | BlueprintSlotFlagEndWidget,
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
							.flags = BlueprintSlotFlagBeginWidget | BlueprintSlotFlagEndWidget,
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

		library->add_template("Select", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Case 1",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Opt 1",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto n = (inputs_count - 1) / 2;
				for (auto i = 0; i < n; i++)
				{
					if (inputs[0].type->compare(inputs[0].data, inputs[i * 2 + 1].data))
					{
						outputs[0].type->copy(outputs[0].data, inputs[i * 2 + 2].data);
						return;
					}
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					auto n = 1;
					auto case_type = TypeInfo::get<float>();
					auto opt_type = TypeInfo::get<float>();
					auto sp = SUS::to_string_vector(SUS::split(info.template_string, ','));
					if (sp.size() >= 1)
						n = s2t<uint>(sp[0]);
					if (sp.size() >= 2)
						case_type = blueprint_type_from_template_str(sp[1]);
					if (sp.size() >= 3)
						opt_type = blueprint_type_from_template_str(sp[2]);
					if (!n || !case_type || !opt_type)
						return false;

					info.new_inputs.resize(n * 2 + 1);
					info.new_inputs[0] = {
						.name = "V",
						.allowed_types = { case_type }
					};
					for (auto i = 0; i < n; i++)
					{
						info.new_inputs[i * 2 + 1] = {
							.name = "Case " + str(i + 1),
							.allowed_types = { case_type }
						};
						info.new_inputs[i * 2 + 2] = {
							.name = "Opt " + str(i + 1),
							.allowed_types = { opt_type }
						};
					}
					info.new_outputs.resize(1);
					info.new_outputs[0] = {
						.name = "V",
						.allowed_types = { opt_type }
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
					.flags = BlueprintSlotFlagBeginWidget | BlueprintSlotFlagEndWidget,
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
							.flags = BlueprintSlotFlagBeginWidget | BlueprintSlotFlagEndWidget,
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

		library->add_template("Ramp Select", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Stop 1",
					.flags = BlueprintSlotFlagBeginWidget,
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Opt 1",
					.flags = BlueprintSlotFlagEndWidget,
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto v = *(float*)inputs[0].data;
				auto n = (inputs_count - 1) / 2;
				for (auto i = 0; i < n; i++)
				{
					if (v < *(float*)inputs[i * 2 + 1].data)
					{
						outputs[0].type->copy(outputs[0].data, inputs[i * 2 + 2].data);
						return;
					}
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					auto n = 1;
					auto type = TypeInfo::get<float>();
					auto sp = SUS::to_string_vector(SUS::split(info.template_string, ','));
					if (sp.size() >= 1)
						n = s2t<uint>(sp[0]);
					if (sp.size() >= 2)
						type = blueprint_type_from_template_str(sp[1]);
					if (!n || !type)
						return false;

					info.new_inputs.resize(n * 2 + 1);
					info.new_inputs[0] = {
						.name = "V",
						.allowed_types = { TypeInfo::get<float>() }
					};
					for (auto i = 0; i < n; i++)
					{
						info.new_inputs[i * 2 + 1] = {
							.name = "Stop " + str(i + 1),
							.flags = BlueprintSlotFlagBeginWidget,
							.allowed_types = { TypeInfo::get<float>() }
						};
						info.new_inputs[i * 2 + 2] = {
							.name = "Opt " + str(i + 1),
							.flags = BlueprintSlotFlagEndWidget,
							.allowed_types = { type }
						};
					}
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
				auto interval = *(float*)inputs[0].data;
				auto& t = *(float*)inputs[1].data;

				t += delta_time;
				if (t >= interval)
				{
					t = 0.f;
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
				auto time = *(float*)inputs[0].data;
				auto& t = *(float*)inputs[1].data;

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

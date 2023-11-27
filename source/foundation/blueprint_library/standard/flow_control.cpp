#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_flow_control_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Join Flow", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				(*(BlueprintSignal*)outputs[0].data).v = (*(uint*)inputs[0].data == 1 || *(uint*)inputs[1].data == 1)
					? 1 : 0;
			}
		);

		library->add_template("If", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				bool ok;
				if (inputs[0].type == TypeInfo::get<bool>())
					ok = *(bool*)inputs[0].data;
				else
					ok = (*(voidptr*)inputs[0].data) != nullptr;
				(*(BlueprintSignal*)outputs[0].data).v = ok ? 1 : 0;
				(*(BlueprintSignal*)outputs[1].data).v = ok ? 0 : 1;
			}
		);

		library->add_template("If True", "",
			{
				{
					.name = "Condition",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				}
			},
			{
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				bool ok;
				if (inputs[1].type == TypeInfo::get<bool>())
					ok = *(bool*)inputs[1].data;
				else
					ok = (*(voidptr*)inputs[1].data) != nullptr;
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If False", "",
			{
				{
					.name = "Condition",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				}
			},
			{
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				bool ok;
				if (inputs[1].type == TypeInfo::get<bool>())
					ok = *(bool*)inputs[1].data;
				else
					ok = (*(voidptr*)inputs[1].data) != nullptr;
				execution.block->max_execute_times = ok ? 0 : 1;
			}
		);

		library->add_template("If Less", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto in0_ti = (TypeInfo_Data*)inputs[1].type;
				auto in1_ti = (TypeInfo_Data*)inputs[2].type;
				auto in0_p = (char*)inputs[1].data;
				auto in1_p = (char*)inputs[2].data;
				auto ok = in0_ti->as_float(in0_p) < in1_ti->as_float(in1_p);
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If Greater", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto in0_ti = (TypeInfo_Data*)inputs[1].type;
				auto in1_ti = (TypeInfo_Data*)inputs[2].type;
				auto in0_p = (char*)inputs[1].data;
				auto in1_p = (char*)inputs[2].data;
				auto ok = in0_ti->as_float(in0_p) > in1_ti->as_float(in1_p);
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If Equal", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto in0_ti = (TypeInfo_Data*)inputs[1].type;
				auto in1_ti = (TypeInfo_Data*)inputs[2].type;
				auto in0_p = (char*)inputs[1].data;
				auto in1_p = (char*)inputs[2].data;
				auto ok = in0_ti->as_float(in0_p) == in1_ti->as_float(in1_p);
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If Not Equal", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto in0_ti = (TypeInfo_Data*)inputs[1].type;
				auto in1_ti = (TypeInfo_Data*)inputs[2].type;
				auto in0_p = (char*)inputs[1].data;
				auto in1_p = (char*)inputs[2].data;
				auto ok = in0_ti->as_float(in0_p) != in1_ti->as_float(in1_p);
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If Less Or Equal", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto in0_ti = (TypeInfo_Data*)inputs[1].type;
				auto in1_ti = (TypeInfo_Data*)inputs[2].type;
				auto in0_p = (char*)inputs[1].data;
				auto in1_p = (char*)inputs[2].data;
				auto ok = in0_ti->as_float(in0_p) <= in1_ti->as_float(in1_p);
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If Greater Or Equal", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto in0_ti = (TypeInfo_Data*)inputs[1].type;
				auto in1_ti = (TypeInfo_Data*)inputs[2].type;
				auto in0_p = (char*)inputs[1].data;
				auto in1_p = (char*)inputs[2].data;
				auto ok = in0_ti->as_float(in0_p) >= in1_ti->as_float(in1_p);
				execution.block->max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("Loop", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				execution.block->max_execute_times = *(uint*)inputs[1].data;
			}
		);

		library->add_template("Loop Index", "",
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				*(uint*)outputs[0].data = execution.block->executed_times;
			}
		);

		library->add_template("Foreach", "",
			{
				{
					.name = "Vector",
					.allowed_types = { TypeInfo::get<voidptr>() }
				}
			},
			{
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
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

		library->add_template("Foreach File", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
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

#define LOOP_VAR_TEMPLATE(TYPE, DV) \
		library->add_template("Loop Var " #TYPE, "",\
			{\
			},\
			{\
				{\
					.name = "V",\
					.allowed_types = { TypeInfo::get<TYPE>() }\
				}\
			},\
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {\
				auto block_node = execution.block->node;\
				auto vec_idx = execution.block->loop_vector_index;\
				if (vec_idx != -1)\
				{\
					BlueprintAttribute vec_arg = { nullptr, nullptr };\
					if (vec_idx < block_node->inputs.size())\
						vec_arg = block_node->inputs[vec_idx];\
					else\
					{\
						vec_idx -= block_node->inputs.size();\
						if (vec_idx < block_node->outputs.size())\
							vec_arg = block_node->outputs[vec_idx];\
					}\
					if (vec_arg.data && vec_arg.type)\
					{\
						auto i = execution.block->executed_times;\
						auto item_type = vec_arg.type->get_wrapped();\
						if (item_type == TypeInfo::get<TYPE>())\
						{\
							auto& vec = *(std::vector<char>*)vec_arg.data;\
							auto length = vec.size() / item_type->size;\
							if (i < length)\
								*(TYPE*)outputs[0].data = *(TYPE*)(vec.data() + i * item_type->size);\
							else\
								*(TYPE*)outputs[0].data = TYPE(DV);\
						}\
						else\
							*(TYPE*)outputs[0].data = TYPE(DV);\
					}\
					else\
						*(TYPE*)outputs[0].data = TYPE(DV);\
				}\
				else\
					*(TYPE*)outputs[0].data = TYPE(DV);\
			}\
		);

		LOOP_VAR_TEMPLATE(bool, false);
		LOOP_VAR_TEMPLATE(int, 0);
		LOOP_VAR_TEMPLATE(uint, 0);
		LOOP_VAR_TEMPLATE(float, 0);
		LOOP_VAR_TEMPLATE(ivec2, 0);
		LOOP_VAR_TEMPLATE(ivec3, 0);
		LOOP_VAR_TEMPLATE(ivec4, 0);
		LOOP_VAR_TEMPLATE(uvec2, 0);
		LOOP_VAR_TEMPLATE(uvec3, 0);
		LOOP_VAR_TEMPLATE(uvec4, 0);
		LOOP_VAR_TEMPLATE(cvec2, 0);
		LOOP_VAR_TEMPLATE(cvec3, 0);
		LOOP_VAR_TEMPLATE(cvec4, 0);
		LOOP_VAR_TEMPLATE(vec2, 0);
		LOOP_VAR_TEMPLATE(vec3, 0);
		LOOP_VAR_TEMPLATE(vec4, 0);
		LOOP_VAR_TEMPLATE(std::string, "");
		LOOP_VAR_TEMPLATE(std::wstring, L"");
		LOOP_VAR_TEMPLATE(std::filesystem::path, L"");

#undef LOOP_VAR_TEMPLATE

#define RETURN_TEMPLATE(TYPE) \
		library->add_template("Return " #TYPE, "",\
			{\
				{\
					.name = "V",\
					.allowed_types = { TypeInfo::get<TYPE>() }\
				},\
				{\
					.name = "Levels",\
					.allowed_types = { TypeInfo::get<uint>() },\
					.default_value = "2"\
				}\
			},\
			{\
			},\
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {\
				auto levels = *(uint*)inputs[1].data - 1;\
				auto target_block = execution.block;\
				while (levels && target_block)\
				{\
					target_block = target_block->parent;\
					levels--;\
				}\
				if (target_block)\
				{\
					auto block_node = target_block->node;\
					auto out_idx = target_block->block_output_index;\
					if (out_idx != -1)\
					{\
						BlueprintAttribute out_arg = { nullptr, nullptr };\
						if (out_idx < block_node->inputs.size())\
							out_arg = block_node->inputs[out_idx];\
						else\
						{\
							out_idx -= block_node->inputs.size();\
							if (out_idx < block_node->outputs.size())\
								out_arg = block_node->outputs[out_idx];\
						}\
						if (out_arg.data && out_arg.type)\
						{\
							if (out_arg.type == TypeInfo::get<TYPE>())\
								*(TYPE*)out_arg.data = *(TYPE*)inputs[0].data;\
						}\
					}\
				}\
			}\
		);

		RETURN_TEMPLATE(bool);
		RETURN_TEMPLATE(int);
		RETURN_TEMPLATE(uint);
		RETURN_TEMPLATE(float);
		RETURN_TEMPLATE(ivec2);
		RETURN_TEMPLATE(ivec3);
		RETURN_TEMPLATE(ivec4);
		RETURN_TEMPLATE(uvec2);
		RETURN_TEMPLATE(uvec3);
		RETURN_TEMPLATE(uvec4);
		RETURN_TEMPLATE(cvec2);
		RETURN_TEMPLATE(cvec3);
		RETURN_TEMPLATE(cvec4);
		RETURN_TEMPLATE(vec2);
		RETURN_TEMPLATE(vec3);
		RETURN_TEMPLATE(vec4);
		RETURN_TEMPLATE(std::string);
		RETURN_TEMPLATE(std::wstring);
		RETURN_TEMPLATE(std::filesystem::path);

#undef RETURN_TEMPLATE

		library->add_template("Break", "",
			{
				{
					.name = "Levels",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "2"
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto levels = *(uint*)inputs[0].data;
				auto stop_block = execution.block;
				while (levels && stop_block)
				{
					stop_block = stop_block->parent;
					levels--;
				}
				if (stop_block)
				{
					auto block = execution.block;
					while (block != stop_block)
					{
						block->_break();
						block = block->parent;
					}
				}
			}
		);

		library->add_template("Semaphore", "",
			{
				{
					.name = "Semaphore",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			{
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
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

#define BRANCH_TEMPLATE(N) \
		{\
			std::vector<BlueprintSlotDesc> inputs;\
			std::vector<BlueprintSlotDesc> outputs;\
			for (auto i = 0; i < N; i++) \
			{\
				inputs.push_back({\
					.name = "Case " + std::to_string(i + 1),\
					.allowed_types = { TypeInfo::get<bool>() }\
				});\
			};\
			for (auto i = 0; i < N; i++) \
			{\
				outputs.push_back({\
					.name = "Branch " + std::to_string(i + 1),\
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }\
				});\
			};\
			outputs.push_back({\
				.name = "Else Branch",\
				.allowed_types = { TypeInfo::get<BlueprintSignal>() }\
			});\
			library->add_template(std::format("Branch {}", N), "",\
				inputs,\
				outputs,\
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {\
					for (auto i = 0; i < N + 1; i++) \
						(*(BlueprintSignal*)outputs[i].data).v = 0;\
					for (auto i = 0; i < N; i++) \
					{\
						if (*(bool*)inputs[i].data)\
						{\
							(*(BlueprintSignal*)outputs[i].data).v = 1;\
							return;\
						}\
					}\
					(*(BlueprintSignal*)outputs[N].data).v = 1;\
				}\
			);\
		}

		BRANCH_TEMPLATE(2);
		BRANCH_TEMPLATE(3);
		BRANCH_TEMPLATE(4);
		BRANCH_TEMPLATE(5);
		BRANCH_TEMPLATE(6);
		BRANCH_TEMPLATE(7);
		BRANCH_TEMPLATE(8);

#undef BRANCH_TEMPLATE

#define SELECT_BRANCH_TEMPLATE(N) \
		{\
			std::vector<BlueprintSlotDesc> inputs;\
			std::vector<BlueprintSlotDesc> outputs;\
			inputs.push_back({\
				.name = "V",\
				.allowed_types = { TypeInfo::get<uint>() }\
			});\
			for (auto i = 0; i < N; i++) \
			{\
				inputs.push_back({\
					.name = "Case " + std::to_string(i + 1),\
					.allowed_types = { TypeInfo::get<uint>() }\
				});\
			};\
			for (auto i = 0; i < N; i++) \
			{\
				outputs.push_back({\
					.name = "Branch " + std::to_string(i + 1),\
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }\
				});\
			};\
			outputs.push_back({\
				.name = "Else Branch",\
				.allowed_types = { TypeInfo::get<BlueprintSignal>() }\
			});\
			library->add_template(std::format("Select Branch {}", N), "",\
				inputs,\
				outputs,\
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {\
					auto v = *(uint*)inputs[0].data;\
					for (auto i = 0; i < N + 1; i++) \
						(*(BlueprintSignal*)outputs[i].data).v = 0;\
					for (auto i = 0; i < N; i++) \
					{\
						if (v == *(uint*)inputs[i + 1].data)\
						{\
							(*(BlueprintSignal*)outputs[i].data).v = 1;\
							return;\
						}\
					}\
					(*(BlueprintSignal*)outputs[N].data).v = 1;\
				}\
			);\
		}

		SELECT_BRANCH_TEMPLATE(2);
		SELECT_BRANCH_TEMPLATE(3);
		SELECT_BRANCH_TEMPLATE(4);
		SELECT_BRANCH_TEMPLATE(5);
		SELECT_BRANCH_TEMPLATE(6);
		SELECT_BRANCH_TEMPLATE(7);
		SELECT_BRANCH_TEMPLATE(8);

#undef SELECT_BRANCH_TEMPLATE

#define RAMP_BRANCH_TEMPLATE(N) \
		{\
			std::vector<BlueprintSlotDesc> inputs;\
			std::vector<BlueprintSlotDesc> outputs;\
			inputs.push_back({\
				.name = "V",\
				.allowed_types = { TypeInfo::get<float>() }\
			});\
			for (auto i = 0; i < N; i++) \
			{\
				inputs.push_back({\
					.name = "Stop " + std::to_string(i + 1),\
					.allowed_types = { TypeInfo::get<float>() }\
				});\
			};\
			for (auto i = 0; i < N; i++) \
			{\
				outputs.push_back({\
					.name = "Branch " + std::to_string(i + 1),\
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }\
				});\
			};\
			outputs.push_back({\
				.name = "Else Branch",\
				.allowed_types = { TypeInfo::get<BlueprintSignal>() }\
			});\
			library->add_template(std::format("Ramp Branch {}", N), "",\
				inputs,\
				outputs,\
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {\
					auto v = *(float*)inputs[0].data;\
					for (auto i = 0; i < N + 1; i++) \
						(*(BlueprintSignal*)outputs[i].data).v = 0;\
					for (auto i = 0; i < N; i++) \
					{\
						if (v < *(float*)inputs[i + 1].data)\
						{\
							(*(BlueprintSignal*)outputs[i].data).v = 1;\
							return;\
						}\
					}\
					(*(BlueprintSignal*)outputs[N].data).v = 1;\
				}\
			);\
		}

		RAMP_BRANCH_TEMPLATE(2);
		RAMP_BRANCH_TEMPLATE(3);
		RAMP_BRANCH_TEMPLATE(4);
		RAMP_BRANCH_TEMPLATE(5);
		RAMP_BRANCH_TEMPLATE(6);
		RAMP_BRANCH_TEMPLATE(7);
		RAMP_BRANCH_TEMPLATE(8);

#undef RAMP_BRANCH_TEMPLATE

		library->add_template("Timer", "",
			{
				{
					.name = "Time",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Interval",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				}
			},
			{
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto& time = *(float*)inputs[1].data;
				auto interval = *(float*)inputs[2].data;

				time += delta_time;
				if (time >= interval)
				{
					*(float*)inputs[1].data = 0.f;
					execution.block->max_execute_times = 1;
				}
				else
					execution.block->max_execute_times = 0;
			}
		);

		library->add_template("Co Wait", "",
			{
				{
					.name = "Time",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				execution.group->wait_time = *(float*)inputs[0].data;
			}
		);
	}
}

#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_type_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("To Float", "",
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(float*)outputs[0].data = inputs[0].type->as_float(inputs[0].data);
			}
		);

		library->add_template("To Int", "",
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<int>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(int*)outputs[0].data = inputs[0].type->as_int(inputs[0].data);
			}
		);

		library->add_template("To Uint", "",
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(uint*)outputs[0].data = inputs[0].type->as_uint(inputs[0].data);
			}
		);

		library->add_template("Array Find", "",
			{
				{
					.name = "Array",
					.allowed_types = { TypeInfo::get<voidptr>() }
				},
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<voidptr>() }
				}
			},
			{
				{
					.name = "Index",
					.allowed_types = { TypeInfo::get<int>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto array_type = inputs[0].type;
				if (array_type && is_vector(array_type->tag))
				{
					auto parray = (std::vector<char>*)inputs[0].data;
					auto item_type = array_type->get_wrapped();
					int size = parray->size() / item_type->size;

					auto pvalue = *(voidptr*)inputs[1].data;
					for (auto i = 0; i < size; i++)
					{
						if (item_type->compare(parray->data() + i * item_type->size, pvalue))
						{
							*(int*)outputs[0].data = i;
							return;
						}
					}
				}
				*(int*)outputs[0].data = -1;
			}
		);

		library->add_template("Arrary Remove If", "",
			{
				{
					.name = "Array",
					.allowed_types = { TypeInfo::get<voidptr>() }
				}
			},
			{
				{
					.name = "ok",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<bool>() },
					.default_value = "false"
				},
				{
					.name = "to_remove_indices",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<std::vector<int>>() }
				}
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutingBlock& block) {
				auto array_type = inputs[1].type;
				if (array_type && is_vector(array_type->tag))
				{
					auto parray = (std::vector<char>*)inputs[1].data;
					auto item_type = array_type->get_wrapped();
					int size = parray->size() / item_type->size;

					block.max_execute_times = size;
					block.loop_vector_index = 1;
					block.block_output_index = 3;
				}
				else
					block.max_execute_times = 0;
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& to_remove_list = *(std::vector<int>*)outputs[2].data;
				if (!to_remove_list.empty())
				{
					auto array_type = inputs[1].type;
					auto parray = (std::vector<char>*)inputs[1].data;
					auto item_type = array_type->get_wrapped();

					for (auto i = 0; i < to_remove_list.size(); i++)
					{
						auto index = to_remove_list[i];
						parray->erase(parray->begin() + index * item_type->size, parray->begin() + (index + 1) * item_type->size);
						for (auto j = i + 1; j < to_remove_list.size(); j++)
						{
							if (to_remove_list[j] > index)
								to_remove_list[j]--;
						}
					}
					to_remove_list.clear();
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto ok = *(bool*)outputs[1].data;
				if (ok)
				{
					auto& to_remove_list = *(std::vector<int>*)outputs[2].data;
					to_remove_list.push_back(execution.block->executed_times);
					*(bool*)outputs[1].data = false;
				}
			}
		);

		library->add_template("Array Get Random Samples", "",
			{
				{
					.name = "Array",
					.allowed_types = { TypeInfo::get<voidptr>() }
				},
				{
					.name = "Number",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "1"
				}
			},
			{
				{
					.name = "Indices",
					.allowed_types = { TypeInfo::get<std::vector<uint>>() }
				},
				{
					.name = "ok",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<bool>() },
					.default_value = "false"
				}
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutingBlock& block) {
				auto array_type = inputs[1].type;
				if (array_type && is_vector(array_type->tag))
				{
					auto parray = (std::vector<char>*)inputs[1].data;
					auto item_type = array_type->get_wrapped();
					int size = parray->size() / item_type->size;

					block.max_execute_times = size;
					block.loop_vector_index = 4;
					block.block_output_index = 5;
				}
				else
					block.max_execute_times = 0;

				auto& indices = *(std::vector<uint>*)outputs[1].data;
				indices.clear();
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& indices = *(std::vector<uint>*)outputs[1].data;
				auto number = *(uint*)inputs[2].data;
				if (indices.size() > number)
				{
					std::vector<uint> selected;
					for (auto i = 0; i < number; i++)
					{
						auto idx = linearRand(0, (int)indices.size() - 1);
						selected.push_back(indices[idx]);
						indices.erase(indices.begin() + idx);
					}
					indices = selected;
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto ok = *(bool*)outputs[2].data;
				if (ok)
				{
					auto& indices = *(std::vector<uint>*)outputs[1].data;
					indices.push_back(execution.block->executed_times);
				}
				*(bool*)outputs[2].data = false;
			}
		);

		library->add_template("Array Random Sample", "",
			{
				{
					.name = "Array",
					.allowed_types = { TypeInfo::get<voidptr>() }
				}
			},
			{
				{
					.name = "Index",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "ok",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<bool>() },
					.default_value = "false"
				},
				{
					.name = "temp_array",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<std::vector<uint>>() }
				}
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutingBlock& block) {
				auto array_type = inputs[1].type;
				if (array_type && is_vector(array_type->tag))
				{
					auto parray = (std::vector<char>*)inputs[1].data;
					auto item_type = array_type->get_wrapped();
					int size = parray->size() / item_type->size;

					block.max_execute_times = size;
					block.loop_vector_index = 5;
					block.block_output_index = 4;
				}
				else
					block.max_execute_times = 0;

				auto& temp_array = *(std::vector<uint>*)outputs[3].data;
				temp_array.clear();
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& temp_array = *(std::vector<uint>*)outputs[3].data;
				*(uint*)outputs[1].data = temp_array[linearRand(0, (int)temp_array.size() - 1)];
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto ok = *(bool*)outputs[2].data;
				if (ok)
				{
					auto& temp_array = *(std::vector<uint>*)outputs[3].data;
					temp_array.push_back(execution.block->executed_times);
				}
				*(bool*)outputs[2].data = false;
			}
		);
	}
}

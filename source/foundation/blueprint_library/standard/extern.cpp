#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"
#include "../../sheet_private.h"

namespace flame
{
	void add_extern_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Get Static BPI", "", BlueprintNodeFlagNone,
			{
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
				auto& ret = *(BlueprintInstancePtr*)outputs[0].data;
				if (!ret)
					ret = BlueprintInstance::get(*(uint*)inputs[0].data);
			}
		);

		library->add_template("Get BP V", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Instance", 
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
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
				if (auto instance = *(BlueprintInstancePtr*)inputs[0].data; instance)
				{
					for (auto i = 0; i < outputs_count; i++)
					{
						auto type = outputs[i].type;
						auto it = instance->variables.find(*(uint*)inputs[i + 1].data);
						if (it != instance->variables.end())
						{
							auto& arg = it->second;
							if (arg.type == type ||
								(type == TypeInfo::get<uint>() && arg.type == TypeInfo::get<int>()) ||
								(type == TypeInfo::get<int>() && arg.type == TypeInfo::get<uint>()))
								type->copy(outputs[i].data, arg.data);
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
						.name = "Instance",
						.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
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

		library->add_template("Set BP V", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Instance",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
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
				if (auto instance = *(BlueprintInstancePtr*)inputs[0].data; instance)
				{
					for (auto i = 1; i < inputs_count; i += 2)
					{
						auto it = instance->variables.find(*(uint*)inputs[i].data);
						if (it != instance->variables.end())
						{
							auto type = inputs[i + 1].type;
							auto& arg = it->second;
							if (it->second.type == type ||
								(type == TypeInfo::get<uint>() && it->second.type == TypeInfo::get<int>()) ||
								(type == TypeInfo::get<int>() && it->second.type == TypeInfo::get<uint>()))
								type->copy(it->second.data, inputs[i + 1].data);
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
						.name = "Instance",
						.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
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

		library->add_template("BP Array Clear", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Instance",
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
				if (auto instance = *(BlueprintInstancePtr*)inputs[0].data; instance)
				{
					auto it = instance->variables.find(*(uint*)inputs[1].data);
					if (it != instance->variables.end())
					{
						auto& arg = it->second;
						if (is_array(arg.type->tag))
							resize_vector(arg.data, arg.type->get_wrapped(), 0);
					}
				}
			}
		);

		library->add_template("BP Array Get Item", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Instance",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
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
				if (auto instance = *(BlueprintInstancePtr*)inputs[0].data; instance)
				{
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
						.name = "Instance",
						.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
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

		library->add_template("BP Array Set Item", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Instance",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
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
				if (auto instance = *(BlueprintInstancePtr*)inputs[0].data; instance)
				{
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
						.name = "Instance",
						.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
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

		library->add_template("BP Array Add Item", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Instance",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
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
				auto in_type = inputs[3].type;
				if (auto instance = *(BlueprintInstancePtr*)inputs[0].data; instance)
				{
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
								in_type->copy(array.data() + array_size * item_type->size, inputs[3].data);
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
						.name = "Instance",
						.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
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

		library->add_template("Call BP", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Instance",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
				},
				{
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>(), TypeInfo::get<uint>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto instance = *(BlueprintInstancePtr*)inputs[0].data;
				auto name = *(uint*)inputs[1].data;
				if (instance)
				{
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
							.name = "Instance",
							.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
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

		library->add_template("Get Static Sheet", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "Sheet",
					.allowed_types = { TypeInfo::get<SheetPtr>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& ret = *(SheetPtr*)outputs[0].data;
				if (!ret)
					ret = Sheet::get(*(uint*)inputs[0].data);
			}
		);

		library->add_template("Get Sheet", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Path",
					.allowed_types = { TypeInfo::get<std::filesystem::path>() }
				}
			},
			{
				{
					.name = "Sheet",
					.allowed_types = { TypeInfo::get<SheetPtr>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& ret = *(SheetPtr*)outputs[0].data;
				ret = Sheet::get(*(std::filesystem::path*)inputs[0].data);
			}
		);

		library->add_template("Sheet Columns Count", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Sheet",
					.allowed_types = { TypeInfo::get<SheetPtr>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto sht = *(SheetPtr*)inputs[0].data;
				*(uint*)outputs[0].data = sht ? sht->columns.size() : 0;
			}
		);

		library->add_template("Sheet Get Column Name", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Sheet",
					.allowed_types = { TypeInfo::get<SheetPtr>() }
				},
				{
					.name = "Index",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto sht = *(SheetPtr*)inputs[0].data;
				auto index = *(uint*)inputs[1].data;
				if (sht)
				{
					if (index < sht->columns.size())
						*(uint*)outputs[0].data = sht->columns[index].name_hash;
					else
						*(uint*)outputs[0].data = 0;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Sheet Insert Column", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Sheet",
					.allowed_types = { TypeInfo::get<SheetPtr>() }
				},
				{
					.name = "Name",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Type",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto sht = *(SheetPtr*)inputs[0].data;
				if (sht)
				{
					std::string name = *(std::string*)inputs[1].data;
					std::string type_name = *(std::string*)inputs[2].data;
					auto sp = SUS::to_string_vector(SUS::split(type_name, '@'));
					TypeTag tag;
					TypeInfo::unserialize_t(sp[0], tag);
					auto type = TypeInfo::get(tag, sp[1]);
					if (type)
						sht->insert_column(name, type);
				}
			}
		);

		library->add_template("Sheet Rows Count", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Sheet",
					.allowed_types = { TypeInfo::get<SheetPtr>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto sht = *(SheetPtr*)inputs[0].data;
				*(uint*)outputs[0].data = sht ? sht->rows.size() : 0;
			}
		);

		library->add_template("Find Item In Sheet", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Sheet",
					.allowed_types = { TypeInfo::get<SheetPtr>() }
				},
				{
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Value",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "Index",
					.allowed_types = { TypeInfo::get<int>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto sht = *(SheetPtr*)inputs[0].data;
				if (sht)
				{
					auto name = *(uint*)inputs[1].data;
					auto column_idx = sht->find_column(name);
					auto type = inputs[2].type;
					if (column_idx != -1)
					{
						if (type == sht->columns[column_idx].type)
						{
							for (auto i = 0; i < sht->rows.size(); i++)
							{
								if (type->compare(sht->rows[i].datas[column_idx], inputs[2].data))
								{
									*(int*)outputs[0].data = i;
									break;
								}
							}
						}
						else
							*(int*)outputs[0].data = -1;
					}
					else
						*(int*)outputs[0].data = -1;
				}
				else
					*(int*)outputs[0].data = -1;
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
						.name = "Sheet",
						.allowed_types = { TypeInfo::get<SheetPtr>() }
					};
					info.new_inputs[1] = {
						.name = "Name_hash",
						.allowed_types = { TypeInfo::get<std::string>() }
					};
					info.new_inputs[2] = {
						.name = "Value",
						.allowed_types = { type }
					};
					info.new_outputs.resize(1);
					info.new_outputs[0] = {
						.name = "Index",
						.allowed_types = { TypeInfo::get<int>() }
					};
					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("Get SHT V", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Sheet",
					.allowed_types = { TypeInfo::get<SheetPtr>() }
				},
				{
					.name = "Row",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto sht = *(SheetPtr*)inputs[0].data; sht)
				{
					if (auto row_idx = *(uint*)inputs[1].data; row_idx < sht->rows.size())
					{
						for (auto i = 0; i < outputs_count; i++)
						{
							auto type = outputs[i].type;
							if (auto column_idx = sht->find_column(*(uint*)inputs[i + 2].data); column_idx != -1)
							{
								auto ctype = sht->columns[column_idx].type;
								if (ctype == type ||
									(type == TypeInfo::get<uint>() && ctype == TypeInfo::get<int>()) ||
									(type == TypeInfo::get<int>() && ctype == TypeInfo::get<uint>()))
									type->copy(outputs[i].data, sht->rows[row_idx].datas[column_idx]);
								else if (ctype == TypeInfo::get<StrAndHash>() && type == TypeInfo::get<uint>())
									*(uint*)outputs[i].data = (*(StrAndHash*)(sht->rows[row_idx].datas[column_idx])).h;
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

					info.new_inputs.resize(types.size() + 2);
					info.new_inputs[0] = {
						.name = "Sheet",
						.allowed_types = { TypeInfo::get<SheetPtr>() }
					};
					info.new_inputs[1] = {
						.name = "Row",
						.allowed_types = { TypeInfo::get<uint>() }
					};
					if (types.size() == 1)
					{
						info.new_inputs[2] = {
							.name = "Name_hash",
							.allowed_types = { TypeInfo::get<std::string>() }
						};
					}
					else
					{
						for (auto i = 0; i < types.size(); i++)
						{
							info.new_inputs[i + 2] = {
								.name = "Name" + str(i) + "_hash",
								.allowed_types = { TypeInfo::get<std::string>() }
							};
						}
					}
					info.new_outputs.resize(types.size());
					if (types.size() == 1)
					{
						info.new_outputs[0] = {
							.name = "V",
							.allowed_types = { types.front() }
						};
					}
					else
					{
						for (auto i = 0; i < types.size(); i++)
						{
							info.new_outputs[i] = {
								.name = "V" + str(i),
								.allowed_types = { types[i] }
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

		library->add_template("Set SHT V", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Sheet",
					.allowed_types = { TypeInfo::get<SheetPtr>() }
				},
				{
					.name = "Row",
					.allowed_types = { TypeInfo::get<uint>() }
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
				auto sht = *(SheetPtr*)inputs[0].data;
				if (sht)
				{
					if (auto row_idx = *(uint*)inputs[1].data; row_idx < sht->rows.size())
					{
						for (auto i = 1; i < inputs_count; i += 2)
						{
							auto type = outputs[i].type;
							if (auto column_idx = sht->find_column(*(uint*)inputs[i + 1].data); column_idx != -1)
							{
								auto ctype = sht->columns[column_idx].type;
								if (ctype == type ||
									(type == TypeInfo::get<uint>() && ctype == TypeInfo::get<int>()) ||
									(type == TypeInfo::get<int>() && ctype == TypeInfo::get<uint>()))
									type->copy(sht->rows[row_idx].datas[column_idx], inputs[i + 2].data);
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

					info.new_inputs.resize(types.size() * 2 + 2);
					info.new_inputs[0] = {
						.name = "Sheet",
						.allowed_types = { TypeInfo::get<SheetPtr>() }
					};
					info.new_inputs[1] = {
						.name = "Row",
						.allowed_types = { TypeInfo::get<uint>() }
					};
					if (types.size() == 1)
					{
						info.new_inputs[2] = {
							.name = "Name_hash",
							.allowed_types = { TypeInfo::get<std::string>() }
						};
						info.new_inputs[3] = {
							.name = "V",
							.allowed_types = { types.front() }
						};
					}
					else
					{
						for (auto i = 0; i < types.size(); i++)
						{
							info.new_inputs[i * 2 + 2] = {
								.name = "Name" + str(i) + "_hash",
								.allowed_types = { TypeInfo::get<std::string>() }
							};
							info.new_inputs[i * 2 + 3] = {
								.name = "V" + str(i),
								.allowed_types = { types[i] }
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

		library->add_template("Assign Sheet Row To BPI", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Sheet",
					.allowed_types = { TypeInfo::get<SheetPtr>() }
				},
				{
					.name = "Row",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Instance",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto sht = *(SheetPtr*)inputs[0].data;
				auto row_idx = *(uint*)inputs[1].data;
				auto ins = *(BlueprintInstancePtr*)inputs[2].data;
				if (sht && ins)
				{
					if (row_idx < sht->rows.size())
					{
						auto& row = sht->rows[row_idx];
						for (auto i = 0; i < sht->columns.size(); i++)
						{
							auto& column = sht->columns[i];
							auto it = ins->variables.find(column.name_hash);
							if (it != ins->variables.end())
							{
								if (it->second.type == column.type)
									column.type->copy(it->second.data, row.datas[i]);
							}
						}
					}
				}
			}
		);

		library->add_template("Broadcast", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Instance",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
				},
				{
					.name = "Message_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto instance = *(BlueprintInstancePtr*)inputs[0].data;
				auto message = *(uint*)inputs[1].data;
				if (instance)
					instance->broadcast(message);
			}
		);
	}
}

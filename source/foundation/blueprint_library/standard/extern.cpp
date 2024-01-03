#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"
#include "../../sheet_private.h"

namespace flame
{
	void add_extern_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Get Static Blueprint Instance", "", BlueprintNodeFlagNone,
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
				auto instance = *(BlueprintInstancePtr*)inputs[0].data; 
				auto name = *(uint*)inputs[1].data; 
				auto type = outputs[0].type;
				if (instance)
				{
					auto it = instance->variables.find(name); 
					if (it != instance->variables.end())
					{
						if (it->second.type == type)
							type->copy(outputs[0].data, it->second.data); 
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

					info.new_inputs.resize(2);
					info.new_inputs[0] = {
						.name = "Instance",
						.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
					};
					info.new_inputs[1] = {
						.name = "Name_hash",
						.allowed_types = { TypeInfo::get<std::string>() }
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

		library->add_template("Set BP V", "", BlueprintNodeFlagEnableTemplate,
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
				auto instance = *(BlueprintInstancePtr*)inputs[0].data;
				auto name = *(uint*)inputs[1].data;
				auto type = inputs[2].type;
				if (instance)
				{
					auto it = instance->variables.find(name);
					if (it != instance->variables.end())
					{
						if (it->second.type == type)
							type->copy(it->second.data, inputs[2].data);
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
					.allowed_types = { TypeInfo::get<std::string>() }
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
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Row",
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
				auto sht = *(SheetPtr*)inputs[0].data;
				auto name = *(uint*)inputs[1].data;
				auto type = outputs[0].type;
				if (sht)
				{
					auto column_idx = sht->find_column(name);
					if (column_idx != -1)
					{
						if (sht->columns[column_idx].type == type)
						{
							auto row_idx = *(uint*)inputs[2].data;
							if (row_idx < sht->rows.size())
								type->copy(outputs[0].data, sht->rows[row_idx].datas[column_idx]);
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
						.name = "Row",
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

		library->add_template("Set SHT V", "", BlueprintNodeFlagEnableTemplate,
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
					.name = "Row",
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
				auto sht = *(SheetPtr*)inputs[0].data;
				auto name = *(uint*)inputs[1].data;
				auto type = inputs[3].type;
				if (sht)
				{
					auto column_idx = sht->find_column(name);
					if (column_idx != -1)
					{
						if (sht->columns[column_idx].type == type)
						{
							auto row_idx = *(uint*)inputs[2].data;
							if (row_idx < sht->rows.size())
								type->copy(sht->rows[row_idx].datas[column_idx], inputs[3].data);
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
						.name = "Sheet",
						.allowed_types = { TypeInfo::get<SheetPtr>() }
					};
					info.new_inputs[1] = {
						.name = "Name_hash",
						.allowed_types = { TypeInfo::get<std::string>() }
					};
					info.new_inputs[2] = {
						.name = "Row",
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

		library->add_template("Get Child Blueprint", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
				},
				{
					.name = "Host_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
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
				*(BlueprintInstancePtr*)outputs[0].data = nullptr;
				if (auto parent = *(BlueprintInstancePtr*)inputs[0].data; parent)
				{
					if (auto host = *(uint*)inputs[1].data; host != 0)
					{
						if (auto arg = parent->get_variable(host); arg.data && arg.type->tag == TagVQU)
						{
							auto ti = (TypeInfo_VectorOfUniquePointerOfUdt*)arg.type;
							if (ti->get_wrapped()->get_wrapped() == TypeInfo::get<BlueprintInstance>())
							{
								if (auto name = *(uint*)inputs[2].data; name)
								{
									auto& array = *(std::vector<BlueprintInstancePtr>*)arg.data;
									for (auto ins : array)
									{
										if (ins->blueprint->name_hash == name)
										{
											*(BlueprintInstancePtr*)outputs[0].data = ins;
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		);

		library->add_template("Add Child Blueprint", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
				},
				{
					.name = "Host_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Path",
					.allowed_types = { TypeInfo::get<std::filesystem::path>() }
				}
			},
			{
				{
					.name = "Instance",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(BlueprintInstancePtr*)outputs[0].data = nullptr;
				if (auto parent = *(BlueprintInstancePtr*)inputs[0].data; parent)
				{
					if (auto host = *(uint*)inputs[1].data; host != 0)
					{
						if (auto arg = parent->get_variable(host); arg.data && arg.type->tag == TagVQU)
						{
							auto ti = (TypeInfo_VectorOfUniquePointerOfUdt*)arg.type;
							if (ti->get_wrapped()->get_wrapped() == TypeInfo::get<BlueprintInstance>())
							{	
								if (auto& path = *(std::filesystem::path*)inputs[2].data; !path.empty())
								{
									path = Path::get(path);
									if (std::filesystem::exists(path))
									{
										auto bp = Blueprint::get(path);
										if (bp)
										{
											auto ins = BlueprintInstance::create(bp);
											auto& array = *(std::vector<BlueprintInstancePtr>*)arg.data;
											array.push_back(ins);
											Blueprint::release(bp);
											*(BlueprintInstancePtr*)outputs[0].data = ins;
										}
									}
								}
							}
						}
					}
				}
			}
		);

		library->add_template("Add Child Blueprints", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
				},
				{
					.name = "Host_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Description",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto parent = *(BlueprintInstancePtr*)inputs[0].data; parent)
				{
					if (auto host = *(uint*)inputs[1].data; host != 0)
					{
						if (auto arg = parent->get_variable(host); arg.data && arg.type->tag == TagVQU)
						{
							auto ti = (TypeInfo_VectorOfUniquePointerOfUdt*)arg.type;
							if (ti->get_wrapped()->get_wrapped() == TypeInfo::get<BlueprintInstance>())
							{
								if (auto& description = *(std::string*)inputs[2].data; !description.empty())
								{
									auto& array = *(std::vector<BlueprintInstancePtr>*)arg.data;
									BlueprintInstancePtr last_instance = nullptr;
									auto sp = SUS::to_string_vector(SUS::split(description, '\n'));
									for (auto& l : sp)
									{
										SUS::trim(l);
										if (l.empty())
											continue;
										auto sp2 = SUS::to_string_vector(SUS::split(l, '='));
										if (sp2.size() == 1)
										{
											std::filesystem::path path(l);
											path = Path::get(path);
											if (std::filesystem::exists(path))
											{
												auto bp = Blueprint::get(path);
												if (bp)
												{
													last_instance = BlueprintInstance::create(bp);
													array.push_back(last_instance);
													Blueprint::release(bp);
												}
												else
													last_instance = nullptr;
											}
											else
												last_instance = nullptr;
										}
										else if (sp2.size() == 2)
										{
											auto arg = last_instance->get_variable(sh(sp2[0].c_str()));
											if (arg.type && arg.data)
												arg.type->unserialize(sp2[1], arg.data);
										}
									}
								}
							}
						}
					}
				}
			}
		);

		library->add_template("Remove Child Blueprint", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Parent",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
				},
				{
					.name = "Host_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Instance",
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto parent = *(BlueprintInstancePtr*)inputs[0].data; parent)
				{
					if (auto host = *(uint*)inputs[1].data; host != 0)
					{
						if (auto arg = parent->get_variable(host); arg.data && arg.type->tag == TagVU)
						{
							auto ti = (TypeInfo_VectorOfUniquePointerOfUdt*)arg.type;
							if (ti->get_wrapped()->get_wrapped() == TypeInfo::get<BlueprintInstance>())
							{
								if (auto ins = *(BlueprintInstancePtr*)inputs[2].data; ins)
								{
									auto& array = *(std::vector<BlueprintInstancePtr>*)arg.data;
									for (auto it = array.begin(); it != array.end(); it++)
									{
										if (*it == ins)
										{
											delete (*it);
											array.erase(it);
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		);

		library->add_template("Assign Sheet Row To Blueprint Instance", "", BlueprintNodeFlagNone,
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

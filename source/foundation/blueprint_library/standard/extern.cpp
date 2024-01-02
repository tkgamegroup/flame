#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"
#include "../../sheet_private.h"

namespace flame
{
	TypeInfo* type_from_template_str(std::string_view str)
	{
		TypeInfo* type = nullptr;
		if (str == "b")
			type = TypeInfo::get<bool>();
		else if (str == "f")
			type = TypeInfo::get<float>();
		else if (str == "f2")
			type = TypeInfo::get<vec2>();
		else if (str == "f3")
			type = TypeInfo::get<vec3>();
		else if (str == "f4")
			type = TypeInfo::get<vec4>();
		else if (str == "i")
			type = TypeInfo::get<int>();
		else if (str == "i2")
			type = TypeInfo::get<ivec2>();
		else if (str == "i3")
			type = TypeInfo::get<ivec3>();
		else if (str == "i4")
			type = TypeInfo::get<ivec4>();
		else if (str == "u")
			type = TypeInfo::get<uint>();
		else if (str == "u2")
			type = TypeInfo::get<uvec2>();
		else if (str == "u3")
			type = TypeInfo::get<uvec3>();
		else if (str == "u4")
			type = TypeInfo::get<uvec4>();
		else if (str == "s")
			type = TypeInfo::get<std::string>();
		else if (str == "w")
			type = TypeInfo::get<std::wstring>();
		else if (str == "p")
			type = TypeInfo::get<std::filesystem::path>();
		return type;
	}

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
					auto type = info.template_string.empty() ? TypeInfo::get<float>() : type_from_template_str(info.template_string);
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
					auto type = info.template_string.empty() ? TypeInfo::get<float>() : type_from_template_str(info.template_string);
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
						instance->call(g, nullptr, nullptr);
					}
				}
			}
		);

#define CALL_BP_TEMPLATE_void_T(TYPE) \
		library->add_template("Call BP void_" #TYPE, "", BlueprintNodeFlagNone,\
			{\
				{\
					.name = "Instance",\
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }\
				},\
				{\
					.name = "Name_hash",\
					.allowed_types = { TypeInfo::get<std::string>() }\
				},\
				{\
					.name = "V",\
					.allowed_types = { TypeInfo::get<TYPE>() }\
				}\
			},\
			{\
			},\
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {\
				auto instance = *(BlueprintInstancePtr*)inputs[0].data;\
				auto name = *(uint*)inputs[1].data;\
				if (instance)\
				{\
					if (auto g = instance->find_group(name); g)\
					{\
						std::vector<voidptr> input_args; \
						input_args.push_back(inputs[2].data); \
						instance->call(g, input_args.data(), nullptr); \
					}\
				}\
			}\
		);

		CALL_BP_TEMPLATE_void_T(bool);
		CALL_BP_TEMPLATE_void_T(int);
		CALL_BP_TEMPLATE_void_T(uint);

#undef CALL_BP_TEMPLATE_void_T

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

#define FIND_ITEM_IN_SHEET_TEMPLATE(TYPE) \
		library->add_template("Find " #TYPE " Item In Sheet", "", BlueprintNodeFlagNone,\
			{\
				{\
					.name = "Sheet",\
					.allowed_types = { TypeInfo::get<SheetPtr>() }\
				},\
				{\
					.name = "Name_hash",\
					.allowed_types = { TypeInfo::get<std::string>() }\
				},\
				{\
					.name = "Value",\
					.allowed_types = { TypeInfo::get<TYPE>() }\
				}\
			},\
			{\
				{\
					.name = "Index",\
					.allowed_types = { TypeInfo::get<int>() }\
				}\
			},\
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {\
				auto sht = *(SheetPtr*)inputs[0].data;\
				if (sht)\
				{\
					auto name = *(uint*)inputs[1].data;\
					auto column_idx = sht->find_column(name);\
					for (auto i = 0; i < sht->rows.size(); i++)\
					{\
						if (*(TYPE*)sht->rows[i].datas[column_idx] == *(TYPE*)inputs[2].data)\
						{\
							*(int*)outputs[0].data = i;\
							break;\
						}\
					}\
				}\
				else\
					*(int*)outputs[0].data = -1;\
			}\
		);

		FIND_ITEM_IN_SHEET_TEMPLATE(bool);
		FIND_ITEM_IN_SHEET_TEMPLATE(int);
		FIND_ITEM_IN_SHEET_TEMPLATE(uint);
		FIND_ITEM_IN_SHEET_TEMPLATE(float);
		FIND_ITEM_IN_SHEET_TEMPLATE(ivec2);
		FIND_ITEM_IN_SHEET_TEMPLATE(ivec3);
		FIND_ITEM_IN_SHEET_TEMPLATE(ivec4);
		FIND_ITEM_IN_SHEET_TEMPLATE(uvec2);
		FIND_ITEM_IN_SHEET_TEMPLATE(uvec3);
		FIND_ITEM_IN_SHEET_TEMPLATE(uvec4);
		FIND_ITEM_IN_SHEET_TEMPLATE(cvec2);
		FIND_ITEM_IN_SHEET_TEMPLATE(cvec3);
		FIND_ITEM_IN_SHEET_TEMPLATE(cvec4);
		FIND_ITEM_IN_SHEET_TEMPLATE(vec2);
		FIND_ITEM_IN_SHEET_TEMPLATE(vec3);
		FIND_ITEM_IN_SHEET_TEMPLATE(vec4);
		FIND_ITEM_IN_SHEET_TEMPLATE(std::string);
		FIND_ITEM_IN_SHEET_TEMPLATE(std::wstring);
		FIND_ITEM_IN_SHEET_TEMPLATE(std::filesystem::path);

#undef FIND_ITEM_IN_SHEET_TEMPLATE

#define GET_SHT_TEMPLATE(TYPE, DV) \
		library->add_template("Get SHT " #TYPE, "", BlueprintNodeFlagNone,\
			{\
				{\
					.name = "Sheet",\
					.allowed_types = { TypeInfo::get<SheetPtr>() }\
				},\
				{\
					.name = "Name_hash",\
					.allowed_types = { TypeInfo::get<std::string>() }\
				},\
				{\
					.name = "Row",\
					.allowed_types = { TypeInfo::get<uint>() }\
				}\
			},\
			{\
				{\
					.name = "V",\
					.allowed_types = { TypeInfo::get<TYPE>() }\
				}\
			},\
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {\
				auto sht = *(SheetPtr*)inputs[0].data;\
				auto name = *(uint*)inputs[1].data;\
				if (sht)\
				{\
					auto column_idx = sht->find_column(name);\
					if (column_idx != -1)\
					{\
						if (sht->columns[column_idx].type == TypeInfo::get<TYPE>())\
						{\
							auto row_idx = *(uint*)inputs[2].data;\
							if (row_idx < sht->rows.size())\
								*(TYPE*)outputs[0].data = *(TYPE*)sht->rows[row_idx].datas[column_idx];\
							else\
								*(TYPE*)outputs[0].data = TYPE(DV); \
						}\
						else\
							*(TYPE*)outputs[0].data = TYPE(DV); \
					}\
					else\
						*(TYPE*)outputs[0].data = TYPE(DV);\
				}\
				else\
					*(TYPE*)outputs[0].data = TYPE(DV);\
			}\
		);

		GET_SHT_TEMPLATE(bool, 0);
		GET_SHT_TEMPLATE(int, 0);
		GET_SHT_TEMPLATE(uint, 0);
		GET_SHT_TEMPLATE(float, 0);
		GET_SHT_TEMPLATE(ivec2, 0);
		GET_SHT_TEMPLATE(ivec3, 0);
		GET_SHT_TEMPLATE(ivec4, 0);
		GET_SHT_TEMPLATE(uvec2, 0);
		GET_SHT_TEMPLATE(uvec3, 0);
		GET_SHT_TEMPLATE(uvec4, 0);
		GET_SHT_TEMPLATE(cvec2, 0);
		GET_SHT_TEMPLATE(cvec3, 0);
		GET_SHT_TEMPLATE(cvec4, 0);
		GET_SHT_TEMPLATE(vec2, 0);
		GET_SHT_TEMPLATE(vec3, 0);
		GET_SHT_TEMPLATE(vec4, 0);
		GET_SHT_TEMPLATE(std::string, "");
		GET_SHT_TEMPLATE(std::wstring, L"");
		GET_SHT_TEMPLATE(std::filesystem::path, L"");

#undef GET_SHT_TEMPLATE

#define SET_SHT_TEMPLATE(TYPE) \
		library->add_template("Set SHT " #TYPE, "", BlueprintNodeFlagNone,\
			{\
				{\
					.name = "Sheet",\
					.allowed_types = { TypeInfo::get<SheetPtr>() }\
				},\
				{\
					.name = "Name_hash",\
					.allowed_types = { TypeInfo::get<std::string>() }\
				},\
				{\
					.name = "Row",\
					.allowed_types = { TypeInfo::get<uint>() }\
				},\
				{\
					.name = "V",\
					.allowed_types = { TypeInfo::get<TYPE>() }\
				}\
			},\
			{\
			},\
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {\
				auto sht = *(SheetPtr*)inputs[0].data;\
				auto name = *(uint*)inputs[1].data;\
				if (sht)\
				{\
					auto column_idx = sht->find_column(name);\
					if (column_idx != -1)\
					{\
						if (sht->columns[column_idx].type == TypeInfo::get<TYPE>())\
						{\
							auto row_idx = *(uint*)inputs[2].data;\
							if (row_idx < sht->rows.size())\
								*(TYPE*)sht->rows[row_idx].datas[column_idx] = *(TYPE*)inputs[3].data;\
						}\
					}\
				}\
			}\
		);

		SET_SHT_TEMPLATE(bool);
		SET_SHT_TEMPLATE(int);
		SET_SHT_TEMPLATE(uint);
		SET_SHT_TEMPLATE(float);
		SET_SHT_TEMPLATE(ivec2);
		SET_SHT_TEMPLATE(ivec3);
		SET_SHT_TEMPLATE(ivec4);
		SET_SHT_TEMPLATE(uvec2);
		SET_SHT_TEMPLATE(uvec3);
		SET_SHT_TEMPLATE(uvec4);
		SET_SHT_TEMPLATE(cvec2);
		SET_SHT_TEMPLATE(cvec3);
		SET_SHT_TEMPLATE(cvec4);
		SET_SHT_TEMPLATE(vec2);
		SET_SHT_TEMPLATE(vec3);
		SET_SHT_TEMPLATE(vec4);
		SET_SHT_TEMPLATE(std::string);
		SET_SHT_TEMPLATE(std::wstring);
		SET_SHT_TEMPLATE(std::filesystem::path);

#undef SET_SHT_TEMPLATE

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

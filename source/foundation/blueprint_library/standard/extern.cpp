#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"
#include "../../sheet_private.h"

namespace flame
{
	void add_extern_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Get Static Blueprint Instance", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(BlueprintInstancePtr*)outputs[0].data = BlueprintInstance::get(*(uint*)inputs[0].data);
			}
		);

#define GET_BP_TEMPLATE(TYPE, DV) \
		library->add_template("Get BP " #TYPE, "", \
			{\
				{\
					.name = "Instance",\
					.allowed_types = { TypeInfo::get<BlueprintInstancePtr>() }\
				},\
				{\
					.name = "Name_hash",\
					.allowed_types = { TypeInfo::get<std::string>() }\
				}\
			},\
			{\
				{\
					.name = "V",\
					.allowed_types = { TypeInfo::get<TYPE>() }\
				}\
			},\
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {\
				auto instance = *(BlueprintInstancePtr*)inputs[0].data;\
				auto name = *(uint*)inputs[1].data;\
				if (instance)\
				{\
					auto it = instance->variables.find(name);\
					if (it != instance->variables.end())\
					{\
						if (it->second.type == TypeInfo::get<TYPE>())\
							*(TYPE*)outputs[0].data = *(TYPE*)it->second.data;\
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

		GET_BP_TEMPLATE(bool, false);
		GET_BP_TEMPLATE(int, 0);
		GET_BP_TEMPLATE(uint, 0);
		GET_BP_TEMPLATE(float, 0);
		GET_BP_TEMPLATE(ivec2, 0);
		GET_BP_TEMPLATE(ivec3, 0);
		GET_BP_TEMPLATE(ivec4, 0);
		GET_BP_TEMPLATE(uvec2, 0);
		GET_BP_TEMPLATE(uvec3, 0);
		GET_BP_TEMPLATE(uvec4, 0);
		GET_BP_TEMPLATE(cvec2, 0);
		GET_BP_TEMPLATE(cvec3, 0);
		GET_BP_TEMPLATE(cvec4, 0);
		GET_BP_TEMPLATE(vec2, 0);
		GET_BP_TEMPLATE(vec3, 0);
		GET_BP_TEMPLATE(vec4, 0);
		GET_BP_TEMPLATE(std::string, "");
		GET_BP_TEMPLATE(std::wstring, L"");
		GET_BP_TEMPLATE(std::filesystem::path, L"");

#undef GET_BP_TEMPLATE

#define SET_BP_TEMPLATE(TYPE) \
		library->add_template("Set BP " #TYPE, "", \
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {\
				auto instance = *(BlueprintInstancePtr*)inputs[0].data;\
				auto name = *(uint*)inputs[1].data;\
				if (instance)\
				{\
					auto it = instance->variables.find(name);\
					if (it != instance->variables.end())\
					{\
						if (it->second.type == TypeInfo::get<TYPE>())\
							*(TYPE*)it->second.data = *(TYPE*)inputs[2].data;\
					}\
				}\
			}\
		);

		SET_BP_TEMPLATE(bool);
		SET_BP_TEMPLATE(int);
		SET_BP_TEMPLATE(uint);
		SET_BP_TEMPLATE(float);
		SET_BP_TEMPLATE(ivec2);
		SET_BP_TEMPLATE(ivec3);
		SET_BP_TEMPLATE(ivec4);
		SET_BP_TEMPLATE(uvec2);
		SET_BP_TEMPLATE(uvec3);
		SET_BP_TEMPLATE(uvec4);
		SET_BP_TEMPLATE(cvec2);
		SET_BP_TEMPLATE(cvec3);
		SET_BP_TEMPLATE(cvec4);
		SET_BP_TEMPLATE(vec2);
		SET_BP_TEMPLATE(vec3);
		SET_BP_TEMPLATE(vec4);
		SET_BP_TEMPLATE(std::string);
		SET_BP_TEMPLATE(std::wstring);
		SET_BP_TEMPLATE(std::filesystem::path);

#undef SET_BP_TEMPLATE

		library->add_template("Call BP void_void", "", 
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto instance = *(BlueprintInstancePtr*)inputs[0].data;
				auto name = *(uint*)inputs[1].data;
				if (instance)
					instance->call(name, nullptr, nullptr);
			}
		);

#define CALL_BP_TEMPLATE_void_T(TYPE) \
		library->add_template("Call BP void_" #TYPE, "", \
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {\
				auto instance = *(BlueprintInstancePtr*)inputs[0].data;\
				auto name = *(uint*)inputs[1].data;\
				if (instance)\
				{\
					std::vector<voidptr> input_args;\
					input_args.push_back(inputs[2].data);\
					instance->call(name, input_args.data(), nullptr);\
				}\
			}\
		);

		CALL_BP_TEMPLATE_void_T(bool);
		CALL_BP_TEMPLATE_void_T(int);
		CALL_BP_TEMPLATE_void_T(uint);

#undef CALL_BP_TEMPLATE_void_T

		library->add_template("Get Static Sheet", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(SheetPtr*)outputs[0].data = Sheet::get(*(uint*)inputs[0].data);
			}
		);

#define GET_SHT_TEMPLATE(TYPE, DV) \
		library->add_template("Get SHT " #TYPE, "", \
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {\
				auto sheet = *(SheetPtr*)inputs[0].data;\
				auto name = *(uint*)inputs[1].data;\
				if (sheet)\
				{\
					auto column_idx = sheet->find_column(name);\
					if (column_idx != -1)\
					{\
						if (sheet->columns[column_idx].type == TypeInfo::get<TYPE>())\
						{\
							auto row_idx = *(uint*)inputs[2].data;\
							if (row_idx < sheet->rows.size())\
								*(TYPE*)outputs[0].data = *(TYPE*)sheet->rows[row_idx].datas[column_idx];\
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

		library->add_template("Delta Time", "",
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(float*)outputs[0].data = delta_time;
			}
		);
	}
}

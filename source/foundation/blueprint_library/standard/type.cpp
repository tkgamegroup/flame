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
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
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
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
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
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
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
			},\
			nullptr,\
			nullptr,\
			nullptr,\
			nullptr\
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
			},\
			nullptr,\
			nullptr,\
			nullptr,\
			nullptr\
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
	}
}

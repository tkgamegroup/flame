#include "string.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_string_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("To String", "",
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>(),
										TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
										}
				}
			},
			{
				{
					.name = "String",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto in_ti = (TypeInfo_Data*)inputs[0].type;
				*(std::string*)outputs[0].data = in_ti->serialize(inputs[0].data);
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("To WString", "",
			{
				{
					.name = "In",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(std::wstring*)outputs[0].data = s2w(*(std::string*)inputs[0].data);
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Print", "",
			{
				{
					.name = "String",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& string = *(std::string*)inputs[0].data;
				if (!string.empty())
					printf("%s\n", string.c_str());
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
	}
}

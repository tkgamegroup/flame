#include "string.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_string_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("String Length", "",
			{
				{
					.name = "String",
					.allowed_types = { TypeInfo::get<std::string>(), 
						TypeInfo::get<std::wstring>(), 
						TypeInfo::get<std::filesystem::path>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				if (inputs[0].type == TypeInfo::get<std::string>())
					*(uint*)outputs[0].data = (*(std::string*)inputs[0].data).size();
				else if (inputs[0].type == TypeInfo::get<std::wstring>())
					*(uint*)outputs[0].data = (*(std::wstring*)inputs[0].data).size();
				else if (inputs[0].type == TypeInfo::get<std::filesystem::path>())
					*(uint*)outputs[0].data = (*(std::filesystem::path*)inputs[0].data).native().size();
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("String Empty", "",
			{
				{
					.name = "String",
					.allowed_types = { TypeInfo::get<std::string>(),
						TypeInfo::get<std::wstring>(),
						TypeInfo::get<std::filesystem::path>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				if (inputs[0].type == TypeInfo::get<std::string>())
					*(bool*)outputs[0].data = (*(std::string*)inputs[0].data).empty();
				else if (inputs[0].type == TypeInfo::get<std::wstring>())
					*(bool*)outputs[0].data = (*(std::wstring*)inputs[0].data).empty();
				else if (inputs[0].type == TypeInfo::get<std::filesystem::path>())
					*(bool*)outputs[0].data = (*(std::filesystem::path*)inputs[0].data).native().empty();
				else
					*(bool*)outputs[0].data = true;
			}
		);

		library->add_template("String Equal", "",
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = *(std::string*)inputs[0].data == *(std::string*)inputs[1].data;
			}
		);

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
			}
		);

		library->add_template("String To Int", "",
			{
				{
					.name = "String",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<int>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(int*)outputs[0].data = s2t<int>(*(std::string*)inputs[0].data);
			}
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
			}
		);

		library->add_template("To Path", "",
			{
				{
					.name = "In",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::filesystem::path>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(std::filesystem::path*)outputs[0].data = *(std::wstring*)inputs[0].data;
			}
		);

		library->add_template("String Concatenate", "",
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(std::string*)outputs[0].data = *(std::string*)inputs[0].data + *(std::string*)inputs[1].data;
			}
		);

		library->add_template("WString Concatenate", "",
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(std::wstring*)outputs[0].data = *(std::wstring*)inputs[0].data + *(std::wstring*)inputs[1].data;
			}
		);

		library->add_template("Format1", "",
			{
				{
					.name = "Fmt",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Arg1",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& fmt = *(std::string*)inputs[0].data;
				auto arg1 = inputs[1].data;
				char buf[256];
				sprintf_s(buf, fmt.c_str(), *(int*)arg1);
				*(std::string*)outputs[0].data = buf;
			}
		);

		library->add_template("Format2", "",
			{
				{
					.name = "Fmt",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Arg1",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Arg2",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& fmt = *(std::string*)inputs[0].data;
				auto arg1 = inputs[1].data;
				auto arg2 = inputs[2].data;
				char buf[256];
				sprintf_s(buf, fmt.c_str(), *(int*)arg1, *(int*)arg2);
				*(std::string*)outputs[0].data = buf;
			}
		);

		library->add_template("Format3", "",
			{
				{
					.name = "Fmt",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Arg1",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Arg2",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Arg3",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& fmt = *(std::string*)inputs[0].data;
				auto arg1 = inputs[1].data;
				auto arg2 = inputs[2].data;
				auto arg3 = inputs[3].data;
				char buf[256];
				sprintf_s(buf, fmt.c_str(), *(int*)arg1, *(int*)arg2, *(int*)arg3);
				*(std::string*)outputs[0].data = buf;
			}
		);

		library->add_template("Format4", "",
			{
				{
					.name = "Fmt",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Arg1",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Arg2",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Arg3",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Arg4",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& fmt = *(std::string*)inputs[0].data;
				auto arg1 = inputs[1].data;
				auto arg2 = inputs[2].data;
				auto arg3 = inputs[3].data;
				auto arg4 = inputs[4].data;
				char buf[256];
				sprintf_s(buf, fmt.c_str(), *(int*)arg1, *(int*)arg2, *(int*)arg3, *(int*)arg4);
				*(std::string*)outputs[0].data = buf;
			}
		);

		library->add_template("WFormat1", "",
			{
				{
					.name = "Fmt",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				},
				{
					.name = "Arg1",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& fmt = *(std::wstring*)inputs[0].data;
				auto arg1 = inputs[1].data;
				wchar_t buf[256];
				swprintf_s(buf, fmt.c_str(), *(int*)arg1);
				*(std::wstring*)outputs[0].data = buf;
			}
		);

		library->add_template("WFormat2", "",
			{
				{
					.name = "Fmt",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				},
				{
					.name = "Arg1",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Arg2",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& fmt = *(std::wstring*)inputs[0].data;
				auto arg1 = inputs[1].data;
				auto arg2 = inputs[2].data;
				wchar_t buf[256];
				swprintf_s(buf, fmt.c_str(), *(int*)arg1, *(int*)arg2);
				*(std::wstring*)outputs[0].data = buf;
			}
		);

		library->add_template("WFormat3", "",
			{
				{
					.name = "Fmt",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				},
				{
					.name = "Arg1",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Arg2",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Arg3",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& fmt = *(std::wstring*)inputs[0].data;
				auto arg1 = inputs[1].data;
				auto arg2 = inputs[2].data;
				auto arg3 = inputs[3].data;
				wchar_t buf[256];
				swprintf_s(buf, fmt.c_str(), *(int*)arg1, *(int*)arg2, *(int*)arg3);
				*(std::wstring*)outputs[0].data = buf;
			}
		);

		library->add_template("WFormat4", "",
			{
				{
					.name = "Fmt",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				},
				{
					.name = "Arg1",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Arg2",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Arg3",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Arg4",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& fmt = *(std::wstring*)inputs[0].data;
				auto arg1 = inputs[1].data;
				auto arg2 = inputs[2].data;
				auto arg3 = inputs[3].data;
				auto arg4 = inputs[4].data;
				wchar_t buf[256];
				swprintf_s(buf, fmt.c_str(), *(int*)arg1, *(int*)arg2, *(int*)arg3, *(int*)arg4);
				*(std::wstring*)outputs[0].data = buf;
			}
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
			}
		);
	}
}

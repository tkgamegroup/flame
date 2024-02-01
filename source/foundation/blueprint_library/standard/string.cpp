#include "string.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

#include <format>

template <>
struct std::formatter<flame::BlueprintAttribute, char>
{
	constexpr auto parse(std::format_parse_context& ctx) const
	{
		return ctx.begin();
	}

	auto format(flame::BlueprintAttribute arg, std::format_context& ctx) const
	{
		auto str = arg.type->serialize(arg.data);
		return std::copy(str.begin(), str.end(), ctx.out());
	}
};

template <>
struct std::formatter<flame::BlueprintAttribute, wchar_t>
{
	constexpr auto parse(std::wformat_parse_context& ctx) const
	{
		return ctx.begin();
	}

	auto format(flame::BlueprintAttribute arg, std::wformat_context& ctx) const
	{
		auto str = flame::s2w(arg.type->serialize(arg.data));
		return std::copy(str.begin(), str.end(), ctx.out());
	}
};

namespace flame
{
	void add_string_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("String Length", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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

		library->add_template("String Empty", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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

		library->add_template("String Equal", "", BlueprintNodeFlagNone,
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = *(std::string*)inputs[0].data == *(std::string*)inputs[1].data;
			}
		);

		library->add_template("Str", "", BlueprintNodeFlagNone,
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
					.name = "V",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto in_ti = (TypeInfo_Data*)inputs[0].type;
				*(std::string*)outputs[0].data = in_ti->serialize(inputs[0].data);
			}
		);

		library->add_template("Stoi", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<int>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(int*)outputs[0].data = s2t<int>(*(std::string*)inputs[0].data);
			}
		);

		library->add_template("String Hash", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(uint*)outputs[0].data = sh((*(std::string*)inputs[0].data).c_str());
			}
		);

		library->add_template("To WString", "", BlueprintNodeFlagNone,
			{
				{
					.name = "In",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(std::wstring*)outputs[0].data = s2w(*(std::string*)inputs[0].data);
			}
		);

		library->add_template("To Path", "", BlueprintNodeFlagNone,
			{
				{
					.name = "In",
					.allowed_types = { TypeInfo::get<std::string>(), TypeInfo::get<std::wstring>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<std::filesystem::path>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (inputs[0].type == TypeInfo::get<std::string>())
					*(std::filesystem::path*)outputs[0].data = *(std::string*)inputs[0].data;
				else
					*(std::filesystem::path*)outputs[0].data = *(std::wstring*)inputs[0].data;
			}
		);

		library->add_template("String Concatenate", "", BlueprintNodeFlagNone,
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
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(std::string*)outputs[0].data = *(std::string*)inputs[0].data + *(std::string*)inputs[1].data;
			}
		);

		library->add_template("WString Concatenate", "", BlueprintNodeFlagNone,
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
					.name = "V",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(std::wstring*)outputs[0].data = *(std::wstring*)inputs[0].data + *(std::wstring*)inputs[1].data;
			}
		);

		library->add_template("Format", "", BlueprintNodeFlagEnableTemplate,
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
					.name = "V",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& fmt = *(std::string*)inputs[0].data;
				switch (inputs_count)
				{
				case 2: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1])); break;
				case 3: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2])); break;
				case 4: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3])); break;
				case 5: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4])); break;
				case 6: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5])); break;
				case 7: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6])); break;
				case 8: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7])); break;
				case 9: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8])); break;
				case 10: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9])); break;
				case 11: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10])); break;
				case 12: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10], inputs[11])); break;
				case 13: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10], inputs[11], inputs[12])); break;
				case 14: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10], inputs[11], inputs[12], inputs[13])); break;
				case 15: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10], inputs[11], inputs[12], inputs[13], inputs[14])); break;
				case 16: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10], inputs[11], inputs[12], inputs[13], inputs[14], inputs[15])); break;
				case 17: *(std::string*)outputs[0].data = std::vformat(fmt, std::make_format_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10], inputs[11], inputs[12], inputs[13], inputs[14], inputs[15], inputs[16])); break;
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					auto n = s2t<uint>(info.template_string);
					if (n == 0)
						return false;

					n = clamp(n, 1U, 16U);
					info.new_inputs.resize(n + 1);
					info.new_inputs[0] = {
						.name = "Fmt",
						.allowed_types = { TypeInfo::get<std::string>() }
					};
					for (uint i = 1; i <= n; ++i)
					{
						info.new_inputs[i] = {
							.name = "Arg" + str(i),
							.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
						};
					}
					info.new_outputs.resize(1);
					info.new_outputs[0] = {
						.name = "V",
						.allowed_types = { TypeInfo::get<std::string>() }
					};

					return true;
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return false;
			}
		);

		library->add_template("WFormat", "", BlueprintNodeFlagEnableTemplate,
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
					.name = "V",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& fmt = *(std::wstring*)inputs[0].data;
				switch (inputs_count)
				{
				case 2: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1])); break;
				case 3: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2])); break;
				case 4: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3])); break;
				case 5: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4])); break;
				case 6: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5])); break;
				case 7: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6])); break;
				case 8: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7])); break;
				case 9: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8])); break;
				case 10: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9])); break;
				case 11: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10])); break;
				case 12: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10], inputs[11])); break;
				case 13: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10], inputs[11], inputs[12])); break;
				case 14: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10], inputs[11], inputs[12], inputs[13])); break;
				case 15: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10], inputs[11], inputs[12], inputs[13], inputs[14])); break;
				case 16: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10], inputs[11], inputs[12], inputs[13], inputs[14], inputs[15])); break;
				case 17: *(std::wstring*)outputs[0].data = std::vformat(fmt, std::make_wformat_args(inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], inputs[6], inputs[7], inputs[8], inputs[9], inputs[10], inputs[11], inputs[12], inputs[13], inputs[14], inputs[15], inputs[16])); break;
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					auto n = s2t<uint>(info.template_string);
					if (n == 0)
						return false;

					n = clamp(n, 1U, 16U);
					info.new_inputs.resize(n + 1);
					info.new_inputs[0] = {
						.name = "Fmt",
						.allowed_types = { TypeInfo::get<std::wstring>() }
					};
					for (uint i = 1; i <= n; i++)
					{
						info.new_inputs[i] = {
							.name = "Arg" + str(i),
							.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
						};
					}
					info.new_outputs.resize(1);
					info.new_outputs[0] = {
						.name = "V",
						.allowed_types = { TypeInfo::get<std::wstring>() }
					};
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return true;
			}
		);

		library->add_template("Foreach Line", "", BlueprintNodeFlagBreakTarget,
			{
				{
					.name = "Text",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "temp_array",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<std::vector<std::string>>() }
				}
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto& text = *(std::string*)inputs[0].data;
				auto& temp_array = *(std::vector<std::string>*)outputs[0].data;
				temp_array = SUS::to_string_vector(SUS::split(text, '\n'));

				execution.block->max_execute_times = temp_array.size();
				execution.block->loop_vector_index = 1;
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& temp_array = *(std::vector<uint>*)outputs[0].data;
				temp_array.clear();
			}
		);

		library->add_template("Regex Search", "", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Text",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Regex",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "Matched",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& text = *(std::string*)inputs[0].data;
				auto& regex = *(std::string*)inputs[1].data;
				std::smatch res;
				auto matched = std::regex_search(text, res, std::regex(regex));
				*(bool*)outputs[0].data = matched;
				if (matched)
				{
					auto n = min((int)res.size() - 1, (int)outputs_count - 1);
					for (auto i = 0; i < n; i++)
						*(std::string*)outputs[i + 1].data = res[i + 1].str();
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeTemplateChanged)
				{
					auto n = s2t<uint>(info.template_string);

					n = min(n, 16U);
					info.new_inputs.resize(2);
					info.new_inputs[0] = {
						.name = "Text",
						.allowed_types = { TypeInfo::get<std::string>() }
					};
					info.new_inputs[1] = {
						.name = "Regex",
						.allowed_types = { TypeInfo::get<std::string>() }
					};
					info.new_outputs.resize(n + 1);
					info.new_outputs[0] = {
						.name = "Matched",
						.allowed_types = { TypeInfo::get<bool>() }
					};
					for (uint i = 0; i < n; i++)
					{
						info.new_outputs[i + 1] = {
							.name = "Capture " + str(i),
							.allowed_types = { TypeInfo::get<std::string>() }
						};
					}
				}
				else if (info.reason == BlueprintNodeInputTypesChanged)
					return true;
				return true;
			}
		);

		library->add_template("Get File Name", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<std::filesystem::path>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(std::string*)outputs[0].data = (*(std::filesystem::path*)inputs[0].data).filename().stem().string();
			}
		);

		library->add_template("Print", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& string = *(std::string*)inputs[0].data;
				if (!string.empty())
					printf("%s\n", string.c_str());
			}
		);
	}
}

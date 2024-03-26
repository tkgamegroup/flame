#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_logical_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Less", "<", BlueprintNodeFlagNone,
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
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) < in1_ti->as_float(in1_p);
			}
		);

		library->add_template("Greater", ">", BlueprintNodeFlagNone,
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
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) > in1_ti->as_float(in1_p);
			}
		);

		library->add_template("Equal", "==", BlueprintNodeFlagNone,
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
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) == in1_ti->as_float(in1_p);
			}
		);

		library->add_template("Not Equal", "!=", BlueprintNodeFlagNone,
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
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) != in1_ti->as_float(in1_p);
			}
		);

		library->add_template("Less Or Equal", "<=", BlueprintNodeFlagNone,
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
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) <= in1_ti->as_float(in1_p);
			}
		);

		library->add_template("Greater Or Equal", ">=", BlueprintNodeFlagNone,
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
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) >= in1_ti->as_float(in1_p);
			}
		);

		library->add_template("In Range", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>(),
										TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>() }
				},
				{
					.name = "Range",
					.allowed_types = { TypeInfo::get<vec2>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto range = *(vec2*)inputs[1].data;
				auto ok = true;
				vec4 v;
				in0_ti->as_floats(inputs[0].data, in0_ti->vec_size, &v[0]);
				for (auto i = 0; i < in0_ti->vec_size; i++)
					ok = ok && (v[i] >= range.x && v[i] <= range.y);
				*(bool*)outputs[0].data = ok;
			}
		);

		library->add_template("Not", "!", BlueprintNodeFlagNone,
			{
				{
					.name = "In",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = !(*(bool*)inputs[0].data);
			}
		);

		library->add_template("And", "&&", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				bool b1;
				if (inputs[0].type == TypeInfo::get<bool>())
					b1 = *(bool*)inputs[0].data;
				else
					b1 = (*(voidptr*)inputs[0].data) != nullptr;
				bool b2;
				if (inputs[1].type == TypeInfo::get<bool>())
					b2 = *(bool*)inputs[1].data;
				else
					b2 = (*(voidptr*)inputs[1].data) != nullptr;

				*(bool*)outputs[0].data = b1 && b2;
			}
		);

		library->add_template("Or", "||", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				bool b1;
				if (inputs[0].type == TypeInfo::get<bool>())
					b1 = *(bool*)inputs[0].data;
				else
					b1 = (*(voidptr*)inputs[0].data) != nullptr;
				bool b2;
				if (inputs[1].type == TypeInfo::get<bool>())
					b2 = *(bool*)inputs[1].data;
				else
					b2 = (*(voidptr*)inputs[1].data) != nullptr;

				*(bool*)outputs[0].data = b1 || b2;
			}
		);

		library->add_template("Conditional Operator", "?:", BlueprintNodeFlagEnableTemplate,
			{
				{
					.name = "Cond",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				},
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				bool b;
				if (inputs[0].type == TypeInfo::get<bool>())
					b = *(bool*)inputs[0].data;
				else
					b = (*(voidptr*)inputs[0].data) != nullptr;

				auto in_p = b ? inputs[1].data : inputs[2].data;
				outputs[0].type->copy(outputs[0].data, in_p);
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
						.name = "Cond",
						.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
					};
					info.new_inputs[1] = {
						.name = "A",
						.allowed_types = { type }
					};
					info.new_inputs[2] = {
						.name = "B",
						.allowed_types = { type }
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

		library->add_template("Bit And", "&", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "B",
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
				uint b1 = *(uint*)inputs[0].data;
				uint b2 = *(uint*)inputs[1].data;
				*(uint*)outputs[0].data = b1 & b2;
			}
		);

		library->add_template("Bit Or", "|", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "B",
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
				uint b1 = *(uint*)inputs[0].data;
				uint b2 = *(uint*)inputs[1].data;
				*(uint*)outputs[0].data = b1 | b2;
			}
		);

		library->add_template("Bit Xor", "^", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "B",
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
				uint b1 = *(uint*)inputs[0].data;
				uint b2 = *(uint*)inputs[1].data;
				*(uint*)outputs[0].data = b1 ^ b2;
			}
		);

		library->add_template("Bit Not", "~", BlueprintNodeFlagNone,
			{
				{
					.name = "In",
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
				uint b1 = *(uint*)inputs[0].data;
				*(uint*)outputs[0].data = ~b1;
			}
		);

		library->add_template("Check Bits", "", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = (*(uint*)inputs[0].data) & (*(uint*)inputs[1].data);
			}
		);

		library->add_template("Divisible Evenly", "a%b==0", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = *(uint*)inputs[0].data % *(uint*)inputs[1].data == 0;
			}
		);
	}
}

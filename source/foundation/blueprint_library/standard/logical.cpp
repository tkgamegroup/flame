#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_logical_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Less", "<",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) < in1_ti->as_float(in1_p);
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Greater", ">",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) > in1_ti->as_float(in1_p);
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Equal", "==",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) == in1_ti->as_float(in1_p);
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Not Equal", "!=",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) != in1_ti->as_float(in1_p);
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Less Or Equal", "<=",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) <= in1_ti->as_float(in1_p);
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Greater Or Equal", "<=",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) >= in1_ti->as_float(in1_p);
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Not", "!",
			{
				{
					.name = "In",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = !(*(bool*)inputs[0].data);
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("And", "&&",
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
					.name = "Out",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
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
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Or", "||",
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
					.name = "Out",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
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
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Conditional Operator", "?:",
			{
				{
					.name = "Condition",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				},
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
					.name = "Out",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				bool b;
				if (inputs[0].type == TypeInfo::get<bool>())
					b = *(bool*)inputs[0].data;
				else
					b = (*(voidptr*)inputs[0].data) != nullptr;

				auto in_ti = b ? inputs[1].type : inputs[2].type;
				auto in_p = b ? inputs[1].data : inputs[2].data;
				if (outputs[0].type == TypeInfo::get<float>())
					*(float*)outputs[0].data = in_ti->as_float(in_p);
				else if (outputs[0].type == TypeInfo::get<int>())
					*(int*)outputs[0].data = in_ti->as_int(in_p);
				else if (outputs[0].type == TypeInfo::get<uint>())
					*(uint*)outputs[0].data = in_ti->as_uint(in_p);
			},
			nullptr,
			nullptr,
			[](TypeInfo** input_types, TypeInfo** output_types) {
				*output_types = input_types[1];
			}
		);

		library->add_template("Bit And", "&",
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
					.name = "Out",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				uint b1 = *(uint*)inputs[0].data;
				uint b2 = *(uint*)inputs[1].data;
				*(uint*)outputs[0].data = b1 & b2;
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Bit Or", "|",
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
					.name = "Out",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				uint b1 = *(uint*)inputs[0].data;
				uint b2 = *(uint*)inputs[1].data;
				*(uint*)outputs[0].data = b1 | b2;
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Bit Xor", "^",
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
					.name = "Out",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				uint b1 = *(uint*)inputs[0].data;
				uint b2 = *(uint*)inputs[1].data;
				*(uint*)outputs[0].data = b1 ^ b2;
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Bit Not", "~",
			{
				{
					.name = "In",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				uint b1 = *(uint*)inputs[0].data;
				*(uint*)outputs[0].data = ~b1;
			},
			nullptr,
			nullptr,
			nullptr
		);
	}
}

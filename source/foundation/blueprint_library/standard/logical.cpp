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
	}
}

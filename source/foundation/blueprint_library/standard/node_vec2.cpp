#include "node_vec2.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_template_vec2(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Vec2", "",
			{
				{
					.name = "X",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Y",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<vec2>(), TypeInfo::get<ivec2>(), TypeInfo::get<uvec2>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;

				switch (out_ti->data_type)
				{
				case DataFloat:
				{
					auto& v = *(vec2*)outputs[0].data;
					v.x = in0_ti->as_float(inputs[0].data);
					v.y = in1_ti->as_float(inputs[1].data);
				}
					break;
				case DataInt:
				{
					if (out_ti->is_signed)
					{
						auto& v = *(ivec2*)outputs[0].data;
						v.x = in0_ti->as_int(inputs[0].data);
						v.y = in1_ti->as_int(inputs[1].data);
					}
					else
					{
						auto& v = *(uvec2*)outputs[0].data;
						v.x = in0_ti->as_uint(inputs[0].data);
						v.y = in1_ti->as_uint(inputs[1].data);
					}
				}
					break;
				}
			},
			nullptr,
			nullptr,
			[](TypeInfo** input_types, TypeInfo** output_types) {
				auto ti0 = (TypeInfo_Data*)input_types[0];
				auto ti1 = (TypeInfo_Data*)input_types[1];
				auto data_type = DataFloat;
				auto is_signed = ti0->is_signed || ti1->is_signed;
				if (ti0->data_type == DataInt && ti1->data_type == DataInt)
					data_type = DataInt;
				switch (data_type)
				{
				case DataFloat:
					*output_types = TypeInfo::get<vec2>();
					break;
				case DataInt:
					*output_types = is_signed ? TypeInfo::get<ivec2>() : TypeInfo::get<uvec2>();
					break;
				}
			}
		);
	}
}

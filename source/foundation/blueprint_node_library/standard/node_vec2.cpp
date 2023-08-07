#include "node_vec2.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_template_vec2(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Vec2",
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
				switch (inputs[0].type_idx + (inputs[1].type_idx << 4))
				{
				case  0x00: *(vec2*)outputs[0].data		=	vec2(*(float*)	inputs[0].data,	*(float*)inputs[1].data);	break;
				case  0x01: *(vec2*)outputs[0].data		=	vec2(*(int*)	inputs[0].data,	*(float*)inputs[1].data);	break;
				case  0x02: *(vec2*)outputs[0].data		=	vec2(*(uint*)	inputs[0].data,	*(float*)inputs[1].data);	break;
				case  0x10: *(vec2*)outputs[0].data		=	vec2(*(float*)	inputs[0].data,	*(int*)inputs[1].data);		break;
				case  0x11: *(ivec2*)outputs[0].data	=	ivec2(*(int*)	inputs[0].data,	*(int*)inputs[1].data);		break;
				case  0x12: *(vec2*)outputs[0].data		=	vec2(*(uint*)	inputs[0].data,	*(int*)inputs[1].data);		break;
				case  0x20: *(vec2*)outputs[0].data		=	vec2(*(float*)	inputs[0].data,	*(uint*)inputs[1].data);	break;
				case  0x21: *(ivec2*)outputs[0].data	=	ivec2(*(int*)	inputs[0].data,	*(uint*)inputs[1].data);	break;
				case  0x22: *(uvec2*)outputs[0].data	=	uvec2(*(uint*)	inputs[0].data,	*(uint*)inputs[1].data);	break;
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

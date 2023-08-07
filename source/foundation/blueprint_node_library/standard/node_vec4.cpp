#include "node_vec4.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_template_vec4(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Vec4",
			{
				{
					.name = "X",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Y",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "Z",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "W",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<vec4>(), TypeInfo::get<ivec4>(), TypeInfo::get<uvec4>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				switch (inputs[0].type_idx + (inputs[1].type_idx << 4) + (inputs[1].type_idx << 8) + (inputs[1].type_idx << 12))
				{
				case  0x0000: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(float*)inputs[1].data,	*(float*)inputs[2].data,	*(float*)inputs[3].data);	break;
				case  0x0001: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(float*)inputs[1].data,	*(float*)inputs[2].data,	*(float*)inputs[3].data);	break;
				case  0x0002: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(float*)inputs[1].data,	*(float*)inputs[2].data,	*(float*)inputs[3].data);	break;
				case  0x0010: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(int*)inputs[1].data,		*(float*)inputs[2].data,	*(float*)inputs[3].data);	break;
				case  0x0011: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(int*)inputs[1].data,		*(float*)inputs[2].data,	*(float*)inputs[3].data);	break;
				case  0x0012: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(int*)inputs[1].data,		*(float*)inputs[2].data,	*(float*)inputs[3].data);	break;
				case  0x0020: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(uint*)inputs[1].data,		*(float*)inputs[2].data,	*(float*)inputs[3].data);	break;
				case  0x0021: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(uint*)inputs[1].data,		*(float*)inputs[2].data,	*(float*)inputs[3].data);	break;
				case  0x0022: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(uint*)inputs[1].data,		*(float*)inputs[2].data,	*(float*)inputs[3].data);	break;
				case  0x0100: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(float*)inputs[1].data,	*(int*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0101: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(float*)inputs[1].data,	*(int*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0102: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(float*)inputs[1].data,	*(int*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0110: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(int*)inputs[1].data,		*(int*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0111: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(int*)inputs[1].data,		*(int*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0112: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(int*)inputs[1].data,		*(int*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0120: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(uint*)inputs[1].data,		*(int*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0121: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(uint*)inputs[1].data,		*(int*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0122: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(uint*)inputs[1].data,		*(int*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0200: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(float*)inputs[1].data,	*(uint*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0201: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(float*)inputs[1].data,	*(uint*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0202: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(float*)inputs[1].data,	*(uint*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0210: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(int*)inputs[1].data,		*(uint*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0211: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(int*)inputs[1].data,		*(uint*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0212: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(int*)inputs[1].data,		*(uint*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0220: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(uint*)inputs[1].data,		*(uint*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0221: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(uint*)inputs[1].data,		*(uint*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x0222: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(uint*)inputs[1].data,		*(uint*)inputs[2].data,		*(float*)inputs[3].data);	break;
				case  0x1000: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(float*)inputs[1].data,	*(float*)inputs[2].data,	*(int*)inputs[3].data);		break;
				case  0x1001: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(float*)inputs[1].data,	*(float*)inputs[2].data,	*(int*)inputs[3].data);		break;
				case  0x1002: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(float*)inputs[1].data,	*(float*)inputs[2].data,	*(int*)inputs[3].data);		break;
				case  0x1010: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(int*)inputs[1].data,		*(float*)inputs[2].data,	*(int*)inputs[3].data);		break;
				case  0x1011: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(int*)inputs[1].data,		*(float*)inputs[2].data,	*(int*)inputs[3].data);		break;
				case  0x1012: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(int*)inputs[1].data,		*(float*)inputs[2].data,	*(int*)inputs[3].data);		break;
				case  0x1020: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(uint*)inputs[1].data,		*(float*)inputs[2].data,	*(int*)inputs[3].data);		break;
				case  0x1021: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(uint*)inputs[1].data,		*(float*)inputs[2].data,	*(int*)inputs[3].data);		break;
				case  0x1022: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(uint*)inputs[1].data,		*(float*)inputs[2].data,	*(int*)inputs[3].data);		break;
				case  0x1100: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(float*)inputs[1].data,	*(int*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1101: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(float*)inputs[1].data,	*(int*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1102: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(float*)inputs[1].data,	*(int*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1110: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(int*)inputs[1].data,		*(int*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1111: *(ivec4*)outputs[0].data	= ivec4(*(int*)inputs[0].data,	*(int*)inputs[1].data,		*(int*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1112: *(ivec4*)outputs[0].data	= ivec4(*(uint*)inputs[0].data,	*(int*)inputs[1].data,		*(int*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1120: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(uint*)inputs[1].data,		*(int*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1121: *(ivec4*)outputs[0].data	= ivec4(*(int*)inputs[0].data,	*(uint*)inputs[1].data,		*(int*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1122: *(ivec4*)outputs[0].data	= ivec4(*(uint*)inputs[0].data,	*(uint*)inputs[1].data,		*(int*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1200: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(float*)inputs[1].data,	*(uint*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1201: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(float*)inputs[1].data,	*(uint*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1202: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(float*)inputs[1].data,	*(uint*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1210: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(int*)inputs[1].data,		*(uint*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1211: *(ivec4*)outputs[0].data	= ivec4(*(int*)inputs[0].data,	*(int*)inputs[1].data,		*(uint*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1212: *(ivec4*)outputs[0].data	= ivec4(*(uint*)inputs[0].data,	*(int*)inputs[1].data,		*(uint*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1220: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(uint*)inputs[1].data,		*(uint*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1221: *(ivec4*)outputs[0].data	= ivec4(*(int*)inputs[0].data,	*(uint*)inputs[1].data,		*(uint*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x1222: *(ivec4*)outputs[0].data	= ivec4(*(uint*)inputs[0].data,	*(uint*)inputs[1].data,		*(uint*)inputs[2].data,		*(int*)inputs[3].data);		break;
				case  0x2000: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(float*)inputs[1].data,	*(float*)inputs[2].data,	*(uint*)inputs[3].data);	break;
				case  0x2001: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(float*)inputs[1].data,	*(float*)inputs[2].data,	*(uint*)inputs[3].data);	break;
				case  0x2002: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(float*)inputs[1].data,	*(float*)inputs[2].data,	*(uint*)inputs[3].data);	break;
				case  0x2010: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(int*)inputs[1].data,		*(float*)inputs[2].data,	*(uint*)inputs[3].data);	break;
				case  0x2011: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(int*)inputs[1].data,		*(float*)inputs[2].data,	*(uint*)inputs[3].data);	break;
				case  0x2012: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(int*)inputs[1].data,		*(float*)inputs[2].data,	*(uint*)inputs[3].data);	break;
				case  0x2020: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(uint*)inputs[1].data,		*(float*)inputs[2].data,	*(uint*)inputs[3].data);	break;
				case  0x2021: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(uint*)inputs[1].data,		*(float*)inputs[2].data,	*(uint*)inputs[3].data);	break;
				case  0x2022: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(uint*)inputs[1].data,		*(float*)inputs[2].data,	*(uint*)inputs[3].data);	break;
				case  0x2100: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(float*)inputs[1].data,	*(int*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2101: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(float*)inputs[1].data,	*(int*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2102: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(float*)inputs[1].data,	*(int*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2110: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(int*)inputs[1].data,		*(int*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2111: *(ivec4*)outputs[0].data	= ivec4(*(int*)inputs[0].data,	*(int*)inputs[1].data,		*(int*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2112: *(ivec4*)outputs[0].data	= ivec4(*(uint*)inputs[0].data,	*(int*)inputs[1].data,		*(int*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2120: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(uint*)inputs[1].data,		*(int*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2121: *(ivec4*)outputs[0].data	= ivec4(*(int*)inputs[0].data,	*(uint*)inputs[1].data,		*(int*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2122: *(ivec4*)outputs[0].data	= ivec4(*(uint*)inputs[0].data,	*(uint*)inputs[1].data,		*(int*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2200: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(float*)inputs[1].data,	*(uint*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2201: *(vec4*)outputs[0].data	= vec4(*(int*)inputs[0].data,	*(float*)inputs[1].data,	*(uint*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2202: *(vec4*)outputs[0].data	= vec4(*(uint*)inputs[0].data,	*(float*)inputs[1].data,	*(uint*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2210: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(int*)inputs[1].data,		*(uint*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2211: *(ivec4*)outputs[0].data	= ivec4(*(int*)inputs[0].data,	*(int*)inputs[1].data,		*(uint*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2212: *(ivec4*)outputs[0].data	= ivec4(*(uint*)inputs[0].data,	*(int*)inputs[1].data,		*(uint*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2220: *(vec4*)outputs[0].data	= vec4(*(float*)inputs[0].data,	*(uint*)inputs[1].data,		*(uint*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2221: *(ivec4*)outputs[0].data	= ivec4(*(int*)inputs[0].data,	*(uint*)inputs[1].data,		*(uint*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				case  0x2222: *(uvec4*)outputs[0].data	= uvec4(*(uint*)inputs[0].data,	*(uint*)inputs[1].data,		*(uint*)inputs[2].data,		*(uint*)inputs[3].data);	break;
				}
			},
			nullptr,
			nullptr,
			[](TypeInfo** input_types, TypeInfo** output_types) {
				auto ti0 = (TypeInfo_Data*)input_types[0];
				auto ti1 = (TypeInfo_Data*)input_types[1];
				auto ti2 = (TypeInfo_Data*)input_types[2];
				auto ti3 = (TypeInfo_Data*)input_types[3];
				auto data_type = DataFloat;
				auto is_signed = ti0->is_signed || ti1->is_signed || ti2->is_signed || ti3->is_signed;
				if (ti0->data_type == DataInt && ti1->data_type == DataInt && ti2->data_type == DataInt && ti3->data_type == DataInt)
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

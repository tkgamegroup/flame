#include "node_vec3.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_template_vec3(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Vec3",
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
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<vec3>(), TypeInfo::get<ivec3>(), TypeInfo::get<uvec3>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				switch (inputs[0].type_idx + (inputs[1].type_idx << 4) + (inputs[1].type_idx << 8))
				{
				case  0x000: *(vec3*)outputs[0].data	= vec3(*(float*)inputs[0].data,		*(float*)inputs[1].data,	*(float*)inputs[2].data);	break;
				case  0x001: *(vec3*)outputs[0].data	= vec3(*(int*)inputs[0].data,		*(float*)inputs[1].data,	*(float*)inputs[2].data);	break;
				case  0x002: *(vec3*)outputs[0].data	= vec3(*(uint*)inputs[0].data,		*(float*)inputs[1].data,	*(float*)inputs[2].data);	break;
				case  0x010: *(vec3*)outputs[0].data	= vec3(*(float*)inputs[0].data,		*(int*)inputs[1].data,		*(float*)inputs[2].data);	break;
				case  0x011: *(vec3*)outputs[0].data	= vec3(*(int*)inputs[0].data,		*(int*)inputs[1].data,		*(float*)inputs[2].data);	break;
				case  0x012: *(vec3*)outputs[0].data	= vec3(*(uint*)inputs[0].data,		*(int*)inputs[1].data,		*(float*)inputs[2].data);	break;
				case  0x020: *(vec3*)outputs[0].data	= vec3(*(float*)inputs[0].data,		*(uint*)inputs[1].data,		*(float*)inputs[2].data);	break;
				case  0x021: *(vec3*)outputs[0].data	= vec3(*(int*)inputs[0].data,		*(uint*)inputs[1].data,		*(float*)inputs[2].data);	break;
				case  0x022: *(vec3*)outputs[0].data	= vec3(*(uint*)inputs[0].data,		*(uint*)inputs[1].data,		*(float*)inputs[2].data);	break;
				case  0x100: *(vec3*)outputs[0].data	= vec3(*(float*)inputs[0].data,		*(float*)inputs[1].data,	*(int*)inputs[2].data);		break;
				case  0x101: *(vec3*)outputs[0].data	= vec3(*(int*)inputs[0].data,		*(float*)inputs[1].data,	*(int*)inputs[2].data);		break;
				case  0x102: *(vec3*)outputs[0].data	= vec3(*(uint*)inputs[0].data,		*(float*)inputs[1].data,	*(int*)inputs[2].data);		break;
				case  0x110: *(vec3*)outputs[0].data	= vec3(*(float*)inputs[0].data,		*(int*)inputs[1].data,		*(int*)inputs[2].data);		break;
				case  0x111: *(ivec3*)outputs[0].data	= ivec3(*(int*)inputs[0].data,		*(int*)inputs[1].data,		*(int*)inputs[2].data);		break;
				case  0x112: *(ivec3*)outputs[0].data	= ivec3(*(uint*)inputs[0].data,		*(int*)inputs[1].data,		*(int*)inputs[2].data);		break;
				case  0x120: *(vec3*)outputs[0].data	= vec3(*(float*)inputs[0].data,		*(uint*)inputs[1].data,		*(int*)inputs[2].data);		break;
				case  0x121: *(ivec3*)outputs[0].data	= ivec3(*(int*)inputs[0].data,		*(uint*)inputs[1].data,		*(int*)inputs[2].data);		break;
				case  0x122: *(ivec3*)outputs[0].data	= ivec3(*(uint*)inputs[0].data,		*(uint*)inputs[1].data,		*(int*)inputs[2].data);		break;
				case  0x200: *(vec3*)outputs[0].data	= vec3(*(float*)inputs[0].data,		*(float*)inputs[1].data,	*(uint*)inputs[2].data);	break;
				case  0x201: *(vec3*)outputs[0].data	= vec3(*(int*)inputs[0].data,		*(float*)inputs[1].data,	*(uint*)inputs[2].data);	break;
				case  0x202: *(vec3*)outputs[0].data	= vec3(*(uint*)inputs[0].data,		*(float*)inputs[1].data,	*(uint*)inputs[2].data);	break;
				case  0x210: *(vec3*)outputs[0].data	= vec3(*(float*)inputs[0].data,		*(int*)inputs[1].data,		*(uint*)inputs[2].data);	break;
				case  0x211: *(ivec3*)outputs[0].data	= ivec3(*(int*)inputs[0].data,		*(int*)inputs[1].data,		*(uint*)inputs[2].data);	break;
				case  0x212: *(ivec3*)outputs[0].data	= ivec3(*(uint*)inputs[0].data,		*(int*)inputs[1].data,		*(uint*)inputs[2].data);	break;
				case  0x220: *(vec3*)outputs[0].data	= vec3(*(float*)inputs[0].data,		*(uint*)inputs[1].data,		*(uint*)inputs[2].data);	break;
				case  0x221: *(ivec3*)outputs[0].data	= ivec3(*(int*)inputs[0].data,		*(uint*)inputs[1].data,		*(uint*)inputs[2].data);	break;
				case  0x222: *(uvec3*)outputs[0].data	= uvec3(*(uint*)inputs[0].data,		*(uint*)inputs[1].data,		*(uint*)inputs[2].data);	break;
				}
			},
			nullptr,
			nullptr,
			[](TypeInfo** input_types, TypeInfo** output_types) {
				auto ti0 = (TypeInfo_Data*)input_types[0];
				auto ti1 = (TypeInfo_Data*)input_types[1];
				auto ti2 = (TypeInfo_Data*)input_types[2];
				auto data_type = DataFloat;
				auto is_signed = ti0->is_signed || ti1->is_signed || ti2->is_signed;
				if (ti0->data_type == DataInt && ti1->data_type == DataInt && ti2->data_type == DataInt)
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

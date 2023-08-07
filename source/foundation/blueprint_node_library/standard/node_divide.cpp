#include "node_divide.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_template_divide(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Divide",
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>(),
										TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
										}
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>(),
										TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
										}
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>(),
										TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
										}
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				switch (inputs[0].type_idx + (inputs[1].type_idx << 4))
				{
				case  0x00: *(float*)	outputs[0].data	=			*(float*)	inputs[0].data					/			*(float*)inputs[1].data;			break;
				case  0x01: *(vec2*)	outputs[0].data	=			*(vec2*)	inputs[0].data					/	vec2(	*(float*)inputs[1].data, 1);		break;
				case  0x02: *(vec3*)	outputs[0].data	=			*(vec3*)	inputs[0].data					/	vec3(	*(float*)inputs[1].data, 1, 1);		break;
				case  0x03: *(vec4*)	outputs[0].data	=			*(vec4*)	inputs[0].data					/	vec4(	*(float*)inputs[1].data, 1, 1, 1);	break;
				case  0x04: *(float*)	outputs[0].data	=			*(int*)		inputs[0].data					/			*(float*)inputs[1].data;			break;
				case  0x05: *(vec2*)	outputs[0].data	= vec2(		*(ivec2*)	inputs[0].data)					/	vec2(	*(float*)inputs[1].data, 1);		break;
				case  0x06: *(vec3*)	outputs[0].data	= vec3(		*(ivec3*)	inputs[0].data)					/	vec3(	*(float*)inputs[1].data, 1, 1);		break;
				case  0x07: *(vec4*)	outputs[0].data	= vec4(		*(ivec4*)	inputs[0].data)					/	vec4(	*(float*)inputs[1].data, 1, 1, 1);	break;
				case  0x08: *(float*)	outputs[0].data	=			*(uint*)	inputs[0].data					/			*(float*)inputs[1].data;			break;
				case  0x09: *(vec2*)	outputs[0].data	= vec2(		*(uvec2*)	inputs[0].data)					/	vec2(	*(float*)inputs[1].data, 1);		break;
				case  0x0a: *(vec3*)	outputs[0].data	= vec3(		*(uvec3*)	inputs[0].data)					/	vec3(	*(float*)inputs[1].data, 1, 1);		break;
				case  0x0b: *(vec4*)	outputs[0].data	= vec4(		*(uvec4*)	inputs[0].data)					/	vec4(	*(float*)inputs[1].data, 1, 1, 1);	break;
				case  0x10: *(vec2*)	outputs[0].data	= vec2(		*(float*)	inputs[0].data, 0)				/			*(vec2*)inputs[1].data;				break;
				case  0x11: *(vec2*)	outputs[0].data	=			*(vec2*)	inputs[0].data					/			*(vec2*)inputs[1].data;				break;
				case  0x12: *(vec3*)	outputs[0].data	=			*(vec3*)	inputs[0].data					/	vec3(	*(vec2*)inputs[1].data, 1);			break;
				case  0x13: *(vec4*)	outputs[0].data	=			*(vec4*)	inputs[0].data					/	vec4(	*(vec2*)inputs[1].data, 1, 1);		break;
				case  0x14: *(vec2*)	outputs[0].data	= vec2(		*(int*)		inputs[0].data, 0)				/			*(vec2*)inputs[1].data;				break;
				case  0x15: *(vec2*)	outputs[0].data	= vec2(		*(ivec2*)	inputs[0].data)					/			*(vec2*)inputs[1].data;				break;
				case  0x16: *(vec3*)	outputs[0].data	= vec3(		*(ivec3*)	inputs[0].data)					/	vec3(	*(vec2*)inputs[1].data, 1);			break;
				case  0x17: *(vec4*)	outputs[0].data	= vec4(		*(ivec4*)	inputs[0].data)					/	vec4(	*(vec2*)inputs[1].data, 1, 1);		break;
				case  0x18: *(vec2*)	outputs[0].data	= vec2(		*(uint*)	inputs[0].data, 0)				/			*(vec2*)inputs[1].data;				break;
				case  0x19: *(vec2*)	outputs[0].data	= vec2(		*(uvec2*)	inputs[0].data)					/			*(vec2*)inputs[1].data;				break;
				case  0x1a: *(vec3*)	outputs[0].data	= vec3(		*(uvec3*)	inputs[0].data)					/	vec3(	*(vec2*)inputs[1].data, 1);			break;
				case  0x1b: *(vec4*)	outputs[0].data	= vec4(		*(uvec4*)	inputs[0].data)					/	vec4(	*(vec2*)inputs[1].data, 1, 1);		break;
				case  0x20: *(vec3*)	outputs[0].data	= vec3(		*(float*)	inputs[0].data, 0, 0)			/			*(vec3*)inputs[1].data;				break;
				case  0x21: *(vec3*)	outputs[0].data	= vec3(		*(vec2*)	inputs[0].data, 0)				/			*(vec3*)inputs[1].data;				break;
				case  0x22: *(vec3*)	outputs[0].data	=			*(vec3*)	inputs[0].data					/			*(vec3*)inputs[1].data;				break;
				case  0x23: *(vec4*)	outputs[0].data	=			*(vec4*)	inputs[0].data					/	vec4(	*(vec3*)inputs[1].data, 1);			break;
				case  0x24: *(vec3*)	outputs[0].data	= vec3(		*(int*)		inputs[0].data, 0, 0)			/			*(vec3*)inputs[1].data;				break;
				case  0x25: *(vec3*)	outputs[0].data	= vec3(		*(ivec2*)	inputs[0].data, 0)				/			*(vec3*)inputs[1].data;				break;
				case  0x26: *(vec3*)	outputs[0].data	= vec3(		*(ivec3*)	inputs[0].data)					/			*(vec3*)inputs[1].data;				break;
				case  0x27: *(vec4*)	outputs[0].data	= vec4(		*(ivec4*)	inputs[0].data)					/	vec4(	*(vec3*)inputs[1].data, 1);			break;
				case  0x28: *(vec3*)	outputs[0].data	= vec3(		*(uint*)	inputs[0].data, 0, 0)			/			*(vec3*)inputs[1].data;				break;
				case  0x29: *(vec3*)	outputs[0].data	= vec3(		*(uvec2*)	inputs[0].data, 0)				/			*(vec3*)inputs[1].data;				break;
				case  0x2a: *(vec3*)	outputs[0].data	= vec3(		*(uvec3*)	inputs[0].data)					/			*(vec3*)inputs[1].data;				break;
				case  0x2b: *(vec4*)	outputs[0].data	= vec4(		*(uvec4*)	inputs[0].data)					/	vec4(	*(vec3*)inputs[1].data, 1);			break;
				case  0x30: *(vec4*)	outputs[0].data	= vec4(		*(float*)	inputs[0].data, 0, 0, 0)		/			*(vec4*)inputs[1].data;				break;
				case  0x31: *(vec4*)	outputs[0].data	= vec4(		*(vec2*)	inputs[0].data, 0, 0)			/			*(vec4*)inputs[1].data;				break;
				case  0x32: *(vec4*)	outputs[0].data	= vec4(		*(vec3*)	inputs[0].data, 0)				/			*(vec4*)inputs[1].data;				break;
				case  0x33: *(vec4*)	outputs[0].data	=			*(vec4*)	inputs[0].data					/			*(vec4*)inputs[1].data;				break;
				case  0x34: *(vec4*)	outputs[0].data	= vec4(		*(int*)		inputs[0].data, 0, 0, 0)		/			*(vec4*)inputs[1].data;				break;
				case  0x35: *(vec4*)	outputs[0].data	= vec4(		*(ivec2*)	inputs[0].data, 0, 0)			/			*(vec4*)inputs[1].data;				break;
				case  0x36: *(vec4*)	outputs[0].data	= vec4(		*(ivec3*)	inputs[0].data, 0)				/			*(vec4*)inputs[1].data;				break;
				case  0x37: *(vec4*)	outputs[0].data	= vec4(		*(ivec4*)	inputs[0].data)					/			*(vec4*)inputs[1].data;				break;
				case  0x38: *(vec4*)	outputs[0].data	= vec4(		*(uint*)	inputs[0].data, 0, 0, 0)		/			*(vec4*)inputs[1].data;				break;
				case  0x39: *(vec4*)	outputs[0].data	= vec4(		*(uvec2*)	inputs[0].data, 0, 0)			/			*(vec4*)inputs[1].data;				break;
				case  0x3a: *(vec4*)	outputs[0].data	= vec4(		*(uvec3*)	inputs[0].data, 0)				/			*(vec4*)inputs[1].data;				break;
				case  0x3b: *(vec4*)	outputs[0].data	= vec4(		*(uvec4*)	inputs[0].data)					/			*(vec4*)inputs[1].data;				break;
				case  0x40: *(float*)	outputs[0].data	=			*(float*)	inputs[0].data					/			*(int*)inputs[1].data;				break;
				case  0x41: *(vec2*)	outputs[0].data	=			*(vec2*)	inputs[0].data					/	vec2(	*(int*)inputs[1].data, 1);			break;
				case  0x42: *(vec3*)	outputs[0].data	=			*(vec3*)	inputs[0].data					/	vec3(	*(int*)inputs[1].data, 1, 1);		break;
				case  0x43: *(vec4*)	outputs[0].data	=			*(vec4*)	inputs[0].data					/	vec4(	*(int*)inputs[1].data, 1, 1, 1);	break;
				case  0x44: *(int*)		outputs[0].data	=			*(int*)		inputs[0].data					/			*(int*)inputs[1].data;				break;
				case  0x45: *(ivec2*)	outputs[0].data	=			*(ivec2*)	inputs[0].data					/	ivec2(	*(int*)inputs[1].data, 1);			break;
				case  0x46: *(ivec3*)	outputs[0].data	=			*(ivec3*)	inputs[0].data					/	ivec3(	*(int*)inputs[1].data, 1, 1);		break;
				case  0x47: *(ivec4*)	outputs[0].data	=			*(ivec4*)	inputs[0].data					/	ivec4(	*(int*)inputs[1].data, 1, 1, 1);	break;
				case  0x48: *(int*)		outputs[0].data	=			*(uint*)	inputs[0].data					/			*(int*)inputs[1].data;				break;
				case  0x49: *(ivec2*)	outputs[0].data	= ivec2(	*(uvec2*)	inputs[0].data)					/	ivec2(	*(int*)inputs[1].data, 1);			break;
				case  0x4a: *(ivec3*)	outputs[0].data	= ivec3(	*(uvec3*)	inputs[0].data)					/	ivec3(	*(int*)inputs[1].data, 1, 1);		break;
				case  0x4b: *(ivec4*)	outputs[0].data	= ivec4(	*(uvec4*)	inputs[0].data)					/	ivec4(	*(int*)inputs[1].data, 1, 1, 1);	break;
				case  0x50: *(vec2*)	outputs[0].data	= vec2(		*(float*)	inputs[0].data, 0)				/	vec2(	*(ivec2*)inputs[1].data);			break;
				case  0x51: *(vec2*)	outputs[0].data	=			*(vec2*)	inputs[0].data					/	vec2(	*(ivec2*)inputs[1].data);			break;
				case  0x52: *(vec3*)	outputs[0].data	=			*(vec3*)	inputs[0].data					/	vec3(	*(ivec2*)inputs[1].data, 1);		break;
				case  0x53: *(vec4*)	outputs[0].data	=			*(vec4*)	inputs[0].data					/	vec4(	*(ivec2*)inputs[1].data, 1, 1);		break;
				case  0x54: *(ivec2*)	outputs[0].data	= ivec2(	*(int*)		inputs[0].data, 0)				/			*(ivec2*)inputs[1].data;			break;
				case  0x55: *(ivec2*)	outputs[0].data	= ivec2(	*(ivec2*)	inputs[0].data)					/			*(ivec2*)inputs[1].data;			break;
				case  0x56: *(ivec3*)	outputs[0].data	= ivec3(	*(ivec3*)	inputs[0].data)					/	ivec3(	*(ivec2*)inputs[1].data, 1);		break;
				case  0x57: *(ivec4*)	outputs[0].data	= ivec4(	*(ivec4*)	inputs[0].data)					/	ivec4(	*(ivec2*)inputs[1].data, 1, 1);		break;
				case  0x58: *(ivec2*)	outputs[0].data	= ivec2(	*(uint*)	inputs[0].data, 0)				/			*(ivec2*)inputs[1].data;			break;
				case  0x59: *(ivec2*)	outputs[0].data	= ivec2(	*(uvec2*)	inputs[0].data)					/			*(ivec2*)inputs[1].data;			break;
				case  0x5a: *(ivec3*)	outputs[0].data	= ivec3(	*(uvec3*)	inputs[0].data)					/	ivec3(	*(ivec2*)inputs[1].data, 1);		break;
				case  0x5b: *(ivec4*)	outputs[0].data	= ivec4(	*(uvec4*)	inputs[0].data)					/	ivec4(	*(ivec2*)inputs[1].data, 1, 1);		break;
				case  0x60: *(vec3*)	outputs[0].data	= vec3(		*(float*)	inputs[0].data, 0, 0)			/	vec3(	*(ivec3*)inputs[1].data);			break;
				case  0x61: *(vec3*)	outputs[0].data	= vec3(		*(vec2*)	inputs[0].data, 0)				/	vec3(	*(ivec3*)inputs[1].data);			break;
				case  0x62: *(vec3*)	outputs[0].data	=			*(vec3*)	inputs[0].data					/	vec3(	*(ivec3*)inputs[1].data);			break;
				case  0x63: *(vec4*)	outputs[0].data	=			*(vec4*)	inputs[0].data					/	vec4(	*(ivec3*)inputs[1].data, 1);		break;
				case  0x64: *(ivec3*)	outputs[0].data	= ivec3(	*(int*)		inputs[0].data, 0, 0)			/			*(ivec3*)inputs[1].data;			break;
				case  0x65: *(ivec3*)	outputs[0].data	= ivec3(	*(ivec2*)	inputs[0].data, 0)				/			*(ivec3*)inputs[1].data;			break;
				case  0x66: *(ivec3*)	outputs[0].data	= ivec3(	*(ivec3*)	inputs[0].data)					/			*(ivec3*)inputs[1].data;			break;
				case  0x67: *(ivec4*)	outputs[0].data	= ivec4(	*(ivec4*)	inputs[0].data)					/	ivec4(	*(ivec3*)inputs[1].data, 1);		break;
				case  0x68: *(ivec3*)	outputs[0].data	= ivec3(	*(uint*)	inputs[0].data, 0, 0)			/			*(ivec3*)inputs[1].data;			break;
				case  0x69: *(ivec3*)	outputs[0].data	= ivec3(	*(uvec2*)	inputs[0].data, 0)				/			*(ivec3*)inputs[1].data;			break;
				case  0x6a: *(ivec3*)	outputs[0].data	= ivec3(	*(uvec3*)	inputs[0].data)					/			*(ivec3*)inputs[1].data;			break;
				case  0x6b: *(ivec4*)	outputs[0].data	= ivec4(	*(uvec4*)	inputs[0].data)					/	ivec4(	*(ivec3*)inputs[1].data, 1);		break;
				case  0x70: *(vec4*)	outputs[0].data	= vec4(		*(float*)	inputs[0].data, 0, 0, 0)		/	vec4(	*(ivec4*)inputs[1].data);			break;
				case  0x71: *(vec4*)	outputs[0].data	= vec4(		*(vec2*)	inputs[0].data, 0, 0)			/	vec4(	*(ivec4*)inputs[1].data);			break;
				case  0x72: *(vec4*)	outputs[0].data	= vec4(		*(vec3*)	inputs[0].data, 0)				/	vec4(	*(ivec4*)inputs[1].data);			break;
				case  0x73: *(vec4*)	outputs[0].data	=			*(vec4*)	inputs[0].data					/	vec4(	*(ivec4*)inputs[1].data);			break;
				case  0x74: *(ivec4*)	outputs[0].data	= ivec4(	*(int*)		inputs[0].data, 0, 0, 0)		/			*(ivec4*)inputs[1].data;			break;
				case  0x75: *(ivec4*)	outputs[0].data	= ivec4(	*(ivec2*)	inputs[0].data, 0, 0)			/			*(ivec4*)inputs[1].data;			break;
				case  0x76: *(ivec4*)	outputs[0].data	= ivec4(	*(ivec3*)	inputs[0].data, 0)				/			*(ivec4*)inputs[1].data;			break;
				case  0x77: *(ivec4*)	outputs[0].data	= ivec4(	*(ivec4*)	inputs[0].data)					/			*(ivec4*)inputs[1].data;			break;
				case  0x78: *(ivec4*)	outputs[0].data	= ivec4(	*(uint*)	inputs[0].data, 0, 0, 0)		/			*(ivec4*)inputs[1].data;			break;
				case  0x79: *(ivec4*)	outputs[0].data	= ivec4(	*(uvec2*)	inputs[0].data, 0, 0)			/			*(ivec4*)inputs[1].data;			break;
				case  0x7a: *(ivec4*)	outputs[0].data	= ivec4(	*(uvec3*)	inputs[0].data, 0)				/			*(ivec4*)inputs[1].data;			break;
				case  0x7b: *(ivec4*)	outputs[0].data	= ivec4(	*(uvec4*)	inputs[0].data)					/			*(ivec4*)inputs[1].data;			break;
				case  0x80: *(float*)	outputs[0].data	=			*(float*)	inputs[0].data					/			*(uint*)inputs[1].data;				break;
				case  0x81: *(vec2*)	outputs[0].data	=			*(vec2*)	inputs[0].data					/	vec2(	*(uint*)inputs[1].data, 1);			break;
				case  0x82: *(vec3*)	outputs[0].data	=			*(vec3*)	inputs[0].data					/	vec3(	*(uint*)inputs[1].data, 1, 1);		break;
				case  0x83: *(vec4*)	outputs[0].data	=			*(vec4*)	inputs[0].data					/	vec4(	*(uint*)inputs[1].data, 1, 1, 1);	break;
				case  0x84: *(int*)		outputs[0].data	=			*(int*)		inputs[0].data					/			*(uint*)inputs[1].data;				break;
				case  0x85: *(ivec2*)	outputs[0].data	=			*(ivec2*)	inputs[0].data					/	ivec2(	*(uint*)inputs[1].data, 1);			break;
				case  0x86: *(ivec3*)	outputs[0].data	=			*(ivec3*)	inputs[0].data					/	ivec3(	*(uint*)inputs[1].data, 1, 1);		break;
				case  0x87: *(ivec4*)	outputs[0].data	=			*(ivec4*)	inputs[0].data					/	ivec4(	*(uint*)inputs[1].data, 1, 1, 1);	break;
				case  0x88: *(uint*)	outputs[0].data	=			*(uint*)	inputs[0].data					/			*(uint*)inputs[1].data;				break;
				case  0x89: *(uvec2*)	outputs[0].data	=			*(uvec2*)	inputs[0].data					/	uvec2(	*(uint*)inputs[1].data, 1);			break;
				case  0x8a: *(uvec3*)	outputs[0].data	=			*(uvec3*)	inputs[0].data					/	uvec3(	*(uint*)inputs[1].data, 1, 1);		break;
				case  0x8b: *(uvec4*)	outputs[0].data	=			*(uvec4*)	inputs[0].data					/	uvec4(	*(uint*)inputs[1].data, 1, 1, 1);	break;
				case  0x90: *(vec2*)	outputs[0].data	= vec2(		*(float*)	inputs[0].data, 0)				/	vec2(	*(uvec2*)inputs[1].data);			break;
				case  0x91: *(vec2*)	outputs[0].data	=			*(vec2*)	inputs[0].data					/	vec2(	*(uvec2*)inputs[1].data);			break;
				case  0x92: *(vec3*)	outputs[0].data	=			*(vec3*)	inputs[0].data					/	vec3(	*(uvec2*)inputs[1].data, 1);		break;
				case  0x93: *(vec4*)	outputs[0].data	=			*(vec4*)	inputs[0].data					/	vec4(	*(uvec2*)inputs[1].data, 1, 1);		break;
				case  0x94: *(ivec2*)	outputs[0].data	= ivec2(	*(int*)		inputs[0].data, 0)				/	ivec2(	*(uvec2*)inputs[1].data);			break;
				case  0x95: *(ivec2*)	outputs[0].data	= ivec2(	*(ivec2*)	inputs[0].data)					/	ivec2(	*(uvec2*)inputs[1].data);			break;
				case  0x96: *(ivec3*)	outputs[0].data	= ivec3(	*(ivec3*)	inputs[0].data)					/	ivec3(	*(uvec2*)inputs[1].data, 1);		break;
				case  0x97: *(ivec4*)	outputs[0].data	= ivec4(	*(ivec4*)	inputs[0].data)					/	ivec4(	*(uvec2*)inputs[1].data, 1, 1);		break;
				case  0x98: *(uvec2*)	outputs[0].data	= uvec2(	*(uint*)	inputs[0].data, 0)				/			*(uvec2*)inputs[1].data;			break;
				case  0x99: *(uvec2*)	outputs[0].data	=			*(uvec2*)	inputs[0].data					/			*(uvec2*)inputs[1].data;			break;
				case  0x9a: *(uvec3*)	outputs[0].data	=			*(uvec3*)	inputs[0].data					/	uvec3(	*(uvec2*)inputs[1].data, 1);		break;
				case  0x9b: *(uvec4*)	outputs[0].data	=			*(uvec4*)	inputs[0].data					/	uvec4(	*(uvec2*)inputs[1].data, 1, 1);		break;
				case  0xa0: *(vec3*)	outputs[0].data	= vec3(		*(float*)	inputs[0].data, 0, 0)			/	vec3(	*(uvec3*)inputs[1].data);			break;
				case  0xa1: *(vec3*)	outputs[0].data	= vec3(		*(vec2*)	inputs[0].data, 0)				/	vec3(	*(uvec3*)inputs[1].data);			break;
				case  0xa2: *(vec3*)	outputs[0].data	=			*(vec3*)	inputs[0].data					/	vec3(	*(uvec3*)inputs[1].data);			break;
				case  0xa3: *(vec4*)	outputs[0].data	=			*(vec4*)	inputs[0].data					/	vec4(	*(uvec3*)inputs[1].data, 1);		break;
				case  0xa4: *(ivec3*)	outputs[0].data	= ivec3(	*(int*)		inputs[0].data, 0, 0)			/	ivec3(	*(uvec3*)inputs[1].data);			break;
				case  0xa5: *(ivec3*)	outputs[0].data	= ivec3(	*(ivec2*)	inputs[0].data, 0)				/	ivec3(	*(uvec3*)inputs[1].data);			break;
				case  0xa6: *(ivec3*)	outputs[0].data	= ivec3(	*(ivec3*)	inputs[0].data)					/	ivec3(	*(uvec3*)inputs[1].data);			break;
				case  0xa7: *(ivec4*)	outputs[0].data	= ivec4(	*(ivec4*)	inputs[0].data)					/	ivec4(	*(uvec3*)inputs[1].data, 1);		break;
				case  0xa8: *(uvec3*)	outputs[0].data	= uvec3(	*(uint*)	inputs[0].data, 0, 0)			/			*(uvec3*)inputs[1].data;			break;
				case  0xa9: *(uvec3*)	outputs[0].data	= uvec3(	*(uvec2*)	inputs[0].data, 0)				/			*(uvec3*)inputs[1].data;			break;
				case  0xaa: *(uvec3*)	outputs[0].data	=			*(uvec3*)	inputs[0].data					/			*(uvec3*)inputs[1].data;			break;
				case  0xab: *(uvec4*)	outputs[0].data	=			*(uvec4*)	inputs[0].data					/	uvec4(	*(uvec3*)inputs[1].data, 1);		break;
				case  0xb0: *(vec4*)	outputs[0].data	= vec4(		*(float*)	inputs[0].data, 0, 0, 0)		/	vec4(	*(uvec4*)inputs[1].data);			break;
				case  0xb1: *(vec4*)	outputs[0].data	= vec4(		*(vec2*)	inputs[0].data, 0, 0)			/	vec4(	*(uvec4*)inputs[1].data);			break;
				case  0xb2: *(vec4*)	outputs[0].data	= vec4(		*(vec3*)	inputs[0].data, 0)				/	vec4(	*(uvec4*)inputs[1].data);			break;
				case  0xb3: *(vec4*)	outputs[0].data	=			*(vec4*)	inputs[0].data					/	vec4(	*(uvec4*)inputs[1].data);			break;
				case  0xb4: *(ivec4*)	outputs[0].data	= ivec4(	*(int*)		inputs[0].data, 0, 0, 0)		/	ivec4(	*(uvec4*)inputs[1].data);			break;
				case  0xb5: *(ivec4*)	outputs[0].data	= ivec4(	*(ivec2*)	inputs[0].data, 0, 0)			/	ivec4(	*(uvec4*)inputs[1].data);			break;
				case  0xb6: *(ivec4*)	outputs[0].data	= ivec4(	*(ivec3*)	inputs[0].data, 0)				/	ivec4(	*(uvec4*)inputs[1].data);			break;
				case  0xb7: *(ivec4*)	outputs[0].data	= ivec4(	*(ivec4*)	inputs[0].data)					/	ivec4(	*(uvec4*)inputs[1].data);			break;
				case  0xb8: *(uvec4*)	outputs[0].data	= uvec4(	*(uint*)	inputs[0].data, 0, 0, 0)		/			*(uvec4*)inputs[1].data;			break;
				case  0xb9: *(uvec4*)	outputs[0].data	= uvec4(	*(uvec2*)	inputs[0].data, 0, 0)			/			*(uvec4*)inputs[1].data;			break;
				case  0xba: *(uvec4*)	outputs[0].data	= uvec4(	*(uvec3*)	inputs[0].data, 0)				/			*(uvec4*)inputs[1].data;			break;
				case  0xbb: *(uvec4*)	outputs[0].data	=			*(uvec4*)	inputs[0].data					/			*(uvec4*)inputs[1].data;			break;
				}
			},
			nullptr,
			nullptr,
			[](TypeInfo** input_types, TypeInfo** output_types) {
				auto ti0 = (TypeInfo_Data*)input_types[0];
				auto ti1 = (TypeInfo_Data*)input_types[1];
				auto data_type = DataFloat;
				auto vec_size = max(ti0->vec_size, ti1->vec_size);
				auto is_signed = ti0->is_signed || ti1->is_signed;
				if (ti0->data_type == DataInt && ti1->data_type == DataInt)
					data_type = DataInt;
				switch (data_type)
				{
				case DataFloat:
					switch (vec_size)
					{
					case 1: *output_types = TypeInfo::get<float>(); break;
					case 2: *output_types = TypeInfo::get<vec2>(); break;
					case 3: *output_types = TypeInfo::get<vec3>(); break;
					case 4: *output_types = TypeInfo::get<vec4>(); break;
					}
					break;
				case DataInt:
					switch (vec_size)
					{
					case 1: *output_types = is_signed ? TypeInfo::get<int>() : TypeInfo::get<uint>(); break;
					case 2: *output_types = is_signed ? TypeInfo::get<ivec2>() : TypeInfo::get<uvec2>(); break;
					case 3: *output_types = is_signed ? TypeInfo::get<ivec3>() : TypeInfo::get<uvec3>(); break;
					case 4: *output_types = is_signed ? TypeInfo::get<ivec4>() : TypeInfo::get<uvec4>(); break;
					}
					break;
				}
			}
		);
	}
}

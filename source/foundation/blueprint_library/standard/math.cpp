#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_math_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Scalar", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				switch (out_ti->data_type)
				{
				case DataFloat:
					*(float*)outputs[0].data = *(float*)inputs[0].data;
					break;
				case DataInt:
					if (out_ti->is_signed)
						*(int*)outputs[0].data = *(int*)inputs[0].data;
					else
						*(uint*)outputs[0].data = *(uint*)inputs[0].data;
					break;
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeInputTypesChanged)
				{
					info.output_types[0] = info.input_types[0];
					return true;
				}
				return false;
			}
		);

		library->add_template("Vec2", "", BlueprintNodeFlagNone,
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
					.name = "V",
					.allowed_types = { TypeInfo::get<vec2>(), TypeInfo::get<ivec2>(), TypeInfo::get<uvec2>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeInputTypesChanged)
				{
					auto ti0 = (TypeInfo_Data*)info.input_types[0];
					auto ti1 = (TypeInfo_Data*)info.input_types[1];
					auto data_type = DataFloat;
					auto is_signed = ti0->is_signed || ti1->is_signed;
					if (ti0->data_type == DataInt && ti1->data_type == DataInt)
						data_type = DataInt;
					switch (data_type)
					{
					case DataFloat:
						info.output_types[0] = TypeInfo::get<vec2>();
						break;
					case DataInt:
						info.output_types[0] = is_signed ? TypeInfo::get<ivec2>() : TypeInfo::get<uvec2>();
						break;
					}
					return true;
				}
				return false;
			}
		);
		
		library->add_template("Vec3", "", BlueprintNodeFlagNone,
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
					.name = "V",
					.allowed_types = { TypeInfo::get<vec3>(), TypeInfo::get<ivec3>(), TypeInfo::get<uvec3>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in2_ti = (TypeInfo_Data*)inputs[2].type;

				switch (out_ti->data_type)
				{
				case DataFloat:
				{
					auto& v = *(vec3*)outputs[0].data;
					v.x = in0_ti->as_float(inputs[0].data);
					v.y = in1_ti->as_float(inputs[1].data);
					v.z = in2_ti->as_float(inputs[2].data);
				}
					break;
				case DataInt:
				{
					if (out_ti->is_signed)
					{
						auto& v = *(ivec3*)outputs[0].data;
						v.x = in0_ti->as_int(inputs[0].data);
						v.y = in1_ti->as_int(inputs[1].data);
						v.z = in2_ti->as_int(inputs[2].data);
					}
					else
					{
						auto& v = *(uvec3*)outputs[0].data;
						v.x = in0_ti->as_uint(inputs[0].data);
						v.y = in1_ti->as_uint(inputs[1].data);
						v.z = in2_ti->as_uint(inputs[2].data);
					}
				}
					break;
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeInputTypesChanged)
				{
					auto ti0 = (TypeInfo_Data*)info.input_types[0];
					auto ti1 = (TypeInfo_Data*)info.input_types[1];
					auto ti2 = (TypeInfo_Data*)info.input_types[2];
					auto data_type = DataFloat;
					auto is_signed = ti0->is_signed || ti1->is_signed || ti2->is_signed;
					if (ti0->data_type == DataInt && ti1->data_type == DataInt && ti2->data_type == DataInt)
						data_type = DataInt;
					switch (data_type)
					{
					case DataFloat:
						info.output_types[0] = TypeInfo::get<vec3>();
						break;
					case DataInt:
						info.output_types[0] = is_signed ? TypeInfo::get<ivec3>() : TypeInfo::get<uvec3>();
						break;
					}
					return true;
				}
				return false;
			}
		);
		
		library->add_template("Vec4", "", BlueprintNodeFlagNone,
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
					.name = "V",
					.allowed_types = { TypeInfo::get<vec4>(), TypeInfo::get<ivec4>(), TypeInfo::get<uvec4>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in2_ti = (TypeInfo_Data*)inputs[2].type;
				auto in3_ti = (TypeInfo_Data*)inputs[3].type;

				switch (out_ti->data_type)
				{
				case DataFloat:
				{
					auto& v = *(vec4*)outputs[0].data;
					v.x = in0_ti->as_float(inputs[0].data);
					v.y = in1_ti->as_float(inputs[1].data);
					v.z = in2_ti->as_float(inputs[2].data);
					v.w = in3_ti->as_float(inputs[3].data);
				}
					break;
				case DataInt:
				{
					if (out_ti->is_signed)
					{
						auto& v = *(ivec4*)outputs[0].data;
						v.x = in0_ti->as_int(inputs[0].data);
						v.y = in1_ti->as_int(inputs[1].data);
						v.z = in2_ti->as_int(inputs[2].data);
						v.w = in3_ti->as_int(inputs[3].data);
					}
					else
					{
						auto& v = *(uvec4*)outputs[0].data;
						v.x = in0_ti->as_uint(inputs[0].data);
						v.y = in1_ti->as_uint(inputs[1].data);
						v.z = in2_ti->as_uint(inputs[2].data);
						v.w = in3_ti->as_uint(inputs[3].data);
					}
				}
					break;
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeInputTypesChanged)
				{
					auto ti0 = (TypeInfo_Data*)info.input_types[0];
					auto ti1 = (TypeInfo_Data*)info.input_types[1];
					auto ti2 = (TypeInfo_Data*)info.input_types[2];
					auto ti3 = (TypeInfo_Data*)info.input_types[3];
					auto data_type = DataFloat;
					auto is_signed = ti0->is_signed || ti1->is_signed || ti2->is_signed || ti3->is_signed;
					if (ti0->data_type == DataInt && ti1->data_type == DataInt && ti2->data_type == DataInt && ti3->data_type == DataInt)
						data_type = DataInt;
					switch (data_type)
					{
					case DataFloat:
						info.output_types[0] = TypeInfo::get<vec4>();
						break;
					case DataInt:
						info.output_types[0] = is_signed ? TypeInfo::get<ivec4>() : TypeInfo::get<uvec4>();
						break;
					}
					return true;
				}
				return false;
			}
		);
		
		library->add_template("Decompose", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>(),
										TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
					}
				}
			},
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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto vec_size = in0_ti->vec_size;
				switch (in0_ti->data_type)
				{
				case DataFloat:
					for (auto i = 0; i < vec_size; i++)
						*(float*)outputs[i].data = ((float*)inputs[0].data)[i];
					break;
				case DataInt:
					if (in0_ti->is_signed)
					{
						for (auto i = 0; i < vec_size; i++)
							*(int*)outputs[i].data = ((int*)inputs[0].data)[i];
					}
					else
					{
						for (auto i = 0; i < vec_size; i++)
							*(uint*)outputs[i].data = ((uint*)inputs[0].data)[i];
					}
					break;
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeInputTypesChanged)
				{
					auto ti = (TypeInfo_Data*)info.input_types[0];
					switch (ti->data_type)
					{
					case DataFloat:
						for (auto i = 0; i < 4; i++)
							info.output_types[i] = i < ti->vec_size ? TypeInfo::get<float>() : nullptr;
						break;
					case DataInt:
						for (auto i = 0; i < 4; i++)
							info.output_types[i] = i < ti->vec_size ? (ti->is_signed ? TypeInfo::get<int>() : TypeInfo::get<uint>()) : nullptr;
						break;
					}
					return true;
				}
				return false;
			}
		);

		library->add_template("HSV Color", "", BlueprintNodeFlagNone,
			{
				{
					.name = "H",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "S",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				},
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				},
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				}
			},
			{
				{
					.name = "RGB",
					.allowed_types = { TypeInfo::get<cvec4>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto h = *(float*)inputs[0].data;
				auto s = *(float*)inputs[1].data;
				auto v = *(float*)inputs[2].data;
				auto a = *(float*)inputs[3].data;
				*(cvec4*)outputs[0].data = vec4(rgbColor(vec3(h, s, v)), a) * 255.f;
			}
		);

		library->add_template("Color To Vec4", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<cvec4>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec4>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(vec4*)outputs[0].data = vec4(*(cvec4*)inputs[0].data) / 255.f;
			}
		);

		std::vector<TypeInfo*> generic_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>(),
			TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
			TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
		};

		auto binary_input_types_changed_function = [](BlueprintNodeStructureChangeInfo& info) {
			if (info.reason == BlueprintNodeInputTypesChanged)
			{
				auto ti0 = (TypeInfo_Data*)info.input_types[0];
				auto ti1 = (TypeInfo_Data*)info.input_types[1];
				if (ti0->vec_size != ti1->vec_size)
				{
					if (ti0->vec_size != 1 && ti1->vec_size != 1)
					{
						info.output_types[0] = nullptr;
						return true;
					}
				}

				auto vec_size = max(ti0->vec_size, ti1->vec_size);
				if (ti0->data_type == DataInt && ti1->data_type == DataInt)
				{
					auto is_signed = ti0->is_signed || ti1->is_signed;
					switch (vec_size)
					{
					case 1: info.output_types[0] = is_signed ? TypeInfo::get<int>() : TypeInfo::get<uint>(); break;
					case 2: info.output_types[0] = is_signed ? TypeInfo::get<ivec2>() : TypeInfo::get<uvec2>(); break;
					case 3: info.output_types[0] = is_signed ? TypeInfo::get<ivec3>() : TypeInfo::get<uvec3>(); break;
					case 4: info.output_types[0] = is_signed ? TypeInfo::get<ivec4>() : TypeInfo::get<uvec4>(); break;
					}
				}
				else
				{
					switch (vec_size)
					{
					case 1: info.output_types[0] = TypeInfo::get<float>(); break;
					case 2: info.output_types[0] = TypeInfo::get<vec2>(); break;
					case 3: info.output_types[0] = TypeInfo::get<vec3>(); break;
					case 4: info.output_types[0] = TypeInfo::get<vec4>(); break;
					}
				}

				return true;
			}

			return false;
		};

#define OP_OPERATOR(a, b, op_name) a op_name b
#define OP_FUNCTION(a, b, op_name) op_name(a, b)

#define BINARY_OPERATION_TEMPLATE(node_name, op_name, op)\
		library->add_template(node_name, #op_name, BlueprintNodeFlagNone,\
			{\
				{\
					.name = "A",\
					.allowed_types = generic_types\
				},\
				{\
					.name = "B",\
					.allowed_types = generic_types\
				}\
			},\
			{\
				{\
					.name = "V",\
					.allowed_types = generic_types\
				}\
			},\
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {\
				auto out_ti = (TypeInfo_Data*)outputs[0].type;\
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;\
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;\
				auto in0_p = (char*)inputs[0].data;\
				auto in1_p = (char*)inputs[1].data;\
\
				if (out_ti->data_type == DataFloat)\
				{\
					switch (out_ti->vec_size)\
					{\
					case 1:\
						*(float*)outputs[0].data = op(in0_ti->as_float(in0_p), in1_ti->as_float(in1_p), op_name);\
						break;\
					case 2:\
					{\
						vec2 a, b;\
						if (in0_ti->vec_size == 1)\
							a = vec2(in0_ti->as_float(in0_p));\
						else\
							in0_ti->as_floats(in0_p, 2, &a[0]);\
						if (in1_ti->vec_size == 1)\
							b = vec2(in1_ti->as_float(in1_p));\
						else\
							in1_ti->as_floats(in1_p, 2, &b[0]);\
						*(vec2*)outputs[0].data = op(a, b, op_name);\
					}\
						break;\
					case 3:\
					{\
						vec3 a, b;\
						if (in0_ti->vec_size == 1)\
							a = vec3(in0_ti->as_float(in0_p));\
						else\
							in0_ti->as_floats(in0_p, 3, &a[0]);\
						if (in1_ti->vec_size == 1)\
							b = vec3(in1_ti->as_float(in1_p));\
						else\
							in1_ti->as_floats(in1_p, 3, &b[0]);\
						*(vec3*)outputs[0].data = op(a, b, op_name);\
					}\
						break;\
					case 4:\
					{\
						vec4 a, b;\
						if (in0_ti->vec_size == 1)\
							a = vec4(in0_ti->as_float(in0_p));\
						else\
							in0_ti->as_floats(in0_p, 4, &a[0]);\
						if (in1_ti->vec_size == 1)\
							b = vec4(in1_ti->as_float(in1_p));\
						else\
							in1_ti->as_floats(in1_p, 4, &b[0]);\
						*(vec4*)outputs[0].data = op(a, b, op_name);\
					}\
						break;\
					}\
				}\
				else\
				{\
					if (out_ti->is_signed)\
					{\
						switch (out_ti->vec_size)\
						{\
						case 1:\
							*(int*)outputs[0].data = op(in0_ti->as_int(in0_p), in1_ti->as_int(in1_p), op_name);\
							break;\
						case 2:\
						{\
							ivec2 a, b;\
							if (in0_ti->vec_size == 1)\
								a = ivec2(in0_ti->as_int(in0_p));\
							else\
								in0_ti->as_ints(in0_p, 2, &a[0]);\
							if (in1_ti->vec_size == 1)\
								b = ivec2(in1_ti->as_int(in1_p));\
							else\
								in1_ti->as_ints(in1_p, 2, &b[0]);\
							*(ivec2*)outputs[0].data = op(a, b, op_name);\
						}\
							break;\
						case 3:\
						{\
							ivec3 a, b;\
							if (in0_ti->vec_size == 1)\
								a = ivec3(in0_ti->as_int(in0_p));\
							else\
								in0_ti->as_ints(in0_p, 3, &a[0]);\
							if (in1_ti->vec_size == 1)\
								b = ivec3(in1_ti->as_int(in1_p));\
							else\
								in1_ti->as_ints(in1_p, 3, &b[0]);\
							*(ivec3*)outputs[0].data = op(a, b, op_name);\
						}\
							break;\
						case 4:\
						{\
							ivec4 a, b;\
							if (in0_ti->vec_size == 1)\
								a = ivec4(in0_ti->as_int(in0_p));\
							else\
								in0_ti->as_ints(in0_p, 4, &a[0]);\
							if (in1_ti->vec_size == 1)\
								b = ivec4(in1_ti->as_int(in1_p));\
							else\
								in1_ti->as_ints(in1_p, 4, &b[0]);\
							*(ivec4*)outputs[0].data = op(a, b, op_name);\
						}\
							break;\
						}\
					}\
					else\
					{\
						switch (out_ti->vec_size)\
						{\
						case 1:\
							*(int*)outputs[0].data = op(in0_ti->as_uint(in0_p), in1_ti->as_uint(in1_p), op_name);\
							break;\
						case 2:\
						{\
							uvec2 a, b;\
							if (in0_ti->vec_size == 1)\
								a = uvec2(in0_ti->as_uint(in0_p));\
							else\
								in0_ti->as_uints(in0_p, 2, &a[0]);\
							if (in1_ti->vec_size == 1)\
								b = uvec2(in1_ti->as_uint(in1_p));\
							else\
								in1_ti->as_uints(in1_p, 2, &b[0]);\
							*(uvec2*)outputs[0].data = op(a, b, op_name);\
						}\
							break;\
						case 3:\
						{\
							uvec3 a, b;\
							if (in0_ti->vec_size == 1)\
								a = uvec3(in0_ti->as_uint(in0_p));\
							else\
								in0_ti->as_uints(in0_p, 3, &a[0]);\
							if (in1_ti->vec_size == 1)\
								b = uvec3(in1_ti->as_uint(in1_p));\
							else\
								in1_ti->as_uints(in1_p, 3, &b[0]);\
							*(uvec3*)outputs[0].data = op(a, b, op_name);\
						}\
							break;\
						case 4:\
						{\
							uvec4 a, b;\
							if (in0_ti->vec_size == 1)\
								a = uvec4(in0_ti->as_uint(in0_p));\
							else\
								in0_ti->as_uints(in0_p, 4, &a[0]);\
							if (in1_ti->vec_size == 1)\
								b = uvec4(in1_ti->as_uint(in1_p));\
							else\
								in1_ti->as_uints(in1_p, 4, &b[0]);\
							*(uvec4*)outputs[0].data = op(a, b, op_name);\
						}\
							break;\
						}\
					}\
				}\
			},\
			nullptr,\
			nullptr,\
			binary_input_types_changed_function\
		);

	BINARY_OPERATION_TEMPLATE("Add", +, OP_OPERATOR);
	BINARY_OPERATION_TEMPLATE("Subtract", -, OP_OPERATOR);
	BINARY_OPERATION_TEMPLATE("Multiply", *, OP_OPERATOR);
	BINARY_OPERATION_TEMPLATE("Divide", /, OP_OPERATOR);
		
		library->add_template("Integer Divide", "\\", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
										}
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
										}
				}
			},
			{
				{
					.name = "Q",
					.allowed_types = { TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
										}
				},
				{
					.name = "R",
					.allowed_types = { TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
										}
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto p0 = (int*)outputs[0].data;
				auto p1 = (int*)outputs[1].data;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;

				if (out_ti->is_signed)
				{
					switch (out_ti->vec_size)
					{
					case 1:
					{
						int a, b;
						a = in0_ti->as_uint(in0_p);
						b = in1_ti->as_uint(in1_p);
						*(int*)outputs[0].data = a / b;
						*(int*)outputs[1].data = a % b;
					}
						break;
					case 2:
					{
						ivec2 a, b;
						if (in0_ti->vec_size == 1)
							a = ivec2(in0_ti->as_int(in0_p));
						else
							in0_ti->as_ints(in0_p, 2, &a[0]); 
						if (in1_ti->vec_size == 1)
							b = ivec2(in1_ti->as_int(in1_p));
						else
							in1_ti->as_ints(in1_p, 2, &b[0]); 
						*(ivec2*)outputs[0].data = a / b;
						*(ivec2*)outputs[1].data = a % b;
					}
						break;
					case 3:
					{
						ivec3 a, b;
						if (in0_ti->vec_size == 1)
							a = ivec3(in0_ti->as_int(in0_p));
						else
							in0_ti->as_ints(in0_p, 3, &a[0]); 
						if (in1_ti->vec_size == 1)
							b = ivec3(in1_ti->as_int(in1_p));
						else
							in1_ti->as_ints(in1_p, 3, &b[0]); 
						*(ivec3*)outputs[0].data = a / b;
						*(ivec3*)outputs[1].data = a % b;
					}
						break;
					case 4:
					{
						ivec4 a, b;
						if (in0_ti->vec_size == 1)
							a = ivec4(in0_ti->as_int(in0_p));
						else
							in0_ti->as_ints(in0_p, 4, &a[0]); 
						if (in1_ti->vec_size == 1)
							b = ivec4(in1_ti->as_int(in1_p));
						else
							in1_ti->as_ints(in1_p, 4, &b[0]); 
						*(ivec4*)outputs[0].data = a / b;
						*(ivec4*)outputs[1].data = a % b;
					}
						break;
					}
				}
				else
				{
					switch (out_ti->vec_size)
					{
					case 1:
					{
						uint a, b;
						a = in0_ti->as_uint(in0_p);
						b = in1_ti->as_uint(in1_p);
						*(uint*)outputs[0].data = a / b;
						*(uint*)outputs[1].data = a % b;
					}
						break;
					case 2:
					{
						uvec2 a, b;
						if (in0_ti->vec_size == 1)
							a = uvec2(in0_ti->as_uint(in0_p));
						else
							in0_ti->as_uints(in0_p, 2, &a[0]); 
						if (in1_ti->vec_size == 1)
							b = uvec2(in1_ti->as_uint(in1_p));
						else
							in1_ti->as_uints(in1_p, 2, &b[0]); 
						*(uvec2*)outputs[0].data = a / b;
						*(uvec2*)outputs[1].data = a % b;
					}
						break;
					case 3:
					{
						uvec3 a, b;
						if (in0_ti->vec_size == 1)
							a = uvec3(in0_ti->as_uint(in0_p));
						else
							in0_ti->as_uints(in0_p, 3, &a[0]); 
						if (in1_ti->vec_size == 1)
							b = uvec3(in1_ti->as_uint(in1_p));
						else
							in1_ti->as_uints(in1_p, 3, &b[0]); 
						*(uvec3*)outputs[0].data = a / b;
						*(uvec3*)outputs[1].data = a % b;
					}
						break;
					case 4:
					{
						uvec4 a, b;
						if (in0_ti->vec_size == 1)
							a = uvec4(in0_ti->as_uint(in0_p));
						else
							in0_ti->as_uints(in0_p, 4, &a[0]); 
						if (in1_ti->vec_size == 1)
							b = uvec4(in1_ti->as_uint(in1_p));
						else
							in1_ti->as_uints(in1_p, 4, &b[0]); 
						*(uvec4*)outputs[0].data = a / b;
						*(uvec4*)outputs[1].data = a % b;
					}
						break;
					}
				}
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeInputTypesChanged)
				{
					auto ti0 = (TypeInfo_Data*)info.input_types[0];
					auto ti1 = (TypeInfo_Data*)info.input_types[1];
					if (ti0->data_type != DataInt || ti1->data_type != DataInt)
					{
						info.output_types[0] = info.output_types[1] = nullptr;
						return true;
					}
					if (ti0->vec_size != ti1->vec_size)
					{
						if (ti1->vec_size != 1)
						{
							info.output_types[0] = info.output_types[1] = nullptr;
							return true;
						}
					}

					auto vec_size = max(ti0->vec_size, ti1->vec_size);
					auto is_signed = ti0->is_signed || ti1->is_signed;
					switch (vec_size)
					{
					case 1: info.output_types[0] = info.output_types[1] = is_signed ? TypeInfo::get<int>() : TypeInfo::get<uint>(); break;
					case 2: info.output_types[0] = info.output_types[1] = is_signed ? TypeInfo::get<ivec2>() : TypeInfo::get<uvec2>(); break;
					case 3: info.output_types[0] = info.output_types[1] = is_signed ? TypeInfo::get<ivec3>() : TypeInfo::get<uvec3>(); break;
					case 4: info.output_types[0] = info.output_types[1] = is_signed ? TypeInfo::get<ivec4>() : TypeInfo::get<uvec4>(); break;
					}

					return true;
				}
				return false;
			}
		);

		BINARY_OPERATION_TEMPLATE("Min", min, OP_FUNCTION);
		BINARY_OPERATION_TEMPLATE("Max", max, OP_FUNCTION);

#undef BINARY_OPERATION_TEMPLATE
		
		library->add_template("Floor", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				for (auto i = 0; i < out_ti->vec_size; i++)
					*(float*)outputs[0].data = floor(*(float*)inputs[0].data);
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeInputTypesChanged)
				{
					info.output_types[0] = info.input_types[0];
					return true;
				}
				return false;
			}
		);

		library->add_template("Ceil", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = generic_types
				}
			},
			{
				{
					.name = "V",
					.allowed_types = generic_types
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				for (auto i = 0; i < out_ti->vec_size; i++)
					*(float*)outputs[0].data = ceil(*(float*)inputs[0].data);
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeInputTypesChanged)
				{
					info.output_types[0] = info.input_types[0];
					return true;
				}
				return false;
			}
		);

		library->add_template("Pow", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Exp",
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
				*(float*)outputs[0].data = pow(*(float*)inputs[0].data, *(float*)inputs[1].data);
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Sign", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				for (auto i = 0; i < out_ti->vec_size; i++)
					*(float*)outputs[0].data = sign(*(float*)inputs[0].data);
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeInputTypesChanged)
				{
					info.output_types[0] = info.input_types[0];
					return true;
				}
				return false;
			}
		);

		library->add_template("Abs", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				for (auto i = 0; i < out_ti->vec_size; i++)
					*(float*)outputs[0].data = floor(*(float*)inputs[0].data);
			},
			nullptr,
			nullptr,
			[](BlueprintNodeStructureChangeInfo& info) {
				if (info.reason == BlueprintNodeInputTypesChanged)
				{
					info.output_types[0] = info.input_types[0];
					return true;
				}
				return false;
			}
		);

		library->add_template("Length", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(float*)outputs[0].data = length(*(vec3*)inputs[0].data);
			}
		);

		library->add_template("Normalize", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(vec3*)outputs[0].data = normalize(*(vec3*)inputs[0].data);
			}
		);

		library->add_template("Distance", "", BlueprintNodeFlagNone,
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(float*)outputs[0].data = distance(*(vec3*)inputs[0].data, *(vec3*)inputs[1].data);
			}
		);

		library->add_template("Arc Point", "", BlueprintNodeFlagNone,
			{
				{
					.name = "C",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "R",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto c = *(vec3*)inputs[0].data;
				auto a = *(float*)inputs[1].data;
				auto r = *(float*)inputs[2].data;
				auto rad = radians(a);
				*(vec3*)outputs[0].data = c + vec3(cos(rad) * r, 0.f, sin(rad) * r);
			}
		);

		library->add_template("Bezier", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Ctl0",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Ctl1",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Ctl2",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Ctl3",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "T",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto t = *(float*)inputs[4].data;
				auto p0 = *(vec3*)inputs[0].data;
				auto p1 = *(vec3*)inputs[1].data;
				auto p2 = *(vec3*)inputs[2].data;
				auto p3 = *(vec3*)inputs[3].data;
				auto it = 1.f - t;
				auto it2 = it * it;
				auto t2 = t * t;
				*(vec3*)outputs[0].data = p0 * it2 * it + p1 * 3.f * it2 * t + p2 * 3.f * it * t2 + p3 * t2 * t;
			}
		);

		library->add_template("Random", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Seed",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Min",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "0"
				},
				{
					.name = "Max",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(float*)outputs[0].data = linearRand(*(float*)inputs[1].data, *(float*)inputs[2].data);
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("RandomInt", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Seed",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Min",
					.allowed_types = { TypeInfo::get<int>() }
				},
				{
					.name = "Max",
					.allowed_types = { TypeInfo::get<int>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<int>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(int*)outputs[0].data = linearRand(*(int*)inputs[1].data, *(int*)inputs[2].data);
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Circle Position", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Radius",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				},
				{
					.name = "Angle",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "0"
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto radius = *(float*)inputs[0].data;
				auto angle = *(float*)inputs[1].data;
				auto rad = radians(angle);
				*(vec3*)outputs[0].data = vec3(cos(rad) * radius, 0.f, sin(rad) * radius);
			},
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Circle Random", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Seed",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Min Radius",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "0"
				},
				{
					.name = "Max Radius",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto min_radius = *(float*)inputs[1].data;
				auto max_radius = *(float*)inputs[2].data;
				auto rn = circularRand(max_radius - min_radius);
				rn += normalize(rn) * min_radius;
				*(vec3*)outputs[0].data = vec3(rn.x, 0.f, rn.y);
			},
			nullptr,
			nullptr,
			nullptr
		);
	}
}

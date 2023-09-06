#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_math_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Scalar", "",
			{
				{
					.name = "V",
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
			[](TypeInfo** input_types, TypeInfo** output_types) {
				*output_types = input_types[0];
			}
		);

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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
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
		
		library->add_template("Vec3", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
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
					*output_types = TypeInfo::get<vec3>();
					break;
				case DataInt:
					*output_types = is_signed ? TypeInfo::get<ivec3>() : TypeInfo::get<uvec3>();
					break;
				}
			}
		);
		
		library->add_template("Vec4", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				
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
					*output_types = TypeInfo::get<vec4>();
					break;
				case DataInt:
					*output_types = is_signed ? TypeInfo::get<ivec4>() : TypeInfo::get<uvec4>();
					break;
				}
			}
		);
		
		library->add_template("Decompose", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				switch (in0_ti->data_type)
				{
				case DataFloat:
					for (auto i = 0; i < 4; i++)
						*(float*)outputs[i].data = ((float*)inputs[i].data)[i];
					break;
				case DataInt:
					if (in0_ti->is_signed)
					{
						for (auto i = 0; i < 4; i++)
							*(int*)outputs[i].data = ((int*)inputs[i].data)[i];
					}
					else
					{
						for (auto i = 0; i < 4; i++)
							*(uint*)outputs[i].data = ((uint*)inputs[i].data)[i];
					}
					break;
				}
			},
			nullptr,
			nullptr,
			[](TypeInfo** input_types, TypeInfo** output_types) {
				auto ti = (TypeInfo_Data*)input_types[0];
				switch (ti->data_type)
				{
				case DataFloat:
					for (auto i = 0; i < 4; i++)
						output_types[i] = i < ti->vec_size ? TypeInfo::get<float>() : nullptr;
					break;
				case DataInt:
					for (auto i = 0; i < 4; i++)
						output_types[i] = i < ti->vec_size ? (ti->is_signed ? TypeInfo::get<int>() : TypeInfo::get<uint>()) : nullptr;
					break;
				}
			}
		);
		
		library->add_template("Add", "+",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				switch (out_ti->data_type)
				{
				case DataFloat:
				{
					auto p = (float*)outputs[0].data;
					auto in0_p = (char*)inputs[0].data;
					auto in1_p = (char*)inputs[1].data;
					for (auto i = 0; i < out_ti->vec_size; i++)
					{
						if (i < in0_ti->vec_size)
							p[i] = in0_ti->as_float(in0_p);
						else
							p[i] = 0;

						if (i < in1_ti->vec_size)
							p[i] += in1_ti->as_float(in1_p);
						in0_p += sizeof(float);
						in1_p += sizeof(float);
					}
				}
					break;
				case DataInt:
				{
					if (out_ti->is_signed)
					{
						auto p = (int*)outputs[0].data;
						auto in0_p = (char*)inputs[0].data;
						auto in1_p = (char*)inputs[1].data;
						for (auto i = 0; i < out_ti->vec_size; i++)
						{
							if (i < in0_ti->vec_size)
								p[i] = in0_ti->as_int(in0_p);
							else
								p[i] = 0;

							if (i < in1_ti->vec_size)
								p[i] += in1_ti->as_int(in1_p);
							in0_p += sizeof(int);
							in1_p += sizeof(int);
						}
					}
					else
					{
						auto p = (uint*)outputs[0].data;
						auto in0_p = (char*)inputs[0].data;
						auto in1_p = (char*)inputs[1].data;
						for (auto i = 0; i < out_ti->vec_size; i++)
						{
							if (i < in0_ti->vec_size)
								p[i] = in0_ti->as_uint(in0_p);
							else
								p[i] = 0;

							if (i < in1_ti->vec_size)
								p[i] += in1_ti->as_uint(in1_p);
							in0_p += sizeof(uint);
							in1_p += sizeof(uint);
						}
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
		
		library->add_template("Subtract", "-",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				switch (out_ti->data_type)
				{
				case DataFloat:
				{
					auto p = (float*)outputs[0].data;
					auto in0_p = (char*)inputs[0].data;
					auto in1_p = (char*)inputs[1].data;
					for (auto i = 0; i < out_ti->vec_size; i++)
					{
						if (i < in0_ti->vec_size)
							p[i] = in0_ti->as_float(in0_p);
						else
							p[i] = 0;

						if (i < in1_ti->vec_size)
							p[i] -= in1_ti->as_float(in1_p);
						in0_p += sizeof(float);
						in1_p += sizeof(float);
					}
				}
					break;
				case DataInt:
				{
					if (out_ti->is_signed)
					{
						auto p = (int*)outputs[0].data;
						auto in0_p = (char*)inputs[0].data;
						auto in1_p = (char*)inputs[1].data;
						for (auto i = 0; i < out_ti->vec_size; i++)
						{
							if (i < in0_ti->vec_size)
								p[i] = in0_ti->as_int(in0_p);
							else
								p[i] = 0;

							if (i < in1_ti->vec_size)
								p[i] -= in1_ti->as_int(in1_p);
							in0_p += sizeof(int);
							in1_p += sizeof(int);
						}
					}
					else
					{
						auto p = (uint*)outputs[0].data;
						auto in0_p = (char*)inputs[0].data;
						auto in1_p = (char*)inputs[1].data;
						for (auto i = 0; i < out_ti->vec_size; i++)
						{
							if (i < in0_ti->vec_size)
								p[i] = in0_ti->as_uint(in0_p);
							else
								p[i] = 0;

							if (i < in1_ti->vec_size)
								p[i] -= in1_ti->as_uint(in1_p);
							in0_p += sizeof(uint);
							in1_p += sizeof(uint);
						}
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
		
		library->add_template("Multiply", "*",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				switch (out_ti->data_type)
				{
				case DataFloat:
				{
					auto p = (float*)outputs[0].data;
					auto in0_p = (char*)inputs[0].data;
					auto in1_p = (char*)inputs[1].data;
					for (auto i = 0; i < out_ti->vec_size; i++)
					{
						if (i < in0_ti->vec_size)
							p[i] = in0_ti->as_float(in0_p);
						else
							p[i] = 0;

						if (i < in1_ti->vec_size)
							p[i] *= in1_ti->as_float(in1_p);
						in0_p += sizeof(float);
						in1_p += sizeof(float);
					}
				}
					break;
				case DataInt:
				{
					if (out_ti->is_signed)
					{
						auto p = (int*)outputs[0].data;
						auto in0_p = (char*)inputs[0].data;
						auto in1_p = (char*)inputs[1].data;
						for (auto i = 0; i < out_ti->vec_size; i++)
						{
							if (i < in0_ti->vec_size)
								p[i] = in0_ti->as_int(in0_p);
							else
								p[i] = 0;

							if (i < in1_ti->vec_size)
								p[i] *= in1_ti->as_int(in1_p);
							in0_p += sizeof(int);
							in1_p += sizeof(int);
						}
					}
					else
					{
						auto p = (uint*)outputs[0].data;
						auto in0_p = (char*)inputs[0].data;
						auto in1_p = (char*)inputs[1].data;
						for (auto i = 0; i < out_ti->vec_size; i++)
						{
							if (i < in0_ti->vec_size)
								p[i] = in0_ti->as_uint(in0_p);
							else
								p[i] = 0;

							if (i < in1_ti->vec_size)
								p[i] *= in1_ti->as_uint(in1_p);
							in0_p += sizeof(uint);
							in1_p += sizeof(uint);
						}
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
		
		library->add_template("Divide", "/",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				switch (out_ti->data_type)
				{
				case DataFloat:
				{
					auto p = (float*)outputs[0].data;
					auto in0_p = (char*)inputs[0].data;
					auto in1_p = (char*)inputs[1].data;
					for (auto i = 0; i < out_ti->vec_size; i++)
					{
						if (i < in0_ti->vec_size)
							p[i] = in0_ti->as_float(in0_p);
						else
							p[i] = 0;

						if (i < in1_ti->vec_size)
							p[i] /= in1_ti->as_float(in1_p);
						in0_p += sizeof(float);
						in1_p += sizeof(float);
					}
				}
					break;
				case DataInt:
				{
					if (out_ti->is_signed)
					{
						auto p = (int*)outputs[0].data;
						auto in0_p = (char*)inputs[0].data;
						auto in1_p = (char*)inputs[1].data;
						for (auto i = 0; i < out_ti->vec_size; i++)
						{
							if (i < in0_ti->vec_size)
								p[i] = in0_ti->as_int(in0_p);
							else
								p[i] = 0;

							if (i < in1_ti->vec_size)
								p[i] /= in1_ti->as_int(in1_p);
							in0_p += sizeof(int);
							in1_p += sizeof(int);
						}
					}
					else
					{
						auto p = (uint*)outputs[0].data;
						auto in0_p = (char*)inputs[0].data;
						auto in1_p = (char*)inputs[1].data;
						for (auto i = 0; i < out_ti->vec_size; i++)
						{
							if (i < in0_ti->vec_size)
								p[i] = in0_ti->as_uint(in0_p);
							else
								p[i] = 0;

							if (i < in1_ti->vec_size)
								p[i] /= in1_ti->as_uint(in1_p);
							in0_p += sizeof(uint);
							in1_p += sizeof(uint);
						}
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
		
		library->add_template("Integer Divide", "\\",
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
					.name = "Quotient",
					.allowed_types = { TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
										}
				},
				{
					.name = "Reminder",
					.allowed_types = { TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
										}
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				switch (out_ti->data_type)
				{
				case DataInt:
				{
					if (out_ti->is_signed)
					{
						auto p0 = (int*)outputs[0].data;
						auto p1 = (int*)outputs[1].data;
						auto in0_p = (char*)inputs[0].data;
						auto in1_p = (char*)inputs[1].data;
						for (auto i = 0; i < out_ti->vec_size; i++)
						{
							if (i < in0_ti->vec_size)
								p0[i] = p1[i] = in0_ti->as_int(in0_p);
							else
								p0[i] = p1[i] = 0;

							if (i < in1_ti->vec_size)
							{
								auto divisor = in1_ti->as_int(in1_p);
								p0[i] /= divisor;
								p1[i] %= divisor;
							}
							in0_p += sizeof(int);
							in1_p += sizeof(int);
						}
					}
					else
					{
						auto p0 = (int*)outputs[0].data;
						auto p1 = (int*)outputs[1].data;
						auto in0_p = (char*)inputs[0].data;
						auto in1_p = (char*)inputs[1].data;
						for (auto i = 0; i < out_ti->vec_size; i++)
						{
							if (i < in0_ti->vec_size)
								p0[i] = p1[i] = in0_ti->as_uint(in0_p);
							else
								p0[i] = p1[i] = 0;

							if (i < in1_ti->vec_size)
							{
								auto divisor = in1_ti->as_uint(in1_p);
								p0[i] /= divisor;
								p1[i] %= divisor;
							}
							in0_p += sizeof(uint);
							in1_p += sizeof(uint);
						}
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
				auto vec_size = max(ti0->vec_size, ti1->vec_size);
				auto is_signed = ti0->is_signed || ti1->is_signed;
				if (ti0->data_type == DataInt && ti1->data_type == DataInt)
					data_type = DataInt;
				switch (data_type)
				{
				case DataInt:
					switch (vec_size)
					{
					case 1: output_types[0] = output_types[1] = is_signed ? TypeInfo::get<int>() : TypeInfo::get<uint>(); break;
					case 2: output_types[0] = output_types[1] = is_signed ? TypeInfo::get<ivec2>() : TypeInfo::get<uvec2>(); break;
					case 3: output_types[0] = output_types[1] = is_signed ? TypeInfo::get<ivec3>() : TypeInfo::get<uvec3>(); break;
					case 4: output_types[0] = output_types[1] = is_signed ? TypeInfo::get<ivec4>() : TypeInfo::get<uvec4>(); break;
					}
					break;
				}
			}
		);
		
		library->add_template("Floor", "",
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
					.name = "Out",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec2>(), TypeInfo::get<vec3>(), TypeInfo::get<vec4>(),
										TypeInfo::get<int>(), TypeInfo::get<ivec2>(), TypeInfo::get<ivec3>(), TypeInfo::get<ivec4>(),
										TypeInfo::get<uint>(), TypeInfo::get<uvec2>(), TypeInfo::get<uvec3>(), TypeInfo::get<uvec4>()
										}
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto out_ti = (TypeInfo_Data*)outputs[0].type;
				switch (out_ti->data_type)
				{
				case DataFloat:
					for (auto i = 0; i < out_ti->vec_size; i++)
						*(float*)outputs[0].data = floor(*(float*)inputs[0].data);
					break;
				case DataInt:
					for (auto i = 0; i < out_ti->vec_size; i++)
					{
						if (out_ti->is_signed)
							*(int*)outputs[0].data = floor(*(int*)inputs[0].data);
						else
							*(uint*)outputs[0].data = floor(*(uint*)inputs[0].data);
					}
					break;
				}
			},
			nullptr,
			nullptr,
			[](TypeInfo** input_types, TypeInfo** output_types) {
				*output_types = input_types[0];
			}
		);

		library->add_template("Random", "",
			{
				{
					.name = "Seed",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(float*)outputs[0].data = linearRand(0.f, 1.f);
			},
			nullptr,
			nullptr,
			[](TypeInfo** input_types, TypeInfo** output_types) {
				*output_types = input_types[0];
			}
		);
	}
}

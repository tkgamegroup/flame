#include "node_integer_divide.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_template_integer_divide(BlueprintNodeLibraryPtr library)
	{
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
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
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
	}
}

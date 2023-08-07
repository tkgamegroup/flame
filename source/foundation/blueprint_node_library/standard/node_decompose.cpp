#include "node_decompose.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_template_decompose(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Decompose",
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
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {

			},
			nullptr,
			nullptr,
			[](TypeInfo** input_types, TypeInfo** output_types) {
				auto ti = (TypeInfo_Data*)input_types[0];
				auto data_type = ti->data_type;
				auto vec_size = ti->vec_size;
				auto is_signed = ti->is_signed;
				switch (data_type)
				{
				case DataFloat:
					for (auto i = 0; i < 4; i++)
						output_types[i] = i < vec_size ? TypeInfo::get<float>() : nullptr;
					break;
				case DataInt:
					for (auto i = 0; i < 4; i++)
						output_types[i] = i < vec_size ? (is_signed ? TypeInfo::get<int>() : TypeInfo::get<uint>()) : nullptr;
					break;
				}
			}
		);
	}
}

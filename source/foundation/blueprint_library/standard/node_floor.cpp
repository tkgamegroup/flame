#include "node_floor.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_template_floor(BlueprintNodeLibraryPtr library)
	{
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
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
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

	}
}

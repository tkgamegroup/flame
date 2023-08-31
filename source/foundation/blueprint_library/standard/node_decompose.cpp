#include "node_decompose.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_template_decompose(BlueprintNodeLibraryPtr library)
	{
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
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
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
	}
}

#include "node_scalar.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_template_scalar(BlueprintNodeLibraryPtr library)
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
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
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
	}
}

#include "node_scalar.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_template_scalar(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Scalar",
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
				switch (inputs[0].type_idx)
				{
				case  0x0: *(float*)outputs[0].data = *(float*)inputs[0].data;	break;
				case  0x1: *(int*)outputs[0].data	= *(int*)inputs[0].data;	break;
				case  0x2: *(uint*)outputs[0].data	= *(uint*)inputs[0].data;	break;
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

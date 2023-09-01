#include "node_less.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_template_less(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Less", "",
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				auto in0_ti = (TypeInfo_Data*)inputs[0].type;
				auto in1_ti = (TypeInfo_Data*)inputs[1].type;
				auto in0_p = (char*)inputs[0].data;
				auto in1_p = (char*)inputs[1].data;
				*(bool*)outputs[0].data = in0_ti->as_float(in0_p) < in1_ti->as_float(in1_p);
			},
			nullptr,
			nullptr,
			nullptr
		);
	}
}

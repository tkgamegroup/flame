#include "node_flow_control.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_templates_flow_control(BlueprintNodeLibraryPtr library)
	{
		library->add_template("If", "",
			{
				{
					.name = "Condition",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			{
				{
					.name = "True",
					.allowed_types = { TypeInfo::get<Signal>() }
				},
				{
					.name = "False",
					.allowed_types = { TypeInfo::get<Signal>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				auto ok = *(bool*)inputs[0].data;
				(*(Signal*)outputs[0].data).v = ok ? 1 : 0;
				(*(Signal*)outputs[1].data).v = ok ? 0 : 1;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
		library->add_template("Loop", "",
			{
				{
					.name = "Times",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "1"
				}
			},
			{
				{
					.name = "Loop",
					.allowed_types = { TypeInfo::get<Signal>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				(*(Signal*)outputs[0].data).v = *(uint*)inputs[0].data;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
	}
}

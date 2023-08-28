#include "node_flow_control.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_templates_flow_control(BlueprintNodeLibraryPtr library)
	{
		library->add_template("If",
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

			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
	}
}

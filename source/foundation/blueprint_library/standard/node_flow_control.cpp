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
					.name = "Execute",
					.allowed_types = { TypeInfo::get<Signal>() },
					.default_value = "true"
				},
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
		library->add_template("Do N",
			{
				{
					.name = "Execute",
					.allowed_types = { TypeInfo::get<Signal>() },
					.default_value = "true"
				},
				{
					.name = "N",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			{
				{
					.name = "Execute",
					.allowed_types = { TypeInfo::get<Signal>() }
				},
				{
					.name = "Counter",
					.allowed_types = { TypeInfo::get<uint>() }
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

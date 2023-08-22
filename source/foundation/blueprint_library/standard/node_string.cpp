#include "string.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_nodes_template_string(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Print",
			{
				{
					.name = "Execute",
					.allowed_types = { TypeInfo::get<Signal>() },
					.default_value = "true"
				},
				{
					.name = "String",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "Execute",
					.allowed_types = { TypeInfo::get<Signal>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				auto& in_signal = *(Signal*)inputs[0].data;
				auto& out_signal = *(Signal*)outputs[0].data;
				if (!in_signal.v)
				{
					out_signal.v = false;
					return;
				}
				else
					out_signal.v = true;

				auto& string = *(std::string*)inputs[1].data;
				if (!string.empty())
					printf("%s\n", string.c_str());
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
	}
}

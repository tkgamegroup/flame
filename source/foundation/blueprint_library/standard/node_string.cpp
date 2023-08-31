#include "string.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_node_templates_string(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Print", "",
			{
				{
					.name = "String",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				auto& string = *(std::string*)inputs[0].data;
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

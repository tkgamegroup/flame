#include "string.h"
#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_nodes_template_string(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Self",
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
				printf("%s\n", string.c_str());
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
	}
}

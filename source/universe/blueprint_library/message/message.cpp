#include "../../../foundation/blueprint.h"
#include "../../entity_private.h"
#include "../../world_private.h"

namespace flame
{
	void add_message_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Bordcast", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Root",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Msg_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto root = *(EntityPtr*)inputs[0].data;
				auto msg = *(uint*)inputs[1].data;
				if (msg)
					World::instance()->bordcast(root, msg, nullptr, nullptr);
			}
		);
	}
}

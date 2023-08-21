#include "node_entity.h"
#include "../library.h"
#include "../../../foundation/typeinfo.h"
#include "../../entity_private.h"

namespace flame
{
	void add_node_templates_entity(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Self",
			{
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				*(EntityPtr*)outputs[0].data = bp_self;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
		library->add_template("Get Name",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				*(std::string*)outputs[0].data = entity->name;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
	}
}

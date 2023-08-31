#include "library.h"

#include "entity/node_entity.h"

namespace flame
{
	void init_library()
	{
		auto entity_library = BlueprintNodeLibrary::get(L"universe::entity");

		add_node_templates_entity(entity_library);
	}
}

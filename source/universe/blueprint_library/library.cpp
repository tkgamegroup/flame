#include "library.h"

#include "entity/node_entity.h"

namespace flame
{
	thread_local EntityPtr bp_self = nullptr;

	void init_library()
	{
		auto entity_library = BlueprintNodeLibrary::get(L"universe::entity");

		add_node_templates_entity(entity_library);
	}
}

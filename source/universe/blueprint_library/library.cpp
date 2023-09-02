#include "library.h"

namespace flame
{
	void add_entity_node_templates(BlueprintNodeLibraryPtr library);

	void init_library()
	{
		auto entity_library = BlueprintNodeLibrary::get(L"universe::entity");

		add_entity_node_templates(entity_library);
	}
}

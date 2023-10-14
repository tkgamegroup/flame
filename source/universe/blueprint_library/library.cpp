#include "library.h"

namespace flame
{
	void add_entity_node_templates(BlueprintNodeLibraryPtr library);
	void add_navigation_node_templates(BlueprintNodeLibraryPtr library);
	void add_input_node_templates(BlueprintNodeLibraryPtr library);
	void add_hud_node_templates(BlueprintNodeLibraryPtr library);

	void init_library()
	{
		auto entity_library = BlueprintNodeLibrary::get(L"universe::entity");
		auto navigation_library = BlueprintNodeLibrary::get(L"universe::navigation");
		auto input_library = BlueprintNodeLibrary::get(L"universe::input");
		auto hud_library = BlueprintNodeLibrary::get(L"universe::HUD");

		add_entity_node_templates(entity_library);
		add_navigation_node_templates(navigation_library);
		add_input_node_templates(input_library);
		add_hud_node_templates(hud_library);
	}
}

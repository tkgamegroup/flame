#include "library.h"

namespace flame
{
	void add_entity_node_templates(BlueprintNodeLibraryPtr library);
	void add_navigation_node_templates(BlueprintNodeLibraryPtr library);
	void add_procedural_node_templates(BlueprintNodeLibraryPtr library);
	void add_input_node_templates(BlueprintNodeLibraryPtr library);
	void add_hud_node_templates(BlueprintNodeLibraryPtr library);
	void add_audio_node_templates(BlueprintNodeLibraryPtr library);

	void init_library()
	{
		auto entity_library = BlueprintNodeLibrary::get(L"universe::entity");
		auto navigation_library = BlueprintNodeLibrary::get(L"universe::navigation");
		auto procedural_library = BlueprintNodeLibrary::get(L"universe::procedural");
		auto input_library = BlueprintNodeLibrary::get(L"universe::input");
		auto hud_library = BlueprintNodeLibrary::get(L"universe::HUD");
		auto audio_library = BlueprintNodeLibrary::get(L"universe::audio");

		add_entity_node_templates(entity_library);
		add_navigation_node_templates(navigation_library);
		add_procedural_node_templates(procedural_library);
		add_input_node_templates(input_library);
		add_hud_node_templates(hud_library);
		add_audio_node_templates(audio_library);
	}
}

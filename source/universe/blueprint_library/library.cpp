#include "library.h"
#include "../entity_private.h"

namespace flame
{
	void add_entity_node_templates(BlueprintNodeLibraryPtr library);
	void add_camera_node_templates(BlueprintNodeLibraryPtr library);
	void add_navigation_node_templates(BlueprintNodeLibraryPtr library);
	void add_procedural_node_templates(BlueprintNodeLibraryPtr library);
	void add_input_node_templates(BlueprintNodeLibraryPtr library);
	void add_primitive_node_templates(BlueprintNodeLibraryPtr library);
	void add_hud_node_templates(BlueprintNodeLibraryPtr library);
	void add_audio_node_templates(BlueprintNodeLibraryPtr library);
	void add_resource_node_templates(BlueprintNodeLibraryPtr library);

	void init_library()
	{
		auto entity_library = BlueprintNodeLibrary::get(L"universe::entity");
		auto camera_library = BlueprintNodeLibrary::get(L"universe::camera");
		auto navigation_library = BlueprintNodeLibrary::get(L"universe::navigation");
		auto procedural_library = BlueprintNodeLibrary::get(L"universe::procedural");
		auto input_library = BlueprintNodeLibrary::get(L"universe::input");
		auto primitive_library = BlueprintNodeLibrary::get(L"universe::primitive");
		auto hud_library = BlueprintNodeLibrary::get(L"universe::HUD");
		auto audio_library = BlueprintNodeLibrary::get(L"universe::audio");
		auto resource_library = BlueprintNodeLibrary::get(L"universe::resource");

		BlueprintSystem::template_types.emplace_back("e", TypeInfo::get<EntityPtr>());

		add_entity_node_templates(entity_library);
		add_camera_node_templates(camera_library);
		add_navigation_node_templates(navigation_library);
		add_procedural_node_templates(procedural_library);
		add_input_node_templates(input_library);
		add_primitive_node_templates(primitive_library);
		add_hud_node_templates(hud_library);
		add_audio_node_templates(audio_library);
		add_resource_node_templates(resource_library);
	}
}

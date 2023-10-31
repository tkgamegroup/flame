#include "library.h"
#include "../blueprint_private.h"

namespace flame
{
	void add_type_node_templates(BlueprintNodeLibraryPtr library);
	void add_extern_node_templates(BlueprintNodeLibraryPtr library);
	void add_logical_node_templates(BlueprintNodeLibraryPtr library);
	void add_flow_control_node_templates(BlueprintNodeLibraryPtr library);
	void add_math_node_templates(BlueprintNodeLibraryPtr library);
	void add_string_node_templates(BlueprintNodeLibraryPtr library);

	void init_library()
	{
		auto standard_library = BlueprintNodeLibrary::get(L"standard");
		auto extern_library = BlueprintNodeLibrary::get(L"extern");

		add_type_node_templates(standard_library);
		add_extern_node_templates(extern_library);
		add_logical_node_templates(standard_library);
		add_flow_control_node_templates(standard_library);
		add_math_node_templates(standard_library);
		add_string_node_templates(standard_library);
	}
}

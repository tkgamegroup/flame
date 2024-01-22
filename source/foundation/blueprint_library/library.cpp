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
	void add_time_node_templates(BlueprintNodeLibraryPtr library);

	void init_library()
	{
		auto standard_library = BlueprintNodeLibrary::get(L"standard");
		auto extern_library = BlueprintNodeLibrary::get(L"extern");

		BlueprintSystem::template_types.emplace_back("v", TypeInfo::void_type);
		BlueprintSystem::template_types.emplace_back("b", TypeInfo::get<bool>());
		BlueprintSystem::template_types.emplace_back("f", TypeInfo::get<float>());
		BlueprintSystem::template_types.emplace_back("f2", TypeInfo::get<vec2>());
		BlueprintSystem::template_types.emplace_back("f3", TypeInfo::get<vec3>());
		BlueprintSystem::template_types.emplace_back("f4", TypeInfo::get<vec4>());
		BlueprintSystem::template_types.emplace_back("i", TypeInfo::get<int>());
		BlueprintSystem::template_types.emplace_back("i2", TypeInfo::get<ivec2>());
		BlueprintSystem::template_types.emplace_back("i3", TypeInfo::get<ivec3>());
		BlueprintSystem::template_types.emplace_back("i4", TypeInfo::get<ivec4>());
		BlueprintSystem::template_types.emplace_back("u", TypeInfo::get<uint>());
		BlueprintSystem::template_types.emplace_back("u2", TypeInfo::get<uvec2>());
		BlueprintSystem::template_types.emplace_back("u3", TypeInfo::get<uvec3>());
		BlueprintSystem::template_types.emplace_back("u4", TypeInfo::get<uvec4>());
		BlueprintSystem::template_types.emplace_back("c", TypeInfo::get<uchar>());
		BlueprintSystem::template_types.emplace_back("c2", TypeInfo::get<cvec2>());
		BlueprintSystem::template_types.emplace_back("c3", TypeInfo::get<cvec3>());
		BlueprintSystem::template_types.emplace_back("c4", TypeInfo::get<cvec4>());
		BlueprintSystem::template_types.emplace_back("s", TypeInfo::get<std::string>());
		BlueprintSystem::template_types.emplace_back("w", TypeInfo::get<std::wstring>());
		BlueprintSystem::template_types.emplace_back("p", TypeInfo::get<std::filesystem::path>());

		add_type_node_templates(standard_library);
		add_logical_node_templates(standard_library);
		add_flow_control_node_templates(standard_library);
		add_math_node_templates(standard_library);
		add_string_node_templates(standard_library);
		add_time_node_templates(standard_library);
		add_extern_node_templates(extern_library);
	}
}

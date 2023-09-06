#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../systems/scene_private.h"

namespace flame
{
	void add_navigation_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Generate Navmesh", "",
			{
				{
					.name = "Nodes",
					.allowed_types = { TypeInfo::get<std::vector<EntityPtr>>() }
				},
				{
					.name = "Agent Radius",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "0.6"
				},
				{
					.name = "Agent Height",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1.8"
				},
				{
					.name = "Walkable Climb",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "0.5"
				},
				{
					.name = "Walkable Slope Angle",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "45"
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& nodes = *(std::vector<EntityPtr>*)inputs[0].data;
				auto agent_radius = *(float*)inputs[1].data;
				auto agent_height = *(float*)inputs[2].data;
				auto walkable_climb = *(float*)inputs[3].data;
				auto walkable_slope_angle = *(float*)inputs[4].data;
				add_event([=]() {
					sScene::instance()->navmesh_generate(nodes, agent_radius, agent_height, walkable_climb, walkable_slope_angle);
					return false;
				});
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
	}
}

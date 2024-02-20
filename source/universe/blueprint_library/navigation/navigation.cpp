#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../entity_private.h"
#include "../../components/nav_agent_private.h"
#include "../../components/nav_obstacle_private.h"
#include "../../systems/scene_private.h"

namespace flame
{
	void add_navigation_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Generate Navmesh", "", BlueprintNodeFlagNone,
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
				},
				{
					.name = "Cell Size",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "0.3"
				},
				{
					.name = "Cell Height",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "0.2"
				},
				{
					.name = "Tile Size",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "48"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto nodes = *(std::vector<EntityPtr>*)inputs[0].data;
				auto agent_radius = *(float*)inputs[1].data;
				auto agent_height = *(float*)inputs[2].data;
				auto walkable_climb = *(float*)inputs[3].data;
				auto walkable_slope_angle = *(float*)inputs[4].data;
				auto cell_size = *(float*)inputs[5].data;
				auto cell_height = *(float*)inputs[6].data;
				auto tile_size = *(float*)inputs[7].data;
				add_event([=]() {
					sScene::instance()->navmesh_generate(nodes, agent_radius, agent_height, walkable_climb, walkable_slope_angle, cell_size, cell_height, tile_size);
					return false;
				});
			}
		);

		library->add_template("Nav Agent Set Radius", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Radius",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_agent = entity->get_component<cNavAgent>(); nav_agent)
						nav_agent->radius = *(float*)inputs[1].data;
				}
			}
		);

		library->add_template("Nav Agent Set Height", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Height",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_agent = entity->get_component<cNavAgent>(); nav_agent)
						nav_agent->height = *(float*)inputs[1].data;
				}
			}
		);

		library->add_template("Nav Agent Set Stop Distance", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Distance",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_agent = entity->get_component<cNavAgent>(); nav_agent)
						nav_agent->stop_distance = *(float*)inputs[1].data;
				}
			}
		);

		library->add_template("Nav Agent Set Speed", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Speed",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_agent = entity->get_component<cNavAgent>(); nav_agent)
						nav_agent->speed = *(float*)inputs[1].data;
				}
			}
		);

		library->add_template("Nav Agent Set Speed Scale", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Scale",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_agent = entity->get_component<cNavAgent>(); nav_agent)
						nav_agent->set_speed_scale(*(float*)inputs[1].data);
				}
			}
		);

		library->add_template("Nav Agent Set Turn Speed", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Speed",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_agent = entity->get_component<cNavAgent>(); nav_agent)
						nav_agent->turn_speed = *(float*)inputs[1].data;
				}
			}
		);

		library->add_template("Nav Agent Set Turn Speed Scale", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Scale",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_agent = entity->get_component<cNavAgent>(); nav_agent)
						nav_agent->set_turn_speed_scale(*(float*)inputs[1].data);
				}
			}
		);

		library->add_template("Nav Agent Set Separation Group", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Group",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_agent = entity->get_component<cNavAgent>(); nav_agent)
						nav_agent->separation_group = *(uint*)inputs[1].data;
				}
			}
		);

		library->add_template("Nav Agent Set Separation Weight", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Weight",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_agent = entity->get_component<cNavAgent>(); nav_agent)
						nav_agent->separation_weight = *(float*)inputs[1].data;
				}
			}
		);

		library->add_template("Nav Agent Set Target", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Position",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_agent = entity->get_component<cNavAgent>(); nav_agent)
						nav_agent->set_target(*(vec3*)inputs[1].data);
				}
			}
		);

		library->add_template("Nav Agent Stop", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_agent = entity->get_component<cNavAgent>(); nav_agent)
						nav_agent->stop();
				}
			}
		);

		library->add_template("Nav Agent Set Flying", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_agent = entity->get_component<cNavAgent>(); nav_agent)
						nav_agent->flying = *(bool*)inputs[1].data;
				}
			}
		);

		library->add_template("Nav Obstacle Set Radius", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Radius",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_obstacle = entity->get_component<cNavObstacle>(); nav_obstacle)
						nav_obstacle->radius = *(float*)inputs[1].data;
				}
			}
		);

		library->add_template("Nav Obstacle Set Height", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Height",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto nav_obstacle = entity->get_component<cNavObstacle>(); nav_obstacle)
						nav_obstacle->height = *(float*)inputs[1].data;
				}
			}
		);
	}
}

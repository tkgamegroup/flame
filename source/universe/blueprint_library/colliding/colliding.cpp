#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../entity_private.h"
#include "../../components/collider_private.h"

namespace flame
{
	void add_colliding_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Collider Set Any Filter", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Filter",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto collider = entity->get_component<cCollider>(); collider)
						collider->any_filter = *(uint*)inputs[1].data;
				}
			}
		);

		library->add_template("Collider Set All Filter", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Filter",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto collider = entity->get_component<cCollider>(); collider)
						collider->all_filter = *(uint*)inputs[1].data;
				}
			}
		);

		library->add_template("Collider Set Radius Expand", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Radius Expand",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto collider = entity->get_component<cCollider>(); collider)
						collider->radius_expand = *(float*)inputs[1].data;
				}
			}
		);
	}
}

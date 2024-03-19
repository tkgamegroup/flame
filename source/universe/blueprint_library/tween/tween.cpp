#include "../../../foundation/blueprint.h"
#include "../../entity_private.h"
#include "../../systems/tween_private.h"

namespace flame
{
	void add_tween_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Tween Begin", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				}
			},
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					auto id = sTween::instance()->begin(entity);
					*(uint*)outputs[0].data = id;
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);

		library->add_template("Tween End", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
					sTween::instance()->end(id);
			}
		);

		library->add_template("Tween Move To", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Pos",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
					sTween::instance()->move_to(id, *(vec3*)inputs[1].data, *(float*)inputs[2].data);
			}
		);

		library->add_template("Tween Rotate To", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Eul",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					auto eul = *(vec3*)inputs[1].data;
					auto qut = quat(mat3(eulerAngleYXZ(radians(eul.x), radians(eul.y), radians(eul.z))));
					sTween::instance()->rotate_to(id, qut, *(float*)inputs[2].data);
				}
			}
		);

		library->add_template("Tween Scale To", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Scl",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec3>() }
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto id = *(uint*)inputs[0].data;
				if (id != 0)
				{
					vec3 scl;
					if (inputs[1].type == TypeInfo::get<float>())
						scl = vec3(*(float*)inputs[1].data);
					else
						scl = *(vec3*)inputs[1].data;
					sTween::instance()->scale_to(id, scl, *(float*)inputs[2].data);
				}
			}
		);
	}
}

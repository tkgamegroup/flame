#include "../../../foundation/blueprint.h"
#include "../../entity_private.h"
#include "../../components/node_private.h"
#include "../../components/camera_private.h"
#include "../../systems/renderer_private.h"

namespace flame
{
	void add_camera_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("World To Screen", "",
			{
				{
					.name = "In",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			{
				{
					.name = "Out",
					.allowed_types = { TypeInfo::get<vec2>() }
				},
				{
					.name = "Clip Coord",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto world_pos = *(vec3*)inputs[0].data;
				vec3 clip_coord;
				*(vec2*)outputs[0].data = sRenderer::instance()->render_tasks.front()->camera->world_to_screen(world_pos, &clip_coord);
				*(vec3*)outputs[1].data = clip_coord;
			}
		);

		library->add_template("Fit Camera", "",
			{
				{
					.name = "Camera",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Target",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Extent",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			{
				{
					.name = "Pos",
					.allowed_types = { TypeInfo::get<vec3>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto camera = *(EntityPtr*)inputs[0].data;
				auto target = *(EntityPtr*)inputs[1].data;
				if (camera && target)
				{
					auto c_camera = camera->get_component<cCamera>();
					if (c_camera)
					{
						AABB bounds;
						bounds.reset();
						target->forward_traversal([&](EntityPtr e) {
							auto node = e->get_component<cNode>();
							if (node && !node->bounds.invalid())
								bounds.expand(node->bounds);
						});
						if (bounds.invalid())
							*(vec3*)outputs[0].data = vec3(0.f);
						else
						{
							auto extent = *(vec3*)inputs[2].data;
							bounds.a -= extent;
							bounds.b += extent;
							*(vec3*)outputs[0].data = fit_camera_to_object(mat3(c_camera->node->g_qut), c_camera->fovy, c_camera->zNear, c_camera->aspect, bounds);
						}
					}
					else
						*(vec3*)outputs[0].data = vec3(0.f);
				}
				else
					*(vec3*)outputs[0].data = vec3(0.f);
			}
		);

		library->add_template("Camera Smooth Moving", "",
			{
				{
					.name = "Camera",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Target",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "current_velocity",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<vec3>() },
					.default_value = "0,0,0"
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto camera = *(EntityPtr*)inputs[0].data;
				auto target = *(vec3*)inputs[1].data;
				auto& current_velocity = *(vec3*)inputs[2].data;
				if (camera)
				{
					auto c_camera = camera->get_component<cCamera>();
					if (c_camera)
					{
						auto node = camera->get_component<cNode>();
						node->set_pos(smooth_damp(node->pos, target, current_velocity, 0.3f, 100.f, delta_time));
					}
				}
			}
		);
	}
}

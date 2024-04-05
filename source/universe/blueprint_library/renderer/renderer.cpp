#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../entity_private.h"
#include "../../systems/renderer_private.h"

namespace flame
{
	void add_renderer_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Set Renderer Dof Focus Point", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				sRenderer::instance()->set_dof_focus_point(*(float*)inputs[0].data);
			}
		);

		library->add_template("Set Renderer Dof Focus Scale", "", BlueprintNodeFlagNone,
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				sRenderer::instance()->set_dof_focus_scale(*(float*)inputs[0].data);
			}
		);

	}
}

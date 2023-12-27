#include "../../../foundation/blueprint.h"
#include "../../systems/renderer_private.h"

namespace flame
{
	void add_primitive_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Draw Line", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Pos0",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Pos1",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				},
				{
					.name = "Depth Test",
					.allowed_types = { TypeInfo::get<bool>() },
					.default_value = "false"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				vec3 pts[2];
				pts[0] = *(vec3*)inputs[0].data;
				pts[1] = *(vec3*)inputs[1].data;
				sRenderer::instance()->draw_primitives(PrimitiveLineList, pts, 2, *(cvec4*)inputs[2].data, *(bool*)inputs[3].data);
			}
		);
	}
}

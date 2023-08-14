#include "node_mesh.h"
#include "../../../foundation/typeinfo.h"
#include "../../model_private.h"
#include "../../model_ext.h"

namespace flame
{
	namespace graphics
	{
		void add_node_template_cube(BlueprintNodeLibraryPtr library)
		{
			library->add_template("Cube Mesh",
				{
					{
						.name = "Extent",
						.allowed_types = { TypeInfo::get<vec3>() }
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<Mesh>() }
					}
				},
				[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
					auto extent = *(vec3*)inputs[0].data;
					auto& mesh = *(Mesh*)outputs[0].data;
					mesh.reset();
					mesh_add_cube(mesh, extent, vec3(0.f), mat3(1.f));
					mesh.calc_bounds();
				},
				nullptr,
				nullptr,
				nullptr,
				[](BlueprintArgument* inputs, BlueprintArgument* outputs, BlueprintNodePreview* preview) {
					auto& mesh = *(Mesh*)outputs[0].data;
					preview->type = "mesh"_h;
					preview->data = &mesh;
				}
			);
			library->add_template("Subdivide Mesh",
				{
				},
				{
				},
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr
			);
		}
	}
}

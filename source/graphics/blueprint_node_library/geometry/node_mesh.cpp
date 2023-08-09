#include "node_mesh.h"
#include "../../../foundation/typeinfo.h"
#include "../../model_private.h"

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

				},
				nullptr,
				nullptr,
				nullptr,
				[](BlueprintArgument* inputs, BlueprintArgument* outputs, BlueprintNodePreview* preview) {

				}
			);
		}
	}
}

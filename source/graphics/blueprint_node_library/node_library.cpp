#include "node_library.h"

#include "geometry/node_mesh.h"

namespace flame
{
	namespace graphics
	{
		void init_node_library()
		{
			auto geometry_library = BlueprintNodeLibrary::get(L"graphics::geometry");

			add_node_template_mesh(geometry_library);
		}
	}
}

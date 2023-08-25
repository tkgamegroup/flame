#include "library.h"

#include "texture/node_texture.h"
#include "geometry/node_mesh.h"

namespace flame
{
	namespace graphics
	{
		void init_library()
		{
			auto texture_library = BlueprintNodeLibrary::get(L"graphics::texture");
			auto geometry_library = BlueprintNodeLibrary::get(L"graphics::geometry");

			add_node_templates_texture(texture_library);
			add_node_templates_mesh(geometry_library);
		}
	}
}

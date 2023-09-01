#include "library.h"

#include "noise/node_noise.h"
#include "texture/node_texture.h"
#include "geometry/node_mesh.h"

namespace flame
{
	namespace graphics
	{
		void init_library()
		{
			auto noise_library = BlueprintNodeLibrary::get(L"graphics::noise");
			auto texture_library = BlueprintNodeLibrary::get(L"graphics::texture");
			auto geometry_library = BlueprintNodeLibrary::get(L"graphics::geometry");

			add_node_templates_noise(noise_library);
			add_node_templates_texture(texture_library);
			add_node_templates_mesh(geometry_library);
		}
	}
}

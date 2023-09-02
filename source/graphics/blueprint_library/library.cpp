#include "library.h"

namespace flame
{
	namespace graphics
	{
		void add_noise_node_templates(BlueprintNodeLibraryPtr library);
		void add_texture_node_templates(BlueprintNodeLibraryPtr library);
		void add_mesh_node_templates(BlueprintNodeLibraryPtr library);

		void init_library()
		{
			auto noise_library = BlueprintNodeLibrary::get(L"graphics::noise");
			auto texture_library = BlueprintNodeLibrary::get(L"graphics::texture");
			auto geometry_library = BlueprintNodeLibrary::get(L"graphics::geometry");

			add_noise_node_templates(noise_library);
			add_texture_node_templates(texture_library);
			add_mesh_node_templates(geometry_library);
		}
	}
}

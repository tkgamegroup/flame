#include "node_texture.h"
#include "../../../foundation/typeinfo.h"

namespace flame
{
	namespace graphics
	{
		void add_node_template_texture(BlueprintNodeLibraryPtr library)
		{
			library->add_template("New Texture",
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
			library->add_template("Voronoi Texture",
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

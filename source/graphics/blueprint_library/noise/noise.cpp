#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../noise.h"

namespace flame
{
	namespace graphics
	{
		void add_noise_node_templates(BlueprintNodeLibraryPtr library)
		{
			library->add_template("Perlin", "",
				{
					{
						.name = "UV",
						.allowed_types = { TypeInfo::get<vec2>() }
					}
				},
				{
					{
						.name = "Out",
						.allowed_types = { TypeInfo::get<float>() }
					}
				},
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
					*(float*)outputs[0].data = perlin_noise(*(vec2*)inputs[0].data);
				}
			);
		}
	}
}

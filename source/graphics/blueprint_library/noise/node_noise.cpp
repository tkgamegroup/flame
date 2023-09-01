#include "node_noise.h"
#include "../../../foundation/typeinfo.h"
#include "../../noise.h"

namespace flame
{
	namespace graphics
	{
		void add_node_templates_noise(BlueprintNodeLibraryPtr library)
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
				[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
					*(float*)outputs[0].data = perlin_noise(*(vec2*)inputs[0].data);
				},
				nullptr,
				nullptr,
				nullptr,
				nullptr
			);
		}
	}
}

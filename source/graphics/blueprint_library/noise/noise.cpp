#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../noise.h"

namespace flame
{
	namespace graphics
	{
		void add_noise_node_templates(BlueprintNodeLibraryPtr library)
		{
			library->add_template("Perlin", "", BlueprintNodeFlagNone,
				{
					{
						.name = "UV",
						.allowed_types = { TypeInfo::get<vec2>() }
					}
				},
				{
					{
						.name = "V",
						.allowed_types = { TypeInfo::get<float>() }
					}
				},
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
					*(float*)outputs[0].data = perlin_noise(*(vec2*)inputs[0].data);
				}
			);

			library->add_template("Perlin Gradient", "", BlueprintNodeFlagNone,
				{
					{
						.name = "UV",
						.allowed_types = { TypeInfo::get<vec2>() }
					},
					{
						.name = "Step",
						.allowed_types = { TypeInfo::get<float>() }
					}
				},
				{
					{
						.name = "V",
						.allowed_types = { TypeInfo::get<vec2>() }
					}
				},
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
					*(vec2*)outputs[0].data = perlin_gradient(*(vec2*)inputs[0].data, *(float*)inputs[1].data);
				}
			);
		}
	}
}

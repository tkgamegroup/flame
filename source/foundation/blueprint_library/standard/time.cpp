#include "../../blueprint_private.h"

namespace flame
{
	void add_time_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Delta Time", "",
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(float*)outputs[0].data = delta_time;
			}
		);

		library->add_template("Total Time", "",
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(float*)outputs[0].data = total_time;
			}
		);
	}
}

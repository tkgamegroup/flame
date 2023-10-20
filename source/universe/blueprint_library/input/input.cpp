#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../systems/input_private.h"

namespace flame
{
	void add_input_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Mouse Pos", "",
			{
			},
			{
				{
					.name = "Pos",
					.allowed_types = { TypeInfo::get<vec2>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(vec2*)outputs[0].data = sInput::instance()->mpos;
			}
		);

		library->add_template("Mouse Pressed", "",
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = sInput::instance()->mpressed(Mouse_Left);
			}
		);
	}
}

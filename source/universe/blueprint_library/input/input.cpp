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
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(vec2*)outputs[0].data = sInput::instance()->mpos;
			}
		);

		library->add_template("Mouse Pressed", "",
			{
				{
					.name = "Button",
					.allowed_types = { TypeInfo::get<MouseButton>() },
					.default_value = "Left"
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = sInput::instance()->mpressed(*(MouseButton*)inputs[0].data);
			}
		);

		library->add_template("Mouse Pressing", "",
			{
				{
					.name = "Button",
					.allowed_types = { TypeInfo::get<MouseButton>() },
					.default_value = "Left"
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = sInput::instance()->mbtn[*(MouseButton*)inputs[0].data];
			}
		);

		library->add_template("Key Pressed", "",
			{
				{
					.name = "Key",
					.allowed_types = { TypeInfo::get<KeyboardKey>() },
					.default_value = "A"
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = sInput::instance()->kpressed(*(KeyboardKey*)inputs[0].data);
			}
		);

		library->add_template("Key Pressing", "",
			{
				{
					.name = "Key",
					.allowed_types = { TypeInfo::get<KeyboardKey>() },
					.default_value = "A"
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = sInput::instance()->kbtn[*(KeyboardKey*)inputs[0].data];
			}
		);
	}
}

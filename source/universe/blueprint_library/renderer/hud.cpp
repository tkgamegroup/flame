#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../entity_private.h"
#include "../../systems/hud_private.h"

namespace flame
{
	void add_hud_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Hud Set Cursor", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Pos",
					.allowed_types = { TypeInfo::get<vec2>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto pos = *(vec2*)inputs[0].data;
				sHud::instance()->set_cursor(pos);
			}
		);

		library->add_template("Hud Rect LT", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec2>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto rect = sHud::instance()->item_rect();
				*(vec2*)outputs[0].data = rect.a;
			}
		);

		library->add_template("Hud Rect RT", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec2>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto rect = sHud::instance()->item_rect();
				*(vec2*)outputs[0].data = vec2(rect.b.x, rect.a.y);
			}
		);

		library->add_template("Hud Rect LB", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec2>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto rect = sHud::instance()->item_rect();
				*(vec2*)outputs[0].data = vec2(rect.a.x, rect.b.y);
			}
		);

		library->add_template("Hud Rect RB", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<vec2>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto rect = sHud::instance()->item_rect();
				*(vec2*)outputs[0].data = rect.b;
			}
		);

		library->add_template("Hud Screen Size", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "Size",
					.allowed_types = { TypeInfo::get<vec2>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(vec2*)outputs[0].data = sHud::instance()->screen_size();
			}
		);

		library->add_template("Hud Screen Width", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(float*)outputs[0].data = sHud::instance()->screen_size().x;
			}
		);

		library->add_template("Hud Screen Height", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(float*)outputs[0].data = sHud::instance()->screen_size().y;
			}
		);

		library->add_template("Hud", "", BlueprintNodeFlagNone,
			{
				{
					.name = "ID_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Pos",
					.allowed_types = { TypeInfo::get<vec2>() }
				},
				{
					.name = "Size",
					.allowed_types = { TypeInfo::get<vec2>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "200,200,200,255"
				},
				{
					.name = "Pivot",
					.allowed_types = { TypeInfo::get<vec2>() }
				},
				{
					.name = "Image",
					.allowed_types = { TypeInfo::get<graphics::ImageDesc>() }
				},
				{
					.name = "Border",
					.allowed_types = { TypeInfo::get<vec4>() },
					.default_value = "4,4,4,4"
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto id = *(uint*)inputs[0].data;
				auto pos = *(vec2*)inputs[1].data;
				auto size = *(vec2*)inputs[2].data;
				auto col = *(cvec4*)inputs[3].data;
				auto pivot = *(vec2*)inputs[4].data;
				auto& image = *(graphics::ImageDesc*)inputs[5].data;
				auto border = *(vec4*)inputs[6].data;

				sHud::instance()->begin(id, pos, size, col, pivot, image, border, false);

				execution.block->max_execute_times = 1;
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				sHud::instance()->end();
			}
		);

		library->add_template("Hud Vertical", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Size",
					.allowed_types = { TypeInfo::get<vec2>() },
					.default_value = "0,0"
				},
				{
					.name = "Item Spacing",
					.allowed_types = { TypeInfo::get<vec2>() },
					.default_value = "2,2"
				},
				{
					.name = "Border",
					.allowed_types = { TypeInfo::get<vec4>() },
					.default_value = "0,0,0,0"
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto size = *(vec2*)inputs[0].data;
				auto item_spacing = *(vec2*)inputs[1].data;
				auto border = *(vec4*)inputs[2].data;
				sHud::instance()->begin_layout(HudVertical, size, item_spacing, border);

				execution.block->max_execute_times = 1;
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				sHud::instance()->end_layout();
			}
		);

		library->add_template("Hud Horizontal", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Size",
					.allowed_types = { TypeInfo::get<vec2>() },
					.default_value = "0,0"
				},
				{
					.name = "Item Spacing",
					.allowed_types = { TypeInfo::get<vec2>() },
					.default_value = "2,2"
				},
				{
					.name = "Border",
					.allowed_types = { TypeInfo::get<vec4>() },
					.default_value = "0,0,0,0"
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto size = *(vec2*)inputs[0].data;
				auto item_spacing = *(vec2*)inputs[1].data;
				auto border = *(vec4*)inputs[2].data;
				sHud::instance()->begin_layout(HudHorizontal, size, item_spacing, border);

				execution.block->max_execute_times = 1;
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				sHud::instance()->end_layout();
			}
		);

		library->add_template("Hud Newline", "", BlueprintNodeFlagNone,
			{
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				sHud::instance()->newline();
			}
		);

		library->add_template("Hud Stencil Write", "", BlueprintNodeFlagNone,
			{
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				sHud::instance()->begin_stencil_write();

				execution.block->max_execute_times = 1;
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				sHud::instance()->end_stencil_write();
			}
		);

		library->add_template("Hud Stencil Compare", "", BlueprintNodeFlagNone,
			{
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				sHud::instance()->begin_stencil_compare();

				execution.block->max_execute_times = 1;
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				sHud::instance()->end_stencil_compare();
			}
		);

		library->add_template("Hud Scaling", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Scaling",
					.allowed_types = { TypeInfo::get<vec2>() },
					.default_value = "1,1"
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				sHud::instance()->push_style_var(HudStyleVarScaling, *(vec2*)inputs[0].data);

				execution.block->max_execute_times = 1;
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				sHud::instance()->pop_style_var(HudStyleVarScaling);
			}
		);

		library->add_template("Hud Alpha", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Alpha",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				sHud::instance()->push_style_var(HudStyleVarAlpha, vec2(*(float*)inputs[0].data));

				execution.block->max_execute_times = 1;
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				sHud::instance()->pop_style_var(HudStyleVarAlpha);
			}
		);

		library->add_template("Hud Rect", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Size",
					.allowed_types = { TypeInfo::get<vec2>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto size = *(vec2*)inputs[0].data;
				auto col = *(cvec4*)inputs[1].data;

				sHud::instance()->rect(size, col);
			}
		);

		library->add_template("Hud Text", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Text",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				},
				{
					.name = "Font Size",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "24"
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& text = *(std::wstring*)inputs[0].data;
				auto font_size = *(uint*)inputs[1].data;
				auto col = *(cvec4*)inputs[2].data;

				sHud::instance()->text(text, font_size, col);
			}
		);

		library->add_template("Hud Image", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Size",
					.allowed_types = { TypeInfo::get<vec2>() }
				},
				{
					.name = "Image",
					.allowed_types = { TypeInfo::get<graphics::ImageDesc>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto size = *(vec2*)inputs[0].data;
				auto& image = *(graphics::ImageDesc*)inputs[1].data;
				auto col = *(cvec4*)inputs[2].data;
				if (image.view)
					sHud::instance()->image(size, image, col);
			}
		);

		library->add_template("Hud Image Stretched", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Size",
					.allowed_types = { TypeInfo::get<vec2>() }
				},
				{
					.name = "Image",
					.allowed_types = { TypeInfo::get<graphics::ImageDesc>() }
				},
				{
					.name = "Border",
					.allowed_types = { TypeInfo::get<vec4>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto size = *(vec2*)inputs[0].data;
				auto& image = *(graphics::ImageDesc*)inputs[1].data;
				auto border = *(vec4*)inputs[2].data;
				auto col = *(cvec4*)inputs[3].data;
				if (image.view)
					sHud::instance()->image_stretched(size, image, border, col);
			}
		);

		library->add_template("Hud Image Rotated", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Size",
					.allowed_types = { TypeInfo::get<vec2>() }
				},
				{
					.name = "Image",
					.allowed_types = { TypeInfo::get<graphics::ImageDesc>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				},
				{
					.name = "Angle",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto size = *(vec2*)inputs[0].data;
				auto& image = *(graphics::ImageDesc*)inputs[1].data;
				auto col = *(cvec4*)inputs[2].data;
				auto angle = *(float*)inputs[3].data;
				if (image.view)
					sHud::instance()->image_rotated(size, image, col, angle);
			}
		);

		library->add_template("Hud Button", "", BlueprintNodeFlagHorizontalOutputs,
			{
				{
					.name = "Label",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				},
				{
					.name = "Font Size",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "24"
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto& label = *(std::wstring*)inputs[0].data;
				auto font_size = *(uint*)inputs[1].data;
				auto clicked = sHud::instance()->button(label, font_size);

				execution.block->max_execute_times =  clicked ? 1 : 0;
			}
		);

		library->add_template("Hud Image Button", "", BlueprintNodeFlagHorizontalOutputs,
			{ 
				{
					.name = "Size",
					.allowed_types = { TypeInfo::get<vec2>() }
				},
				{
					.name = "Image",
					.allowed_types = { TypeInfo::get<graphics::ImageDesc>() }
				},
				{
					.name = "Border",
					.allowed_types = { TypeInfo::get<vec4>() }
				}
			},
			{
			},
			true,
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto size = *(vec2*)inputs[0].data;
				auto& image = *(graphics::ImageDesc*)inputs[1].data;
				auto border = *(vec4*)inputs[2].data;
				auto clicked = sHud::instance()->image_button(size, image, border);

				execution.block->max_execute_times = clicked ? 1 : 0;
			}
		);

		library->add_template("Hud Stroke Item", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Thickness",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto thickness = *(float*)inputs[0].data;
				auto col = *(cvec4*)inputs[1].data;

				sHud::instance()->stroke_item(thickness, col);
			}
		);

		library->add_template("Hud Item Hovered", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = sHud::instance()->item_hovered();
			}
		);

		library->add_template("Hud Item Clicked", "", BlueprintNodeFlagNone,
			{
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				*(bool*)outputs[0].data = sHud::instance()->item_clicked();
			}
		);
	}
}

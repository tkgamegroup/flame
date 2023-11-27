#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../entity_private.h"
#include "../../systems/renderer_private.h"
#include "../../systems/floating_text_private.h"

namespace flame
{
	void add_hud_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Hud Set Cursor", "",
			{
				{
					.name = "Pos",
					.allowed_types = { TypeInfo::get<vec2>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto pos = *(vec2*)inputs[0].data;
				sRenderer::instance()->hud_set_cursor(pos);
			}
		);

		library->add_template("Hud Get Rect", "",
			{
			},
			{
				{
					.name = "A",
					.allowed_types = { TypeInfo::get<vec2>() }
				},
				{
					.name = "B",
					.allowed_types = { TypeInfo::get<vec2>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto rect = sRenderer::instance()->hud_get_rect();
				*(vec2*)outputs[0].data = rect.a;
				*(vec2*)outputs[1].data = rect.b;
			}
		);

		library->add_template("Hud", "",
			{
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
					.name = "Image Scale",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				}
			},
			{
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto pos = *(vec2*)inputs[1].data;
				auto size = *(vec2*)inputs[2].data;
				auto col = *(cvec4*)inputs[3].data;
				auto pivot = *(vec2*)inputs[4].data;
				auto& image = *(graphics::ImageDesc*)inputs[5].data;
				auto image_scale = *(float*)inputs[6].data;

				sRenderer::instance()->hud_begin(pos, size, col, pivot, image, image_scale);

				execution.block->max_execute_times = 1;
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				sRenderer::instance()->hud_end();
			}
		);

		library->add_template("Hud Horizontal", "",
			{
			},
			{
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				sRenderer::instance()->hud_begin_horizontal();

				execution.block->max_execute_times = 1;
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				sRenderer::instance()->hud_end_horizontal();
			}
		);

		library->add_template("Hud Stencil Write", "",
			{
			},
			{
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				sRenderer::instance()->hud_begin_stencil_write();

				execution.block->max_execute_times = 1;
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				sRenderer::instance()->hud_end_stencil_write();
			}
		);

		library->add_template("Hud Stencil Compare", "",
			{
			},
			{
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				sRenderer::instance()->hud_begin_stencil_compare();

				execution.block->max_execute_times = 1;
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				sRenderer::instance()->hud_end_stencil_compare();
			}
		);

		library->add_template("Hud Window Padding", "",
			{
				{
					.name = "Padding",
					.allowed_types = { TypeInfo::get<vec2>() },
					.default_value = "2,2"
				}
			},
			{
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				sRenderer::instance()->hud_push_style(HudStyleVarWindowPadding, *(vec2*)inputs[1].data);

				execution.block->max_execute_times = 1;
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				sRenderer::instance()->hud_pop_style(HudStyleVarWindowPadding);
			}
		);

		library->add_template("Hud Item Spacing", "",
			{
				{
					.name = "Spacing",
					.allowed_types = { TypeInfo::get<vec2>() },
					.default_value = "2,2"
				}
			},
			{
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				sRenderer::instance()->hud_push_style(HudStyleVarItemSpacing, *(vec2*)inputs[1].data);

				execution.block->max_execute_times = 1;
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				sRenderer::instance()->hud_pop_style(HudStyleVarItemSpacing);
			}
		);

		library->add_template("Hud Scaling", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				sRenderer::instance()->hud_push_style(HudStyleVarScaling, *(vec2*)inputs[1].data);

				execution.block->max_execute_times = 1;
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				sRenderer::instance()->hud_pop_style(HudStyleVarScaling);
			}
		);

		library->add_template("Hud Rect", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto size = *(vec2*)inputs[0].data;
				auto col = *(cvec4*)inputs[1].data;

				sRenderer::instance()->hud_rect(size, col);
			}
		);

		library->add_template("Hud Text", "",
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
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& text = *(std::wstring*)inputs[0].data;
				auto font_size = *(uint*)inputs[1].data;
				auto col = *(cvec4*)inputs[2].data;

				sRenderer::instance()->hud_text(text, font_size, col);
			}
		);

		library->add_template("Hud Image", "",
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
					.name = "Image Scale",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto size = *(vec2*)inputs[0].data;
				auto& image = *(graphics::ImageDesc*)inputs[1].data;
				auto image_scale = *(float*)inputs[2].data;
				auto col = *(cvec4*)inputs[3].data;
				if (image.view)
					sRenderer::instance()->hud_image(size, image, image_scale, col);
			}
		);

		library->add_template("Hud Button", "",
			{
				{
					.name = "Label",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				},
				{
					.name = "Font Size",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "24"
				},
				{
					.name = "Image",
					.allowed_types = { TypeInfo::get<graphics::ImageDesc>() }
				},
				{
					.name = "Image Scale",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				}
			},
			{
				{
					.name = "Hovered",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintExecutionData& execution) {
				auto& label = *(std::wstring*)inputs[1].data;
				auto font_size = *(uint*)inputs[2].data;
				auto& image = *(graphics::ImageDesc*)inputs[3].data;
				auto image_scale = *(float*)inputs[4].data;
				bool hovered = false;
				auto clicked = sRenderer::instance()->hud_button(label, font_size, image, image_scale, &hovered);

				(*(BlueprintSignal*)outputs[1].data).v = hovered ? 1 : 0;

				execution.block->max_execute_times =  clicked ? 1 : 0;
			}
		);

		library->add_template("Add Floating Text", "",
			{
				{
					.name = "Text",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				},
				{
					.name = "Font Size",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "16"
				},
				{
					.name = "Color",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				},
				{
					.name = "Duration",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				},
				{
					.name = "Bind Target",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Offset 3D",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Offset 2D",
					.allowed_types = { TypeInfo::get<vec2>() }
				},
				{
					.name = "Speed",
					.allowed_types = { TypeInfo::get<vec2>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& text = *(std::wstring*)inputs[0].data;
				auto font_size = *(uint*)inputs[1].data;
				auto color = *(cvec4*)inputs[2].data;
				auto duration = *(float*)inputs[3].data;
				auto bind_target = *(EntityPtr*)inputs[4].data;
				auto offset_3d = *(vec3*)inputs[5].data;
				auto offset_2d = *(vec2*)inputs[6].data;
				auto speed = *(vec2*)inputs[7].data;
				if (!text.empty() && font_size > 0 && color.a > 0 && duration > 0.f)
				{
					FloatingText::instance()->add(text, font_size, color, duration,
						bind_target ? bind_target->get_component<cNodeT>() : nullptr,
						offset_3d, offset_2d, speed);
				}
			}
		);
	}
}

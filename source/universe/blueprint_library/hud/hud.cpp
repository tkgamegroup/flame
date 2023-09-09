#include "../../../foundation/blueprint.h"
#include "../../../foundation/typeinfo.h"
#include "../../systems/renderer_private.h"

namespace flame
{
	void add_hud_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Begin Hud", "",
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
				}
			},
			{
				{
					.name = "Content",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto pos = *(vec2*)inputs[0].data;
				auto size = *(vec2*)inputs[1].data;
				auto col = *(cvec4*)inputs[2].data;
				auto pivot = *(vec2*)inputs[3].data;

				sRenderer::instance()->begin_hud(pos, size, col, pivot);

				(*(BlueprintSignal*)outputs[0].data).v = 1;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("End Hud", "",
			{
				{
					.name = "Execute",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			{
				{
					.name = "Execute",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				sRenderer::instance()->end_hud();

				(*(BlueprintSignal*)outputs[0].data).v = 1;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Hud Text", "",
			{
				{
					.name = "Text",
					.allowed_types = { TypeInfo::get<std::wstring>() }
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
				auto col = *(cvec4*)inputs[1].data;

				sRenderer::instance()->hud_text(text, col);
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Hud Button", "",
			{
				{
					.name = "Label",
					.allowed_types = { TypeInfo::get<std::wstring>() }
				}
			},
			{
				{
					.name = "Click",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& label = *(std::wstring*)inputs[0].data;
				auto clicked = sRenderer::instance()->hud_button(label);

				(*(BlueprintSignal*)outputs[0].data).v = clicked;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
	}
}

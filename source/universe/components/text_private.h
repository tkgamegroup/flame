#pragma once

#include <flame/universe/components/text.h>

namespace flame
{
	struct cTextPrivate : cText
	{
		std::wstring text;

		cTextPrivate(graphics::FontAtlas* font_atlas);
		void on_component_added(Component* c) override;
		Component* copy() override;
	};
}

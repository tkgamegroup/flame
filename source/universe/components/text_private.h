#pragma once

#include <flame/universe/components/text.h>

namespace flame
{
	struct cTextPrivate : cText
	{
		std::wstring text;

		cTextPrivate(graphics::FontAtlas* font_atlas);
		virtual void on_component_added(Component* c) override;
		virtual void update() override;
		virtual Component* copy() override;
	};
}

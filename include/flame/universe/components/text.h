#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cElement;

	struct cText : Component
	{
		cElement* element;

		graphics::FontAtlas* font_atlas;
		Vec4c color;
		float sdf_scale;
		bool auto_size;

		FLAME_UNIVERSE_EXPORTS cText(Entity* e);
		FLAME_UNIVERSE_EXPORTS virtual ~cText() override;

		FLAME_UNIVERSE_EXPORTS const std::wstring& text() const;
		FLAME_UNIVERSE_EXPORTS void set_text(const std::wstring& text);

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cText* create(Entity* e, graphics::FontAtlas* font_atlas);
	};
}

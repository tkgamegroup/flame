#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cElement;
	struct cAligner;

	struct cText : Component
	{
		cElement* element;
		cAligner* aligner;

		graphics::FontAtlas* font_atlas;
		Vec4c color;
		float sdf_scale;
		bool auto_width;
		bool auto_height;

		cText() :
			Component("Text")
		{
		}

		FLAME_UNIVERSE_EXPORTS const std::wstring& text() const;
		FLAME_UNIVERSE_EXPORTS void set_text(const std::wstring& text);

		FLAME_UNIVERSE_EXPORTS void* add_changed_listener(void (*listener)(void* c, const wchar_t* text), const Mail<>& capture);

		FLAME_UNIVERSE_EXPORTS void remove_changed_listener(void* ret_by_add);

		FLAME_UNIVERSE_EXPORTS void on_changed();

		FLAME_UNIVERSE_EXPORTS virtual void on_enter_hierarchy(Component* c) override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cText* create(graphics::FontAtlas* font_atlas);
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_button(graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text);
	FLAME_UNIVERSE_EXPORTS Entity* wrap_standard_text(Entity* e, bool before, graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text);
}

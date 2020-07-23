#pragma once

#include <flame/universe/components/text.h>
#include "element_private.h"
#include "../systems/type_setting_private.h"

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cTextBridge : cText
	{
		void set_text(const wchar_t* text) override;
	};

	struct cTextPrivate : cTextBridge, cElement::Drawer, sTypeSetting::AutoSizer
	{
		std::wstring text;
		uint font_size = 14;

		cElementPrivate* element = nullptr;
		sTypeSettingPrivate* type_setting = nullptr;
		graphics::Canvas* canvas = nullptr;
		graphics::FontAtlas* atlas = nullptr;

		const wchar_t* get_text() const override { return text.c_str(); }
		void set_text(const std::wstring& text);

		uint get_font_size() const override { return font_size; }
		void set_font_size(uint fs) override;

		void mark_size_dirty();

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
		void on_entity_visibility_changed() override;

		void draw(graphics::Canvas* canvas) override;

		Vec2f measure() override;

		static cTextPrivate* create();
	};

	inline void cTextBridge::set_text(const wchar_t* text)
	{
		((cTextPrivate*)this)->set_text(text);
	}
}

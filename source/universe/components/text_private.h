#pragma once

#include <flame/universe/components/text.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
		struct Canvas;
	}

	struct cElementPrivate;

	struct cTextBridge : cText
	{
		void set_text(const wchar_t* text) override;
	};

	struct cTextPrivate : cTextBridge // R ~ on_*
	{
		std::wstring text;
		uint size = 14;
		Vec4c color = Vec4c(0, 0, 0, 255);
		int res_id = -1;

		cElementPrivate* element = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref

		bool auto_width = true;
		bool auto_height = true;

		const wchar_t* get_text() const override { return text.c_str(); }
		uint get_text_length() const override { return text.size(); }
		void set_text(const std::wstring& text);

		uint get_size() const override { return size; }
		void set_size(uint s) override;

		Vec4c get_color() const override { return color; }
		void set_color(const Vec4c& col) override;

		bool get_auto_width() const override { return auto_width; }
		void set_auto_width(bool a) override { auto_width = a; }
		bool get_auto_height() const override { return auto_height; }
		void set_auto_height(bool a) override { auto_height = a; }

		void on_gain_canvas();

		void mark_text_changed();

		void draw(graphics::Canvas* canvas); // R

		void measure(Vec2f& ret); // R
	};

	inline void cTextBridge::set_text(const wchar_t* text)
	{
		((cTextPrivate*)this)->set_text(text);
	}
}

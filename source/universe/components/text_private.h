#pragma once

#include "text.h"

namespace flame
{
	struct cTextPrivate : cText
	{
		std::wstring text;
		uint font_size = 16;
		cvec4 font_color = cvec4(255);

		cElementPrivate* element = nullptr;
		void* drawer = nullptr;
		void* measurer = nullptr;
		sRenderer* renderer = nullptr;

		int res_id = -1;
		graphics::FontAtlas* atlas = nullptr;

		const wchar_t* get_text() const override { return text.c_str(); }
		uint get_text_length() const override { return text.size(); }
		void set_text(const std::wstring& text);
		void set_text(const wchar_t* text) override { set_text(std::wstring(text)); }

		uint get_font_size() const override { return font_size; }
		void set_font_size(uint s) override;

		cvec4 get_font_color() const override { return font_color; }
		void set_font_color(const cvec4& col) override;

		void row_layout(int offset, vec2& size, uint& num_chars);

		void mark_text_changed();

		uint draw(uint layer, sRenderer* renderer);
		bool measure(vec2* s);

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}

#pragma once

#include "text.h"
#include "element_private.h"

namespace flame
{
	struct cTextPrivate : cText, ElementDrawer, ElementMeasurer
	{
		std::wstring text;
		std::wstring atlas_name = L"c:\\windows\\fonts\\msyh.ttc;default_assets\\fa-regular-400.ttf;default_assets\\fa-solid-900.ttf";
		uint font_size = 16;
		cvec4 font_color = cvec4(255);
		Align text_align = AlignMin;

		cElementPrivate* element = nullptr;
		sRenderer* s_renderer = nullptr;

		int res_id = -1;
		graphics::FontAtlas* atlas = nullptr;

		const wchar_t* get_text() const override { return text.c_str(); }
		uint get_text_length() const override { return text.size(); }
		void set_text(std::wstring_view text);
		void set_text(const wchar_t* text) override { set_text(std::wstring(text)); }

		uint get_font_size() const override { return font_size; }
		void set_font_size(uint s) override;

		cvec4 get_font_color() const override { return font_color; }
		void set_font_color(const cvec4& col) override;

		Align get_text_align() const override { return text_align; }
		void set_text_align(Align a) override;

		void row_layout(int offset, vec2& size, uint& num_chars);

		void mark_text_changed();

		uint draw(uint layer, sRendererPtr s_renderer) override;
		bool measure(vec2* s) override;

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}

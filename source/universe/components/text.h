#pragma once

#include "../component.h"

namespace flame
{
	struct cText : Component
	{
		inline static auto type_name = "flame::cText";
		inline static auto type_hash = ch(type_name);

		cText() :
			Component(type_name, type_hash)
		{
		}

		virtual const wchar_t* get_text() const = 0;
		virtual uint get_text_length() const = 0;
		virtual void set_text(const wchar_t* text) = 0;

		virtual uint get_font_size() const = 0;
		virtual void set_font_size(uint s) = 0;

		virtual cvec4 get_font_color() const = 0;
		virtual void set_font_color(const cvec4& col) = 0;

		virtual Align get_text_align() const = 0;
		virtual void set_text_align(Align a) = 0;

		FLAME_UNIVERSE_EXPORTS static cText* create();
	};
}

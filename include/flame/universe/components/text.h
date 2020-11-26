#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cText : Component // R !ctor !dtor !type_name !type_hash
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

		virtual uint get_size() const = 0;
		virtual void set_size(uint s) = 0;

		virtual cvec4 get_color() const = 0;
		virtual void set_color(const cvec4& col) = 0;

		virtual bool get_auto_width() const = 0;
		virtual void set_auto_width(bool a) = 0;
		virtual bool get_auto_height() const = 0;
		virtual void set_auto_height(bool a) = 0;

		FLAME_UNIVERSE_EXPORTS static cText* create();
	};
}

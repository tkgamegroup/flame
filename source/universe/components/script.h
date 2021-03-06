#pragma once

#include "../component.h"

namespace flame
{
	struct cScript : Component
	{
		inline static auto type_name = "flame::cScript";
		inline static auto type_hash = ch(type_name);

		cScript() :
			Component(type_name, type_hash)
		{
		}

		virtual const wchar_t* get_src() const = 0;
		virtual void set_src(const wchar_t* fn) = 0;

		virtual const char* get_content() const = 0;
		virtual void set_content(const char* content) = 0;

		FLAME_UNIVERSE_EXPORTS static cScript* create(void* parms = nullptr);
	};
}

#pragma once

#include <flame/universe/component.h>

namespace flame
{
//	struct cElement;
//	struct cText;
//	struct cEventReceiver;

	struct FLAME_RU(cStyle : Component, all)
	{
		inline static auto type_name = "cStyle";
		inline static auto type_hash = ch(type_name);

		cStyle() :
			Component(type_name, type_hash, true)
		{
		}

		virtual const char* get_rule() const = 0;
		virtual void set_rule(const char* rule) = 0;

		FLAME_UNIVERSE_EXPORTS static cStyle* create();
	};
}

#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cMenu : Component
	{
		inline static auto type_name = "flame::cMenu";
		inline static auto type_hash = ch(type_name);

		cMenu() :
			Component(type_name, type_hash)
		{
		}

		virtual MenuType get_type() const = 0;
		virtual void set_type(MenuType type) = 0;

		FLAME_UNIVERSE_EXPORTS static cMenu* create();
	};
}

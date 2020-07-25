#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct FLAME_RU(cCheckbox : Component, all)
	{
		inline static auto type_name = "cCheckbox";
		inline static auto type_hash = ch(type_name);

		cCheckbox() :
			Component(type_name, type_hash, true)
		{
		}

		virtual bool get_checked() const = 0;
		virtual void set_checked(bool checked) = 0;

		FLAME_UNIVERSE_EXPORTS static cCheckbox* create();
	};
}

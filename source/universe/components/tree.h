#pragma once

#include "../component.h"

namespace flame
{
	struct cTree : Component
	{
		inline static auto type_name = "flame::cTree";
		inline static auto type_hash = ch(type_name);

		cTree() : Component(type_name, type_hash)
		{
		}

		virtual EntityPtr get_selected() const = 0;
		virtual void set_selected(EntityPtr v) = 0;

		virtual void expand_to_selected() = 0;

		FLAME_UNIVERSE_EXPORTS static cTree* create(void* parms = nullptr);
	};
}

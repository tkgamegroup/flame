#pragma once

#include "../component.h"

namespace flame
{
	struct cInputDialog : Component
	{
		inline static auto type_name = "flame::cInputDialog";
		inline static auto type_hash = ch(type_name);

		cInputDialog() : Component(type_name, type_hash)
		FLAME_UNIVERSE_EXPORTS static cInputDialog* create(void* parms = nullptr);
	};
}

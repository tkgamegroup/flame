#pragma once

#include "../component.h"

namespace flame
{
	struct cDragEdit : Component
	{
		inline static auto type_name = "flame::cDragEdit";
		inline static auto type_hash = ch(type_name);

		cDragEdit() : Component(type_name, type_hash)
		FLAME_UNIVERSE_EXPORTS static cDragEdit* create(void* parms = nullptr);
	};
}

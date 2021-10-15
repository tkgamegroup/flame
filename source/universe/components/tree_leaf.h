#pragma once

#include "../component.h"

namespace flame
{
	struct cTreeLeaf : Component
	{
		inline static auto type_name = "flame::cTreeLeaf";
		inline static auto type_hash = ch(type_name);

		cTreeLeaf() : Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cTreeLeaf* create(void* parms = nullptr);
	};
}

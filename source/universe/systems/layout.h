#pragma once

#include "../system.h"

namespace flame
{
	struct sLayout : System
	{
		inline static auto type_name = "flame::sLayout";
		inline static auto type_hash = ch(type_name);

		sLayout() :
			System(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static sLayout* create(void* parms = nullptr);
	};
}

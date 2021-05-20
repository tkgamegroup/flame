#pragma once

#include "../system.h"

namespace flame
{
	struct sScene : System
	{
		inline static auto type_name = "flame::sScene";
		inline static auto type_hash = ch(type_name);

		sScene() :
			System(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static sScene* create(void* parms = nullptr);
	};
}

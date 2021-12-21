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

		virtual void setup(NativeWindow* window) = 0;

		FLAME_UNIVERSE_EXPORTS static sScene* create();
	};
}

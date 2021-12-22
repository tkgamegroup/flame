#pragma once

#include "../system.h"

namespace flame
{
	struct sElementRenderer : System
	{
		inline static auto type_name = "flame::sElementRenderer";
		inline static auto type_hash = ch(type_name);

		sElementRenderer() : System(type_name, type_hash)
		{
		}

		struct Create
		{
			virtual sElementRendererPtr operator()() = 0;
		};
		FLAME_UNIVERSE_EXPORTS static Create& create;
	};
}

#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct Window;
	struct cEventReceiver;

	struct sEventDispatcher : System
	{
		inline static auto type_name = "sEventDispatcher";
		inline static auto type_hash = ch(type_name);

		sEventDispatcher() :
			System(type_name, type_hash)
		{
		}
//
		FLAME_UNIVERSE_EXPORTS static sEventDispatcher* create();
	};
}

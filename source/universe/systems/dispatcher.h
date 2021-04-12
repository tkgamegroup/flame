#pragma once

#include "../system.h"

namespace flame
{
	struct sDispatcher : System
	{
		inline static auto type_name = "flame::sDispatcher";
		inline static auto type_hash = ch(type_name);

		sDispatcher() :
			System(type_name, type_hash)
		{
		}

		virtual cReceiverPtr get_hovering() const = 0;
		virtual cReceiverPtr get_focusing() const = 0;
		virtual cReceiverPtr get_active() const = 0;
		virtual void set_next_focusing(cReceiverPtr er) = 0;

		FLAME_UNIVERSE_EXPORTS static sDispatcher* create(void* parms = nullptr);
	};
}

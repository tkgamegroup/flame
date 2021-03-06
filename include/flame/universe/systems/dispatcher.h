#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct Window;
	struct cReceiver;

	struct sDispatcher : System
	{
		inline static auto type_name = "flame::sDispatcher";
		inline static auto type_hash = ch(type_name);

		sDispatcher() :
			System(type_name, type_hash)
		{
		}

		virtual cReceiver* get_hovering() const = 0;
		virtual cReceiver* get_focusing() const = 0;
		virtual cReceiver* get_active() const = 0;
		virtual void set_next_focusing(cReceiver* er) = 0;

		FLAME_UNIVERSE_EXPORTS static sDispatcher* create(void* parms = nullptr);
	};
}

#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cCommand : Component
	{
		inline static auto type_name = "flame::cCommand";
		inline static auto type_hash = ch(type_name);

		cCommand() :
			Component(type_name, type_hash)
		{
		}

		virtual void excute(const char* cmd) = 0;

		virtual void* add_processor(void (*callback)(Capture& c, const char* cmd), const Capture& capture) = 0;
		virtual void remove_processor(void* p) = 0;

		FLAME_UNIVERSE_EXPORTS static cCommand* create();
	};
}

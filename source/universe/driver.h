#pragma once

#include "universe.h"

namespace flame
{
	struct Driver
	{
		const char* type_name;
		const uint type_hash;

		EntityPtr entity = nullptr;

		int src_id = -1;

		bool load_finished = false;

		Driver(const char* name, uint hash) :
			type_name(name),
			type_hash(hash)
		{
		}

		virtual void on_load_finished() {}
		virtual bool on_child_added(EntityPtr e) { return false; }
	};
}
